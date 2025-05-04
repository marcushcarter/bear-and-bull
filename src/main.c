#include <SDL3/SDL.h>

#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

void setup();
void inputs();
void update();
void render();

SDL_Window* window;
SDL_Renderer* renderer;
SDL_Event event;
bool running;

#define NUM_KEYS 256
Uint8 currKeyState[NUM_KEYS];
Uint8 prevKeyState[NUM_KEYS];
Uint32 currMouseState;
Uint32 prevMouseState;
float mousex, mousey;

int wW = 1500, wH = 1000;

clock_t previous_time = 0;
float dt;

float get_delta_time() {
    clock_t current_time = clock();
    float delta_time = (float)(current_time - previous_time) / CLOCKS_PER_SEC;
    previous_time = current_time;
    return delta_time;
}

void control_fps(float target_fps) {
	float frame_duration = 1.0f / target_fps;
	clock_t now = clock();
	float elapsed = (float)(now - previous_time) / CLOCKS_PER_SEC;
	float remaining_time = frame_duration - elapsed;
	if (remaining_time > 0) {
		usleep((useconds_t)(remaining_time * 1000000.0f));
	}
}

int main(int argc, char* argv[]) {

    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Retro FPS Deckbuilder", 1500, 1000, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, NULL);
	SDL_SetRenderScale(renderer, 1, 1);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    setup();

    SDL_HideCursor();

    running = true;
    while (running) {
        dt = get_delta_time();
        inputs();
        update();
        render();
        SDL_memcpy(prevKeyState, currKeyState, NUM_KEYS);
        control_fps(120.0f);
    }
    
    return 0;
}

void setup() {
    const Uint8* sdlKeys = (Uint8*)SDL_GetKeyboardState(NULL);
    SDL_memcpy(currKeyState, sdlKeys, NUM_KEYS);
    SDL_memcpy(prevKeyState, sdlKeys, NUM_KEYS);
}

void inputs() {
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_EVENT_QUIT) running = false;
    }
    const Uint8* sdlKeys = (Uint8*)SDL_GetKeyboardState(NULL);
    SDL_memcpy(currKeyState, sdlKeys, NUM_KEYS);
    
    prevMouseState = currMouseState;
    currMouseState = SDL_GetMouseState(&mousex, &mousey);
}

void update() {
    SDL_GetWindowSize(window, &wW, &wH);
}

void render() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    int TYPE = 1;

    // backround color
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_FRect background = {0, 0, (float)wW, (float)wW/1.5f};
    SDL_RenderFillRect(renderer, &background);

    if (TYPE == 1) {
    
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        // deck visual (bottom left corner)
        SDL_FRect deck = {0, (float)(wW/1.5*0.5), (float)(wW*0.25), (float)(wW*0.5/1.5)};
        SDL_RenderRect(renderer, &deck);

        // weapon slot holder
        SDL_FRect weapon = {0, 0, 0, 0};
        SDL_RenderRect(renderer, &weapon);

        // accessory slot holder
        SDL_FRect hand = {(float)(wW*0.02), (float)(wW/1.5*0.02), (float)(wW*0.5), (float)(wW*0.25/1.5)};
        SDL_RenderRect(renderer, &hand);

        SDL_FRect play = {0, 0, 0, 0};
        SDL_RenderRect(renderer, &play);

        SDL_FRect del = {0, 0, 0, 0};
        SDL_RenderRect(renderer, &del);

        // pickup tickets visual
        SDL_FRect pickuptix = {0, 0, 0, 0};
        SDL_RenderRect(renderer, &pickuptix);

        // menu cursor
        SDL_FRect cursor = {mousex, mousey, (float)(wW*0.02f), (float)(wW*0.02f)};
        SDL_RenderRect(renderer, &cursor);

    } else {

        // middle cursor
        SDL_FRect cursor = {(float)(wW/2-wW*0.01), (float)(wH/2-wW*0.01), (float)(wW*0.02f), (float)(wW*0.02f)};
        SDL_RenderRect(renderer, &cursor);

    }

    SDL_RenderPresent(renderer);
}