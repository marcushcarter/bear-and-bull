#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_ttf/SDL_ttf.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

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
int seed;

int pick_tix = 0;
int hand_slots = 3;

#define LERP_SPEED 0.25

bool isDragging;

#define MAX_CARDS 99
#define MAX_ZONES 5

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
#define CARD_GROW 0.05
#define CARD_MARGIN 50

typedef struct {
    int ID[MAX_ZONES];

    float x[MAX_ZONES];
    float y[MAX_ZONES];
    float w[MAX_ZONES];
    float h[MAX_ZONES];

    int max_cards[MAX_ZONES];
    int num_cards[MAX_ZONES];

    float isActive[MAX_ZONES];
} Zones; 

Zones playzones;

typedef enum {
    ZONE_DECK,
    ZONE_HAND,
    ZONE_DISCARD,
    ZONE_EQUIP_1,
    ZONE_EQUIP_2,
    ZONE_EQUIP_3,
} ZoneType;

#define TOTAL_CARDS 1000

typedef enum {
    WEAPON_NONE,
    WEAPON_HITSCAN,
    WEAPON_MELEE,
    WEAPON_PROJECTILE,
} WeaponType;

typedef enum {
    EFFECT_BURN,
    EFFECT_FREEZE,
    EFFECT_POISON,
    EFFECT_SHOCK,
} EffectType;

typedef struct {
    SDL_Texture* cardtexture[TOTAL_CARDS];
    SDL_Texture* spritesheet[TOTAL_CARDS];
    char cardpath[TOTAL_CARDS][256];
    char spritepath[TOTAL_CARDS][256];
    char name[TOTAL_CARDS][256];
    char description[TOTAL_CARDS][256];
    WeaponType type[TOTAL_CARDS];

    // stats

    float range[TOTAL_CARDS]; // for ranged
    float min_damage[TOTAL_CARDS];
    float max_damage[TOTAL_CARDS];
    float lifesteal[TOTAL_CARDS];
    float ammo[TOTAL_CARDS];
    float reload[TOTAL_CARDS];

    // passive

    // float speed_up[TOTAL_CARDS];
    // float stamina_up[TOTAL_CARDS];
    // float health_up[TOTAL_CARDS];
    // float luck_up[TOTAL_CARDS];
    // float ammo_up[TOTAL_CARDS];
    // float armor_up[TOTAL_CARDS]; // also called defense
    // float damage_up[TOTAL_CARDS];
    // float heal_up[TOTAL_CARDS]; // health points per second amount over time (heal per second)

    // used card stats

    // float health_use[TOTAL_CARDS];
    // float armor_use[TOTAL_CARDS];
    // float tickets_use[TOTAL_CARDS];

} CardID; CardID cards;

// COMMON FUNCTIONS ----------------------------------------------------------------------------------------------------

bool point_box_collision(float px, float py, float bx, float by, float bw, float bh) {
    return (px >= bx && px <= bx + bw && py >= by && py <= by + bh);
}

float* floatarr(int num, ...) {
	va_list args;
	float* combined_array = (float*)malloc(num * sizeof(float)); // Allocate memory for the integers
	int count = 0;
	va_start(args, num);
	for (int i = 0; i < num; i++) combined_array[count++] = va_arg(args, double);
	va_end(args);

	return combined_array;
}

// DELTA TIME (dt) FUNCTIONS ----------------------------------------------------------------------------------------------------

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

// DRAWING / DISCARDING CARDS FUNCTIONS ----------------------------------------------------------------------------------------------------

void add_card(int id, bool message) {

    if (!(indeck.num+1 < MAX_CARDS)) return; // checks quickly for if there is any space in your deck

    // checks for the first space that is open in your deck

    int index = -1;
    for (int i = 0; i < MAX_CARDS; i++) {
        if (!indeck.isActive[i]) {
            index = i;
            break;
        }
    }

    if (index == -1) return;

    // add the card to that index
    
    indeck.ID[index] = id;

    indeck.x[index] = 0;
    indeck.y[index] = 0;
    indeck.vx[index] = 0;
    indeck.vy[index] = 0;
    indeck.tx[index] = 0;
    indeck.ty[index] = 0;
    indeck.w[index] = CARD_WIDTH;
    indeck.h[index] = CARD_HEIGHT;

    indeck.zoneID[index] = -1;
    indeck.zoneNum[index] = 0;

    indeck.isDragging[index] = false;
    indeck.isActive[index] = true;
    
    indeck.num += 1;
                            
    if (message) printf("added to deck\n");

    return;
}

void discard_card(Cards* cards, int id, bool message) {

    cards->ID[id] = -1;

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
        if (playzones.num_cards[ZONE_HAND] >= playzones.max_cards[ZONE_HAND]) continue; // check quickly to make sure the hand is not full
        if (!pick_tix > 0) continue; // checks if you have any pickup tickets
        if (indeck.num <= 0) continue; // quick check for if there is actually any cards in the deck

        // cehcks if there are any valid cards in your deck

        int validIndex[MAX_CARDS];
        int numValid = 0;
        for (int j = 0; j < MAX_CARDS; j++) {
            if (indeck.isActive[j]) 
                validIndex[numValid++] = j;
        }

        if (numValid == 0) return;

        int randIndex = rand() % numValid; 
        int indexNum = validIndex[randIndex]; // picks a random card out of the valid ones and giveus us the index of that card (indexNum)

        // check if there is any space in the hand

        int inplayIndex = -1;
        for (int k = 0; k < MAX_CARDS; k++) {
            if (!inplay.isActive[k]) {
                inplayIndex = k;
                break;
            }
        }

        if (inplayIndex == -1) return;

        // copy card to that inplayIndex

        inplay.ID[inplayIndex] = indeck.ID[indexNum];
    
        inplay.x[inplayIndex] = (playzones.x[ZONE_DECK]+playzones.w[ZONE_DECK]/2)*window_scale;
        inplay.y[inplayIndex] = (playzones.y[ZONE_DECK]+playzones.h[ZONE_DECK]/2)*window_scale;
        inplay.w[inplayIndex] = CARD_WIDTH;
        inplay.h[inplayIndex] = CARD_HEIGHT;
    
        playzones.num_cards[ZONE_HAND]+=1;
        inplay.zoneID[inplayIndex] = ZONE_HAND;
        inplay.zoneNum[inplayIndex]=playzones.num_cards[ZONE_HAND];
    
        isDragging = true;
        inplay.isDragging[inplayIndex] = true;
        inplay.isActive[inplayIndex]=true;
        
        inplay.num += 1;
        pick_tix-=1;

        if (message) printf("draw card\n");

        discard_card(&indeck, indexNum, false); // delete card at indexNum
    }
}

// ----------------------------------------------------------------------------------------------------

void make_card(int id, const char* cardpath, const char* spritepath, const char* name, const char* description, WeaponType type, float stats[6]) {
    strcpy(cards.cardpath[id], cardpath);
    strcpy(cards.spritepath[id], spritepath);
    strcpy(cards.name[id], name);
    strcpy(cards.description[id], description);

    if (stats != NULL) {
        cards.range[id] = stats[0]; // for ranged
        cards.min_damage[id] = stats[1];
        cards.max_damage[id] = stats[2];
        cards.lifesteal[id] = stats[3];
        cards.ammo[id] = stats[4];
        cards.reload[id] = stats[5];
    }


}

void make_zone(ZoneType num, int slots, int x, int y) {
    playzones.max_cards[num] = slots;
    playzones.x[num]=x*window_scale;
    playzones.y[num]=y*window_scale;
    if (num == ZONE_DECK) slots = 1;
    playzones.w[num]=(CARD_SPACING+(CARD_WIDTH+CARD_SPACING)*slots)*window_scale;
    playzones.h[num]=(CARD_HEIGHT+CARD_SPACING*2)*window_scale;
    playzones.isActive[num]=true;
}

void update_zones() {
    // original prototype
    make_zone(ZONE_DISCARD, 1, (1500-CARD_MARGIN-CARD_WIDTH-CARD_SPACING), CARD_MARGIN);
    make_zone(ZONE_HAND, 9, CARD_MARGIN, (1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN));
    make_zone(ZONE_EQUIP_1, 1, (1500-CARD_MARGIN-CARD_WIDTH-CARD_SPACING-CARD_MARGIN-CARD_WIDTH-CARD_SPACING), CARD_MARGIN);
    make_zone(ZONE_DECK, 0, CARD_MARGIN, CARD_MARGIN);

    // new version
    // make_zone(ZONE_DISCARD, 1, (1500-CARD_MARGIN-CARD_WIDTH-CARD_SPACING), CARD_MARGIN);
    // make_zone(ZONE_HAND, 5, CARD_MARGIN, (1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN));
    // make_zone(ZONE_EQUIP_1, 1, (1500-CARD_MARGIN-CARD_WIDTH-CARD_SPACING-CARD_MARGIN-CARD_WIDTH-CARD_SPACING), CARD_MARGIN);
    // make_zone(ZONE_DECK, 0, CARD_MARGIN, 1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN);
    // make_zone(ZONE_HAND, hand_slots, CARD_MARGIN+CARD_SPACING+CARD_WIDTH+CARD_MARGIN, 1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN);
    // make_zone(ZONE_DISCARD, 1, 1500-CARD_MARGIN-CARD_SPACING-CARD_WIDTH, 1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN);
}

bool load_textures() {

    for (int i = 0; i < TOTAL_CARDS; i++) {
        cards.cardtexture[i] = IMG_LoadTexture(renderer, cards.cardpath[i]);
        SDL_SetTextureScaleMode(cards.cardtexture[i], SDL_SCALEMODE_NEAREST);
        cards.spritesheet[i] = IMG_LoadTexture(renderer, cards.spritepath[i]);
        SDL_SetTextureScaleMode(cards.spritesheet[i], SDL_SCALEMODE_NEAREST);
    }

	return true;
}

void update_window() {
    // get window sizes and set window scaling coefficient 
    SDL_HideCursor();
    SDL_GetWindowSize(window, &window_width, &window_height);  
    window_scale = window_width/1500.0f;
}

// LOOP FUNCTIONS ----------------------------------------------------------------------------------------------------

void setup() {
    // LOAD CARDS

    make_card(0, "./resources/textures/card1.png", "./resources/textures/card1.png", "card 1", "card of the number of one", WEAPON_NONE, NULL);
    make_card(1, "./resources/textures/card2.jpg", "./resources/textures/card2.jpg", "card 2", "card of the number of two", WEAPON_MELEE, floatarr(6, 0, 1, 2, 3, 4, 6));
    make_card(2, "./resources/textures/card3.png", "./resources/textures/card3.png", "card 3", "card of the number of three", WEAPON_HITSCAN, floatarr(6, 10, 3, 9, 8, 6, 1));
    make_card(3, "./resources/textures/card4.png", "./resources/textures/card4.png", "card 4", "card of the number of four", WEAPON_PROJECTILE, floatarr(6, 5, 4, 3, 2, 1, 1));
    make_card(4, "./resources/textures/card5.png", "./resources/textures/card5.png", "card 5", "card of the number of five", WEAPON_NONE, NULL);
    make_card(5, "./resources/textures/card6.png", "./resources/textures/card6.png", "card 6", "card of the number of six", WEAPON_NONE, NULL);
    make_card(6, "./resources/textures/card7.png", "./resources/textures/card7.png", "card 7", "card of the number of seven", WEAPON_NONE, NULL);
    make_card(7, "./resources/textures/card8.png", "./resources/textures/card8.png", "card 8", "card of the number of eight", WEAPON_NONE, NULL);
    make_card(8, "./resources/textures/card9.png", "./resources/textures/card9.png", "card 9", "card of the number of nine", WEAPON_NONE, NULL);
    make_card(9, "./resources/textures/card10.png", "./resources/textures/card10.png", "card 10", "card of the number of ten", WEAPON_NONE, NULL);

    update_window();
    update_zones();

    const Uint8* sdlKeys = (Uint8*)SDL_GetKeyboardState(NULL);
    SDL_memcpy(currKeyState, sdlKeys, NUM_KEYS);
    SDL_memcpy(prevKeyState, sdlKeys, NUM_KEYS);

    for (int i = 0; i < MAX_CARDS; i++) { discard_card(&inplay, i, false); inplay.num = 0; }
    for (int i = 0; i < MAX_CARDS; i++) { discard_card(&indeck, i, false); indeck.num = 0; }

    for (int i = 0; i < MAX_ZONES; i++) {
        playzones.isActive[i] = false;
        playzones.num_cards[i] = 0;
        playzones.max_cards[i]=0;
    }

    for (int i = 0; i < 54; i++) { add_card(rand() % 10, false); }

    // pick_tix = 10;
    pick_tix = 1000;

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

    update_window();
    update_zones();
    
    // update card locations
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
                if (!playzones.isActive[j]) continue;

                // if you put the card into a zone (and it is not full)
                if (point_box_collision(inplay.tx[i], inplay.ty[i], playzones.x[j], playzones.y[j], playzones.w[j], playzones.h[j]) && (playzones.num_cards[j] < playzones.max_cards[j])) {

                    // in the zone the card just left, every card after that one shifts down one space
                    for (int l = 0; l < MAX_CARDS; l++) { if (inplay.zoneID[l] == inplay.zoneID[i] && inplay.zoneNum[l] > inplay.zoneNum[i]) inplay.zoneNum[l] -= 1; }
                    
                    // if the zone is a discard pile, we delete the card
                    // if it is not, we put the card in that zone
                    playzones.num_cards[inplay.zoneID[i]] -= 1;
                    if (j == ZONE_DISCARD) {
                        add_card(inplay.ID[i], true);
                        discard_card(&inplay, i, true);
                    } else {
                        playzones.num_cards[j] += 1;
                        inplay.zoneNum[i] = playzones.num_cards[j];
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
        // if it is in a multi card zone, the cards will fan
        if (inplay.isDragging[i]) {
            inplay.tx[i] = mousex;
            inplay.ty[i] = mousey;
        } else {
            float fanning = playzones.w[inplay.zoneID[i]] / 2 - ((CARD_WIDTH + CARD_SPACING) / 2.0f) * window_scale * playzones.num_cards[inplay.zoneID[i]] - 5*window_scale;
            inplay.tx[i] = playzones.x[inplay.zoneID[i]] + (CARD_WIDTH/2 + CARD_SPACING + ((inplay.zoneNum[i] - 1) * (CARD_WIDTH + CARD_SPACING)) )*window_scale + fanning;
            inplay.ty[i] = playzones.y[inplay.zoneID[i]] + playzones.h[inplay.zoneID[i]]/2;
        }

        inplay.vx[i] = (inplay.tx[i] - inplay.x[i]) / LERP_SPEED;
        inplay.vy[i] = (inplay.ty[i] - inplay.y[i]) / LERP_SPEED;
        inplay.y[i] += inplay.vy[i] * dt;
        inplay.x[i] += inplay.vx[i] * dt;

        inplay.w[i] = CARD_WIDTH*window_scale;
        inplay.h[i] = CARD_HEIGHT*window_scale;

    }

    // if you click on the deck, it will draw a card
    if (!isDragging && (currMouseState & SDL_BUTTON_LMASK) && !(prevMouseState & SDL_BUTTON_LMASK) && point_box_collision(mousex, mousey, playzones.x[ZONE_DECK], playzones.y[ZONE_DECK], playzones.w[ZONE_DECK], playzones.h[ZONE_DECK])) {
        if (playzones.num_cards[ZONE_HAND] < playzones.max_cards[ZONE_HAND]) draw_cards(1, true);
    }

    if (currKeyState[SDL_SCANCODE_1] && !prevKeyState[SDL_SCANCODE_1]) {
        hand_slots+=1;
        if (hand_slots > 6) hand_slots = 6;
    }
}

void render() {

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);

    // cards
    for (int i = 0; i < MAX_CARDS; i++) {
        if (!inplay.isActive[i]) continue;

        float card_grow_w = inplay.isDragging[i]*CARD_GROW*inplay.w[i]*window_scale;
        float card_grow_h = inplay.isDragging[i]*CARD_GROW*inplay.h[i]*window_scale;

        float sine = 0, fanning = 0;
        if (!inplay.isDragging[i]) sine = 5 *  sin(2.0*((SDL_GetTicks() / 1000.0f) + (inplay.ID[i]*15)));
        if (!inplay.isDragging[i]) {
            sine = 5 *  sin(2.0*((SDL_GetTicks() / 1000.0f) + (inplay.ID[i]*15)));
            fanning = (inplay.zoneNum[i] - ((playzones.num_cards[inplay.zoneID[i]] + 1) / 2.0f)) * 2.5f;
        }

        SDL_FRect cardhitbox = {inplay.x[i] - (inplay.w[i] / 2), inplay.y[i] - (inplay.h[i] / 2), inplay.w[i], inplay.h[i]};
        SDL_FRect card = {inplay.x[i] - (inplay.w[i] / 2)  - (card_grow_w/2), inplay.y[i] - (inplay.h[i] / 2)  - (card_grow_h/2) - sine*window_scale, inplay.w[i] + card_grow_w, inplay.h[i] + card_grow_h};
        SDL_FPoint center = {card.w / 2, card.h / 2};
        if (!inplay.isDragging[i]) sine = 5 *  sin(1*((SDL_GetTicks() / 1000.0f) + (inplay.ID[i]*15)));
        double angle = inplay.vx[i]/60 + sine/2 + fanning;

        SDL_RenderTextureRotated(renderer, cards.cardtexture[inplay.ID[i]], NULL, &card, angle, &center, SDL_FLIP_NONE);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        if (currKeyState[SDL_SCANCODE_SPACE]) SDL_RenderRect(renderer, &cardhitbox);

    }

    // zones
    for (int j = 0; j < MAX_ZONES; j++) {
        if (!playzones.isActive[j]) continue;

        if (j == ZONE_DISCARD) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        } else {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        }
        
        SDL_FRect zone = {playzones.x[j], playzones.y[j], playzones.w[j], playzones.h[j]};
        SDL_RenderRect(renderer, &zone);
    }

    // cursor
    SDL_FRect cursor = {mousex, mousey, 30.0f*window_scale, 30.0f*window_scale};
    SDL_RenderRect(renderer, &cursor);

    SDL_RenderPresent(renderer);
}

void debug() {

    // if (inplay.zoneID[0] != -1) {
    //     printf("This card is in zone ID %d\n", inplay.zoneID[0]);
    // }
    // printf("%d\n", inplay.num);
    // printf("Zone 3 cards: %d Card Number: %d\n", playzones.num_cards[3], cards.zoneNum[0]);
    // if (indeck.num > -1) printf("%d\n", indeck.ID[0]);
    // printf("%d\n", inplay.ID[0]);

    // for (int i = 0; i < MAX_CARDS; i++) {
    //     if (inplay.zoneID[i] == ZONE_HAND) {
    //         printf("%d\n", inplay.ID[i]);
    //         // break;
    //     }
    // }
}

// MAIN FUNCTION ----------------------------------------------------------------------------------------------------

int main() {
    
    // set the seed for the game
    // seed = time(NULL);
    seed = 0;
    srand(seed);

    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow("Retro FPS Deckbuilder", 1500, 1000, SDL_WINDOW_RESIZABLE);
    renderer = SDL_CreateRenderer(window, NULL);
	SDL_SetRenderScale(renderer, 1, 1);
	SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    setup();
    load_textures();

    running = 1;
    while (running) {

        dt = get_delta_time()*game_speed;

        inputs();
        update();
        render();
        debug();

        SDL_memcpy(prevKeyState, currKeyState, NUM_KEYS);
        control_fps(120.0f);
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    return 0;
}
