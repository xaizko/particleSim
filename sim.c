#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdlib.h>  
#include <time.h>
#include <stdio.h>
#include <math.h>    
#include <stdbool.h> 

#ifndef M_PI
#define M_PI 3.14159265358979323846  
#endif

const float magnetStrength = 10.0f; 

// Define global window and renderer
static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;

// Define particle and nucleus structures
static struct point particle;
static struct nucleus nucleus;

// Frame rate constants
const int TARGET_FPS = 60;
const int FRAME_DELAY = 1000 / TARGET_FPS;  // ~16 ms

// Define the point and nucleus structures
struct point {
    float xcord;
    float ycord;
    float xvel;
    float yvel;
};

struct nucleus {
    struct point center;
    float magnetStrength;
};

// Define the number of particles and nuclei
#define PARTICLE_COUNT 20
#define NUCLEUS_COUNT 1
// Define arrays for particles and nuclei to simulate more than 1
static struct point particles[PARTICLE_COUNT];
static struct nucleus necleuses[NUCLEUS_COUNT];

struct point createDefaultPoint(int seedOffset) {
    struct point p;
    srand(time(NULL) + seedOffset); // different seed per particle
    p.xcord = rand() % 800;
    p.ycord = rand() % 800;

    int speed = (rand() % 5) + 2;
    p.xvel = (rand() % 2 == 0) ? speed : -speed;

    speed = (rand() % 5) + 2;
    p.yvel = (rand() % 2 == 0) ? speed : -speed;

    return p;
}

struct nucleus createDefaultNucleus(int seedOffset) {
    struct nucleus n;
    srand(time(NULL) + seedOffset); // different seed per nucleus
    n.center.xcord = rand() % 800;
    n.center.ycord = rand() % 800;

    int speed =(rand() % 5) + 2 ;
    n.center.xvel = (rand() % 2 == 0) ? speed : -speed;

    speed = (rand() % 5) + 2;
    n.center.yvel = (rand() % 2 == 0) ? speed : -speed;

    n.magnetStrength = (rand() % 5) + 1; // Random strength between 1 and 5

    return n;
}

/* This function runs once at startup. */
SDL_AppResult SDL_AppInit(void **appstate, int argc, char *argv[])
{
    int i;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    if (!SDL_CreateWindowAndRenderer("Particle Sim", 800, 800, 0, &window, &renderer)) {
        SDL_Log("Couldn't create window/renderer: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    for (int i = 0; i < PARTICLE_COUNT; ++i) {
        particles[i] = createDefaultPoint(i * 137); // prime offset to vary seeds
    }

    for (int i = 0; i < NUCLEUS_COUNT; i++) {
        necleuses[i] = createDefaultNucleus(i * 137); // prime offset to vary seeds
    }


    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs when a new event (mouse input, keypresses, etc) occurs. */
SDL_AppResult SDL_AppEvent(void *appstate, SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS;  /* end the program, reporting success to the OS. */
    }
    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

//Draw Circle
void drawCircle(SDL_Renderer *renderer, int centerX, int centerY, int radius)
{
    // Using the midpoint circle algorithm
    int x = radius;
    int y = 0;
    int err = 0;

    while (x >= y)
    {
        SDL_RenderLine(renderer, centerX - x, centerY + y, centerX + x, centerY + y);
        SDL_RenderLine(renderer, centerX - x, centerY - y, centerX + x, centerY - y);
        SDL_RenderLine(renderer, centerX - y, centerY + x, centerX + y, centerY + x);
        SDL_RenderLine(renderer, centerX - y, centerY - x, centerX + y, centerY - x);
        
        
        if (err <= 0)
        {
            y += 1;
            err += 2*y + 1;
        }
        
        if (err > 0)
        {
            x -= 1;
            err -= 2*x + 1;
        }
    }
}

void moveParticle(struct point *p, float nucleusX, float nucleusY) {
    // Calculate vector from particle to nucleus
    float dx = nucleusX - p->xcord;
    float dy = nucleusY - p->ycord;
    float distance = SDL_sqrtf(dx * dx + dy * dy);
    
    int particleIndex = (p - particles);
    if (particleIndex < 0 || particleIndex >= PARTICLE_COUNT) {
        particleIndex = 0; 
    }
    
    if (distance > 0) {
        dx /= distance;
        dy /= distance;
        
        // Define the minimum distance particles can get to the nucleus center
        float minDistance = 40.0f;
        float orbitRadius = 60.0f + (particleIndex % 5) * 10.0f;
        
        if (distance < minDistance) {
            // Repulsion force - stronger when closer to nucleus
            float repulsionStrength = 0.5f * (minDistance - distance) / minDistance;
            
            // Apply repulsion force (away from nucleus)
            p->xvel -= dx * repulsionStrength;
            p->yvel -= dy * repulsionStrength;
        } else {
            // Normal attraction force - only when outside minimum distance
            float attractionStrength = 0.1f + (distance - minDistance) / 400.0f;
            
            // Apply attraction force
            p->xvel += dx * attractionStrength;
            p->yvel += dy * attractionStrength;
            
            // Add a slight tangential force for orbital motion
            float tangentialFactor = 0.05f;
            p->xvel += -dy * tangentialFactor;
            p->yvel += dx * tangentialFactor;
        }
        
        // Damping factor to prevent infinite acceleration
        p->xvel *= 0.98f;
        p->yvel *= 0.98f;
        
        // Update position
        p->xcord += p->xvel;
        p->ycord += p->yvel;
        
        // Wall collisions
        if (p->xcord <= 0 || p->xcord >= 800) {
            p->xvel = -p->xvel * 0.9f; // Lose some energy on collision
            if (p->xcord < 0) p->xcord = 0;
            if (p->xcord > 800) p->xcord = 800;
        }
        
        if (p->ycord <= 0 || p->ycord >= 800) {
            p->yvel = -p->yvel * 0.9f; // Lose some energy on collision
            if (p->ycord < 0) p->ycord = 0;
            if (p->ycord > 800) p->ycord = 800;
        }
        
        // Particle size
        int particleRadius = 4;
        
        // Set particle color - blue
        SDL_SetRenderDrawColor(renderer, 0, 119, 204, 255);
        
        drawCircle(renderer, (int)p->xcord, (int)p->ycord, particleRadius);
    }
}

void moveNucleus(struct nucleus *n, float mouseX, float mouseY, bool mouseDown) {

    float dx = mouseX - n->center.xcord;
    float dy = mouseY - n->center.ycord;
    float distance = SDL_sqrtf(dx * dx + dy * dy);

    if (mouseDown) {
        if (distance > 10) {
            float length = SDL_sqrtf(dx * dx + dy * dy);
            if (length > 0) {
                dx /= length;
                dy /= length;
                
                // Move directly toward mouse position at a fixed speed
                float speed = 5.0f;
                n->center.xcord += dx * speed;
                n->center.ycord += dy * speed;
            }
        } 
    } else {
        // When not clicked, continue normal movement
        n->center.xcord += n->center.xvel;
        n->center.ycord += n->center.yvel;
    }

    // Collision with walls
    if (n->center.xcord <= 0 || n->center.xcord >= 800) {
        n->center.xvel = -n->center.xvel;
        if (n->center.xcord < 0) n->center.xcord = 0;
        if (n->center.xcord > 800) n->center.xcord = 800;
    }

    if (n->center.ycord <= 0 || n->center.ycord >= 800) {
        n->center.yvel = -n->center.yvel;
        if (n->center.ycord < 0) n->center.ycord = 0;
        if (n->center.ycord > 800) n->center.ycord = 800;
    }
    
    // Adjust nucleus size
    int nucleusRadius = 20;

    drawCircle(renderer, (int)n->center.xcord, (int)n->center.ycord, nucleusRadius);
}

// Function to calculate repulsion between particles
void calculateParticleRepulsion(struct point *particles, int particleCount) {
    const float repulsionDistance = 15.0f; // Minimum distance between particles
    const float repulsionStrength = 0.2f;  // Strength of the repulsion
    
    for (int i = 0; i < particleCount; i++) {
        for (int j = i + 1; j < particleCount; j++) {
            float dx = particles[j].xcord - particles[i].xcord;
            float dy = particles[j].ycord - particles[i].ycord;
            float distanceSquared = dx * dx + dy * dy;
            
            // Only apply repulsion if particles are close enough
            if (distanceSquared < repulsionDistance * repulsionDistance && distanceSquared > 0) {
                float distance = SDL_sqrtf(distanceSquared);
                float force = repulsionStrength * (repulsionDistance - distance) / repulsionDistance;
                
                // Normalize direction vector
                dx /= distance;
                dy /= distance;
                
                // Apply repulsion to both particles in opposite directions
                particles[i].xvel -= dx * force;
                particles[i].yvel -= dy * force;
                particles[j].xvel += dx * force;
                particles[j].yvel += dy * force;
            }
        }
    }
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    Uint64 start = SDL_GetTicks();

    // Clear the screen (black background)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Black color
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 0, 119, 204, 255); 

    // Get the current mouse position and check if the button is pressed
    float mouseX, mouseY;
    Uint32 buttons = SDL_GetMouseState(&mouseX, &mouseY);
    bool mouseDown = buttons & SDL_BUTTON_LEFT;

    // Loop through each particle and update its position based on mouse interaction

    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // Red color for nucleus
    for (int i = 0; i < NUCLEUS_COUNT; i++) {
        moveNucleus(&necleuses[i], mouseX, mouseY, mouseDown);
        SDL_SetRenderDrawColor(renderer, 0, 119, 204, 255);
        for (int j = 0; j < PARTICLE_COUNT; j++) {
            moveParticle(&particles[j], necleuses[i].center.xcord, necleuses[i].center.ycord);
        }
    }

    // Apply particle-to-particle repulsion
    calculateParticleRepulsion(particles, PARTICLE_COUNT);

    // Update the display with the new particle positions
    SDL_RenderPresent(renderer);

    // Frame rate control
    Uint64 end = SDL_GetTicks();
    int elapsed = (int)(end - start);
    if (elapsed < FRAME_DELAY) {
        SDL_Delay(FRAME_DELAY - elapsed);
    }

    return SDL_APP_CONTINUE;  // Continue the program
}


/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}
