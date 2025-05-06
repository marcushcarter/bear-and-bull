#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
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

int window_width, window_height;
float window_scale = 1.0f;
float game_speed = 2;

#define LERP_SPEED 0.25

bool isDragging;

#define MAX_CARDS 20
#define MAX_ZONES 20

typedef struct {
    int ID[MAX_CARDS];
    char name[MAX_CARDS][256];
    char description[MAX_CARDS][256];

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
    ZONE_DECK,
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

void add_card(Cards* cards, int id, bool message) {}

void discard_card(Cards* cards, int id, bool message) {
    
    cards->ID[id] = -1;
    strcpy(cards->name[id], "");
    strcpy(cards->description[id], "");

    cards->x[id] = 0;
    cards->y[id] = 0;
    cards->vx[id] = 0;
    cards->vy[id] = 0;
    cards->tx[id] = 0;
    cards->ty[id] = 0;
    cards->w[id] = CARD_WIDTH;
    cards->h[id] = CARD_HEIGHT;

    cards->zoneID[id] = -1;
    cards->zoneNum[id] = 0;

    cards->isDragging[id] = false;
    cards->isActive[id] = false;
    
    cards->num -= 1;
                            
    if (message) printf("discard card\n");

    return;
}

void draw_cards(int num, bool message) {
    for (int i = 0; i < num; i++) {
        if (pausezones.num_cards[ZONE_HAND] >= pausezones.max_cards[ZONE_HAND]) continue;
        if (indeck.num <= 0) continue;
        for (int j = 0; j < MAX_CARDS; j++) {
            if (inplay.isActive[j]) continue;

            int randNum = rand() % indeck.num;
            // ^^^ this has to find a valid card in the deck

            randNum = 0;

            // set inplay card to random deck card

            inplay.ID[j] = 1;
            // strcpy(inplay.name[j], indeck.name[randNum]);
            // strcpy(inplay.description[j], indeck.description[randNum]);
        
            inplay.x[j] = (pausezones.x[ZONE_DECK]+pausezones.w[ZONE_DECK]/2)*window_scale;
            inplay.y[j] = (pausezones.y[ZONE_DECK]+pausezones.h[ZONE_DECK]/2)*window_scale;
            inplay.vx[j] = 0;
            inplay.vy[j] = 0;
            inplay.tx[j] = 0;
            inplay.ty[j] = 0;
            inplay.w[j] = CARD_WIDTH;
            inplay.h[j] = CARD_HEIGHT;
        
            pausezones.num_cards[ZONE_HAND]+=1;
            inplay.zoneID[j] = ZONE_HAND;
            inplay.zoneNum[j]=pausezones.num_cards[ZONE_HAND];
        
            isDragging = true;
            inplay.isDragging[j] = true;
            inplay.isActive[j]=true;
            
            inplay.num += 1;


            // delete the deck card

            discard_card(&indeck, randNum, true);



            // indeck.num -= 1;
            // slide the deck cards down

            if (message) printf("draw card\n");
            
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
    for (int i = 0; i < MAX_CARDS; i++) { discard_card(&inplay, i, false); }

    for (int i = 0; i < MAX_ZONES; i++) {
        pausezones.isActive[i] = false;
        pausezones.num_cards[i] = 0;
        pausezones.max_cards[i]=0;
    }

    indeck.num = 5;

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
        pausezones.x[zone_num]=(1500-((CARD_WIDTH+CARD_SPACING*2)+50)*1)*window_scale;
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
        pausezones.y[zone_num]=(1000-((CARD_HEIGHT+CARD_SPACING*2)+50)*1)*window_scale;
        pausezones.w[zone_num]=(CARD_SPACING+(CARD_WIDTH+CARD_SPACING)*pausezones.max_cards[zone_num])*window_scale;
        pausezones.h[zone_num]=(CARD_HEIGHT+CARD_SPACING*2)*window_scale;
        pausezones.isActive[zone_num]=true;

        zone_num = ZONE_EQUIP_1;
        strcpy(pausezones.name[zone_num], "equip slot");
        pausezones.max_cards[zone_num] = 1;
        pausezones.x[zone_num]=(1500-((CARD_WIDTH+CARD_SPACING*2)+50)*2)*window_scale;
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

            // if the card is being dragged and it is let go of (putting the card down)
            if (inplay.isDragging[i] && !(currMouseState & SDL_BUTTON_LMASK)) {

                for (int j = 0; j < MAX_ZONES; j++) {
                    if (!pausezones.isActive[j]) continue;

                    // if you put the card into a zone (and it is not full)
                    if (point_box_collision(inplay.tx[i], inplay.ty[i], pausezones.x[j], pausezones.y[j], pausezones.w[j], pausezones.h[j]) && (pausezones.num_cards[j] < pausezones.max_cards[j])) {

                        // in the zone the card just left, every card after that one shifts down one space
                        for (int l = 0; l < MAX_CARDS; l++) { if (inplay.zoneID[l] == inplay.zoneID[i] && inplay.zoneNum[l] > inplay.zoneNum[i]) inplay.zoneNum[l] -= 1; }
                        
                        // if the zone is a discard pile, we delete the card
                        // if it is not, we put the card in that zone
                        pausezones.num_cards[inplay.zoneID[i]] -= 1;
                        if (j == ZONE_DISCARD) {
                            discard_card(&inplay, i, true);
                        } else {
                            pausezones.num_cards[j] += 1;
                            inplay.zoneNum[i] = pausezones.num_cards[j];
                            inplay.zoneID[i] = j;
                        }

                        break;
                    }
                }

                inplay.isDragging[i] = false;
                isDragging = false;
            }

            // if you are dragging a card it goes to your mouse
            // if you are not dragging it, it goes back to the zone id it belongs to (with spacing / no overlap)
            if (inplay.isDragging[i]) {
                inplay.tx[i] = mousex;
                inplay.ty[i] = mousey;
            } else {
                inplay.tx[i] = pausezones.x[inplay.zoneID[i]] + (CARD_WIDTH/2+CARD_SPACING+((inplay.zoneNum[i]-1) * (CARD_WIDTH+CARD_SPACING)) ) * window_scale;
                inplay.ty[i] = pausezones.y[inplay.zoneID[i]] + pausezones.h[inplay.zoneID[i]]/2;
            }

            inplay.vx[i] = (inplay.tx[i] - inplay.x[i]) / LERP_SPEED;
            inplay.vy[i] = (inplay.ty[i] - inplay.y[i]) / LERP_SPEED;
            inplay.y[i] += inplay.vy[i] * dt;
            inplay.x[i] += inplay.vx[i] * dt;

            inplay.w[i] = CARD_WIDTH*window_scale;
            inplay.h[i] = CARD_HEIGHT*window_scale;

        }

        for (int i = 0; i < MAX_CARDS; i++) {
            if (!inplay.isActive[i]) continue;

            SDL_FRect rect = {inplay.x[i] - (inplay.w[i] / 2), inplay.y[i] - (inplay.h[i] / 2), inplay.w[i], inplay.h[i]};
            SDL_FPoint center = {rect.w / 2, rect.h / 2};
            double angle = inplay.vx[i]/60;

            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderRect(renderer, &rect);
            SDL_RenderTextureRotated(renderer, texture, NULL, &rect, angle, &center, SDL_FLIP_NONE);
        }

        // if you click on the deck, it will draw a card
        if (!isDragging && (currMouseState & SDL_BUTTON_LMASK) && !(prevMouseState & SDL_BUTTON_LMASK) && point_box_collision(mousex, mousey, pausezones.x[ZONE_DECK], pausezones.y[ZONE_DECK], pausezones.w[ZONE_DECK], pausezones.h[ZONE_DECK])) {
            if (pausezones.num_cards[ZONE_HAND] < pausezones.max_cards[ZONE_HAND]) draw_cards(1, true);
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
        // if (indeck.num > -1) printf("%d\n", indeck.ID[0]);

        SDL_RenderPresent(renderer);

        SDL_memcpy(prevKeyState, currKeyState, NUM_KEYS);
        control_fps(120.0f);
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    return 0;
}
