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
int profile;

bool show_hitboxes = true;
bool show_textures = true;
bool custom_cursor = false;

#define LERP_SPEED 0.25
#define CARD_HEIGHT 190
#define CARD_WIDTH 142
#define CARD_SPACING 10
#define CARD_GROW 0.05
#define CARD_MARGIN 40

#define MAX_CARDS 10
#define MAX_ZONES 10
#define TOTAL_CARDS 11
#define MAX_BUTTONS 20
#define MAX_PROFILES 1

// CLASSES ====================================================================================================

typedef struct ProfileInfo {
    int ID[MAX_PROFILES]; // profile number
    char name[MAX_PROFILES][256]; // profile name

    int money[MAX_PROFILES]; // currency
    // int permachips[MAX_PROFILES]; // used to delete unsellable cards
    // int loans[MAX_PROFILES]; // how many loans you have left (3 per run)

    int handslots[MAX_PROFILES]; // the maximum cards you can have in your hand
    // int drawnum[MAX_PROFILES]; // number of cards you draw at the beggining of each round
    // int exdrawprice[MAX_PROFILES]; // how much money the first extra draw is
    // int exdrawinflation[MAX_PROFILES]; // how much money the draws increase by after the first one

    // extra card drawing
    // ------------
    int extraprice[MAX_PROFILES]; // how much an extra card draw costs
    int extrainflation[MAX_PROFILES]; // how much money the extra draw price increases by every time
    int extracount[MAX_PROFILES]; // how many times you have drawn an extra card in a round

    // Loans
    int loans[MAX_PROFILES]; // how many more loans you can take out
    

    // int money[MAX_PROFILES];
} ProfileInfo;

ProfileInfo profileinfo;

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

Cards inplay;
Cards indeck;

typedef struct Zones {
    int ID[MAX_ZONES];
    SDL_Texture* zonetexture[MAX_ZONES];
    char zonepath[MAX_ZONES][256];

    float x[MAX_ZONES];
    float y[MAX_ZONES];
    float w[MAX_ZONES];
    float h[MAX_ZONES];

    int max_cards[MAX_ZONES];
    int num_cards[MAX_ZONES];

    float isActive[MAX_ZONES];
} Zones; 

Zones playzones;
Zones eventzone;

typedef enum ZoneType {
    ZONE_HAND,
    ZONE_SELL,
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

CardID cards;

typedef enum ButtonType {
    BUTTON_DECK,
    BUTTON_LOAN,
    BUTTON_BUY_STOCK,
    BUTTON_SELL_STOCK,
    BUTTON_PAUSE,
} ButtonType;

typedef struct Buttons {
    int ID[MAX_BUTTONS];
    SDL_Texture* buttontexture[MAX_BUTTONS];
    char buttonpath[MAX_BUTTONS][256];


    float x[MAX_BUTTONS];
    float y[MAX_BUTTONS];
    float w[MAX_BUTTONS];
    float h[MAX_BUTTONS];

    bool isPressed[MAX_BUTTONS];
    bool isClicked[MAX_BUTTONS];
    bool isActive[MAX_BUTTONS];
} Buttons;

Buttons gamebuttons;

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

void clear_cards(Cards* cards) {

    for (int i = 0; i < MAX_CARDS; i++) {
        
        // DELET CARD AT INDEX
        // -------------------

        cards->ID[i] = -1;

        cards->x[i] = 0;
        cards->y[i] = 0;
        cards->vx[i] = 0;
        cards->vy[i] = 0;
        cards->tx[i] = 0;
        cards->ty[i] = 0;
        cards->w[i] = CARD_WIDTH;
        cards->h[i] = CARD_HEIGHT;

        cards->zoneID[i] = -1;
        cards->zoneNum[i] = 0;

        cards->isDragging[i] = false;
        cards->isActive[i] = false;
        cards->isSellable[i] = false;
    }

    cards->num = 0;

    return;
}

void add_to_deck(int id) {

    // FIND IF AND WHERE A DECK SLOT IS OPEN
    // -------------------------------------
    int index = -1;
    for (int i = 0; i < MAX_CARDS; i++) {
        if (!indeck.isActive[i]) {
            index = i;
            break;
        }
    }
    if (index == -1) return;

    // ACTIVATE NEW CARD WITH ID AT THAT INDEX
    // ---------------------------------------
    
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

    return;
}

void add_to_hand(int id) {

    if (playzones.num_cards[ZONE_HAND] >= playzones.max_cards[ZONE_HAND]) return; // check if there is any space in your hand

    // FIND IF AND WHERE AN IN PLAY SLOT IS OPEN
    // -------------------------------------
    int index = -1;
    for (int i = 0; i < MAX_CARDS; i++) {
        if (!inplay.isActive[i]) {
            index = i;
            break;
        }
    }
    if (index == -1) return;

    // ACTIVATE NEW CARD WITH ID AT THAT INDEX
    // ---------------------------------------
    
    inplay.ID[index] = id;

    inplay.x[index] = (1500/2);
    inplay.y[index] = ((playzones.y[ZONE_EVENT] + playzones.h[ZONE_EVENT]/2) + 1000);
    inplay.w[index] = CARD_WIDTH;
    inplay.h[index] = CARD_HEIGHT;

    playzones.num_cards[ZONE_HAND] += 1;
    inplay.zoneID[index] = ZONE_HAND;
    inplay.zoneNum[index] = playzones.num_cards[ZONE_HAND];
    inplay.zoneTime[index] = (SDL_GetTicks()/1000.0f);

    inplay.isDragging[index] = false;
    inplay.isActive[index] = true;
    inplay.isSellable[index] = true;
    
    inplay.num += 1;

    return;
}

void deck_to_hand() {

    if (playzones.num_cards[ZONE_HAND] >= playzones.max_cards[ZONE_HAND]) return; // check if there is any space in your hand

    // PICKS A RANDOM VALID CARD FROM YOUR DECK
    // ----------------------------------------
    int validIndex[MAX_CARDS];
    int numValid = 0;
    for (int i = 0; i < MAX_CARDS; i++) {
        if (indeck.isActive[i]) 
            validIndex[numValid++] = i;
    }
    if (numValid == 0) return;
    int randIndex = rand() % numValid; 
    int indeckIndex = validIndex[randIndex];

    // FIND IF AND WHERE AN IN PLAY SLOT IS OPEN
    // -----------------------------------------
    int inplayIndex = -1;
    for (int i = 0; i < MAX_CARDS; i++) {
        if (!inplay.isActive[i]) {
            inplayIndex = i;
            break;
        }
    }
    if (inplayIndex == -1) return;

    // COPY CARD AT INPLAYINDEX FROM DECK TO HAND
    // ---------------------------------

    inplay.ID[inplayIndex] = indeck.ID[indeckIndex];

    inplay.x[inplayIndex] = (gamebuttons.x[BUTTON_DECK]+gamebuttons.w[BUTTON_DECK]/2);
    inplay.y[inplayIndex] = (gamebuttons.y[BUTTON_DECK]+gamebuttons.h[BUTTON_DECK]/2);
    inplay.w[inplayIndex] = CARD_WIDTH;
    inplay.h[inplayIndex] = CARD_HEIGHT;

    playzones.num_cards[ZONE_HAND] += 1;
    inplay.zoneID[inplayIndex] = ZONE_HAND;
    inplay.zoneNum[inplayIndex] = playzones.num_cards[ZONE_HAND];
    inplay.zoneTime[inplayIndex] = (SDL_GetTicks()/1000.0f);

    isDragging = true;
    inplay.isDragging[inplayIndex] = true;
    inplay.isActive[inplayIndex] = true;
    inplay.isSellable[inplayIndex] = indeck.isSellable[indeckIndex];
    
    inplay.num += 1;

    // profileinfo.money[profile] -= (profileinfo.extraprice[profile] + profileinfo.extrainflation[profile] * profileinfo.extracount[profile]);
    // profileinfo.extracount[profile] += 1;

    // DISCARD DECK CARD
    // -----------------
    
    indeck.ID[indeckIndex] = -1;

    indeck.x[indeckIndex] = 0;
    indeck.y[indeckIndex] = 0;
    indeck.vx[indeckIndex] = 0;
    indeck.vy[indeckIndex] = 0;
    indeck.tx[indeckIndex] = 0;
    indeck.ty[indeckIndex] = 0;
    indeck.w[indeckIndex] = CARD_WIDTH;
    indeck.h[indeckIndex] = CARD_HEIGHT;

    indeck.zoneID[indeckIndex] = -1;
    indeck.zoneNum[indeckIndex] = 0;

    indeck.isDragging[indeckIndex] = false;
    indeck.isActive[indeckIndex] = false;
    indeck.isSellable[indeckIndex] = false;
    
    indeck.num -= 1;

    return;
}

void hand_to_deck(int inplayIndex) {

    // FIND IF AND WHERE A DECK SLOT IS OPEN
    // -----------------------------------------
    int indeckIndex = -1;
    for (int i = 0; i < MAX_CARDS; i++) {
        if (!indeck.isActive[i]) {
            indeckIndex = i;
            break;
        }
    }
    if (indeckIndex == -1) return;

    // COPY CARD AT ID FROM HAND TO DECK
    // ---------------------------------

    indeck.ID[indeckIndex] = inplay.ID[inplayIndex];

    indeck.x[indeckIndex] = 0;
    indeck.y[indeckIndex] = 0;
    indeck.vx[indeckIndex] = 0;
    indeck.vy[indeckIndex] = 0;
    indeck.tx[indeckIndex] = 0;
    indeck.ty[indeckIndex] = 0;
    indeck.w[indeckIndex] = CARD_WIDTH;
    indeck.h[indeckIndex] = CARD_HEIGHT;

    indeck.zoneID[indeckIndex] = -1;
    indeck.zoneNum[indeckIndex] = 0;

    indeck.isDragging[indeckIndex] = false;
    indeck.isActive[indeckIndex] = true;
    indeck.isSellable[indeckIndex] = inplay.isSellable[inplayIndex];
    
    indeck.num += 1;

    // HAND COPY DELETED

    playzones.num_cards[inplay.zoneID[inplayIndex]] -= 1;
    inplay.ID[inplayIndex] = -1;

    inplay.x[inplayIndex] = 0;
    inplay.y[inplayIndex] = 0;
    inplay.vx[inplayIndex] = 0;
    inplay.vy[inplayIndex] = 0;
    inplay.tx[inplayIndex] = 0;
    inplay.ty[inplayIndex] = 0;
    inplay.w[inplayIndex] = CARD_WIDTH;
    inplay.h[inplayIndex] = CARD_HEIGHT;

    inplay.zoneID[inplayIndex] = -1;
    inplay.zoneNum[inplayIndex] = 0;
    inplay.isSellable[inplayIndex] = false;

    inplay.isDragging[inplayIndex] = false;
    inplay.isActive[inplayIndex] = false;
    inplay.isSellable[inplayIndex] = false;
    
    inplay.num -= 1;

    return;
}

void event_card(int id) {

    if (playzones.num_cards[ZONE_EVENT] >= playzones.max_cards[ZONE_EVENT]) return; // check if there is any space in the event zone

    // finds the first empty space in your hand
    // ----------------------------------------
    int inplayIndex = -1;
    for (int i = 0; i < MAX_CARDS; i++) {
        if (!inplay.isActive[i]) {
            inplayIndex = i;
            break;
        }
    }
    if (inplayIndex == -1) return;

    // adds the card with the ID to the EVENT zone
    // ------------------------------------------------------------

    inplay.ID[inplayIndex] = id;

    inplay.x[inplayIndex] = (playzones.x[ZONE_EVENT] + playzones.w[ZONE_EVENT]/2);
    inplay.y[inplayIndex] = ((playzones.y[ZONE_EVENT] + playzones.h[ZONE_EVENT]/2) - 1000);
    inplay.w[inplayIndex] = CARD_WIDTH;
    inplay.h[inplayIndex] = CARD_HEIGHT;

    playzones.num_cards[ZONE_EVENT] += 1;
    inplay.zoneID[inplayIndex] = ZONE_EVENT;
    inplay.zoneNum[inplayIndex] = playzones.num_cards[ZONE_EVENT];
    inplay.zoneTime[inplayIndex] = (SDL_GetTicks()/1000.0f);

    inplay.isDragging[inplayIndex] = false;
    inplay.isActive[inplayIndex] = true;
    inplay.isSellable[inplayIndex] = false;
    
    inplay.num += 1;

    return;
}

void loan_card() {
    
    if (playzones.num_cards[ZONE_HAND] >= playzones.max_cards[ZONE_HAND]) return; // chack if there is any space in your hand
    if (profileinfo.loans[profile] <= 0) return;

    int id = 0; // loan card index in CardID (cards.ID)

    // FIND IF AND WHERE AN IN PLAY SLOT IS OPEN
    // -------------------------------------
    int index = -1;
    for (int i = 0; i < MAX_CARDS; i++) {
        if (!inplay.isActive[i]) {
            index = i;
            break;
        }
    }
    if (index == -1) return;

    // ACTIVATE NEW CARD WITH ID AT THAT INDEX
    // ---------------------------------------
    
    profileinfo.money[profile] += 100;
    inplay.ID[index] = id;

    inplay.x[index] = (1500/2);
    inplay.y[index] = ((playzones.y[ZONE_EVENT] + playzones.h[ZONE_EVENT]/2) + 1000);
    inplay.w[index] = CARD_WIDTH;
    inplay.h[index] = CARD_HEIGHT;

    playzones.num_cards[ZONE_HAND] += 1;
    inplay.zoneID[index] = ZONE_HAND;
    inplay.zoneNum[index] = playzones.num_cards[ZONE_HAND];
    inplay.zoneTime[index] = (SDL_GetTicks()/1000.0f);

    inplay.isDragging[index] = false;
    inplay.isActive[index] = true;
    inplay.isSellable[index] = true;
    
    inplay.num += 1;
    profileinfo.loans[profile] -= 1;

    return;
}

void shuffle_hand() {
    for (int i = 0; i < MAX_CARDS; i++) {
        if (!inplay.isActive[i]) continue;
        if (inplay.zoneID[i] == ZONE_SELL) continue;
        if (inplay.zoneID[i] == ZONE_EVENT) continue;
        hand_to_deck(i);
    }

    return;
}

void sell_card(int inplayIndex) {

    if (!inplay.isSellable[inplayIndex]) return; // checks if the card is sellable

    // INCREASE MONEY
    // --------------
    
    if (inplay.ID[inplayIndex] == 0) {
        profileinfo.money[profile] -= 100;
    } else {
        profileinfo.money[profile] += (cards.price[inplay.ID[inplayIndex]])/2;
    }

    // DELETE INPLAY CARD
    // ------------------

    playzones.num_cards[inplay.zoneID[inplayIndex]] -= 1;
    
    inplay.ID[inplayIndex] = -1;

    inplay.x[inplayIndex] = 0;
    inplay.y[inplayIndex] = 0;
    inplay.vx[inplayIndex] = 0;
    inplay.vy[inplayIndex] = 0;
    inplay.tx[inplayIndex] = 0;
    inplay.ty[inplayIndex] = 0;
    inplay.w[inplayIndex] = CARD_WIDTH;
    inplay.h[inplayIndex] = CARD_HEIGHT;

    playzones.num_cards[ZONE_SELL] = 0;
    inplay.zoneID[inplayIndex] = -1;
    inplay.zoneNum[inplayIndex] = 0;

    inplay.isDragging[inplayIndex] = false;
    inplay.isActive[inplayIndex] = false;
    inplay.isSellable[inplayIndex] = false;
    
    inplay.num -= 1;
                            
    return;
}

// SETUP FUNCTIONS ====================================================================================================

void make_card(int id, const char* cardpath, const char* name, const char* description, float stats[100]) {

    // set string data
    // ---------------
    strcpy(cards.cardpath[id], cardpath);
    strcpy(cards.name[id], name);
    strcpy(cards.description[id], description);

    // set stat data
    // -------------

    if (stats != NULL) {
        cards.price[id] = stats[0]; // for ranged
        // cards.min_damage[id] = stats[1];
        // cards.max_damage[id] = stats[2];
        // cards.lifesteal[id] = stats[3];
        // cards.ammo[id] = stats[4];
        // cards.reload[id] = stats[5];
    }

}

void make_zone(ZoneType zone, const char* zonepath, int slots, int x, int y, int w, int h) {
    strcpy(playzones.zonepath[zone], zonepath);

    playzones.max_cards[zone] = slots;
    playzones.x[zone] = x * window_scale_x;
    playzones.y[zone] = y * window_scale_y;
    if (w != 0) {
        playzones.w[zone] = w * window_scale_x;
    } else {
        playzones.w[zone] = (CARD_SPACING + (CARD_WIDTH + CARD_SPACING) * slots) * window_scale_x;
    }

    if (h != 0) {
        playzones.h[zone] = h * window_scale_y;
    } else {
        playzones.h[zone] = (CARD_HEIGHT + CARD_SPACING * 2) * window_scale_y;
    }
    
    playzones.isActive[zone] = true;
}

void make_button(ButtonType button, const char* buttonpath, int x, int y, int w, int h) {
    strcpy(gamebuttons.buttonpath[button], buttonpath);

    gamebuttons.ID[button] = button;
    gamebuttons.x[button] = x * window_scale_x;
    gamebuttons.y[button] = y * window_scale_y;
    gamebuttons.w[button] = w * window_scale_x;
    gamebuttons.h[button] = h * window_scale_y;

    gamebuttons.isActive[button] = true;
    // gamebuttons.[button] = ;
}

void update_zones() {
    // original prototype
    // make_zone(ZONE_SELL, "", 1, (1500-CARD_MARGIN-CARD_WIDTH-CARD_SPACING), CARD_MARGIN, 0, 0);
    // make_zone(ZONE_HAND, "", 9, CARD_MARGIN, (1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN), 0, 0);
    // make_zone(ZONE_EQUIP_1, "", 1, (1500-CARD_MARGIN-CARD_WIDTH-CARD_SPACING-CARD_MARGIN-CARD_WIDTH-CARD_SPACING), CARD_MARGIN, 0, 0);
    // make_button(BUTTON_DECK, "./resources/textures/deck.png", CARD_MARGIN, CARD_MARGIN, CARD_WIDTH+CARD_SPACING, CARD_HEIGHT+CARD_SPACING);

    // new version
    // make_button(BUTTON_DECK, "./resources/textures/deck.png", CARD_MARGIN, 1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, CARD_WIDTH+CARD_SPACING, CARD_HEIGHT+CARD_SPACING);
    // make_zone(ZONE_HAND, "", profileinfo.handslots[profile], CARD_MARGIN+CARD_SPACING+CARD_WIDTH+CARD_MARGIN, 1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, 0, 0);
    // make_zone(ZONE_SELL, "", 1, 1500-CARD_MARGIN-CARD_SPACING-CARD_WIDTH, 1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, 0, 0);
    // make_zone(ZONE_EQUIP_1, "", 1, CARD_MARGIN, CARD_MARGIN, 0, 0);
    // make_zone(ZONE_EQUIP_2, "", 1, CARD_MARGIN+CARD_WIDTH+CARD_SPACING+CARD_MARGIN, CARD_MARGIN, 0, 0);

    // pre idea change version
    // make_zone(ZONE_HAND, "", profileinfo.handslots[profile], CARD_MARGIN, CARD_MARGIN, 0, 0);
    // make_zone(ZONE_SELL, "", 1, 1500-CARD_MARGIN-CARD_SPACING-CARD_WIDTH, CARD_MARGIN, 0, 0);
    // make_zone(ZONE_EQUIP_1, "", 1, 1500-CARD_MARGIN-CARD_SPACING-CARD_WIDTH, 1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, 0, 0);
    // make_zone(ZONE_EVENT, "", 1, 1500-CARD_MARGIN-CARD_SPACING-CARD_WIDTH-CARD_MARGIN-CARD_WIDTH, 1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, 0, 0);
    // make_button(BUTTON_DECK, "./resources/textures/deck.png", CARD_MARGIN, 1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, CARD_WIDTH+CARD_SPACING, CARD_HEIGHT+CARD_SPACING);

    // post idea change version
    make_zone(ZONE_SELL, "", 1, 1500-CARD_MARGIN-CARD_SPACING-CARD_WIDTH, 1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, 0, 0);
    make_zone(ZONE_HAND, "", profileinfo.handslots[profile], 210, 1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, 1080, 0);
    make_zone(ZONE_EVENT, "", 1, 1500-CARD_MARGIN-CARD_SPACING-CARD_WIDTH, CARD_MARGIN, 0, 0);
    make_button(BUTTON_DECK, "./resources/textures/deck.png", CARD_MARGIN, 1000-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, CARD_WIDTH+CARD_SPACING, CARD_HEIGHT+CARD_SPACING);
    make_button(BUTTON_LOAN, "./resources/textures/deck.png", CARD_MARGIN, 500, 100, 50);
}

bool load_textures() {

    // LOAD TEXTURES FOR CARDS
    // -----------------------
    for (int i = 0; i < TOTAL_CARDS; i++) {
        cards.cardtexture[i] = IMG_LoadTexture(renderer, cards.cardpath[i]);
        SDL_SetTextureScaleMode(cards.cardtexture[i], SDL_SCALEMODE_NEAREST);
    }

    // LOAD TEXTURES FOR BUTTONS
    // -----------------------
    for (int i = 0; i < MAX_BUTTONS; i++) {
        gamebuttons.buttontexture[i] = IMG_LoadTexture(renderer, gamebuttons.buttonpath[i]);
        SDL_SetTextureScaleMode(gamebuttons.buttontexture[i], SDL_SCALEMODE_NEAREST);
    }

    // LOAD TEXTURES FOR ZONES
    // -----------------------
    for (int i = 0; i < MAX_ZONES; i++) {
        playzones.zonetexture[i] = IMG_LoadTexture(renderer, playzones.zonepath[i]);
        SDL_SetTextureScaleMode(playzones.zonetexture[i], SDL_SCALEMODE_NEAREST);
    }

	return true;
}

void update_window() {
    // get window sizes and set window scaling x/y coefficients
    SDL_GetWindowSize(window, &window_width, &window_height);
    window_scale_x = window_width / 1500.0f;  
    window_scale_y = window_height / 1000.0f;
}

void setup() {

    // LOAD CARDS
    // ----------
    make_card(0, "./resources/textures/loan.png", "$100 Loan", "card of the number of one", floatarr(6, -100.0f, 4, 3, 2, 1, 1));
    
    make_card(1, "./resources/textures/card2.png", "Flame Boy", "card of the number of two", floatarr(6, 5.0f, 4, 3, 2, 1, 1));
    make_card(2, "./resources/textures/card3.png", "Peaked", "card of the number of three", floatarr(6, 5.0f, 4, 3, 2, 1, 1));
    make_card(3, "./resources/textures/card4.png", "Eg", "card of the number of four", floatarr(6, 5.0f, 4, 3, 2, 1, 1));
    make_card(4, "./resources/textures/card5.png", "Ballot Joker", "card of the number of five", floatarr(6, 5.0f, 4, 3, 2, 1, 1));
    make_card(5, "./resources/textures/card6.png", "Pokemon Card Joker", "card of the number of six", floatarr(6, 5.0f, 4, 3, 2, 1, 1));
    make_card(6, "./resources/textures/card7.png", "Bunanu", "card of the number of seven", floatarr(6, 5.0f, 4, 3, 2, 1, 1));
    make_card(7, "./resources/textures/card8.png", "Caino", "card of the number of eight", floatarr(6, 5.0f, 4, 3, 2, 1, 1));
    make_card(8, "./resources/textures/card9.png", "Purple Magic", "card of the number of nine", floatarr(6, 5.0f, 4, 3, 2, 1, 1));
    make_card(9, "./resources/textures/card10.png", "Planet Joker", "card of the number of ten", floatarr(6, 5.0f, 4, 3, 2, 1, 1));
    make_card(10, "./resources/textures/card1.png", "Normal Joker", "card of the number of one", floatarr(6, 5.0f, 4, 3, 2, 1, 1));
    
    update_window();
    update_zones();

    const Uint8* sdlKeys = (Uint8*)SDL_GetKeyboardState(NULL);
    SDL_memcpy(currKeyState, sdlKeys, NUM_KEYS);
    SDL_memcpy(prevKeyState, sdlKeys, NUM_KEYS);

    // SETUP CARDS, BUTTONS AND ZONES
    // ---------------------
    for (int i = 0; i < MAX_CARDS; i++) { clear_cards(&inplay); }
    for (int i = 0; i < MAX_CARDS; i++) { clear_cards(&indeck); }
    for (int i = 0; i < MAX_ZONES; i++) {
        playzones.isActive[i] = false;
        playzones.num_cards[i] = 0;
        playzones.max_cards[i] = 0;
    }
    for (int i = 0; i < MAX_BUTTONS; i++) {
        gamebuttons.isActive[i] = false;
        gamebuttons.isPressed[i] = false;
        gamebuttons.ID[i] = -1;
    }

    // ADD CARDS TO DECK
    // ---------------------
    for (int i = 0; i < 1; i++) { add_to_deck(rand() % TOTAL_CARDS); }

    // INITIALIZE PROFILE INFO
    // -----------------------
    profile = 0;
    // used
    profileinfo.money[profile] = 100;
    profileinfo.handslots[profile] = 3;
    profileinfo.loans[profile] = 3;
    // not used
    // profileinfo.extraprice[profile] = 7;
    // profileinfo.extrainflation[profile] = 2;
    // profileinfo.extracount[profile] = 0;

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
        profileinfo.handslots[profile] += 1;
        if (profileinfo.handslots[profile] > 7) profileinfo.handslots[profile] = 7;
    }

    // 4 - shuffle hand into deck
    // --------------------------
    if (currKeyState[SDL_SCANCODE_4] && !prevKeyState[SDL_SCANCODE_4]) {
        shuffle_hand();
    }

    // 5 - spawn event card
    // --------------------
    if (currKeyState[SDL_SCANCODE_5] && !prevKeyState[SDL_SCANCODE_5]) {
        event_card(rand() % TOTAL_CARDS);
    }

    // 6 - draw card id to hand
    // --------------------------
    if (currKeyState[SDL_SCANCODE_6] && !prevKeyState[SDL_SCANCODE_6]) {
        add_to_hand(rand() % TOTAL_CARDS);
    }

}

void update() {

    update_window();
    update_zones();

    // Draws an event card once every 15 seconds
    // ------------------------------------
    int ticks = (int)(SDL_GetTicks() / 100.0f) % 50;
    if (ticks == 50 - 1) { event_card((rand() % TOTAL_CARDS-1) + 1); }

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
                if (!inplay.isSellable[i] && j == ZONE_SELL) continue;
                if (j == ZONE_EVENT) continue;

                // CHecks a card is put into a zone that is not full
                // Also checks it is not put into a invalid zone
                // ---------------------------------------------
                if (point_box_collision(inplay.tx[i], inplay.ty[i], playzones.x[j], playzones.y[j], playzones.w[j], playzones.h[j]) && playzones.num_cards[j] < playzones.max_cards[j]) {
                    
                    // every card in the card's original zone shifts down one space
                    for (int l = 0; l < MAX_CARDS; l++) { 
                        if (inplay.zoneID[l] == inplay.zoneID[i] && inplay.zoneNum[l] > inplay.zoneNum[i]) 
                            inplay.zoneNum[l] -= 1; 
                    }

                    playzones.num_cards[inplay.zoneID[i]] -= 1;
                    playzones.num_cards[j] += 1;
                    inplay.zoneNum[i] = playzones.num_cards[j];
                    inplay.zoneID[i] = j;
                }
            }

            inplay.isDragging[i] = false;
            isDragging = false;
            inplay.zoneTime[i] = (SDL_GetTicks() / 1000.0f);
        }

        // Checks if a card should be sold
        // -------------------------------
        // if (((SDL_GetTicks()/1000.0f)-inplay.zoneTime[i]) > 1.5) {
        if (inplay.zoneID[i] == ZONE_SELL && ((SDL_GetTicks()/1000.0f)-inplay.zoneTime[i]) > 1.5 && inplay.isSellable[i] && !inplay.isDragging[i]) {
            // visual
            sell_card(i);
        }

        // if you are dragging a card it goes to your mouse
        // if you are not, it goes back to its zone
        // ------------------------------------------------
        if (inplay.isDragging[i]) {
            inplay.tx[i] = mousex;
            inplay.ty[i] = mousey;
        } else {
            float fanning = playzones.w[inplay.zoneID[i]]/2 - ((CARD_WIDTH + CARD_SPACING) / 2.0f)*window_scale_x * playzones.num_cards[inplay.zoneID[i]] - 5*window_scale_x;
            inplay.tx[i] = playzones.x[inplay.zoneID[i]] + (CARD_WIDTH/2 + CARD_SPACING + ((inplay.zoneNum[i] - 1)*(CARD_WIDTH + CARD_SPACING)))*window_scale_x + fanning;
            inplay.ty[i] = playzones.y[inplay.zoneID[i]] + playzones.h[inplay.zoneID[i]]/2;
        }

        inplay.w[i] = CARD_WIDTH*window_scale_x;
        inplay.h[i] = CARD_HEIGHT*window_scale_y;

        inplay.vx[i] = (inplay.tx[i] - inplay.x[i]) / LERP_SPEED;
        inplay.vy[i] = (inplay.ty[i] - inplay.y[i]) / LERP_SPEED;
        inplay.y[i] += inplay.vy[i] * dt;
        inplay.x[i] += inplay.vx[i] * dt;

    }

    // BUTTON UPDATES
    // --------------
    for (int i = 0; i < MAX_BUTTONS; i++) {
        if (!gamebuttons.isActive[i]) continue;

        // Checks different button states (pressed/clicked)
        // ------------------------------------------------
        if (point_box_collision(mousex, mousey, gamebuttons.x[i], gamebuttons.y[i], gamebuttons.w[i], gamebuttons.h[i])) {
            if (currMouseState & SDL_BUTTON_LMASK) { gamebuttons.isPressed[i] = true; } else { gamebuttons.isPressed[i] = false; }
            if ((currMouseState & SDL_BUTTON_LMASK) && !(prevMouseState & SDL_BUTTON_LMASK)) { gamebuttons.isClicked[i] = true; } else { gamebuttons.isClicked[i] = false; }
        } else {
            gamebuttons.isPressed[i] = false;
            gamebuttons.isClicked[i] = false;
        }

        if (gamebuttons.isClicked[i] && gamebuttons.ID[i] == BUTTON_DECK) deck_to_hand(); // DECK BUTTON

        if (gamebuttons.isClicked[i] && gamebuttons.ID[i] == BUTTON_LOAN) loan_card(); // LOAN BUTTON
    }
}

void render() {

    // BACKGROUND
    // ----------

    // ZONES TEXTURES / HITBOX
    // -----------------------
    for (int i = 0; i < MAX_ZONES; i++) {
        if (!playzones.isActive[i]) continue;

        if (show_textures) {
            float angle = 0;
            SDL_FRect zonetexture = { playzones.x[i], playzones.y[i], playzones.w[i], playzones.h[i] };
            SDL_FPoint center = {zonetexture.w / 2, zonetexture.h / 2};
            SDL_RenderTextureRotated(renderer, playzones.zonetexture[i], NULL, &zonetexture, angle, &center, SDL_FLIP_NONE);
        }

        if (show_hitboxes) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_FRect zonehitbox = {playzones.x[i], playzones.y[i], playzones.w[i], playzones.h[i]};
            SDL_RenderRect(renderer, &zonehitbox);
        }
    }

    // BUTTONS
    // -------
    for (int i = 0; i < MAX_BUTTONS; i++) {
        if (!gamebuttons.isActive[i]) continue;

        if (show_textures) {
            float sineh = 5 * sin(2.0f * (SDL_GetTicks() / 1000.0f));
            float sinea = 2.5 * sin(SDL_GetTicks() / 1000.0f);
            float angle = sinea;

            float button_grow_w = gamebuttons.isPressed[i] * CARD_GROW * gamebuttons.w[i];
            float button_grow_h = gamebuttons.isPressed[i] * CARD_GROW * gamebuttons.h[i];

            SDL_FRect buttontexture = {
                gamebuttons.x[i] - (button_grow_w/2), 
                gamebuttons.y[i] - (button_grow_h/2), 
                gamebuttons.w[i] + (button_grow_w*window_scale_x), 
                gamebuttons.h[i] + (button_grow_h*window_scale_y)
            };

            SDL_FPoint center = {buttontexture.w / 2, buttontexture.h / 2};
            SDL_RenderTextureRotated(renderer, gamebuttons.buttontexture[i], NULL, &buttontexture, angle, &center, SDL_FLIP_NONE);
        }

        if (show_hitboxes) {
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            if (gamebuttons.isPressed[i]) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
            SDL_FRect buttonhitbox = {gamebuttons.x[i], gamebuttons.y[i], gamebuttons.w[i], gamebuttons.h[i]};
            SDL_RenderRect(renderer, &buttonhitbox);
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

            float card_grow_w = inplay.isDragging[i] * CARD_GROW * inplay.w[i];
            float card_grow_h = inplay.isDragging[i] * CARD_GROW * inplay.h[i];

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
            SDL_FRect cardhitbox = {inplay.x[i] - inplay.w[i]/2, inplay.y[i] - inplay.h[i]/2 , inplay.w[i], inplay.h[i]};
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
            SDL_FRect cursorhitbox = {mousex, mousey, 30.0f * window_scale_x, 30.0f * window_scale_y};
            SDL_RenderRect(renderer, &cursorhitbox);
        }

    } else {
        SDL_ShowCursor();
    }

}

void debug() {

    // current lifted card
    // -------------------
    if ((currMouseState & SDL_BUTTON_LMASK) && !(prevMouseState & SDL_BUTTON_LMASK)) {
        for (int i = 0; i < MAX_CARDS; i++) {
            if (!inplay.isDragging[i]) continue;
            printf("%d %s -> sell: %d, i: %d\n", inplay.ID[i], cards.name[inplay.ID[i]], inplay.isSellable[i], i);
        }
    }

    // printf("%d/%d\n", playzones.num_cards[ZONE_HAND], playzones.max_cards[ZONE_HAND]);

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

    // printf("w:%d h:%d", window_width, window_height);

    // printf("%d\n", profileinfo.money[profile]);

    // printf("%f\n", cards.price[3]);
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
