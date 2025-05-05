#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

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

int window_width, window_height, window_scale = 1.0f;
float game_speed = 4;

#define LERP_SPEED 0.25

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
    float w[MAX_CARDS];
    float h[MAX_CARDS];

    int zoneID[MAX_CARDS];
    int zoneNum[MAX_CARDS];
    
    bool isDragging[MAX_CARDS];
    bool isActive[MAX_CARDS];

    int num;
} Cards; 

Cards inplay;
Cards indeck;

#define CARD_HEIGHT 190
#define CARD_WIDTH 142
#define CARD_SPACING 10

typedef struct {
    int ID[MAX_ZONES];
    char name[MAX_ZONES][256];

    float x[MAX_ZONES];
    float y[MAX_ZONES];
    float w[MAX_ZONES];
    float h[MAX_ZONES];

    int max_cards[MAX_ZONES];
    int num_cards[MAX_ZONES];

    float isActive[MAX_ZONES];
} Zones; 

Zones pausezones;

typedef enum {
    ZONE_DECK = 1,
    ZONE_HAND,
    ZONE_DISCARD,
    ZONE_EQUIP_1,
    ZONE_EQUIP_2,
    ZONE_EQUIP_3,
} ZoneType;

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

void draw_cards(int num) {
    for (int i = 0; i < num; i++) {
        if (pausezones.num_cards[ZONE_HAND] >= pausezones.max_cards[ZONE_HAND]) continue;
        for (int k = 0; k < MAX_CARDS; k++) {
            if (inplay.isActive[k]) continue;

            inplay.num+=1;
            // initialize a random card from the deck
            isDragging = true;
            inplay.zoneID[k] = ZONE_HAND;
            pausezones.num_cards[ZONE_HAND]+=1;
            inplay.zoneNum[k]=pausezones.num_cards[ZONE_HAND];
            inplay.x[k] = (pausezones.x[ZONE_DECK]+pausezones.w[ZONE_DECK]/2)*window_scale;
            inplay.y[k] = (pausezones.y[ZONE_DECK]+pausezones.h[ZONE_DECK]/2)*window_scale;
            inplay.isDragging[k] = true;
            inplay.isActive[k]=true;
            printf("draw card\n");
            break;
        }
    }
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Retro FPS Deckbuilder", 1500, 1000, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, NULL);
	SDL_SetRenderScale(renderer, 1, 1);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    load_textures();
    SDL_HideCursor();
    SDL_GetWindowSize(window, &window_width, &window_height);  
    window_scale = window_width/1500.0f;

    const Uint8* sdlKeys = (Uint8*)SDL_GetKeyboardState(NULL);
    SDL_memcpy(currKeyState, sdlKeys, NUM_KEYS);
    SDL_memcpy(prevKeyState, sdlKeys, NUM_KEYS);

    inplay.num = 0;
    for (int i = 0; i < MAX_CARDS; i++) {
        inplay.isActive[i] = false;
        inplay.zoneID[i] = -1;
        inplay.ID[i] = i;
        inplay.w[i]=CARD_WIDTH;
        inplay.h[i]=CARD_HEIGHT;
    }

    for (int i = 0; i < MAX_ZONES; i++) {
        pausezones.isActive[i] = false;
        pausezones.num_cards[i] = 0;
        pausezones.max_cards[i]=0;
    }

    int running = 1;
    while (running) {

        // get window sizes and set window scaling coefficient 
        SDL_GetWindowSize(window, &window_width, &window_height);  
        window_scale = window_width/1500.0f;

        dt = get_delta_time()*game_speed;

        int zone_num;

        zone_num = ZONE_DISCARD;
        strcpy(pausezones.name[zone_num], "discard slot");
        pausezones.max_cards[zone_num] = 1;
        pausezones.x[zone_num]=(window_width-((CARD_WIDTH+CARD_SPACING*2)+50)*1)*window_scale;
        pausezones.y[zone_num]=50*window_scale;
        pausezones.w[zone_num]=(CARD_SPACING+(CARD_WIDTH+CARD_SPACING)*pausezones.max_cards[zone_num])*window_scale;
        pausezones.h[zone_num]=(CARD_HEIGHT+CARD_SPACING*2)*window_scale;
        pausezones.isActive[zone_num]=true;

        // zone_num = ZONE_HAND;
        // strcpy(pausezones.name[zone_num], "hand");
        // pausezones.max_cards[zone_num] = 9;
        // pausezones.x[zone_num]=50*window_scale;
        // pausezones.y[zone_num]=(window_height-((CARD_HEIGHT+CARD_SPACING*2)+50)*1)*window_scale;
        // pausezones.w[zone_num]=(CARD_SPACING+(CARD_WIDTH+CARD_SPACING)*pausezones.max_cards[zone_num])*window_scale;
        // pausezones.h[zone_num]=(CARD_HEIGHT+CARD_SPACING*2)*window_scale;
        // pausezones.isActive[zone_num]=true;

        zone_num = ZONE_HAND;
        strcpy(pausezones.name[zone_num], "hand");
        pausezones.max_cards[zone_num] = 9;
        pausezones.x[zone_num]=60*window_scale;
        pausezones.y[zone_num]=(window_height-((CARD_HEIGHT+CARD_SPACING*2)+50)*1)*window_scale;
        pausezones.w[zone_num]=(CARD_SPACING+(CARD_WIDTH+CARD_SPACING)*pausezones.max_cards[zone_num])*window_scale;
        pausezones.h[zone_num]=(CARD_HEIGHT+CARD_SPACING*2)*window_scale;
        pausezones.isActive[zone_num]=true;

        zone_num = ZONE_EQUIP_1;
        strcpy(pausezones.name[zone_num], "equip slot");
        pausezones.max_cards[zone_num] = 1;
        pausezones.x[zone_num]=(window_width-((CARD_WIDTH+CARD_SPACING*2)+50)*2)*window_scale;
        pausezones.y[zone_num]=50*window_scale;
        pausezones.w[zone_num]=(CARD_SPACING+(CARD_WIDTH+CARD_SPACING)*pausezones.max_cards[zone_num])*window_scale;
        pausezones.h[zone_num]=(CARD_HEIGHT+CARD_SPACING*2)*window_scale;
        pausezones.isActive[zone_num]=true;

        zone_num = ZONE_DECK;
        strcpy(pausezones.name[zone_num], "deck");
        pausezones.max_cards[zone_num] = 0;
        pausezones.x[zone_num]=50*window_scale;
        pausezones.y[zone_num]=50*window_scale;
        pausezones.w[zone_num]=(CARD_SPACING+(CARD_WIDTH+CARD_SPACING))*window_scale;
        pausezones.h[zone_num]=(CARD_HEIGHT+CARD_SPACING*2)*window_scale;
        pausezones.isActive[zone_num]=true;
        
        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_EVENT_QUIT) running = false;
        }
        const Uint8* sdlKeys = (Uint8*)SDL_GetKeyboardState(NULL);
        SDL_memcpy(currKeyState, sdlKeys, NUM_KEYS);
        prevMouseState = currMouseState;
        currMouseState = SDL_GetMouseState(&mousex, &mousey);
        
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);

        for (int i = 0; i < MAX_CARDS; i++) {
            if (!inplay.isActive[i]) continue;

            // if the cursor is not dragging anything, the card is not being dragged and the mouse clicks on the card, it will be picked up
            if (!isDragging && !inplay.isDragging[i] && point_box_collision(mousex, mousey, inplay.x[i] - (inplay.w[i] / 2), inplay.y[i] - (inplay.h[i] / 2), inplay.w[i], inplay.h[i]) && (currMouseState & SDL_BUTTON_LMASK) && !(prevMouseState & SDL_BUTTON_LMASK)) {
                inplay.isDragging[i] = true;
                isDragging = true;
            }

            // if the card is being dragged and it is let go of
            if (inplay.isDragging[i] && !(currMouseState & SDL_BUTTON_LMASK)) {

                for (int j = 0; j < MAX_ZONES; j++) {
                    if (!pausezones.isActive[j]) continue;

                    // if the cards is in a zone and the zone is not full, its origin position is set to that
                    if (point_box_collision(inplay.tx[i], inplay.ty[i], pausezones.x[j], pausezones.y[j], pausezones.w[j], pausezones.h[j]) && (pausezones.num_cards[j] < pausezones.max_cards[j])) {

                        for (int l = 0; l < MAX_CARDS; l++) {
                            // if the zone id is the one we move the card out of and is in the slot higher than the original, the card shifts down
                            if (inplay.zoneID[l] == inplay.zoneID[i] && inplay.zoneNum[l] > inplay.zoneNum[i]) {
                                inplay.zoneNum[l] -= 1;
                            }
                        }
                        
                        // subtract num cards for original deck
                        pausezones.num_cards[inplay.zoneID[i]] -= 1;
                        if (j == ZONE_DISCARD) {
                            // if the zone is a discard, it will just remove the card
                            inplay.isActive[i] = false;
                            inplay.zoneID[i] = -1;
                            inplay.num -= 1;
                            
                            printf("discard card\n");
                        } else {
                            // if it isnt a discard zone, it will add the card to that zone
                            // fils up a slot in that zone
                            pausezones.num_cards[j] += 1;
                            inplay.zoneNum[i] = pausezones.num_cards[j];
                            // the cards zone id is set to the zone number
                            inplay.zoneID[i] = j;
                        }
                        break;
                    }
                }

                // puts the card down / stops it from dragging
                inplay.isDragging[i] = false;
                isDragging = false;
            }

            if (inplay.isDragging[i]) {
                // if you are dragging a card it goes to your mouse
                inplay.tx[i] = mousex;
                inplay.ty[i] = mousey;
            } else {
                // if you are not dragging a card it goes back to its original position in its corresponding zone area
                inplay.tx[i] = pausezones.x[inplay.zoneID[i]] + (CARD_WIDTH/2+CARD_SPACING+((inplay.zoneNum[i]-1) * (CARD_WIDTH+CARD_SPACING)) ) * window_scale;
                inplay.ty[i] = pausezones.y[inplay.zoneID[i]] + pausezones.h[inplay.zoneID[i]]/2;
            }

            inplay.vx[i] = (inplay.tx[i] - inplay.x[i]) / LERP_SPEED;
            inplay.vy[i] = (inplay.ty[i] - inplay.y[i]) / LERP_SPEED;
            inplay.y[i] += inplay.vy[i] * dt;
            inplay.x[i] += inplay.vx[i] * dt;

            inplay.w[i] = CARD_WIDTH*window_scale;
            inplay.h[i] = CARD_HEIGHT*window_scale;

            SDL_FRect rect = {inplay.x[i] - (inplay.w[i] / 2), inplay.y[i] - (inplay.h[i] / 2), inplay.w[i], inplay.h[i]};
            SDL_FPoint center = {rect.w / 2, rect.h / 2};
            double angle = inplay.vx[i]/60;

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderRect(renderer, &rect);
            SDL_RenderTextureRotated(renderer, texture, NULL, &rect, angle, &center, SDL_FLIP_NONE);

        }

        // drawing cards from your deck, finding an open card slot
        if (!isDragging && (currMouseState & SDL_BUTTON_LMASK) && !(prevMouseState & SDL_BUTTON_LMASK) && point_box_collision(mousex, mousey, pausezones.x[ZONE_DECK], pausezones.y[ZONE_DECK], pausezones.w[ZONE_DECK], pausezones.h[ZONE_DECK])) {
            if (pausezones.num_cards[ZONE_HAND] < pausezones.max_cards[ZONE_HAND]) draw_cards(1);
        }

        for (int j = 0; j < MAX_ZONES; j++) {
            if (!pausezones.isActive[j]) continue;

            if (j == ZONE_DISCARD) {
                SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            } else {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }
            
            SDL_FRect zone = {pausezones.x[j], pausezones.y[j], pausezones.w[j], pausezones.h[j]};
            SDL_RenderRect(renderer, &zone);
        }

        // render the cursor
        SDL_FRect cursor = {mousex, mousey, 30.0f*window_scale, 30.0f*window_scale};
        SDL_RenderRect(renderer, &cursor);

        // if (inplay.zoneID[0] != -1) {
        //     printf("This card is in zone ID %d\n", inplay.zoneID[0]);
        // }
        // printf("%d\n", inplay.num);
        // printf("Zone 3 cards: %d Card Number: %d\n", pausezones.num_cards[3], cards.zoneNum[0]);

        SDL_RenderPresent(renderer);

        SDL_memcpy(prevKeyState, currKeyState, NUM_KEYS);
        control_fps(120.0f);
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    return 0;
}
