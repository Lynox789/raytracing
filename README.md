# 2D Optical Physics & Raytracing Engine

A real-time 2D raytracing engine built in C using the SDL2 library. This project simulates fundamental optical physics, including ray marching, specular reflection, refraction, and light attenuation.

---

## Interactive Controls

The simulation features real-time interactions using mouse inputs, allowing you to dynamically test optical phenomena.

| Input | Action | Target Object |
| :--- | :--- | :--- |
| **Left Click + Drag** | Moves the light source and origin of the rays. | Sun (Light Source) |
| **Right Click + Drag** | Moves the opaque obstacle. | Mirror (Opaque Circle) |
| **Middle Click + Drag** | Moves the transparent refractive obstacle. | Glass (Refractive Circle) |

---

## Scientific & Mathematical Principles

### 1. Ray Marching & Coordinate Systems
The engine casts 500 rays in an isotropic distribution ($360^\circ$ or $2\pi$ radians). The rays advance step-by-step (Ray Marching) using polar coordinates converted to Cartesian coordinates:
$$x = x_0 + r\cos(\theta)$$
$$y = y_0 + r\sin(\theta)$$

Collision detection is based on the Cartesian equation of a disk. A ray intersects or resides inside a circle if the following mathematical condition is met:
$$(x - x_c)^2 + (y - y_c)^2 < r^2$$

### 2. Light Attenuation (Inverse-Square Law)
In physics, light intensity is inversely proportional to the square of the distance from the source. The engine approximates this by decreasing the RGB values of the ray based on the distance traveled ($d$), simulating energy dissipation in the environment:
$$I = I_0 \times \left(1 - \frac{d}{d_{max}}\right)$$

### 3. Specular Reflection
When a ray hits the opaque obstacle, it undergoes specular reflection. The normal vector $\vec{n}$ at the point of impact is calculated, and the reflected vector $\vec{r}$ is derived from the incident vector $\vec{i}$ using the dot product:
$$\vec{r} = \vec{i} - 2(\vec{i} \cdot \vec{n})\vec{n}$$
Upon bouncing, the color of the ray shifts to a warmer, darker tone, simulating the absorption of part of the light spectrum by the physical material.

### 4. Refraction & Snell's Law
When a ray intersects the glass obstacle, its trajectory bends according to the Snell-Descartes law. The engine uses the absolute refractive indices of air ($n_1 \approx 1.0$) and glass ($n_2 \approx 1.5$) to calculate the ratio $\eta = \frac{n_1}{n_2}$. 

The vector formulation of Snell's law requires evaluating a discriminant $k$:
$$k = 1 - \eta^2(1 - (\vec{i} \cdot \vec{n})^2)$$

* **Refraction:** If $k \geq 0$, the ray successfully passes through the boundary of the medium, and its new trajectory is computed.
* **Total Internal Reflection (TIR):** If $k < 0$, the incident angle exceeds the critical angle. The light cannot escape the refractive medium and is entirely reflected back inside the glass.

---

## Compilation & Execution

### Prerequisites
* A C compiler (e.g., `gcc`, `clang`)
* SDL2 Development Library installed on your system.

### Compilation Example (Linux / MSYS2 MinGW)

```gcc raytracing.c -o raytracing -lSDL2 - ```

### Execution
```./raytracing ```