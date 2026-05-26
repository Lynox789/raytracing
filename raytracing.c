//Fichier créé le 24/05/2026 à 00:04:36
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>

#define WIDTH 900
#define HEIGHT 600

#define SUN 0xff8559
#define WHITE 0xffffffff
#define BLACK 0x00000000
#define COLOR_RAYS 0xffd23a
#define COLOR_GLASS 0xff88ccff
#define RAYS 500
#define NUMBER_BOUNCES 4

typedef struct Circle{
    double x; //x pos
    double y; //y pos
    double r; //radius
}Circle;

typedef struct Ray{
    double x,y;
    double angle;
}Ray;

void FillCircle(SDL_Surface* surface, Circle circle, Uint32 color){
    double radius_squared = pow(circle.r, 2);
    
    // Optimization: instead of iterating over the entire screen, we only iterate over the circle's bounding box.
    for (double x = circle.x - circle.r; x <= circle.x + circle.r; x++){
        for (double y = circle.y - circle.r; y <= circle.y + circle.r; y++){
            
            // Cartesian equation of a disk: (x - x_c)^2 + (y - y_c)^2 < r^2
            // https://en.wikipedia.org/wiki/Circle#Cartesian_coordinates
            double distance_squared = pow(x - circle.x, 2) + pow(y - circle.y, 2);
            
            if (distance_squared < radius_squared){
                SDL_Rect pixel = (SDL_Rect) {x,y,1,1};
                SDL_FillRect(surface, &pixel, color);
            }
        }
    }
}

void FillRays(SDL_Surface* surface, struct Ray rays[RAYS], Uint32 color, Circle shadow, Circle glass){
    
    double shadow_r_sq = pow(shadow.r, 2);
    double glass_r_sq = pow(glass.r, 2);
    
    // Extraction of RGB (Red, Green, Blue) components via bitwise shifts and a logical mask (AND 0xFF).
    Uint8 orig_r = (color >> 16) & 0xFF;
    Uint8 orig_g = (color >> 8) & 0xFF;
    Uint8 orig_b = color & 0xFF;

    // Calculation of the color after collision. We simulate the absorption of a part of the light spectrum by the material.
    Uint8 bounce_r = (orig_r * 4) / 5; 
    Uint8 bounce_g = (orig_g * 3) / 5; 
    Uint8 bounce_b = (orig_b * 3) / 5;

    for (int i = 0; i < RAYS; i++){
        struct Ray ray = rays[i];
        
        int end_screen = 0;
        double step = 1;
        double x_draw = ray.x;
        double y_draw = ray.y;
        double current_angle = ray.angle;

        int bounces = 0;
        
        // Determination of the ray's initial state (inside or outside the refractive medium).
        int in_glass = (pow(x_draw - glass.x, 2) + pow(y_draw - glass.y, 2) < glass_r_sq);
        int travelled_distance = 0;
        
        while (!end_screen && bounces < NUMBER_BOUNCES){
            
            // Ray Marching: Incremental advancement of the ray along its direction vector using polar coordinates.
            // x = x_0 + r * cos(theta), y = y_0 + r * sin(theta)
            // https://en.wikipedia.org/wiki/Polar_coordinate_system
            x_draw += step * cos(current_angle);
            y_draw += step * sin(current_angle);
            travelled_distance++;

            // Algorithmic safeguard preventing infinite loops caused by floating-point precision errors.
            if (travelled_distance > 2000) end_screen = 1;
            
            Uint8 current_r = (bounces == 0) ? orig_r : bounce_r;
            Uint8 current_g = (bounces == 0) ? orig_g : bounce_g;
            Uint8 current_b = (bounces == 0) ? orig_b : bounce_b;
            
            // Light attenuation proportional to the distance traveled (linear approximation of the inverse-square law).
            // https://en.wikipedia.org/wiki/Inverse-square_law
            double attenuation = 1.0 - (travelled_distance / 1000.0);
            if (attenuation < 0) attenuation = 0;
            
            // Recomposition of the final color in Uint32 (ARGB) format with the attenuation factor.
            Uint32 draw_color = (0xFF << 24) | 
                                ((int)(current_r * attenuation) << 16) | 
                                ((int)(current_g * attenuation) << 8) | 
                                (int)(current_b * attenuation);
            
            SDL_Rect pixel = (SDL_Rect) {(int)x_draw, (int)y_draw, 1, 1};
            SDL_FillRect(surface, &pixel, draw_color);

            if (x_draw < 0 || x_draw > WIDTH) end_screen = 1;
            if (y_draw < 0 || y_draw > HEIGHT) end_screen = 1;

            double dist_shadow_sq = pow(x_draw - shadow.x, 2) + pow(y_draw - shadow.y, 2);
            if (dist_shadow_sq < shadow_r_sq) {
                // Calculation of the normal vector (nx, ny) to the circle's surface at the point of impact.
                // https://en.wikipedia.org/wiki/Normal_(geometry)
                double nx = (x_draw - shadow.x) / shadow.r;
                double ny = (y_draw - shadow.y) / shadow.r;
                
                // Normalized incident vector.
                double ix = cos(current_angle);
                double iy = sin(current_angle);
                
                // Dot product between the incident vector and the normal.
                double dot = ix * nx + iy * ny;
                
                // Mathematical formula for vector reflection: r = i - 2(i . n)n
                // https://en.wikipedia.org/wiki/Specular_reflection#Vector_formulation
                double rx = ix - 2 * dot * nx;
                double ry = iy - 2 * dot * ny;
                
                // Conversion of the resulting vector into an angle (radians) using the two-argument arctangent.
                current_angle = atan2(ry, rx);
                
                bounces++;

                // Projection of the ray outside the object to prevent multiple micro-collisions (shadow acne / self-intersection).
                while (pow(x_draw - shadow.x, 2) + pow(y_draw - shadow.y, 2) < shadow_r_sq) {
                    x_draw += cos(current_angle);
                    y_draw += sin(current_angle);
                    travelled_distance++;
                    if(travelled_distance > 2000) break;
                }
            }

            int currently_inside = (pow(x_draw - glass.x, 2) + pow(y_draw - glass.y, 2) < glass_r_sq);
            
            if (currently_inside != in_glass) { 
                double nx = (x_draw - glass.x) / glass.r;
                double ny = (y_draw - glass.y) / glass.r;
                
                // Inversion of the normal if the ray emerges from the refractive medium to respect the geometry.
                if (in_glass) { nx = -nx; ny = -ny; } 
                
                double ix = cos(current_angle);
                double iy = sin(current_angle);
                
                // Absolute refractive indices: Air approx. 1.0, Glass approx. 1.5.
                // The ratio of indices (eta = n1 / n2) is required for Snell's law.
                double n1 = in_glass ? 1.5 : 1.0;
                double n2 = in_glass ? 1.0 : 1.5;
                double eta = n1 / n2;
                
                // Calculation of the discriminant k derived from the vector form of Snell's law.
                // https://en.wikipedia.org/wiki/Snell%27s_law#Vector_form
                double dot = -(ix * nx + iy * ny);
                double k = 1.0 - eta * eta * (1.0 - dot * dot);
                
                if (k < 0.0) { 
                    // Total Internal Reflection (TIR) phenomenon. The angle of incidence exceeds the critical angle, making refraction impossible.
                    // https://en.wikipedia.org/wiki/Total_internal_reflection
                    double rx = ix + 2 * dot * nx;
                    double ry = iy + 2 * dot * ny;
                    current_angle = atan2(ry, rx);
                } else { 
                    // Vector resolution of refraction according to Snell's equation.
                    double tx = eta * ix + (eta * dot - sqrt(k)) * nx;
                    double ty = eta * iy + (eta * dot - sqrt(k)) * ny;
                    current_angle = atan2(ty, tx);
                    in_glass = currently_inside; 
                }
                
                bounces++;
                
                int target_state = in_glass; 
                while ((pow(x_draw - glass.x, 2) + pow(y_draw - glass.y, 2) < glass_r_sq) != target_state) {
                    x_draw += cos(current_angle);
                    y_draw += sin(current_angle);
                    travelled_distance++;
                    if(travelled_distance > 2000) break;
                }
            }
        }
    }
}

int main(int argc, char *argv[]){
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Raytracing", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    
    SDL_Surface* surface = SDL_GetWindowSurface(window);

    struct Circle circle = {200,200,80};
    struct Ray rays[RAYS];
    
    // Isotropic distribution of rays over 360 degrees (2 * Pi radians).
    for (int i = 0; i < RAYS; i++){
        rays[i].x = circle.x;
        rays[i].y = circle.y;
        rays[i].angle = ((double) i / RAYS) * 2 * M_PI;
    }
    
    struct Circle CircleShadow = {650, 300, 140};
    struct Circle CircleGlass = {450, 450, 100};
    SDL_Rect erase_rectangle = {0,0, WIDTH, HEIGHT};

    int running = 1;
    SDL_Event event;

    // Asynchronous render and event polling loop. Keeps the program running frame by frame.
    while (running){
        
        // Popping the queue of system and user events received by the SDL context.
        while (SDL_PollEvent(&event)){
        
            // Listening for the system window interrupt (e.g., clicking the red cross).
            if (event.type == SDL_QUIT){
                running = 0;
            }

            // Handling of "Drag and Drop" interaction.
            // Triggers only if the mouse moves AND a mouse button is held down.
            if (event.type == SDL_MOUSEMOTION){

                // Bitmask checking the specific state of the Left Mouse Button.
                // Used here to smoothly move the light source coordinates.
                if (event.motion.state & SDL_BUTTON_LMASK) {
                    circle.x = event.motion.x;
                    circle.y = event.motion.y;
                    
                    for (int i = 0; i < RAYS; i++){
                        rays[i].x = circle.x;
                        rays[i].y = circle.y;
                    }
                }

                // Bitmask checking the state of the Right Mouse Button. Moves the opaque obstacle (Mirror).
                if (event.motion.state & SDL_BUTTON_RMASK) {
                    CircleShadow.x = event.motion.x;
                    CircleShadow.y = event.motion.y;
                }

                // Bitmask checking the state of the Middle Mouse Button. Moves the transparent obstacle (Glass).
                if (event.motion.state & SDL_BUTTON_MMASK) {
                    CircleGlass.x = event.motion.x;
                    CircleGlass.y = event.motion.y;
                }
            }
        }
        
        // Clearing the memory buffer (Frame buffer) to prepare the new display frame.
        SDL_FillRect(surface, &erase_rectangle, BLACK);

        FillRays(surface, rays, COLOR_RAYS, CircleShadow, CircleGlass);
        FillCircle(surface, circle, SUN);
        FillCircle(surface, CircleShadow, WHITE);
        FillCircle(surface, CircleGlass, COLOR_GLASS);
        
        // Synchronization and writing of the surface frame buffer to the physical screen.
        SDL_UpdateWindowSurface(window);
    
        // Artificial time limitation to avoid monopolizing 100% of the CPU (Approx. 100 FPS).
        SDL_Delay(10);
    }
    
    // Freeing the dynamically allocated memory by SDL.
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}