#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>

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

bool isDragging;

#define MAX_CARDS 20

typedef struct {
    int id[MAX_CARDS];
    float x[MAX_CARDS];
    float y[MAX_CARDS];
    float vx[MAX_CARDS];
    float vy[MAX_CARDS];
    float tx[MAX_CARDS];
    float ty[MAX_CARDS];
    float px[MAX_CARDS];
    float py[MAX_CARDS];
    bool isDragging[MAX_CARDS];
} Cards; Cards cards;

SDL_Texture* texture;

bool load_textures() {

	texture = IMG_LoadTexture(renderer, "./resources/textures/card.png");
	SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);

	return true;
}

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

bool point_box_collision(float px, float py, float bx, float by, float bw, float bh) {
    return (px >= bx && px <= bx + bw && py >= by && py <= by + bh);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Retro FPS Deckbuilder", 1500, 1000, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, NULL);
	SDL_SetRenderScale(renderer, 1, 1);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    load_textures();
    SDL_HideCursor();

    // setup keystates
    const Uint8* sdlKeys = (Uint8*)SDL_GetKeyboardState(NULL);
    SDL_memcpy(currKeyState, sdlKeys, NUM_KEYS);
    SDL_memcpy(prevKeyState, sdlKeys, NUM_KEYS);

    for (int i = 0; i < MAX_CARDS; i++) {
        cards.id[i] = 0;
    }

    // card 1
    cards.id[0]=1;
    cards.tx[0]=1500*0.5;
    cards.ty[0]=1000*0.5;

    // card 2
    cards.id[1]=1;
    cards.tx[1]=1500*0.2;
    cards.ty[1]=1000*0.2;
    
    // card 3
    cards.id[2]=1;
    cards.tx[2]=1500*0.2;
    cards.ty[2]=1000*0.8;
    
    // card 4
    cards.id[3]=1;
    cards.tx[3]=1500*0.8;
    cards.ty[3]=1000*0.8;

    // card 5
    cards.id[4]=1;
    cards.tx[4]=1500*0.8;
    cards.ty[4]=1000*0.2;

    int running = 1;
    while (running) {
        dt = get_delta_time();
        
        // inputs
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_EVENT_QUIT) running = false;
        }
        const Uint8* sdlKeys = (Uint8*)SDL_GetKeyboardState(NULL);
        SDL_memcpy(currKeyState, sdlKeys, NUM_KEYS);
        
        prevMouseState = currMouseState;
        currMouseState = SDL_GetMouseState(&mousex, &mousey);

        // updates
        SDL_GetWindowSize(window, &wW, &wH);  
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        for (int i = 0; i < MAX_CARDS; i++) {
            if (cards.id[i] == 0) continue;

            if (!isDragging && !cards.isDragging[i] && point_box_collision(mousex, mousey, cards.x[i] - (142 / 2), cards.y[i] - (190 / 2), 142, 190) && (currMouseState & SDL_BUTTON_LMASK) && !(prevMouseState & SDL_BUTTON_LMASK)) {
                cards.isDragging[i] = true;
                isDragging = true;
            }

            if (cards.isDragging[i] && !(currMouseState & SDL_BUTTON_LMASK)) {
                // if the cards is in a zone, its origin position is set to that
                if (point_box_collision(mousex, mousey, 0, wH*0.5, 9000, 9000)) {
                    cards.px[i] = cards.tx[i];
                    cards.py[i] = cards.ty[i];
                }
                // puts the card down / stops it from dragging
                cards.isDragging[i] = false;
                isDragging = false;
            }

            if (cards.isDragging[i]) {
                // if you are dragging a card it goes to your mouse
                cards.tx[i] = mousex;
                cards.ty[i] = mousey;
            } else {
                // if you are not dragging a card it goes back to its original position (which, if is in a zone area, becomes in the zone area)
                cards.tx[i] = cards.px[i];
                cards.ty[i] = cards.py[i];
            }

            cards.vx[i] = (cards.tx[i] - cards.x[i]) / 0.25;
            cards.vy[i] = (cards.ty[i] - cards.y[i]) / 0.25;
            cards.y[i] += cards.vy[i] * dt;
            cards.x[i] += cards.vx[i] * dt;

            SDL_FRect rect = {cards.x[i] - (142 / 2), cards.y[i] - (190 / 2), 142, 190};
            SDL_FPoint center = {rect.w / 2, rect.h / 2}; // rotate around center of image
            double angle = cards.vx[i]/30; // rotate 45 degrees clockwise

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderRect(renderer, &rect);
            SDL_RenderTextureRotated(renderer, texture, NULL, &rect, angle, &center, SDL_FLIP_NONE);

        }

        // zone 1
        SDL_FRect zone1 = {0, wH*0.5, 9000, 9000};
        SDL_RenderRect(renderer, &zone1);

        // menu cursor
        SDL_FRect cursor = {mousex, mousey, (float)(wW*0.02f), (float)(wW*0.02f)};
        SDL_RenderRect(renderer, &cursor);
        

        SDL_RenderPresent(renderer);

        // other
        SDL_memcpy(prevKeyState, currKeyState, NUM_KEYS);
        control_fps(120.0f);
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    return 0;
}
