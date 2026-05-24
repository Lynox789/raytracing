//Fichier créé le 24/05/2026 à 00:04:36
#include <stdio.h>
#include <SDL2/SDL.h>
#include <math.h>

#define WIDTH 900
#define HEIGHT 600

#define WHITE 0xffffffff
#define BLACK 0x00000000
#define COLOR_RAYS 0xefefefef

#define RAYS 100

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
    for (double x = circle.x - circle.r; x <= circle.x + circle.r; x++){

        for (double y = circle.y - circle.r; y <= circle.y + circle.r; y++){
    
            double distance_squared = pow(x - circle.x, 2) + pow(y - circle.y, 2);
            
            if (distance_squared < radius_squared){
                SDL_Rect pixel = (SDL_Rect) {x,y,1,1};
                SDL_FillRect(surface, &pixel, color);
            }
        }
    }
}

void FillRays(SDL_Surface* surface, struct Ray rays[RAYS], Uint32 color){
    
    for (int i = 0; i < RAYS; i++){
        struct Ray ray = rays[i];
        
        int end_screen = 0;
        int object_hit = 0;

        double step = 1;
        double x_draw = ray.x;
        double y_draw = ray.y;
        
        while (!end_screen && !object_hit){
            x_draw += step * cos(ray.angle);
            y_draw += step * sin(ray.angle);
            
            SDL_Rect pixel = (SDL_Rect) {x_draw, y_draw, 1,1};
            SDL_FillRect(surface, &pixel, color);

            if (x_draw < 0 || x_draw > WIDTH) end_screen = 1;
            if (y_draw < 0 || y_draw > HEIGHT) end_screen = 1;
        }
    }
}

int main(int argc, char *argv[]){
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Raytracing", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, 0);
    
    SDL_Surface* surface = SDL_GetWindowSurface(window);

    struct Circle circle = {200,200,80};
    struct Ray rays[RAYS];
    
    for (int i = 0; i < RAYS; i++){
        rays[i].x = circle.x;
        rays[i].y = circle.y;
        rays[i].angle = ((double) i / RAYS) * 2 * M_PI;
    }
    struct Circle CircleShadow = {650, 300, 140};
    SDL_Rect erase_rectangle = {0,0, WIDTH, HEIGHT};

    int running = 1;
    SDL_Event event;

    while (running){
        while (SDL_PollEvent(&event)){
        
            if (event.type == SDL_QUIT){
                running = 0;
            }

            if (event.type == SDL_MOUSEMOTION && event.motion.state != 0){
                circle.x = event.motion.x;
                circle.y = event.motion.y;
                    
                for (int i = 0; i < RAYS; i++){
                    rays[i].x = circle.x;
                    rays[i].y = circle.y;
                }
            }
        }
        SDL_FillRect(surface, &erase_rectangle, BLACK);

        FillCircle(surface, circle, WHITE);
        FillRays(surface, rays, COLOR_RAYS);
        FillCircle(surface, CircleShadow, WHITE);
        SDL_UpdateWindowSurface(window);
    
        SDL_Delay(10);
    }
	return 0;
}
