#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stdlib.h>  
#include <time.h>    

static SDL_Window *window = NULL;
static SDL_Renderer *renderer = NULL;
static struct point particle;

const int TARGET_FPS = 60;
const int FRAME_DELAY = 1000 / TARGET_FPS;  // ~16 ms



struct point {
    int xcord;
    int ycord;
    int xvel;
    int yvel;
};

struct point createDefaultPoint() {
    struct point p;
    p.xcord = 400;
    p.ycord = 400;
    
    srand(time(NULL));

    // Random velocity between -5 and 5, excluding 0
    do {
        p.xvel = (rand() % 11) - 5;
    } while (p.xvel == 0);

    do {
        p.yvel = (rand() % 11) - 5;
    } while (p.yvel == 0);
    
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

    particle = createDefaultPoint();

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

void moveParticle(struct point *p) {
    p->xcord += p->xvel;
    p->ycord += p->yvel;

    // Bounce on X-axis
    if (p->xcord <= 0 || p->xcord >= 800) {
        p->xvel = -p->xvel;
        if (p->xcord < 0) p->xcord = 0;
        if (p->xcord > 800) p->xcord = 800;
    }

    // Bounce on Y-axis
    if (p->ycord <= 0 || p->ycord >= 800) {
        p->yvel = -p->yvel;
        if (p->ycord < 0) p->ycord = 0;
        if (p->ycord > 800) p->ycord = 800;
    }

    drawCircle(renderer, p->xcord, p->ycord, 10);
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void *appstate)
{
    Uint64 start = SDL_GetTicks();

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    //drawCircle(renderer, 400, 400, 10);
    moveParticle(&particle);
    SDL_RenderPresent(renderer);

    Uint64 end = SDL_GetTicks();
    int elapsed = (int)(end - start);
    if (elapsed < FRAME_DELAY) {
        SDL_Delay(FRAME_DELAY - elapsed);
    }

    return SDL_APP_CONTINUE;  /* carry on with the program! */
}

/* This function runs once at shutdown. */
void SDL_AppQuit(void *appstate, SDL_AppResult result)
{
    /* SDL will clean up the window/renderer for us. */
}
