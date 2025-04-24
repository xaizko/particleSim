#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdlib.h>  
#include <time.h>
#include <stdio.h>
const float magnetStrength = 5.0f; 


static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static struct point particle;

const int TARGET_FPS = 60;
const int FRAME_DELAY = 1000 / TARGET_FPS;  // ~16 ms

struct point {
    float xcord;
    float ycord;
    float xvel;
    float yvel;
};


#define PARTICLE_COUNT 100
static struct point particles[PARTICLE_COUNT];

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

void moveParticle(struct point *p, float mouseX, float mouseY, bool mouseDown) {
    float dx = mouseX - p->xcord;
    float dy = mouseY - p->ycord;
    float distance = SDL_sqrtf(dx * dx + dy * dy);
    
    if (mouseDown) {
        if (distance > 50) { 
            // Calculate the direction to move towards the mouse
            float length = SDL_sqrtf(dx * dx + dy * dy);
            if (length > 0) {
                dx /= length;
                dy /= length;

                // Update the particle's velocity towards the mouse
                p->xvel += dx * magnetStrength;
                p->yvel += dy * magnetStrength;
            }
        } else {
            // Once close enough to the mouse, start orbiting (perpendicular velocity)
            float tempX = p->xvel;
            float tempY = p->yvel;

            // Calculate perpendicular velocity for orbital motion 
            p->xvel = -dy * 0.5f;  
            p->yvel = dx * 0.5f; 

            // Smooth out the transition with the previous velocity
            p->xvel += tempX * 0.3f;
            p->yvel += tempY * 0.3f;
        }
    }

    // Move the particle based on its velocity
    p->xcord += p->xvel;
    p->ycord += p->yvel;

    // Collision with walls 
    if (p->xcord <= 0 || p->xcord >= 800) {
        p->xvel = -p->xvel;
        if (p->xcord < 0) p->xcord = 0;
        if (p->xcord > 800) p->xcord = 800;
    }

    if (p->ycord <= 0 || p->ycord >= 800) {
        p->yvel = -p->yvel;
        if (p->ycord < 0) p->ycord = 0;
        if (p->ycord > 800) p->ycord = 800;
    }

    // Draw the particle
    drawCircle(renderer, (int)p->xcord, (int)p->ycord, 5);
}



/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    Uint64 start = SDL_GetTicks();

    // Clear the screen (black background)
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);  // Black color
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, (rand() % 255), (rand() % 255), (rand() % 255), (rand() % 255)); 

    // Get the current mouse position and check if the button is pressed
    float mouseX, mouseY;
    Uint32 buttons = SDL_GetMouseState(&mouseX, &mouseY);
    bool mouseDown = buttons & SDL_BUTTON_LEFT;

    // Loop through each particle and update its position based on mouse interaction
    for (int i = 0; i < PARTICLE_COUNT; ++i) {
        moveParticle(&particles[i], mouseX, mouseY, mouseDown);
    }

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
