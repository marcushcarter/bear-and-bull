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
bool isDragging;

int window_width, window_height;

float window_scale_x = 1.0f;
float window_scale_y = 1.0f;

int seed;
float game_speed = 2;
int hand_slots = 3;

bool show_hitboxes = true;
bool show_textures = true;
bool custom_cursor = false;

#define LERP_SPEED 0.25

#define MAX_CARDS 10
#define MAX_ZONES 10

#define CARD_HEIGHT 190
#define CARD_WIDTH 142
#define CARD_SPACING 10
#define CARD_GROW 0.05
#define CARD_MARGIN 50

#define TOTAL_CARDS 10

typedef struct Cards {
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
    float zoneTime[MAX_CARDS];
    
    bool isDragging[MAX_CARDS];
    bool isActive[MAX_CARDS];
    bool isSellable[MAX_CARDS];

    int num;
} Cards;

typedef struct Zones {
    int ID[MAX_ZONES];

    float x[MAX_ZONES];
    float y[MAX_ZONES];
    float w[MAX_ZONES];
    float h[MAX_ZONES];

    int max_cards[MAX_ZONES];
    int num_cards[MAX_ZONES];

    float isActive[MAX_ZONES];
} Zones; 

typedef enum ZoneType {
    ZONE_DECK,
    ZONE_HAND,
    ZONE_DISCARD,
    ZONE_EQUIP_1,
    ZONE_EQUIP_2,
    ZONE_EQUIP_3,
    ZONE_EVENT,
} ZoneType;

typedef struct CardID{
    SDL_Texture* cardtexture[TOTAL_CARDS];
    char cardpath[TOTAL_CARDS][256];
    char name[TOTAL_CARDS][256];
    char description[TOTAL_CARDS][256];

    // float money_mult[TOTAL_CARDS]; // added form each card to a base of 1 and that is multiplied by the money at the end of the round
    // float capital[TOTAL_CARDS]; // this extra money added at the end of every round
    // float luck[TOTAL_CARDS];
    // float passive[TOTAL_CARDS]; // how many seconds to earn 1 coin passively
    // float price[TOTAL_CARDS]; // card can be bought for this price and sold for half of it
    // float speed[TOTAL_CARDS]; // higher your speed stat, the slower the 60 second round goes
    // float market_stability[TOTAL_CARDS]; //

    // GENERAL

    float price[TOTAL_CARDS]; // price of the card
    float passive[TOTAL_CARDS]; // (int) money earned at the end of every round
    float mult[TOTAL_CARDS]; // added to 1 and that number is multiplied by the total money earned at the end of the round (before the passive income addition)

    // IN ROUND STOCKS

    float stock_cost[TOTAL_CARDS];

    // SPECIFIC

    // float range[TOTAL_CARDS]; // for ranged
    // float min_damage[TOTAL_CARDS];
    // float max_damage[TOTAL_CARDS];
    // float lifesteal[TOTAL_CARDS];
    // float ammo[TOTAL_CARDS];
    // float reload[TOTAL_CARDS];

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

} CardID; 

Cards inplay;
Cards indeck;

Zones playzones;
Zones eventzone;

CardID cards;

// COMMON FUNCTIONS ====================================================================================================

bool point_box_collision(float px, float py, float bx, float by, float bw, float bh) { return (px >= bx && px <= bx + bw && py >= by && py <= by + bh); }

float* floatarr(int num, ...) {
	va_list args;
	float* combined_array = (float*)malloc(num * sizeof(float));
	int count = 0;
	va_start(args, num);
	for (int i = 0; i < num; i++) combined_array[count++] = va_arg(args, double);
	va_end(args);

	return combined_array;
}

// DELTA TIME (dt) FUNCTIONS ====================================================================================================

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

// DRAWING / DISCARDING CARDS FUNCTIONS ====================================================================================================

void add_card(int id, bool message) {
    // checks if there is any space in your deck
    if (!(indeck.num+1 < MAX_CARDS)) return;

    // finds the first open space in your deck
    // ---------------------------------------
    int index = -1;
    for (int i = 0; i < MAX_CARDS; i++) {
        if (!indeck.isActive[i]) {
            index = i;
            break;
        }
    }
    if (index == -1) return;

    // adds the card at that index
    // ---------------------------
    
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
    indeck.isSellable[index] = true;
    
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
    cards->isSellable[id] = true;
    
    cards->num -= 1;
                            
    if (message) printf("discard card\n");

    return;
}

void draw_cards(int num, bool message) {
    for (int i = 0; i < num; i++) {
        // checks if there is any space in your hand
        if (playzones.num_cards[ZONE_HAND] >= playzones.max_cards[ZONE_HAND]) continue;
        // checks if there are any valid cards in your deck
        if (indeck.num <= 0) continue;

        // Finds all the valid cards in your deck
        // --------------------------------------
        int validIndex[MAX_CARDS];
        int numValid = 0;
        for (int j = 0; j < MAX_CARDS; j++) {
            if (indeck.isActive[j]) 
                validIndex[numValid++] = j;
        }
        if (numValid == 0) return;

        // Picks a random one out of those valid cards
        // -------------------------------------------
        int randIndex = rand() % numValid; 
        int indexNum = validIndex[randIndex];

        // finds the first empty space in your hand
        // ----------------------------------------
        int inplayIndex = -1;
        for (int k = 0; k < MAX_CARDS; k++) {
            if (!inplay.isActive[k]) {
                inplayIndex = k;
                break;
            }
        }
        if (inplayIndex == -1) return;

        // copies that random valid card to that empty space in your hand
        // ------------------------------------------------------------

        inplay.ID[inplayIndex] = indeck.ID[indexNum];
    
        inplay.x[inplayIndex] = (playzones.x[ZONE_DECK]+playzones.w[ZONE_DECK]/2)*window_scale_x;
        inplay.y[inplayIndex] = (playzones.y[ZONE_DECK]+playzones.h[ZONE_DECK]/2)*window_scale_y;
        inplay.w[inplayIndex] = CARD_WIDTH;
        inplay.h[inplayIndex] = CARD_HEIGHT;
    
        playzones.num_cards[ZONE_HAND]+=1;
        inplay.zoneID[inplayIndex] = ZONE_HAND;
        inplay.zoneNum[inplayIndex]=playzones.num_cards[ZONE_HAND];
        inplay.zoneTime[inplayIndex] = (SDL_GetTicks()/1000.0f);
    
        isDragging = true;
        inplay.isDragging[inplayIndex] = true;
        inplay.isActive[inplayIndex] = true;
        inplay.isSellable[inplayIndex] = true;
        
        inplay.num += 1;

        if (message) printf("draw card\n");

        // removes the card from your deck
        discard_card(&indeck, indexNum, false);
    }
}

void draw_card_id(ZoneType ZONE, int id, bool message) {

    // checks if there any space in the targetted zone
    if (playzones.num_cards[ZONE] >= playzones.max_cards[ZONE]) return;

    // finds the first empty space in your hand
    // ----------------------------------------
    int inplayIndex = -1;
    for (int k = 0; k < MAX_CARDS; k++) {
        if (!inplay.isActive[k]) {
            inplayIndex = k;
            break;
        }
    }
    if (inplayIndex == -1) return;

    // adds the card with the ID to that zone
    // ------------------------------------------------------------

    inplay.ID[inplayIndex] = id;

    if (ZONE == ZONE_EVENT) {
        inplay.x[inplayIndex] = (playzones.x[ZONE]+playzones.w[ZONE]/2)*window_scale_x;
        inplay.y[inplayIndex] = ((playzones.y[ZONE]+playzones.h[ZONE]/2)+500)*window_scale_y;
    } else {
        inplay.x[inplayIndex] = (playzones.x[ZONE_DECK]+playzones.w[ZONE_DECK]/2)*window_scale_x;
        inplay.y[inplayIndex] = ((playzones.y[ZONE_DECK]+playzones.h[ZONE_DECK]/2)+500)*window_scale_y;
    }
    
    inplay.w[inplayIndex] = CARD_WIDTH;
    inplay.h[inplayIndex] = CARD_HEIGHT;

    playzones.num_cards[ZONE]+=1;
    inplay.zoneID[inplayIndex] = ZONE;
    inplay.zoneNum[inplayIndex]=playzones.num_cards[ZONE];
    inplay.zoneTime[inplayIndex] = (SDL_GetTicks()/1000.0f);

    inplay.isDragging[inplayIndex] = false;
    inplay.isActive[inplayIndex] = true;
    inplay.isSellable[inplayIndex] = false;
    
    inplay.num += 1;

    if (message) printf("draw card\n");
}

void shuffle_hand(bool message) {
    // moves card from your hand to the deck
    // -------------------------------------
    for (int i = 0; i < MAX_CARDS; i++) {
        if (inplay.isActive[i] == false) continue;

        if (inplay.isDragging[i]) isDragging = false;
        playzones.num_cards[inplay.zoneID[i]] -= 1;
        add_card(inplay.ID[i], false);
        discard_card(&inplay, i, false);
    }

    if (message) printf("shuffled hand into deck\n");
}

// SETUP FUNCTIONS ====================================================================================================

void make_card(int id, const char* cardpath, const char* name, const char* description, float stats[6]) {

    // set string data
    // ---------------
    strcpy(cards.cardpath[id], cardpath);
    strcpy(cards.name[id], name);
    strcpy(cards.description[id], description);

    // set stat data
    // -------------

    // if (stats != NULL) {
    //     cards.range[id] = stats[0]; // for ranged
    //     cards.min_damage[id] = stats[1];
    //     cards.max_damage[id] = stats[2];
    //     cards.lifesteal[id] = stats[3];
    //     cards.ammo[id] = stats[4];
    //     cards.reload[id] = stats[5];
    // }

}

void make_zone(ZoneType zone, int slots, int x, int y, int w, int h) {
    playzones.max_cards[zone] = slots;
    playzones.x[zone]=x*window_scale_x;
    playzones.y[zone]=y*window_scale_y;
    if (zone == ZONE_DECK) slots = 1;
    if (w != 0) {
        playzones.w[zone] = w*window_scale_x;
    } else {
        playzones.w[zone]=(CARD_SPACING+(CARD_WIDTH+CARD_SPACING)*slots)*window_scale_x;
    }

    if (h != 0) {
        playzones.h[zone] = h*window_scale_y;
    } else {
        playzones.h[zone]=(CARD_HEIGHT+CARD_SPACING*2)*window_scale_y;
    }
    
    playzones.isActive[zone]=true;
}

void update_zones() {
    // original prototype
    // make_zone(ZONE_DISCARD, 1, (1500-CARD_MARGIN-CARD_WIDTH-CARD_SPACING), CARD_MARGIN);
    // make_zone(ZONE_HAND, 9, CARD_MARGIN, (1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN));
    // make_zone(ZONE_EQUIP_1, 1, (1500-CARD_MARGIN-CARD_WIDTH-CARD_SPACING-CARD_MARGIN-CARD_WIDTH-CARD_SPACING), CARD_MARGIN);
    // make_zone(ZONE_DECK, 0, CARD_MARGIN, CARD_MARGIN);

    // new version
    // make_zone(ZONE_DECK, 0, CARD_MARGIN, 1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN);
    // make_zone(ZONE_HAND, hand_slots, CARD_MARGIN+CARD_SPACING+CARD_WIDTH+CARD_MARGIN, 1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN);
    // make_zone(ZONE_DISCARD, 1, 1500-CARD_MARGIN-CARD_SPACING-CARD_WIDTH, 1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN);
    // make_zone(ZONE_EQUIP_1, 1, CARD_MARGIN, CARD_MARGIN);
    // make_zone(ZONE_EQUIP_2, 1, CARD_MARGIN+CARD_WIDTH+CARD_SPACING+CARD_MARGIN, CARD_MARGIN);

    // Planned version
    make_zone(ZONE_DECK, 0, CARD_MARGIN, 1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, 0, 0);
    make_zone(ZONE_HAND, hand_slots, CARD_MARGIN, CARD_MARGIN, 0, 0);
    make_zone(ZONE_DISCARD, 1, 1500-CARD_MARGIN-CARD_SPACING-CARD_WIDTH, CARD_MARGIN, 0, 0);
    make_zone(ZONE_EQUIP_1, 1, 1500-CARD_MARGIN-CARD_SPACING-CARD_WIDTH, 1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, 0, 0);
    make_zone(ZONE_EVENT, 1, 1500-CARD_MARGIN-CARD_SPACING-CARD_WIDTH-CARD_MARGIN-CARD_WIDTH, 1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, 0, 0);
}

SDL_Texture* deck;
bool load_textures() {

    for (int i = 0; i < TOTAL_CARDS; i++) {
        cards.cardtexture[i] = IMG_LoadTexture(renderer, cards.cardpath[i]);
        SDL_SetTextureScaleMode(cards.cardtexture[i], SDL_SCALEMODE_NEAREST);
    }

    deck = IMG_LoadTexture(renderer, "./resources/textures/deck.png");
    SDL_SetTextureScaleMode(deck, SDL_SCALEMODE_NEAREST);

	return true;
}

void update_window() {
    // get window sizes and set window scaling x/y coefficients
    SDL_GetWindowSize(window, &window_width, &window_height);
    window_scale_x = window_width/1500.0f;  
    window_scale_y = window_height/1000.0f;
}

void setup() {

    // LOAD CARDS
    // ----------
    make_card(0, "./resources/textures/card1.png", "card 1", "card of the number of one", NULL);
    make_card(1, "./resources/textures/card2.png", "card 2", "card of the number of two", floatarr(6, 0, 1, 2, 3, 4, 6));
    make_card(2, "./resources/textures/card3.png", "card 3", "card of the number of three", floatarr(6, 10, 3, 9, 8, 6, 1));
    make_card(3, "./resources/textures/card4.png", "card 4", "card of the number of four", floatarr(6, 5, 4, 3, 2, 1, 1));
    make_card(4, "./resources/textures/card5.png", "card 5", "card of the number of five", NULL);
    make_card(5, "./resources/textures/card6.png", "card 6", "card of the number of six", NULL);
    make_card(6, "./resources/textures/card7.png", "card 7", "card of the number of seven", NULL);
    make_card(7, "./resources/textures/card8.png", "card 8", "card of the number of eight", NULL);
    make_card(8, "./resources/textures/card9.png", "card 9", "card of the number of nine", NULL);
    make_card(9, "./resources/textures/card10.png", "card 10", "card of the number of ten", NULL);

    update_window();
    update_zones();

    const Uint8* sdlKeys = (Uint8*)SDL_GetKeyboardState(NULL);
    SDL_memcpy(currKeyState, sdlKeys, NUM_KEYS);
    SDL_memcpy(prevKeyState, sdlKeys, NUM_KEYS);

    // SETUP CARDS AND ZONES
    // ---------------------
    for (int i = 0; i < MAX_CARDS; i++) { discard_card(&inplay, i, false); inplay.num = 0; }
    for (int i = 0; i < MAX_CARDS; i++) { discard_card(&indeck, i, false); indeck.num = 0; }
    for (int i = 0; i < MAX_ZONES; i++) {
        playzones.isActive[i] = false;
        playzones.num_cards[i] = 0;
        playzones.max_cards[i]=0;
    }

    // ADD CARDS TO DECK
    // ---------------------
    for (int i = 0; i < 10; i++) { add_card(rand() % TOTAL_CARDS, false); }

}

// LOOP FUNCTIONS ====================================================================================================

void inputs() {
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_EVENT_QUIT) running = false;
    }
    const Uint8* sdlKeys = (Uint8*)SDL_GetKeyboardState(NULL);
    SDL_memcpy(currKeyState, sdlKeys, NUM_KEYS);
    prevMouseState = currMouseState;
    currMouseState = SDL_GetMouseState(&mousex, &mousey);
}

void dev_tools() {

    // 1 - toggle hitboxes
    // -------------------
    if (currKeyState[SDL_SCANCODE_1] && !prevKeyState[SDL_SCANCODE_1]){
        if (show_hitboxes) {
            show_hitboxes = false;
            show_textures = true;
        } else {
            show_hitboxes = true;
            show_textures = false;
        }
    }

    // 2 - toggle custom cursor
    // ------------------------
    if (currKeyState[SDL_SCANCODE_2] && !prevKeyState[SDL_SCANCODE_2]) {
        custom_cursor = !custom_cursor;
    }

    // 3 - increase hand slots
    // -----------------------
    if (currKeyState[SDL_SCANCODE_3] && !prevKeyState[SDL_SCANCODE_3]) {
        hand_slots+=1;
        if (hand_slots > 8) hand_slots = 8;
    }

    // 4 - shuffle hand into deck
    // --------------------------
    if (currKeyState[SDL_SCANCODE_4] && !prevKeyState[SDL_SCANCODE_4]) {
        shuffle_hand(true);
    }

    // 5 - spawn event card
    // --------------------
    if (currKeyState[SDL_SCANCODE_5] && !prevKeyState[SDL_SCANCODE_5]) {
        draw_card_id(ZONE_HAND, rand() % TOTAL_CARDS, false);
    }

}

void update() {

    update_window();
    update_zones();

    // Draws an event card once every 30 seconds
    // ------------------------------------
    int ticks = (int)(SDL_GetTicks() / 100.0f) % 300;
    if (ticks == 300-1) {draw_card_id(ZONE_EVENT, 0, false);}

    // Checks if you want to draw a card
    // ---------------------------------
    if (!isDragging && (currMouseState & SDL_BUTTON_LMASK) && !(prevMouseState & SDL_BUTTON_LMASK) && point_box_collision(mousex, mousey, playzones.x[ZONE_DECK], playzones.y[ZONE_DECK], playzones.w[ZONE_DECK], playzones.h[ZONE_DECK])) {
        if (playzones.num_cards[ZONE_HAND] < playzones.max_cards[ZONE_HAND]) draw_cards(1, true);
    }

    // CARD UPDATES
    // ------------
    for (int i = 0; i < MAX_CARDS; i++) {
        if (!inplay.isActive[i]) continue;

        // Checks if a card should be picked up
        // ------------------------------------
        if (!isDragging && !inplay.isDragging[i] && point_box_collision(mousex, mousey, inplay.x[i] - (inplay.w[i] / 2), inplay.y[i] - (inplay.h[i] / 2), inplay.w[i], inplay.h[i]) && (currMouseState & SDL_BUTTON_LMASK) && !(prevMouseState & SDL_BUTTON_LMASK)) {
            inplay.isDragging[i] = true;
            isDragging = true;
        }

        // Checks if a card should be released
        // -----------------------------------
        if (inplay.isDragging[i] && !(currMouseState & SDL_BUTTON_LMASK)) {

            // checks for every active zone
            for (int j = 0; j < MAX_ZONES; j++) {
                if (!playzones.isActive[j]) continue;

                // CHecks a card is put into a zone that is not full
                // Also checks it is not put into a invalid zone
                // ---------------------------------------------
                if (point_box_collision(inplay.tx[i], inplay.ty[i], playzones.x[j], playzones.y[j], playzones.w[j], playzones.h[j]) && (playzones.num_cards[j] < playzones.max_cards[j])) {
                    if (!(!inplay.isSellable[i] && j == ZONE_DISCARD) && j != ZONE_EVENT) {
                        // every card in the card's original zone shifts down one space
                        for (int l = 0; l < MAX_CARDS; l++) { 
                            if (inplay.zoneID[l] == inplay.zoneID[i] && inplay.zoneNum[l] > inplay.zoneNum[i]) 
                                inplay.zoneNum[l] -= 1; 
                        }
                        playzones.num_cards[inplay.zoneID[i]] -= 1;
                        playzones.num_cards[j] += 1;
                        inplay.zoneNum[i] = playzones.num_cards[j];
                        inplay.zoneID[i] = j;
                        inplay.zoneTime[i] = (SDL_GetTicks()/1000.0f);
                    }
                }
            }

            inplay.isDragging[i] = false;
            isDragging = false;
        }

        // if you are dragging a card it goes to your mouse
        // if you are not, it goes back to its zone
        // ------------------------------------------------
        if (inplay.isDragging[i]) {
            inplay.tx[i] = mousex;
            inplay.ty[i] = mousey;
        } else {
            float fanning = playzones.w[inplay.zoneID[i]] / 2 - ((CARD_WIDTH + CARD_SPACING) / 2.0f) * window_scale_x * playzones.num_cards[inplay.zoneID[i]] - 5*window_scale_x;
            inplay.tx[i] = playzones.x[inplay.zoneID[i]] + (CARD_WIDTH/2 + CARD_SPACING + ((inplay.zoneNum[i] - 1) * (CARD_WIDTH + CARD_SPACING)) )*window_scale_x + fanning;
            inplay.ty[i] = playzones.y[inplay.zoneID[i]] + playzones.h[inplay.zoneID[i]]/2;
        }

        // Checks if a card should be sold
        // ----------------------------------
        if (inplay.zoneID[i] == ZONE_DISCARD && ((SDL_GetTicks()/1000.0f)-inplay.zoneTime[i]) > 1.5 && inplay.isSellable[i]) {
            if (inplay.isDragging[i]) isDragging = false;
            playzones.num_cards[inplay.zoneID[i]] -= 1;
            // add_card(inplay.ID[i], true);
            discard_card(&inplay, i, true);
            // sell the card for / give player money
        }

        inplay.w[i] = CARD_WIDTH*window_scale_x;
        inplay.h[i] = CARD_HEIGHT*window_scale_y;

        inplay.vx[i] = (inplay.tx[i] - inplay.x[i]) / LERP_SPEED;
        inplay.vy[i] = (inplay.ty[i] - inplay.y[i]) / LERP_SPEED;
        inplay.y[i] += inplay.vy[i] * dt;
        inplay.x[i] += inplay.vx[i] * dt;

    }
}

void render() {

    // ZONES TEXTURES / HITBOX
    // -----------------------
    for (int i = 0; i < MAX_ZONES; i++) {
        if (!playzones.isActive[i]) continue;

        if (show_textures) {
            if (i == ZONE_DECK) {
                float sineh = 5 *  sin(2.0*((SDL_GetTicks() / 1000.0f)));
                float sinea = 2.5 *  sin(1*((SDL_GetTicks() / 1000.0f)));
                float angle = sinea;

                SDL_FRect zone = {
                    playzones.x[i], 
                    playzones.y[i] - sineh*window_scale_y, 
                    playzones.w[i], 
                    playzones.h[i]
                };

                SDL_FPoint center = {zone.w / 2, zone.h / 2};
                SDL_RenderTextureRotated(renderer, deck, NULL, &zone, angle, &center, SDL_FLIP_NONE);
            }
        }

        if (show_hitboxes) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_FRect zonehitbox = {playzones.x[i], playzones.y[i], playzones.w[i], playzones.h[i]};
            SDL_RenderRect(renderer, &zonehitbox);
        }
    }

    // STOCK CHARTS TEXTURES / HITBOX
    // ------------------------------

    // ASSISTANT TEXTURES / HITBOX
    // ---------------------------

    // CARDS TEXTURES / HITBOX
    // -----------------------
    for (int i = 0; i < MAX_CARDS; i++) {
        if (!inplay.isActive[i]) continue;

        if (show_textures) {

            float card_grow_w = inplay.isDragging[i]*CARD_GROW*inplay.w[i];
            float card_grow_h = inplay.isDragging[i]*CARD_GROW*inplay.h[i];

            float sineh = 0, sinea = 0, fanning = 0;
            if (!inplay.isDragging[i]) {
                sineh = 5 *  sin(2 * (inplay.zoneTime[i] - SDL_GetTicks() / 1000.0f));
                fanning = (inplay.zoneNum[i] - ((playzones.num_cards[inplay.zoneID[i]] + 1) / 2.0f)) * 2.5f;
                sinea = 2.5f * sin(inplay.zoneTime[i] - SDL_GetTicks() / 1000.0f);
            }
            double angle = inplay.vx[i]/60 + sinea + fanning;

            SDL_FRect cardtexture = {  
                inplay.x[i] - (inplay.w[i]/2) - (card_grow_w/2), 
                inplay.y[i] - (inplay.h[i]/2) - (card_grow_h/2) - (sineh*window_scale_y), 
                inplay.w[i] + (card_grow_w*window_scale_x), 
                inplay.h[i] + (card_grow_h*window_scale_y)
            };

            SDL_FPoint center = {cardtexture.w / 2, cardtexture.h / 2};
            SDL_RenderTextureRotated(renderer, cards.cardtexture[inplay.ID[i]], NULL, &cardtexture, angle, &center, SDL_FLIP_NONE);
        }

        if (show_hitboxes) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_FRect cardhitbox = {inplay.x[i] - (inplay.w[i] / 2), inplay.y[i] - (inplay.h[i] / 2), inplay.w[i], inplay.h[i]};
            SDL_RenderRect(renderer, &cardhitbox);
        }
    }

    // CURSOR TEXTURES / HITBOX
    // ------------------------
    if (custom_cursor) {
        SDL_HideCursor();

        if (show_textures) {}

        if (show_hitboxes) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_FRect cursorhitbox = {mousex, mousey, 30.0f*window_scale_x, 30.0f*window_scale_y};
            SDL_RenderRect(renderer, &cursorhitbox);
        }

    } else {
        SDL_ShowCursor();
    }

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

// MAIN FUNCTION ====================================================================================================

int main() {
    
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
        if (dt > 0.3) continue;

        inputs();
        dev_tools();
        update();

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderClear(renderer);
        render();
        SDL_RenderPresent(renderer);

        debug();

        SDL_memcpy(prevKeyState, currKeyState, NUM_KEYS);
        control_fps(120.0f);
    }

    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    return 0;
}
