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
#define MAX_ZONES 20

typedef struct {
    int ID[MAX_CARDS];
    float x[MAX_CARDS];
    float y[MAX_CARDS];
    float vx[MAX_CARDS];
    float vy[MAX_CARDS];
    float tx[MAX_CARDS];
    float ty[MAX_CARDS];
    // float px[MAX_CARDS];
    // float py[MAX_CARDS];
    bool isDragging[MAX_CARDS];
    bool isActive[MAX_CARDS];
    int zoneID[MAX_CARDS];
    int num;
} Cards; Cards cards;

typedef enum {
    ZONETYPE_NORMAL,
    ZONETYPE_DELETE,
    ZONETYPE_PLAY,
    ZONETYPE_DECK,
} ZoneType;

typedef struct {
    int ID[MAX_ZONES];
    float x[MAX_ZONES];
    float y[MAX_ZONES];
    float w[MAX_ZONES];
    float h[MAX_ZONES];
    float max_cards[MAX_ZONES];
    float num_cards[MAX_ZONES];
    float isActive[MAX_ZONES];
    ZoneType zoneType[MAX_ZONES];
} Zones; Zones zones;



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

    cards.num = 0;
    for (int i = 0; i < MAX_CARDS; i++) {
        cards.isActive[i] = false;
        cards.zoneID[i] = -1;
        cards.ID[i] = i;
    }

    for (int i = 0; i < MAX_ZONES; i++) {
        zones.isActive[i] = false;
        zones.num_cards[i] = 0;
    }

    // zone 1, hand
    zones.isActive[2]=true;
    zones.x[2]=50;
    zones.y[2]=1000-210-50;
    zones.w[2]=1400;
    zones.h[2]=210;
    zones.max_cards[2] = 5;
    zones.zoneType[2] = ZONETYPE_NORMAL;

    // zone 2, delete pile
    zones.isActive[1]=true;
    zones.x[1]=1500-160-50;
    zones.y[1]=50;
    zones.w[1]=160;
    zones.h[1]=210;
    zones.max_cards[1] = 1;
    zones.zoneType[1] = ZONETYPE_DELETE;

    // zone 3, equip
    zones.isActive[3]=true;
    zones.x[3]=1500-160-50-160-50;
    zones.y[3]=50;
    zones.w[3]=160;
    zones.h[3]=210;
    zones.max_cards[3] = 1;
    zones.zoneType[3] = ZONETYPE_NORMAL;

    // zone 4, deck
    // zones.isActive[4]=true;
    // zones.x[4]=50;
    // zones.y[4]=50;
    // zones.w[4]=160;
    // zones.h[4]=210;
    // zones.max_cards[4] = 0;
    // zones.zoneType[4] = ZONETYPE_DECK;

    int running = 1;
    while (running) {

            // zone 1, hand
            zones.isActive[2]=true;
            zones.x[2]=wW*50/1500;
            zones.y[2]=wW*(1000-210-50)/1500;
            zones.w[2]=wW*1400/1500;
            zones.h[2]=wW*210/1500;
            zones.max_cards[2] = 5;
            zones.zoneType[2] = ZONETYPE_NORMAL;

            // zone 2, delete pile
            zones.isActive[1]=true;
            zones.x[1]=wW*(1500-160-50)/1500;
            zones.y[1]=wW*50/1500;
            zones.w[1]=wW*160/1500;
            zones.h[1]=wW*210/1500;
            zones.max_cards[1] = 1;
            zones.zoneType[1] = ZONETYPE_DELETE;

            // zone 3, equip
            zones.isActive[3]=true;
            zones.x[3]=wW*(1500-160-50-160-50)/1500;
            zones.y[3]=wW*50/1500;
            zones.w[3]=wW*160/1500;
            zones.h[3]=wW*210/1500;
            zones.max_cards[3] = 1;
            zones.zoneType[3] = ZONETYPE_NORMAL;

            // zone 4, deck
            zones.isActive[4]=true;
            zones.x[4]=wW*50/1500;
            zones.y[4]=wW*50/1500;
            zones.w[4]=wW*160/1500;
            zones.h[4]=wW*210/1500;
            zones.max_cards[4] = 0;
            zones.zoneType[4] = ZONETYPE_DECK;

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
            if (!cards.isActive[i]) continue;

            // if the cursor is not dragging anything, the card is not being dragged and the mouse clicks on the card, it will be picked up
            if (!isDragging && !cards.isDragging[i] && point_box_collision(mousex, mousey, cards.x[i] - (142 / 2), cards.y[i] - (190 / 2), 142, 190) && (currMouseState & SDL_BUTTON_LMASK) && !(prevMouseState & SDL_BUTTON_LMASK)) {
                cards.isDragging[i] = true;
                isDragging = true;
            }

            // if the card is being dragged and it is let go of
            if (cards.isDragging[i] && !(currMouseState & SDL_BUTTON_LMASK)) {

                for (int j = 0; j < MAX_ZONES; j++) {
                    if (!zones.isActive[j]) continue;

                    // if the cards is in a zone and the zone is not full, its origin position is set to that
                    if (point_box_collision(cards.tx[i], cards.ty[i], zones.x[j], zones.y[j], zones.w[j], zones.h[j]) && (zones.num_cards[j] < zones.max_cards[j])) {

                        // subtract num cards for original deck
                        zones.num_cards[cards.zoneID[i]]-=1;

                        if (zones.zoneType[j] == ZONETYPE_DELETE) {
                            cards.isActive[i] = false;
                            cards.zoneID[i] = -1;
                            cards.num-=1;
                        } else {
                            
                            // the cards origin is set to the xones center
                            // cards.px[i] = zones.x[j]+(zones.w[j]/2);
                            // cards.py[i] = zones.y[j]+(zones.h[j]/2);

                            // fils up a slot in that zone
                            zones.num_cards[j]+=1;

                            // the cards zone id is set to the zone number
                            cards.zoneID[i] = j;
                        }

                        break;
                    }

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
                cards.tx[i] = zones.x[cards.zoneID[i]] + zones.w[cards.zoneID[i]]/2;
                cards.ty[i] = zones.y[cards.zoneID[i]] + zones.h[cards.zoneID[i]]/2;
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

        // drawing cards from your deck, finding an empty card slot
        if (!isDragging && (currMouseState & SDL_BUTTON_LMASK) && !(prevMouseState & SDL_BUTTON_LMASK) && point_box_collision(mousex, mousey, zones.x[4], zones.y[4], zones.w[4], zones.h[4])) {
            // bool found_card = false;
            for (int k = 0; k < MAX_CARDS; k++) {
                if (cards.isActive[k]) continue;

                    cards.num+=1;
                    // initialize a random card from the deck
                    isDragging = true;
                    // cards.px[k] = zones.x[2]+(zones.w[2]/2);
                    // cards.py[k] = zones.y[2]+(zones.h[2]/2);
                    cards.zoneID[k] = 2;
                    cards.x[k] = zones.x[4]+71;
                    cards.y[k] = zones.y[4]+95;
                    cards.isDragging[k] = true;
                    cards.isActive[k]=true;
                    printf("new card\n");
                    break;
                // }
            }
        }

        // ZONES
        for (int j = 0; j < MAX_ZONES; j++) {
            if (!zones.isActive[j]) continue;

            if (zones.zoneType[j] == ZONETYPE_DELETE) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }
            
            SDL_FRect zone = {zones.x[j], zones.y[j], zones.w[j], zones.h[j]};
            SDL_RenderRect(renderer, &zone);
        }

        // zone 1
        // SDL_FRect zone1 = {0, wH*0.5, 9000, 9000};
        // SDL_RenderRect(renderer, &zone1);

        // menu cursor
        SDL_FRect cursor = {mousex, mousey, (float)(wW*0.02f), (float)(wW*0.02f)};
        SDL_RenderRect(renderer, &cursor);

        // if (cards.zoneID[0] != -1) {
        //     printf("This card is in zone ID %d\n", cards.zoneID[0]);
        // }
        // printf("%d\n", cards.num);
        

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
