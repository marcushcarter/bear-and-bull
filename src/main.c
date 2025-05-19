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

char title[256] = "bear and bull";
char version[256] = "demo v0.1.15";

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

float WINDOW_WIDTH;
float WINDOW_HEIGHT;
int WIDTH_RATIO = 3;
int HEIGHT_RATIO = 2;
int window_width, window_height;
float window_scale_x = 1.0f;
float window_scale_y = 1.0f;
bool isFullscreen = false;
int aspect_ratio;
float TARGET_FPS = 120.0f;

bool show_hitboxes = true;
bool show_textures = true;
bool custom_cursor = false;

int seed;
float game_speed = 2;
float event_timer = 0;
float state_timer = 0;
int profile;

#define LERP_SPEED 0.25
#define CARD_HEIGHT 160
#define CARD_WIDTH 120
#define CARD_SPACING 10
#define CARD_GROW 0.05
#define CARD_MARGIN 55

#define MAX_CARDS 100
#define MAX_ZONES 20
#define TOTAL_CARDS 50
// #define TOTAL_CARDS 11
#define MAX_BUTTONS 50
#define MAX_PROFILES 1

// STAT CLASSES ====================================================================================================

typedef struct RunStats {} GameStats; GameStats gamestats;
typedef struct RoundStats {} RoundStats; RoundStats roundstats;

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

// CARDS / ZONES CLASSES ====================================================================================================

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
    bool isHover[MAX_CARDS];

    int rarity[MAX_CARDS];

    int num;
} Cards;

Cards inplay;
Cards indeck;
Cards infeature;
Cards indraft;

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
Zones menuzone;
Zones draftzones;

typedef enum ZoneType {
    ZONE_HAND,
    ZONE_SELL,
    ZONE_EQUIP_1,
    ZONE_EQUIP_2,
    ZONE_EQUIP_3,
    ZONE_EVENT,

    ZONE_MENU,

    ZONE_DRAFT_HAND,
    ZONE_DRAFT_SELECT,
} ZoneType;

bool cardsunlocked[TOTAL_CARDS] = {0};
void reset_unlocked_cards() { for (int i = 0; i < TOTAL_CARDS; i++) cardsunlocked[i] = 0; }

typedef struct CardID{
    SDL_Texture* cardtexture[TOTAL_CARDS];
    char cardpath[TOTAL_CARDS][256];
    char name[TOTAL_CARDS][256];
    char description[TOTAL_CARDS][256];

    // GENERAL

    float price[TOTAL_CARDS]; // price of the card
    float passive[TOTAL_CARDS]; // (int) money earned at the end of every round
    float mult[TOTAL_CARDS]; // added to 1 and that number is multiplied by the total money earned at the end of the round (before the passive income addition)

    float stock_cost[TOTAL_CARDS];

} CardID;

CardID cards;

typedef enum ButtonType {
    // PLAY
    BUTTON_DECK,
    BUTTON_LOAN,
    BUTTON_BUY_STOCK,
    BUTTON_SELL_STOCK,
    BUTTON_PAUSE,

    // MAIN MENU
    BUTTON_NEW_GAME,
    BUTTON_STATS,
    BUTTON_SETTINGS,
    BUTTON_QUIT,

    // SETTINGS
    BUTTON_SETTINGS_EXIT,
    BUTTON_SETTINGS_FULLSCREEN,
    BUTTON_SETTINGS_ASPECT_PREV,
    BUTTON_SETTINGS_ASPECT_NEXT,
    BUTTON_SETTINGS_ASPECT_RATIO,
    BUTTON_SETTINGS_SPEED_PREV,
    BUTTON_SETTINGS_SPEED_NEXT,
    BUTTON_SETTINGS_HITBOXES,
    BUTTON_SETTINGS_CURSOR,
    BUTTON_SETTINGS_FPS_PREV,
    BUTTON_SETTINGS_FPS_NEXT,
    BUTTON_SETTINGS_SPEEDRUN,
    BUTTON_SETTINGS_PARTICLES,

    // DRAFT
    BUTTON_DRAFT_SKIP,
    BUTTON_DRAFT_CONTINUE,

    // DECK
    BUTTON_DECK_CONTINUE,

    // ALL CARDS
    BUTTON_COLLECTION_CONTINUE,

} ButtonType;

typedef struct Buttons {
    int ID[MAX_BUTTONS];
    SDL_Texture* buttontexture[MAX_BUTTONS];
    char buttonpath[MAX_BUTTONS][256];

    float x[MAX_BUTTONS];
    float y[MAX_BUTTONS];
    float w[MAX_BUTTONS];
    float h[MAX_BUTTONS];
    
    float clickTime[MAX_BUTTONS];

    bool isPressed[MAX_BUTTONS];
    bool isClicked[MAX_BUTTONS];
    bool isActive[MAX_BUTTONS];
    bool isHover[MAX_BUTTONS];
} Buttons;

Buttons gamebuttons;
Buttons menubuttons;
Buttons settingbuttons;
Buttons draftbuttons;
Buttons deckbuttons;
Buttons collectionbuttons;

// RARITY FUNCTIONS / ENUMS ====================================================================================================

typedef enum CardRarity {
    RARITY_NONE,
    RARITY_LEVEL1,
    RARITY_LEVEL2,
    RARITY_LEVEL3,
    RARITY_LEVEL4,

    RARITY_TOTAL,
} CardRarity;

// #define MAX_RARITY_ENTRIES 1000

int card_rarity_table[1000];
int rarity_table_num = 0;

void make_rarity_table(int r0, int r1, int r2, int r3, int r4) {
    rarity_table_num = 0;

    for (int i = 0; i < r0; i++) card_rarity_table[rarity_table_num++] = 0;
    for (int i = 0; i < r1; i++) card_rarity_table[rarity_table_num++] = 1;
    for (int i = 0; i < r2; i++) card_rarity_table[rarity_table_num++] = 2;
    for (int i = 0; i < r3; i++) card_rarity_table[rarity_table_num++] = 3;
    for (int i = 0; i < r4; i++) card_rarity_table[rarity_table_num++] = 4;
}

int random_rarity() {
    if (rarity_table_num == 0) { printf("Rarity table not initialized!\n"); return 0; }
    int index = rand() % rarity_table_num;
    return card_rarity_table[index];
}

// STONK CLASSES ====================================================================================================

typedef struct StonkData {
    char name[256];

    float time_started;
    float price[61];
    float h[61];
    float o[61];
    float c[61];
    float l[61];
    float integrity;
} StonkData; StonkData stonk;

// SCREEN CLASSES ====================================================================================================

#define MAX_ANIMATIONS 20

typedef enum GameState {
    STATE_PLAY,
    STATE_MENU,
    STATE_SETTINGS,
    STATE_DRAFT,
    STATE_DECK,
    STATE_COLLECTION,
} GameState;

int gamestate = STATE_MENU;
bool pause = false;

typedef enum AnimTypes {
    ANIM_BLANK,

    ANIM_TRANSITION_1, // slide from both sides
    ANIM_TRANSITION_2, // slide from left
    ANIM_TRANSITION_3, // slide from top

    ANIM_PRESENT_CARD, // card spins up from bottom

    ANIM_DRAFT_PHASE,
    ANIM_TRADE_PHASE,
} AnimTypes;

typedef struct Animation {
    int ID[MAX_ANIMATIONS];
    int targetState[MAX_ANIMATIONS];

    void (*atMidpoint[MAX_ANIMATIONS])(void); // callback
    void (*onComplete[MAX_ANIMATIONS])(void); // callback

    float x[MAX_ANIMATIONS];
    float y[MAX_ANIMATIONS];
    float value[MAX_ANIMATIONS];

    float runtime[MAX_ANIMATIONS];
    float maxtime[MAX_ANIMATIONS];

    bool isActive[MAX_ANIMATIONS];
} Animation; Animation anim;

typedef enum AspectRatio {
    AR_1920x1280,
    AR_1920x1080,
    AR_1920x1440,
    AR_1920x1200,

    AR_3_2,
    AR_16_9,
    AR_4_3,
    AR_16_10,

    TOTAL_ASPECTS,

} AspectRatio;

void set_aspect_ratio() {
    if (aspect_ratio == AR_1920x1280 || aspect_ratio == AR_3_2) { WIDTH_RATIO = 3; HEIGHT_RATIO = 2; }
    if (aspect_ratio == AR_1920x1080 || aspect_ratio == AR_16_9) { WIDTH_RATIO = 16; HEIGHT_RATIO = 9; }
    if (aspect_ratio == AR_1920x1440 || aspect_ratio == AR_4_3) { WIDTH_RATIO = 4; HEIGHT_RATIO = 3; }
    if (aspect_ratio == AR_1920x1200 || aspect_ratio == AR_16_10) { WIDTH_RATIO = 16; HEIGHT_RATIO = 10; }
}

void adjust_window_size() {
    // scaled down by 10/16 and then subrtract 75 from the height

    WINDOW_WIDTH = 800.0f / HEIGHT_RATIO * WIDTH_RATIO;
    WINDOW_HEIGHT = 800.0f;

    // get window sizes and set window scaling x/y coefficients
    SDL_GetWindowSize(window, &window_width, &window_height);
    window_scale_x = window_width / WINDOW_WIDTH;  
    window_scale_y = window_height / WINDOW_HEIGHT;

    isFullscreen = (SDL_GetWindowFlags(window) & (SDL_WINDOW_FULLSCREEN | SDL_WINDOW_BORDERLESS)) != 0;
}

// COMMON FUNCTIONS ====================================================================================================

#define point_box_collision(px, py, bx, by, bw, bh) (px >= bx && px <= bx + bw && py >= by && py <= by + bh)

float* floatarr(int num, ...) {
	va_list args;
	float* combined_array = (float*)malloc(num * sizeof(float));
	int count = 0;
	va_start(args, num);
	for (int i = 0; i < num; i++) combined_array[count++] = va_arg(args, double);
	va_end(args);

	return combined_array;
}

char* stringf(const char* format, ...) { //converts a data type to a string ("%d", x)
    static char output[128];
    va_list args;
    va_start(args, format);
    vsprintf(output, format, args);
    va_end(args);
    return output;
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
        cards->isHover[i] = false;

        cards->rarity[i] = 0;
    }

    cards->num = 0;

    return;
}

void clear_draft() {
    
    for (int i = 0; i < MAX_CARDS; i++) {
        
        // DELET CARD AT INDEX
        // -------------------

        draftzones.num_cards[indraft.zoneID[i]] -= 1;
        indraft.ID[i] = -1;

        indraft.x[i] = 0;
        indraft.y[i] = 0;
        indraft.vx[i] = 0;
        indraft.vy[i] = 0;
        indraft.tx[i] = 0;
        indraft.ty[i] = 0;
        indraft.w[i] = CARD_WIDTH;
        indraft.h[i] = CARD_HEIGHT;

        indraft.zoneID[i] = -1;
        indraft.zoneNum[i] = 0;

        indraft.isDragging[i] = false;
        indraft.isActive[i] = false;
        indraft.isSellable[i] = false;
        indraft.isHover[i] = false;

        indraft.rarity[i] = 0;

    }

    return;
}

void add_to_deck(int id, int rarity) {

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
    
    cardsunlocked[id] = true;
    // animation
    
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

    indeck.rarity[index] = rarity;
    
    indeck.num += 1;

    return;
}

void add_to_hand(int id, int rarity) {

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

    inplay.x[index] = (WINDOW_WIDTH/2);
    inplay.y[index] = ((playzones.y[ZONE_EVENT] + playzones.h[ZONE_EVENT]/2) + WINDOW_HEIGHT);
    inplay.w[index] = CARD_WIDTH;
    inplay.h[index] = CARD_HEIGHT;

    playzones.num_cards[ZONE_HAND] += 1;
    inplay.zoneID[index] = ZONE_HAND;
    inplay.zoneNum[index] = playzones.num_cards[ZONE_HAND];
    inplay.zoneTime[index] = 0;

    inplay.isDragging[index] = false;
    inplay.isActive[index] = true;
    inplay.isSellable[index] = true;

    inplay.rarity[index] = rarity;
    
    inplay.num += 1;

    return;
}

void deck_to_hand() {

    if (playzones.num_cards[ZONE_HAND] >= playzones.max_cards[ZONE_HAND]) return; // check if there is any space in your hand

    if (profileinfo.money[profile] < profileinfo.extraprice[profile] + profileinfo.extrainflation[profile]*profileinfo.extracount[profile]) return; // checks if you have enough money to draw a card

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
    inplay.zoneTime[inplayIndex] = 0;

    isDragging = true;
    inplay.isDragging[inplayIndex] = true;
    inplay.isActive[inplayIndex] = true;
    inplay.isSellable[inplayIndex] = indeck.isSellable[indeckIndex];

    inplay.rarity[inplayIndex] = indeck.rarity[indeckIndex];
    
    inplay.num += 1;

    profileinfo.money[profile] -= profileinfo.extraprice[profile] + profileinfo.extrainflation[profile]*profileinfo.extracount[profile];
    profileinfo.extracount[profile] += 1;

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

    indeck.rarity[indeckIndex] = 0;
    
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
    
    cardsunlocked[inplay.ID[inplayIndex]] = true;
    // animation

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

    indeck.rarity[indeckIndex] = inplay.rarity[inplayIndex];
    
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

    inplay.rarity[inplayIndex] = 0;
    
    inplay.num -= 1;

    return;
}

void event_card(int id, int rarity) {

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

    inplay.x[inplayIndex] = (playzones.x[ZONE_EVENT] + playzones.w[ZONE_EVENT]/2)  * window_scale_x;
    inplay.y[inplayIndex] = ((playzones.y[ZONE_EVENT] + playzones.h[ZONE_EVENT]/2) - 1000)  * window_scale_y;
    inplay.w[inplayIndex] = CARD_WIDTH;
    inplay.h[inplayIndex] = CARD_HEIGHT;

    playzones.num_cards[ZONE_EVENT] += 1;
    inplay.zoneID[inplayIndex] = ZONE_EVENT;
    inplay.zoneNum[inplayIndex] = playzones.num_cards[ZONE_EVENT];
    inplay.zoneTime[inplayIndex] = 0;

    inplay.isDragging[inplayIndex] = false;
    inplay.isActive[inplayIndex] = true;
    inplay.isSellable[inplayIndex] = false;

    inplay.rarity[inplayIndex] = rarity;
    
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

    inplay.x[index] = (WINDOW_WIDTH/2) * window_scale_x;
    inplay.y[index] = ((playzones.y[ZONE_EVENT] + playzones.h[ZONE_EVENT]/2) + 1000) * window_scale_y;
    inplay.w[index] = CARD_WIDTH;
    inplay.h[index] = CARD_HEIGHT;

    playzones.num_cards[ZONE_HAND] += 1;
    inplay.zoneID[index] = ZONE_HAND;
    inplay.zoneNum[index] = playzones.num_cards[ZONE_HAND];
    inplay.zoneTime[index] = 0;

    inplay.isDragging[index] = false;
    inplay.isActive[index] = true;
    inplay.isSellable[index] = true;

    inplay.rarity[index] = 0;
    
    inplay.num += 1;
    profileinfo.loans[profile] -= 1;

    return;
}

void draft_card(int id, int rarity) {
    
    // if (draftzones.num_cards[ZONE_DRAFT_HAND] >= draftzones.max_cards[ZONE_DRAFT_HAND]) return; // chack if there is any space in your hand

    // FIND IF AND WHERE AN DRAFT CARD SLOT IS OPEN
    // -------------------------------------
    int index = -1;
    for (int i = 0; i < MAX_CARDS; i++) {
        if (!indraft.isActive[i]) {
            index = i;
            break;
        }
    }
    if (index == -1) return;

    // ACTIVATE NEW CARD WITH ID AT THAT INDEX
    // ---------------------------------------
    
    indraft.ID[index] = id;

    indraft.x[index] = (WINDOW_WIDTH/2) * window_scale_x;
    indraft.y[index] = (-WINDOW_HEIGHT) * window_scale_y;
    indraft.w[index] = CARD_WIDTH;
    indraft.h[index] = CARD_HEIGHT;

    draftzones.num_cards[ZONE_DRAFT_HAND] += 1;
    indraft.zoneID[index] = ZONE_DRAFT_HAND;
    indraft.zoneNum[index] = draftzones.num_cards[ZONE_DRAFT_HAND];
    indraft.zoneTime[index] = 0;

    indraft.isDragging[index] = false;
    indraft.isActive[index] = true;
    indraft.isSellable[index] = true;

    indraft.rarity[index] = rarity;
    
    indraft.num += 1;

    return;
}

void menu_card(int id, int rarity) {
    
    if (menuzone.num_cards[ZONE_MENU] >= menuzone.max_cards[ZONE_MENU]) return; // check if there is any space in your hand

    // FIND IF AND WHERE AN IN PLAY SLOT IS OPEN
    // -------------------------------------
    int index = -1;
    for (int i = 0; i < MAX_CARDS; i++) {
        if (!infeature.isActive[i]) {
            index = i;
            break;
        }
    }
    if (index == -1) return;

    // ACTIVATE NEW CARD WITH ID AT THAT INDEX
    // ---------------------------------------
    
    infeature.ID[index] = id;

    infeature.x[index] = (WINDOW_WIDTH/2);
    infeature.y[index] = ((menuzone.y[ZONE_MENU] + menuzone.h[ZONE_MENU]/2) + WINDOW_HEIGHT*2);
    infeature.w[index] = CARD_WIDTH;
    infeature.h[index] = CARD_HEIGHT;

    menuzone.num_cards[ZONE_MENU] += 1;
    infeature.zoneID[index] = ZONE_MENU;
    infeature.zoneNum[index] = menuzone.num_cards[ZONE_MENU];
    infeature.zoneTime[index] = 0;

    infeature.isDragging[index] = false;
    infeature.isActive[index] = true;
    infeature.isSellable[index] = true;

    infeature.rarity[index] = rarity;
    
    inplay.num += 1;
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
    if (inplay.ID[inplayIndex] == 0 && profileinfo.money[profile] < 100) return; // checks if you have enough money to sell a loan

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

    inplay.rarity[inplayIndex] = 0;
    
    inplay.num -= 1;
                            
    return;
}

// ANIMATION FUNCTIONS ====================================================================================================

void start_animation(int id, void (*at_midpoint)(void), void (*on_complete)(void), int targetstate, float x, float y, float value, float seconds) {

    for (int i = 0; i < MAX_ANIMATIONS; i++) { // duplicate transitions
        if (anim.ID[i] == ANIM_TRANSITION_1 && anim.isActive[i] && id == ANIM_TRANSITION_1) return;
        if (anim.ID[i] == ANIM_TRANSITION_2 && anim.isActive[i] && id == ANIM_TRANSITION_2) return;
        if (anim.ID[i] == ANIM_TRANSITION_3 && anim.isActive[i] && id == ANIM_TRANSITION_3) return;
    }

    int index = -1;
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        if (!anim.isActive[i]) {
            index = i;
            break;
        }
    }
    if (index == -1) return;

    anim.ID[index] = id;
    anim.targetState[index] = targetstate;

    anim.atMidpoint[index] = at_midpoint;
    anim.onComplete[index] = on_complete;
    
    anim.x[index] = x;
    anim.y[index] = y;
    anim.value[index] = value;

    anim.runtime[index] = 0;
    anim.maxtime[index] = seconds;

    anim.isActive[index] = true;

    return;
}

void end_animation(int index) {

    anim.ID[index] = -1;
    anim.targetState[index] = -1;

    anim.onComplete[index] = NULL;
    anim.atMidpoint[index] = NULL;
    
    anim.x[index] = 0;
    anim.y[index] = 0;
    anim.value[index] = 0;

    anim.runtime[index] = 0;
    anim.maxtime[index] = 0;

    anim.isActive[index] = false;
}

void callback_change_to_play_screen() { gamestate = STATE_PLAY; state_timer = 0; 
    start_animation(ANIM_BLANK, NULL, deck_to_hand, -1, 0, 0, 0, 3.0f);
    start_animation(ANIM_BLANK, NULL, deck_to_hand, -1, 0, 0, 0, 3.25f);
    start_animation(ANIM_BLANK, NULL, deck_to_hand, -1, 0, 0, 0, 3.5f);
}
void callback_change_to_menu_screen() { gamestate = STATE_MENU; state_timer = 0; }
void callback_change_to_settings_screen() { gamestate = STATE_SETTINGS; state_timer = 0; }
void callback_change_to_deck_screen() { shuffle_hand(); gamestate = STATE_DECK; state_timer = 0; }
void callback_change_to_collection_screen() { gamestate = STATE_COLLECTION; state_timer = 0; }

void callback_change_to_draft_screen() { gamestate = STATE_DRAFT; state_timer = 0; clear_draft(); }
void callback_random_draft_cards() { for (int i = 0; i < 3; i++) { draft_card(rand() % TOTAL_CARDS, random_rarity()); } }
void callback_select_draft_card() { 
    shuffle_hand();
    gamestate = STATE_DECK; 
    state_timer = 0; 
    for (int i = 0; i < MAX_CARDS; i++) {
        if (!indraft.isActive[i]) continue;
        if (indraft.zoneID[i] == ZONE_DRAFT_SELECT) {
            add_to_deck(indraft.ID[i], indraft.rarity[i]);
        }
    }
    clear_draft(); 
}

void callback_quit_game() { running = false; }
void callback_start_new_run() {}

// ROUND FUNCTIONS ====================================================================================================

void new_run() {
    // clear deck
    // clear stats
    // reset other stats
    // 
}

void new_round() {
    // inc round num
    // reset extra draw price
    // reset stock
    // shuffle deck
    // switch to STATE_PLAY
    // READY! SET! TRADE animation
    // draw your number of cards (start_animation)
}

void reset_stock() {
    
    stonk.time_started = SDL_GetTicks() / 1000.0f;
    for (int i = 0; i < 60; i++) {
        stonk.h[i] = 0;
        stonk.o[60] = 0;
        stonk.c[60] = 0;
        stonk.l[60] = 0;
        stonk.integrity = 1.0f;
    }
}

void update_stonk() {
    float price = 10;
    for (int i = 0; i < 60; i++) {
        float delta = ((rand() % 2001) - 1000) / 1000.0f; // -10.0 to +10.0
        price += delta;
        stonk.c[i] = price;
        stonk.o[i] = stonk.c[i-1];
        stonk.h[i] = price+1;
        stonk.l[i] = price-1;
    }
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

void make_zone(Zones *zone, ZoneType type, const char* zonepath, int slots, int x, int y, int w, int h) {
    strcpy(zone->zonepath[type], zonepath);

    zone->max_cards[type] = slots;
    zone->x[type] = x * window_scale_x;
    zone->y[type] = y * window_scale_y;
    if (w != 0) {
        zone->w[type] = w * window_scale_x;
    } else {
        zone->w[type] = (CARD_SPACING + (CARD_WIDTH + CARD_SPACING) * slots) * window_scale_x;
    }

    if (h != 0) {
        zone->h[type] = h * window_scale_y;
    } else {
        zone->h[type] = (CARD_HEIGHT + CARD_SPACING * 2) * window_scale_y;
    }
    
    zone->isActive[type] = true;
}

void make_button(Buttons *button, ButtonType type, const char* buttonpath, int x, int y, int w, int h) {
    strcpy(button->buttonpath[type], buttonpath);

    button->ID[type] = type;
    button->x[type] = x * window_scale_x;
    button->y[type] = y * window_scale_y;
    button->w[type] = w * window_scale_x;
    button->h[type] = h * window_scale_y;

    button->isActive[type] = true;
    button->isClicked[type] = false;
    button->isPressed[type] = false;
    // button->
    // gamebuttons.[button] = ;
}

void load_zones() {

    // original prototype
    // make_zone(ZONE_SELL, "", 1, (WINDOW_WIDTH-CARD_MARGIN-CARD_WIDTH-CARD_SPACING), CARD_MARGIN, 0, 0);
    // make_zone(ZONE_HAND, "", 9, CARD_MARGIN, (WINDOW_HEIGHT-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN), 0, 0);
    // make_zone(ZONE_EQUIP_1, "", 1, (WINDOW_WIDTH-CARD_MARGIN-CARD_WIDTH-CARD_SPACING-CARD_MARGIN-CARD_WIDTH-CARD_SPACING), CARD_MARGIN, 0, 0);
    // make_button(BUTTON_DECK, "./resources/textures/deck.png", CARD_MARGIN, CARD_MARGIN, CARD_WIDTH+CARD_SPACING, CARD_HEIGHT+CARD_SPACING);

    // new version
    // make_button(BUTTON_DECK, "./resources/textures/deck.png", CARD_MARGIN, WINDOW_HEIGHT-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, CARD_WIDTH+CARD_SPACING, CARD_HEIGHT+CARD_SPACING);
    // make_zone(ZONE_HAND, "", profileinfo.handslots[profile], CARD_MARGIN+CARD_SPACING+CARD_WIDTH+CARD_MARGIN, WINDOW_HEIGHT-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, 0, 0);
    // make_zone(ZONE_SELL, "", 1, WINDOW_WIDTH-CARD_MARGIN-CARD_SPACING-CARD_WIDTH, WINDOW_HEIGHT-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, 0, 0);
    // make_zone(ZONE_EQUIP_1, "", 1, CARD_MARGIN, CARD_MARGIN, 0, 0);
    // make_zone(ZONE_EQUIP_2, "", 1, CARD_MARGIN+CARD_WIDTH+CARD_SPACING+CARD_MARGIN, CARD_MARGIN, 0, 0);

    // pre idea change version
    // make_zone(ZONE_HAND, "", profileinfo.handslots[profile], CARD_MARGIN, CARD_MARGIN, 0, 0);
    // make_zone(ZONE_SELL, "", 1, WINDOW_WIDTH-CARD_MARGIN-CARD_SPACING-CARD_WIDTH, CARD_MARGIN, 0, 0);
    // make_zone(ZONE_EQUIP_1, "", 1, WINDOW_WIDTH-CARD_MARGIN-CARD_SPACING-CARD_WIDTH, WINDOW_HEIGHT-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, 0, 0);
    // make_zone(ZONE_EVENT, "", 1, WINDOW_WIDTH-CARD_MARGIN-CARD_SPACING-CARD_WIDTH-CARD_MARGIN-CARD_WIDTH, WINDOW_HEIGHT-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, 0, 0);
    // make_button(BUTTON_DECK, "./resources/textures/deck.png", CARD_MARGIN, WINDOW_HEIGHT-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, CARD_WIDTH+CARD_SPACING, CARD_HEIGHT+CARD_SPACING);

    // // post idea change version
    // make_zone(ZONE_SELL, "", 1, WINDOW_WIDTH-CARD_MARGIN-CARD_SPACING-CARD_WIDTH, WINDOW_HEIGHT-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, 0, 0);
    // make_zone(ZONE_HAND, "", profileinfo.handslots[profile], 210, WINDOW_HEIGHT-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, 1080, 0);
    // make_zone(ZONE_EVENT, "", 1, WINDOW_WIDTH-CARD_MARGIN-CARD_SPACING-CARD_WIDTH, CARD_MARGIN, 0, 0);
    // make_button(BUTTON_DECK, "./resources/textures/deck.png", CARD_MARGIN, WINDOW_HEIGHT-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, CARD_WIDTH+CARD_SPACING, CARD_HEIGHT+CARD_SPACING);
    // make_button(BUTTON_LOAN, "./resources/textures/deck.png", CARD_MARGIN, 500, 100, 50);

    // somewhat planned version
    make_zone(&playzones, ZONE_SELL, "", 1, WINDOW_WIDTH-CARD_MARGIN-CARD_SPACING-CARD_WIDTH, WINDOW_HEIGHT-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, 0, 0);
    make_zone(&playzones, ZONE_HAND, "", profileinfo.handslots[profile], WINDOW_WIDTH/2 - 725/2, WINDOW_HEIGHT-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, 725, 0);
    make_zone(&playzones, ZONE_EVENT, "", 1, WINDOW_WIDTH-CARD_MARGIN-CARD_SPACING-CARD_WIDTH, CARD_MARGIN, 0, 0);
    make_button(&gamebuttons, BUTTON_DECK, "resources/textures/play-button-deck.png", CARD_MARGIN, WINDOW_HEIGHT-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, CARD_WIDTH+CARD_SPACING, CARD_HEIGHT+CARD_SPACING);
    make_button(&gamebuttons, BUTTON_LOAN, "", 15, CARD_MARGIN+130, (CARD_MARGIN+CARD_SPACING+CARD_WIDTH+CARD_MARGIN-15-15), 50);
    make_button(&gamebuttons, BUTTON_PAUSE, "resources/textures/play-button-pause.png", 15, 15, 30, 30);
    make_button(&gamebuttons, BUTTON_SELL_STOCK, "resources/textures/play-button-minus.png", 15, CARD_MARGIN+195, 50, 50);
    make_button(&gamebuttons, BUTTON_BUY_STOCK, "resources/textures/play-button-plus.jpg", 175, CARD_MARGIN+195, 50, 50);
    // MENU
    make_button(&menubuttons, BUTTON_NEW_GAME, "resources/textures/menu-button-new-game.png", (WINDOW_WIDTH/2)-(300/2), (WINDOW_HEIGHT/2)+(50+20)*0, 300, 50);
    make_button(&menubuttons, BUTTON_SETTINGS, "resources/textures/menu-button-settings.png", (WINDOW_WIDTH/2)-(300/2), (WINDOW_HEIGHT/2)+(50+20)*1, 300, 50);
    make_button(&menubuttons, BUTTON_STATS, "", (WINDOW_WIDTH/2)-(300/2), (WINDOW_HEIGHT/2)+(50+20)*2, 300, 50);
    make_button(&menubuttons, BUTTON_QUIT, "resources/textures/menu-button-quit.png", (WINDOW_WIDTH/2)-(300/2), (WINDOW_HEIGHT/2)+(50+20)*3, 300, 50);
    make_zone(&menuzone, ZONE_MENU, "", 1, (WINDOW_WIDTH/2)-((CARD_WIDTH+CARD_SPACING)/2), (WINDOW_HEIGHT/4)-((CARD_HEIGHT+CARD_SPACING)/2), 0, 0);
    // SETTINGS
    make_button(&settingbuttons, BUTTON_SETTINGS_EXIT, "resources/textures/play-button-deck.png", 15, 15, 30, 30);
    make_button(&settingbuttons, BUTTON_SETTINGS_ASPECT_RATIO, "resources/textures/play-button-minus.png", (WINDOW_WIDTH/2)-(250/2), CARD_SPACING+(40+CARD_SPACING)*2, 250, 40);
    make_button(&settingbuttons, BUTTON_SETTINGS_ASPECT_PREV, "resources/textures/general-left.png", (WINDOW_WIDTH/2)-(250/2)-CARD_SPACING-20, CARD_SPACING+(40+CARD_SPACING)*2, 20, 40);
    make_button(&settingbuttons, BUTTON_SETTINGS_ASPECT_NEXT, "resources/textures/general-right.png", (WINDOW_WIDTH/2)+(250/2)+CARD_SPACING, CARD_SPACING+(40+CARD_SPACING)*2, 20, 40);
    make_button(&settingbuttons, BUTTON_SETTINGS_FULLSCREEN, "resources/textures/play-button-minus.png", (WINDOW_WIDTH/2)-(250/2), CARD_SPACING+(40+CARD_SPACING)*4, 250, 40);
    make_button(&settingbuttons, BUTTON_SETTINGS_SPEED_PREV, "resources/textures/general-left.png", (WINDOW_WIDTH/2)-((40)/2)-CARD_SPACING-20, CARD_SPACING+(40+CARD_SPACING)*6, 20, 40);
    make_button(&settingbuttons, BUTTON_SETTINGS_SPEED_NEXT, "resources/textures/general-right.png", (WINDOW_WIDTH/2)+(40/2)+CARD_SPACING, CARD_SPACING+(40+CARD_SPACING)*6, 20, 40);
    make_button(&settingbuttons, BUTTON_SETTINGS_HITBOXES, "resources/textures/play-button-minus.png", (WINDOW_WIDTH/2)-(250/2), CARD_SPACING+(40+CARD_SPACING)*7, 250, 40);
    make_button(&settingbuttons, BUTTON_SETTINGS_CURSOR, "resources/textures/play-button-minus.png", (WINDOW_WIDTH/2)-(250/2), CARD_SPACING+(40+CARD_SPACING)*8, 250, 40);
    make_button(&settingbuttons, BUTTON_SETTINGS_FPS_PREV, "resources/textures/general-left.png", (WINDOW_WIDTH/2)-((100)/2)-CARD_SPACING-20, CARD_SPACING+(40+CARD_SPACING)*10, 20, 40);
    make_button(&settingbuttons, BUTTON_SETTINGS_FPS_NEXT, "resources/textures/general-right.png", (WINDOW_WIDTH/2)+(100/2)+CARD_SPACING, CARD_SPACING+(40+CARD_SPACING)*10, 20, 40);
    // DRAFT
    make_button(&draftbuttons, BUTTON_DRAFT_CONTINUE, "resources/textures/general-right.png", WINDOW_WIDTH-150-CARD_MARGIN, WINDOW_HEIGHT-30-CARD_MARGIN, 150, 40);
    make_zone(&draftzones, ZONE_DRAFT_HAND, "", 5, WINDOW_WIDTH/2 - 725/2, CARD_SPACING, 0, 0);
    make_zone(&draftzones, ZONE_DRAFT_SELECT, "", 1, WINDOW_WIDTH/2 - (CARD_MARGIN+CARD_WIDTH)/2, WINDOW_HEIGHT-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN, 0, 0);
    // DECK
    make_button(&deckbuttons, BUTTON_DECK_CONTINUE, "resources/textures/general-right.png", WINDOW_WIDTH-150-CARD_MARGIN, WINDOW_HEIGHT-30-CARD_MARGIN, 150, 40);
    // COLLECTION
    make_button(&collectionbuttons, BUTTON_COLLECTION_CONTINUE, "resources/textures/general-right.png", WINDOW_WIDTH-150-CARD_MARGIN, WINDOW_HEIGHT-30-CARD_MARGIN, 150, 40);
    
}

void load_cards() {
    
    // LOAD CARDS
    // ----------
    make_card(0, "resources/textures/card1.png", "$100 Loan", "pay off your loans to validate your run as speedrun eligible", floatarr(6, -100.0f, 4, 3, 2, 1, 1));
    make_card(1, "resources/textures/card2.png", "Flame Boy", "For every wooden card you have in your hand, draw an extra card", floatarr(6, 5.0f, 4, 3, 2, 1, 1));
    make_card(2, "resources/textures/card3.png", "Peaked", "If you are visited by the russians, you recieve $200", floatarr(6, 5.0f, 4, 3, 2, 1, 1));
    make_card(3, "resources/textures/card4.png", "Eg", "WTF you think it is, its an egg", floatarr(6, 5.0f, 4, 3, 2, 1, 1));
    make_card(4, "resources/textures/card5.png", "Ballot Joker", "At any given time, your stock price is multiplied by a random number", floatarr(6, 5.0f, 4, 3, 2, 1, 1));
    make_card(5, "resources/textures/card6.png", "Pokemon Card Joker", "Gotta Catch em all", floatarr(6, 5.0f, 4, 3, 2, 1, 1));
    make_card(6, "resources/textures/card7.png", "Bunanu", "Use as a consumable to refill 1 loan slot", floatarr(6, 5.0f, 4, 3, 2, 1, 1));
    make_card(7, "resources/textures/card8.png", "Caino", "He got ice running throuhg his viens", floatarr(6, 5.0f, 4, 3, 2, 1, 1));
    make_card(8, "resources/textures/card9.png", "Purple Magic", "Any damage you do to tech stonks if multiplied by 3", floatarr(6, 5.0f, 4, 3, 2, 1, 1));
    make_card(9, "resources/textures/card10.png", "Planet Joker", "You now own the entire planet", floatarr(6, 5.0f, 4, 3, 2, 1, 1));
    make_card(10, "resources/textures/card11.png", "Normal Joker", "Sherrel, nobody cares", floatarr(6, 5.0f, 4, 3, 2, 1, 1));

    // TEMPORARY
    // ----------
    for (int i = 11; i < TOTAL_CARDS; i++ ) {
        make_card(i, "resources/textures/card1.png", "$100 Loan", "pay off your loans to validate your run as speedrun eligible", floatarr(6, -100.0f, 4, 3, 2, 1, 1));
    }

}

SDL_Texture* texture_idea;
SDL_Texture* texture_dim1;
SDL_Texture* texture_cardlock;
SDL_Texture* texture_event;

SDL_Texture* texture_rarity[10];

TTF_Font* fontBalatro;
TTF_Font* fontTimesNewRoman;
TTF_Font* fontDSDigital;

bool load_textures() {

    // LOAD TEXTURES FOR CARDS
    // -----------------------
    for (int i = 0; i < TOTAL_CARDS; i++) {
        cards.cardtexture[i] = IMG_LoadTexture(renderer, cards.cardpath[i]);
        SDL_SetTextureScaleMode(cards.cardtexture[i], SDL_SCALEMODE_NEAREST);
    }
    texture_cardlock = IMG_LoadTexture(renderer, "resources/textures/card-lock.png");
    SDL_SetTextureScaleMode(texture_cardlock, SDL_SCALEMODE_NEAREST);
    texture_event = IMG_LoadTexture(renderer, "resources/textures/card-event.png");
    SDL_SetTextureScaleMode(texture_event, SDL_SCALEMODE_NEAREST);
    
    // DIM OVERLAYS
    // ------------
    texture_dim1 = IMG_LoadTexture(renderer, "resources/textures/dim1.png");
    SDL_SetTextureScaleMode(texture_dim1, SDL_SCALEMODE_NEAREST);

    // RARITIES
    // --------
    texture_rarity[1] = IMG_LoadTexture(renderer, "resources/textures/card-rarity1.png");
    SDL_SetTextureScaleMode(texture_rarity[1], SDL_SCALEMODE_NEAREST);
    texture_rarity[2] = IMG_LoadTexture(renderer, "resources/textures/card-rarity2.png");
    SDL_SetTextureScaleMode(texture_rarity[2], SDL_SCALEMODE_NEAREST);
    texture_rarity[3] = IMG_LoadTexture(renderer, "resources/textures/card-rarity3.png");
    SDL_SetTextureScaleMode(texture_rarity[3], SDL_SCALEMODE_NEAREST);
    texture_rarity[4] = IMG_LoadTexture(renderer, "resources/textures/card-rarity4.png");
    SDL_SetTextureScaleMode(texture_rarity[4], SDL_SCALEMODE_NEAREST);

    // LOAD TEXTURES FOR BUTTONS
    // -----------------------
    for (int i = 0; i < MAX_BUTTONS; i++) {
        gamebuttons.buttontexture[i] = IMG_LoadTexture(renderer, gamebuttons.buttonpath[i]);
        SDL_SetTextureScaleMode(gamebuttons.buttontexture[i], SDL_SCALEMODE_NEAREST);
        menubuttons.buttontexture[i] = IMG_LoadTexture(renderer, menubuttons.buttonpath[i]);
        SDL_SetTextureScaleMode(menubuttons.buttontexture[i], SDL_SCALEMODE_NEAREST);
        settingbuttons.buttontexture[i] = IMG_LoadTexture(renderer, settingbuttons.buttonpath[i]);
        SDL_SetTextureScaleMode(settingbuttons.buttontexture[i], SDL_SCALEMODE_NEAREST);
        draftbuttons.buttontexture[i] = IMG_LoadTexture(renderer, draftbuttons.buttonpath[i]);
        SDL_SetTextureScaleMode(draftbuttons.buttontexture[i], SDL_SCALEMODE_NEAREST);
        deckbuttons.buttontexture[i] = IMG_LoadTexture(renderer, deckbuttons.buttonpath[i]);
        SDL_SetTextureScaleMode(deckbuttons.buttontexture[i], SDL_SCALEMODE_NEAREST);
        collectionbuttons.buttontexture[i] = IMG_LoadTexture(renderer, collectionbuttons.buttonpath[i]);
        SDL_SetTextureScaleMode(collectionbuttons.buttontexture[i], SDL_SCALEMODE_NEAREST);
    }

    // LOAD TEXTURES FOR ZONES
    // -----------------------
    for (int i = 0; i < MAX_ZONES; i++) {
        playzones.zonetexture[i] = IMG_LoadTexture(renderer, playzones.zonepath[i]);
        SDL_SetTextureScaleMode(playzones.zonetexture[i], SDL_SCALEMODE_NEAREST);
        menuzone.zonetexture[i] = IMG_LoadTexture(renderer, menuzone.zonepath[i]);
        SDL_SetTextureScaleMode(menuzone.zonetexture[i], SDL_SCALEMODE_NEAREST);
    }

    
    texture_idea = IMG_LoadTexture(renderer, "resources/idea.png");
    SDL_SetTextureScaleMode(texture_idea, SDL_SCALEMODE_NEAREST);

    fontBalatro= TTF_OpenFont("resources/fonts/balatro.ttf", 24);
    fontTimesNewRoman = TTF_OpenFont("resources/fonts/Times New Roman.ttf", 24);
    fontDSDigital = TTF_OpenFont("resources/fonts/DS-DIGII.ttf", 24);
    
	return true;
}

void setup() {
    
    seed = time(NULL);
    // seed = 0;
    srand(seed);

    const Uint8* sdlKeys = (Uint8*)SDL_GetKeyboardState(NULL);
    SDL_memcpy(currKeyState, sdlKeys, NUM_KEYS);
    SDL_memcpy(prevKeyState, sdlKeys, NUM_KEYS);

    make_rarity_table(67, 20, 9, 3, 1);
    reset_unlocked_cards();

    // SETUP CARDS, BUTTONS AND ZONES
    // ---------------------
    load_cards();
    load_zones();
    for (int i = 0; i < MAX_CARDS; i++) { clear_cards(&inplay); }
    for (int i = 0; i < MAX_CARDS; i++) { clear_cards(&indeck); }
    for (int i = 0; i < MAX_CARDS; i++) { clear_cards(&indraft); }
    for (int i = 0; i < MAX_CARDS; i++) { clear_cards(&infeature); }
    for (int i = 0; i < MAX_BUTTONS; i++) {
        gamebuttons.isActive[i] = false;
        gamebuttons.isPressed[i] = false;
        gamebuttons.ID[i] = -1;
    }

    // ADD CARDS TO DECK
    // ---------------------
    menu_card(rand() % TOTAL_CARDS, random_rarity());
    // for (int i = 0; i < 54; i++) { add_to_deck(rand() % TOTAL_CARDS, random_rarity()); }

    // INITIALIZE PROFILE INFO
    // -----------------------
    profile = 0;
    // used
    profileinfo.money[profile] = 100;
    profileinfo.handslots[profile] = 3;
    profileinfo.loans[profile] = 3;
    // not used
    profileinfo.extraprice[profile] = 7;
    profileinfo.extrainflation[profile] = 2;
    profileinfo.extracount[profile] = 0;

}

// UPDATE FUNCTIONS ====================================================================================================

void inputs() {
    while (SDL_PollEvent(&event) != 0) {
        if (event.type == SDL_EVENT_QUIT) running = false;
    }
    const Uint8* sdlKeys = (Uint8*)SDL_GetKeyboardState(NULL);
    SDL_memcpy(currKeyState, sdlKeys, NUM_KEYS);
    prevMouseState = currMouseState;
    currMouseState = SDL_GetMouseState(&mousex, &mousey);

    if (currKeyState[SDL_SCANCODE_ESCAPE] && !prevKeyState[SDL_SCANCODE_ESCAPE]) SDL_SetWindowFullscreen(window, 0);
    if (currKeyState[SDL_SCANCODE_TAB] && !prevKeyState[SDL_SCANCODE_TAB]) start_animation(ANIM_TRANSITION_1, callback_change_to_menu_screen, NULL, -1, 0, 0, 0, 2 * M_PI);

    adjust_window_size();
}

void dev_tools() {

    // 4 - shuffle hand into deck
    // --------------------------
    if (currKeyState[SDL_SCANCODE_4] && !prevKeyState[SDL_SCANCODE_4]) {}

    // 5 - spawn event card
    // --------------------
    if (currKeyState[SDL_SCANCODE_5] && !prevKeyState[SDL_SCANCODE_5]) {}

    // 6 - draw card id to hand
    // --------------------------
    if (currKeyState[SDL_SCANCODE_6] && !prevKeyState[SDL_SCANCODE_6]) {}

    // 7 - menu / unmenu
    // --------------------------
    if (currKeyState[SDL_SCANCODE_SPACE] && !prevKeyState[SDL_SCANCODE_SPACE]) {
        pause = !pause;
    }

}

void update() {
    
    load_zones();

    if (gamestate == STATE_DECK) {
        if (!pause) state_timer += dt;

        if (gamestate == STATE_DECK && state_timer*5 > (CARD_SPACING + (ceil((indeck.num/9)) - 3.5) * (CARD_HEIGHT+CARD_SPACING)) && state_timer > 10)
            state_timer = (CARD_SPACING + (ceil((indeck.num/9)) - 3.5) * (CARD_HEIGHT+CARD_SPACING))/5;

        if (currKeyState[SDL_SCANCODE_W] && state_timer > 0) {state_timer -= 50*dt; } 
        if (currKeyState[SDL_SCANCODE_S] && state_timer*5 < (CARD_SPACING + (ceil((indeck.num/9)) - 3.5) * (CARD_HEIGHT+CARD_SPACING))) {state_timer += 50*dt; }

    }

    if (gamestate == STATE_COLLECTION) {
        // if (!pause) state_timer += dt;

        if (gamestate == STATE_DECK && state_timer*5 > (CARD_SPACING + (ceil((TOTAL_CARDS/9)) - 3.5) * (CARD_HEIGHT+CARD_SPACING)) && state_timer > 10)
            state_timer = (CARD_SPACING + (ceil((TOTAL_CARDS/9)) - 3.5) * (CARD_HEIGHT+CARD_SPACING))/5;

        if (currKeyState[SDL_SCANCODE_W] && state_timer > 0) {state_timer -= 50*dt; } 
        if (currKeyState[SDL_SCANCODE_S] && state_timer*5 < (CARD_SPACING + (ceil((TOTAL_CARDS/9)) - 3.5) * (CARD_HEIGHT+CARD_SPACING))) {state_timer += 50*dt; }

    }

    if (gamestate == STATE_DECK || gamestate == STATE_COLLECTION) {
        // if (!pause) state_timer += dt;

        // if (state_timer*5 > (CARD_SPACING + (ceil((indeck.num/9)) - 3.5) * (CARD_HEIGHT+CARD_SPACING)) && state_timer > 10)
        // // start_animation(ANIM_TRANSITION_1, callback_change_to_draft_screen, callback_random_draft_cards, -1, 0, 0, 0, 2 * M_PI);
        // // start_animation(ANIM_TRANSITION_1, callback_change_to_play_screen, NULL, -1, 0, 0, 0, 2 * M_PI);
        // state_timer = (CARD_SPACING + (ceil((indeck.num/9)) - 3.5) * (CARD_HEIGHT+CARD_SPACING))/5;

        
        // if (currKeyState[SDL_SCANCODE_W] && state_timer > 0) {state_timer -= 50*dt; } 
        // if (currKeyState[SDL_SCANCODE_S] && state_timer*5 < (CARD_SPACING + (ceil((indeck.num/9)) - 3.5) * (CARD_HEIGHT+CARD_SPACING))) {state_timer += 50*dt; }






    } else {
        state_timer += dt;
    }




    
    // Draws an event card once every 10 seconds
    if (gamestate == STATE_PLAY && !pause) 
        event_timer += dt;
    if (event_timer > 10) { event_card((rand() % TOTAL_CARDS-1) + 1, random_rarity()); event_timer = 0; }


    // BUTTON UPDATES
    // --------------
    for (int i = 0; i < MAX_BUTTONS; i++) {
        if (!gamebuttons.isActive[i] && !menubuttons.isActive[i] && !settingbuttons.isActive[i] && !draftbuttons.isActive[i] && !deckbuttons.isActive[i] && !collectionbuttons.isActive[i]) continue;

        if (gamebuttons.isPressed[i]) gamebuttons.clickTime[i] = 0;
        if (menubuttons.isPressed[i]) menubuttons.clickTime[i] = 0;
        if (settingbuttons.isPressed[i]) settingbuttons.clickTime[i] = 0;
        if (draftbuttons.isPressed[i]) draftbuttons.clickTime[i] = 0;
        if (deckbuttons.isPressed[i]) deckbuttons.clickTime[i] = 0;
        if (collectionbuttons.isPressed[i]) collectionbuttons.clickTime[i] = 0;

        gamebuttons.clickTime[i] += dt / game_speed;
        menubuttons.clickTime[i] += dt / game_speed;
        settingbuttons.clickTime[i] += dt / game_speed;
        draftbuttons.clickTime[i] += dt / game_speed;
        deckbuttons.clickTime[i] += dt / game_speed;
        collectionbuttons.clickTime[i] += dt / game_speed;

        // CHECK BUTTON CLICKS
        // -------------------

        if (point_box_collision(mousex, mousey, gamebuttons.x[i], gamebuttons.y[i], gamebuttons.w[i], gamebuttons.h[i]) && gamestate == STATE_PLAY && gamebuttons.isActive[i] && !isDragging) {
            if (currMouseState & SDL_BUTTON_LMASK) { gamebuttons.isPressed[i] = true; } else { gamebuttons.isPressed[i] = false; }
            if ((currMouseState & SDL_BUTTON_LMASK) && !(prevMouseState & SDL_BUTTON_LMASK)) { gamebuttons.isClicked[i] = true; } else { gamebuttons.isClicked[i] = false; }
            gamebuttons.isHover[i] = true;
        } else {
            gamebuttons.isPressed[i] = false;
            gamebuttons.isClicked[i] = false;
            gamebuttons.isHover[i] = false;
        }

        if (point_box_collision(mousex, mousey, menubuttons.x[i], menubuttons.y[i], menubuttons.w[i], menubuttons.h[i]) && gamestate == STATE_MENU && menubuttons.isActive[i] && !isDragging) {
            if (currMouseState & SDL_BUTTON_LMASK) { menubuttons.isPressed[i] = true; } else { menubuttons.isPressed[i] = false; }
            if ((currMouseState & SDL_BUTTON_LMASK) && !(prevMouseState & SDL_BUTTON_LMASK)) { menubuttons.isClicked[i] = true; } else { menubuttons.isClicked[i] = false; }
            menubuttons.isHover[i] = true;
        } else {
            menubuttons.isPressed[i] = false;
            menubuttons.isClicked[i] = false;
            menubuttons.isHover[i] = false;
        }

        if (point_box_collision(mousex, mousey, settingbuttons.x[i], settingbuttons.y[i], settingbuttons.w[i], settingbuttons.h[i]) && gamestate == STATE_SETTINGS && settingbuttons.isActive[i] && !isDragging) {
            if (currMouseState & SDL_BUTTON_LMASK) { settingbuttons.isPressed[i] = true; } else { settingbuttons.isPressed[i] = false; }
            if ((currMouseState & SDL_BUTTON_LMASK) && !(prevMouseState & SDL_BUTTON_LMASK)) { settingbuttons.isClicked[i] = true; } else { settingbuttons.isClicked[i] = false; }
            settingbuttons.isHover[i] = true;
        } else {
            settingbuttons.isPressed[i] = false;
            settingbuttons.isClicked[i] = false;
            settingbuttons.isHover[i] = false;
        }

        if (point_box_collision(mousex, mousey, draftbuttons.x[i], draftbuttons.y[i], draftbuttons.w[i], draftbuttons.h[i]) && gamestate == STATE_DRAFT && draftbuttons.isActive[i] && !isDragging) {
            if (currMouseState & SDL_BUTTON_LMASK) { draftbuttons.isPressed[i] = true; } else { draftbuttons.isPressed[i] = false; }
            if ((currMouseState & SDL_BUTTON_LMASK) && !(prevMouseState & SDL_BUTTON_LMASK)) { draftbuttons.isClicked[i] = true; } else { draftbuttons.isClicked[i] = false; }
            draftbuttons.isHover[i] = true;
        } else {
            draftbuttons.isPressed[i] = false;
            draftbuttons.isClicked[i] = false;
            draftbuttons.isHover[i] = false;
        }

        if (point_box_collision(mousex, mousey, deckbuttons.x[i], deckbuttons.y[i], deckbuttons.w[i], deckbuttons.h[i]) && gamestate == STATE_DECK && deckbuttons.isActive[i] && !isDragging) {
            if (currMouseState & SDL_BUTTON_LMASK) { deckbuttons.isPressed[i] = true; } else { deckbuttons.isPressed[i] = false; }
            if ((currMouseState & SDL_BUTTON_LMASK) && !(prevMouseState & SDL_BUTTON_LMASK)) { deckbuttons.isClicked[i] = true; } else { deckbuttons.isClicked[i] = false; }
            deckbuttons.isHover[i] = true;
        } else {
            deckbuttons.isPressed[i] = false;
            deckbuttons.isClicked[i] = false;
            deckbuttons.isHover[i] = false;
        }

        if (point_box_collision(mousex, mousey, collectionbuttons.x[i], collectionbuttons.y[i], collectionbuttons.w[i], collectionbuttons.h[i]) && gamestate == STATE_COLLECTION && collectionbuttons.isActive[i] && !isDragging) {
            if (currMouseState & SDL_BUTTON_LMASK) { collectionbuttons.isPressed[i] = true; } else { collectionbuttons.isPressed[i] = false; }
            if ((currMouseState & SDL_BUTTON_LMASK) && !(prevMouseState & SDL_BUTTON_LMASK)) { collectionbuttons.isClicked[i] = true; } else { collectionbuttons.isClicked[i] = false; }
            collectionbuttons.isHover[i] = true;
        } else {
            collectionbuttons.isPressed[i] = false;
            collectionbuttons.isClicked[i] = false;
            collectionbuttons.isHover[i] = false;
        }

        // BUTTON EVENTS
        // -------------

        if (gamebuttons.isClicked[i] && gamebuttons.ID[i] == BUTTON_DECK && !pause) deck_to_hand(); // DECK BUTTON
        if (gamebuttons.isClicked[i] && gamebuttons.ID[i] == BUTTON_LOAN && !pause) loan_card(); // LOAN BUTTON
        if (gamebuttons.isClicked[i] && gamebuttons.ID[i] == BUTTON_BUY_STOCK && !pause) profileinfo.money[profile] += 50; // MINUS STOCK BUTTON
        if (gamebuttons.isClicked[i] && gamebuttons.ID[i] == BUTTON_SELL_STOCK && !pause) profileinfo.handslots[profile] += 1; // PLUSS STOCK BUTTON
        if (gamebuttons.isClicked[i] && gamebuttons.ID[i] == BUTTON_PAUSE) start_animation(ANIM_TRANSITION_3, callback_change_to_draft_screen, callback_random_draft_cards, -1, 0, 0, 0, 2 * M_PI); // PAUSE BUTTON

        if (menubuttons.isClicked[i] && menubuttons.ID[i] == BUTTON_NEW_GAME) start_animation(ANIM_TRANSITION_1, callback_change_to_draft_screen, callback_random_draft_cards, -1, 0, 0, 0, 2 * M_PI); // NEW GAME BUTTON
        if (menubuttons.isClicked[i] && menubuttons.ID[i] == BUTTON_SETTINGS) start_animation(ANIM_TRANSITION_1, callback_change_to_settings_screen, NULL, -1, 0, 0, 0, 2 * M_PI); // SETTINGS / OPTIONS BUTTON
        if (menubuttons.isClicked[i] && menubuttons.ID[i] == BUTTON_STATS) start_animation(ANIM_TRANSITION_1, callback_change_to_collection_screen, NULL, -1, 0, 0, 0, 2 * M_PI); // STATS OR STEAM BUTTON
        if (menubuttons.isClicked[i] && menubuttons.ID[i] == BUTTON_QUIT) start_animation(ANIM_TRANSITION_1, callback_quit_game, NULL, -1, 0, 0, 0, 2 * M_PI); // QUIT BUTTON

        if (settingbuttons.isClicked[i] && settingbuttons.ID[i] == BUTTON_SETTINGS_EXIT) start_animation(ANIM_TRANSITION_3, callback_change_to_menu_screen, NULL, -1, 0, 0, 0, 2 * M_PI); // EXIT SETTINGS
        if (settingbuttons.isClicked[i] && settingbuttons.ID[i] == BUTTON_SETTINGS_FULLSCREEN) { if (!isFullscreen) { SDL_SetWindowFullscreen(window, 1); } else { SDL_SetWindowFullscreen(window, 0); } } // TOGGLE FULLSCREEN
        if (settingbuttons.isClicked[i] && settingbuttons.ID[i] == BUTTON_SETTINGS_ASPECT_PREV) { if (aspect_ratio > 0) aspect_ratio -= 1; } // PREVIOUS ASPECT RATIO CHOICE
        if (settingbuttons.isClicked[i] && settingbuttons.ID[i] == BUTTON_SETTINGS_ASPECT_NEXT)  { if (aspect_ratio < TOTAL_ASPECTS) aspect_ratio += 1; } // NEXT ASPECT RATIO CHOICE
        if (settingbuttons.isClicked[i] && settingbuttons.ID[i] == BUTTON_SETTINGS_ASPECT_RATIO) set_aspect_ratio(); // APPLY ASPECT RATIO
        if (settingbuttons.isClicked[i] && settingbuttons.ID[i] == BUTTON_SETTINGS_SPEED_PREV) { if (game_speed > 1) game_speed -= 1; } // LOWER THE GAME SPEED
        if (settingbuttons.isClicked[i] && settingbuttons.ID[i] == BUTTON_SETTINGS_SPEED_NEXT) { if (game_speed < 4) game_speed += 1; } // RAISE THE GAME SPEED
        if (settingbuttons.isClicked[i] && settingbuttons.ID[i] == BUTTON_SETTINGS_FPS_PREV) { if (TARGET_FPS > 30) TARGET_FPS -= 30.0f; }
        if (settingbuttons.isClicked[i] && settingbuttons.ID[i] == BUTTON_SETTINGS_FPS_NEXT) { if (TARGET_FPS < 120) TARGET_FPS += 30.0f; }
        if (settingbuttons.isClicked[i] && settingbuttons.ID[i] == BUTTON_SETTINGS_HITBOXES) show_hitboxes = !show_hitboxes; // SHOW HITBOXES 
        if (settingbuttons.isClicked[i] && settingbuttons.ID[i] == BUTTON_SETTINGS_CURSOR) custom_cursor = !custom_cursor; // CUSTOM CURSOR

        if (draftbuttons.isClicked[i] && draftbuttons.ID[i] == BUTTON_DRAFT_CONTINUE) { if (draftzones.num_cards[ZONE_DRAFT_SELECT] != 0) start_animation(ANIM_TRANSITION_3, callback_select_draft_card, NULL, -1, 0, 0, 0, 2 * M_PI); } // SELECT DRAFT

        if (deckbuttons.isClicked[i] && deckbuttons.ID[i] == BUTTON_DECK_CONTINUE) start_animation(ANIM_TRANSITION_1, callback_change_to_play_screen, NULL, -1, 0, 0, 0, 2 * M_PI); // DECK CONTINUE

        if (collectionbuttons.isClicked[i] && collectionbuttons.ID[i] == BUTTON_COLLECTION_CONTINUE) start_animation(ANIM_TRANSITION_1, callback_change_to_menu_screen, NULL, -1, 0, 0, 0, 2 * M_PI); // DECK CONTINUE

    }

    // ANIMATIONS
    // ----------
    for (int i = 0; i < MAX_ANIMATIONS; i++) {
        if (!anim.isActive[i]) continue;

        if (anim.runtime[i] > anim.maxtime[i]) { 
            if (anim.onComplete[i]) anim.onComplete[i]();
            anim.onComplete[i] = NULL;
            end_animation(i);
            if (anim.targetState[i] != -1) gamestate = anim.targetState[i];
        }

        if (anim.runtime[i] > anim.maxtime[i]/2) {
            if (anim.atMidpoint[i]) anim.atMidpoint[i]();
            anim.atMidpoint[i] = NULL;
        }

        anim.runtime[i] += dt;

        // if (anim.ID[i] == ANIM_TRANSITION &&)
    }

    if (pause) return;

    // CARD UPDATES
    // ------------
    for (int i = 0; i < MAX_CARDS; i++) {
        if (!inplay.isActive[i] && !infeature.isActive[i] && !indraft.isActive[i] && !indeck.isActive[i]) continue;

        if (gamestate == STATE_DECK && indeck.isActive[i]) {
            indeck.zoneTime[i] += dt;
        }

        if (gamestate == STATE_PLAY) { inplay.zoneTime[i] += dt / game_speed; }
        if (gamestate == STATE_MENU) { infeature.zoneTime[i] += dt / game_speed; }
        if (gamestate == STATE_DRAFT) { indraft.zoneTime[i] += dt / game_speed; }

        if (point_box_collision(mousex, mousey, inplay.x[i] - (inplay.w[i] / 2), inplay.y[i] - (inplay.h[i] / 2), inplay.w[i], inplay.h[i]) && !(isDragging && !inplay.isDragging[i]) || (isDragging && inplay.isDragging[i])) { inplay.isHover[i] = true; } else { inplay.isHover[i] = false; }
        if (point_box_collision(mousex, mousey, infeature.x[i] - (infeature.w[i] / 2), infeature.y[i] - (infeature.h[i] / 2), infeature.w[i], infeature.h[i]) && !(isDragging && !infeature.isDragging[i]) || (isDragging && infeature.isDragging[i])) { infeature.isHover[i] = true; } else { infeature.isHover[i] = false; }
        if (point_box_collision(mousex, mousey, indraft.x[i] - (indraft.w[i] / 2), indraft.y[i] - (indraft.h[i] / 2), indraft.w[i], indraft.h[i]) && !(isDragging && !indraft.isDragging[i]) || (isDragging && indraft.isDragging[i])) { indraft.isHover[i] = true; } else { indraft.isHover[i] = false; }

        // Checks if a card should be picked up
        // ------------------------------------
        if (!isDragging && !inplay.isDragging[i] && point_box_collision(mousex, mousey, inplay.x[i] - (inplay.w[i] / 2), inplay.y[i] - (inplay.h[i] / 2), inplay.w[i], inplay.h[i]) && (currMouseState & SDL_BUTTON_LMASK) && !(prevMouseState & SDL_BUTTON_LMASK) && gamestate == STATE_PLAY) {
            inplay.isDragging[i] = true;
            isDragging = true;
        }

        if (!isDragging && !infeature.isDragging[i] && point_box_collision(mousex, mousey, infeature.x[i] - (infeature.w[i] / 2), infeature.y[i] - (infeature.h[i] / 2), infeature.w[i], infeature.h[i]) && (currMouseState & SDL_BUTTON_LMASK) && !(prevMouseState & SDL_BUTTON_LMASK) && gamestate == STATE_MENU) {
            infeature.isDragging[i] = true;
            isDragging = true;
        }

        if (!isDragging && !indraft.isDragging[i] && point_box_collision(mousex, mousey, indraft.x[i] - (indraft.w[i] / 2), indraft.y[i] - (indraft.h[i] / 2), indraft.w[i], indraft.h[i]) && (currMouseState & SDL_BUTTON_LMASK) && !(prevMouseState & SDL_BUTTON_LMASK) && gamestate == STATE_DRAFT) {
            indraft.isDragging[i] = true;
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

                // Checks a card is put into a zone that is not full
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
            inplay.zoneTime[i] = 0;
        }

        if (infeature.isDragging[i] && !(currMouseState & SDL_BUTTON_LMASK)) {
            infeature.isDragging[i] = false;
            isDragging = false;
            infeature.zoneTime[i] = 0;
        }

        if (indraft.isDragging[i] && !(currMouseState & SDL_BUTTON_LMASK)) {

            // checks for every active zone
            for (int j = 0; j < MAX_ZONES; j++) {
                if (!draftzones.isActive[j]) continue;

                // Checks a card is put into a zone that is not full
                // Also checks it is not put into a invalid zone
                // ---------------------------------------------
                if (point_box_collision(indraft.tx[i], indraft.ty[i], draftzones.x[j], draftzones.y[j], draftzones.w[j], draftzones.h[j]) && draftzones.num_cards[j] < draftzones.max_cards[j]) {
                    
                    // every card in the card's original zone shifts down one space
                    for (int l = 0; l < MAX_CARDS; l++) { 
                        if (indraft.zoneID[l] == indraft.zoneID[i] && indraft.zoneNum[l] > indraft.zoneNum[i]) 
                            indraft.zoneNum[l] -= 1; 
                    }

                    draftzones.num_cards[indraft.zoneID[i]] -= 1;
                    draftzones.num_cards[j] += 1;
                    indraft.zoneNum[i] = draftzones.num_cards[j];
                    indraft.zoneID[i] = j;
                }
            }

            indraft.isDragging[i] = false;
            isDragging = false;
            indraft.zoneTime[i] = 0;
        }

        // Checks if a card should be sold
        // -------------------------------
        if (inplay.zoneID[i] == ZONE_SELL && inplay.zoneTime[i] > 1.5 && inplay.isSellable[i] && !inplay.isDragging[i]) {
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

        if (infeature.isDragging[i]) {
            infeature.tx[i] = mousex;
            infeature.ty[i] = mousey;
        } else {
            infeature.tx[i] = menuzone.x[infeature.zoneID[i]] + (CARD_WIDTH/2 + CARD_SPACING + ((infeature.zoneNum[i] - 1)*(CARD_WIDTH + CARD_SPACING)))*window_scale_x;
            infeature.ty[i] = menuzone.y[infeature.zoneID[i]] + menuzone.h[infeature.zoneID[i]]/2;
        }

        if (indraft.isDragging[i]) {
            indraft.tx[i] = mousex;
            indraft.ty[i] = mousey;
        } else {
            float fanning = draftzones.w[indraft.zoneID[i]]/2 - ((CARD_WIDTH + CARD_SPACING) / 2.0f)*window_scale_x * draftzones.num_cards[indraft.zoneID[i]] - 5*window_scale_x;
            indraft.tx[i] = draftzones.x[indraft.zoneID[i]] + (CARD_WIDTH/2 + CARD_SPACING + ((indraft.zoneNum[i] - 1)*(CARD_WIDTH + CARD_SPACING)))*window_scale_x + fanning;
            indraft.ty[i] = draftzones.y[indraft.zoneID[i]] + draftzones.h[indraft.zoneID[i]]/2;
        }
        
        // MOVE THE CARD
        // -------------
        if (gamestate == STATE_PLAY) {
            inplay.w[i] = CARD_WIDTH*window_scale_x;
            inplay.h[i] = CARD_HEIGHT*window_scale_y;

            inplay.vx[i] = (inplay.tx[i] - inplay.x[i]) / LERP_SPEED;
            inplay.vy[i] = (inplay.ty[i] - inplay.y[i]) / LERP_SPEED;
            inplay.y[i] += inplay.vy[i] * dt;
            inplay.x[i] += inplay.vx[i] * dt;
        }

        if (gamestate == STATE_MENU) {
            infeature.w[i] = CARD_WIDTH*window_scale_x;
            infeature.h[i] = CARD_HEIGHT*window_scale_y;

            infeature.vx[i] = (infeature.tx[i] - infeature.x[i]) / LERP_SPEED;
            infeature.vy[i] = (infeature.ty[i] - infeature.y[i]) / LERP_SPEED;
            infeature.y[i] += infeature.vy[i] * dt;
            infeature.x[i] += infeature.vx[i] * dt;
        }

        if (gamestate == STATE_DRAFT) {
            indraft.w[i] = CARD_WIDTH*window_scale_x;
            indraft.h[i] = CARD_HEIGHT*window_scale_y;

            indraft.vx[i] = (indraft.tx[i] - indraft.x[i]) / LERP_SPEED;
            indraft.vy[i] = (indraft.ty[i] - indraft.y[i]) / LERP_SPEED;
            indraft.y[i] += indraft.vy[i] * dt;
            indraft.x[i] += indraft.vx[i] * dt;
        }

    }

}

void debug() {

    // current lifted card
    // -------------------
    // if ((currMouseState & SDL_BUTTON_LMASK) && !(prevMouseState & SDL_BUTTON_LMASK)) {
    //     for (int i = 0; i < MAX_CARDS; i++) {
    //         if (!inplay.isDragging[i]) continue;
    //         printf("%d %s -> sell: %d, i: %d\n", inplay.ID[i], cards.name[inplay.ID[i]], inplay.isSellable[i], i);
    //     }
    // }

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

// RENDER FUNCTIONS ====================================================================================================

void draw_candle(float x, float h, float o, float c, float l) {
    if (o > c) {
        // loss
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    } else {
        // gain
        SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    }
    SDL_RenderLine(renderer, (x*3)*window_scale_x, h*window_scale_y, (x*3)*window_scale_x, l*window_scale_y);
    SDL_FRect candle = {((x*3)-3)*window_scale_x, (o)*window_scale_y, 6*window_scale_x, (c-o)*window_scale_y};
    SDL_RenderRect(renderer, &candle);
    // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    // SDL_RenderLine(renderer, (x-1)*3*window_scale_x, o*window_scale_y, (x)*3*window_scale_x, c*window_scale_y);
}

void render_charts() {
    
    // STOCK CHARTS TEXTURES
    // ------------------------------
    if (show_textures) {
        int bottom_border = 0;
        int left_border = 0;
        // repeat for each of the five stocks selected
        // draw points of the graph up to the round time passed (eg 20s only render 20 points)
        // draw the lines connecting the points together
        
        // for (int i = 0; i < 60; i++) {
        //     draw_candle(i*3, 0, stonk.o[i]*25, stonk.c[i]*25, 0);
        // }

        // for (int i = 0; i < 60; i++) {
        //     SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        //     SDL_RenderLine(renderer, (i-1)*3*3*window_scale_x, stonk.o[i]*25*window_scale_y, i*3*window_scale_x, stonk.c[i]*25*window_scale_y); 
        // }

        // draw_candle(10.0f*window_scale_x, 235.0f*window_scale_y, 190.0f*window_scale_y, 212.0f*window_scale_y, 185.0f*window_scale_y);
        // draw_candle(20.0f*window_scale_x, 217.0f*window_scale_y, 212.0f*window_scale_y, 186.0f*window_scale_y, 186.0f*window_scale_y);
    }
    // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    // SDL_RenderLine(renderer, 0, mousey*window_scale_y, window_width, mousey*window_scale_y);
    // SDL_RenderLine(renderer, mousex*window_scale_x, 0, mousex*window_scale_x, window_height);

    // STOCK CHARTS HITBOX
    // ------------------------------

    if (show_hitboxes) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_FRect charthitbox = {(WINDOW_WIDTH/2 - 725/2)*window_scale_x, CARD_MARGIN*window_scale_y, 725*window_scale_x, 440*window_scale_y};
        SDL_RenderRect(renderer, &charthitbox);
    }
}

void render_assistant() {
    // ASSISTANT TEXTURES / HITBOX
    // ---------------------------
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_FRect assistanthitbox = {((600+725/2) + 15)*window_scale_x, (CARD_MARGIN+CARD_SPACING+CARD_HEIGHT+20)*window_scale_y, (WINDOW_WIDTH-CARD_MARGIN-CARD_MARGIN+CARD_SPACING-30)*window_scale_x, 250*window_scale_y};
    SDL_RenderRect(renderer, &assistanthitbox);
}

void render_headline() {

    // HEADLINE TEXTURES / HITBOX
    // --------------------------
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_FRect headlinehitbox = {(CARD_MARGIN+15)*window_scale_x, (WINDOW_HEIGHT-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN-15-50)*window_scale_y, (WINDOW_WIDTH-CARD_MARGIN-CARD_MARGIN+CARD_SPACING-30)*window_scale_x, 50*window_scale_y};
    SDL_RenderRect(renderer, &headlinehitbox);

    // TEXT
    // ----
    char headline[256];
    strcpy(headline, "--NULL--");
    for (int i = 0; i < MAX_CARDS; i++) {
        if (!inplay.isActive[i]) continue;
        // strcpy(headline, cards.name[inplay.ID[i]]);
        if (inplay.zoneID[i] == ZONE_EVENT) {
            strcpy(headline, stringf("EVENT: %s", cards.description[inplay.ID[i]]));
        }
    }
    if (isDragging) {
        for (int i = 0; i < MAX_CARDS; i++) {
            if (!inplay.isDragging[i]) continue;
            // strcpy(headline, cards.name[inplay.ID[i]]);
            strcpy(headline, stringf("#%d - %s (rar %d) -> %s", inplay.ID[i], cards.name[inplay.ID[i]], inplay.rarity[i], cards.description[inplay.ID[i]]));
        }
    }

    float sine = 25*sin(SDL_GetTicks()/1000.0f) + 215;
    SDL_Color color = {(Uint8)sine, (Uint8)sine, 0, 255};
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_Surface *textSurface = TTF_RenderText_Solid(fontDSDigital, headline, strlen(headline), color);
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_FRect textQuad = { (600-textSurface->w/2)*window_scale_x, (WINDOW_HEIGHT-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN-15-50+12)*window_scale_y, (float)textSurface->w*window_scale_x, (float)textSurface->h*window_scale_y };
    SDL_RenderTexture(renderer, textTexture, NULL, &textQuad);
    SDL_DestroyTexture(textTexture);
    SDL_DestroySurface(textSurface);
}

void render() {

    // BACKGROUND TEXTURES
    // ------------------------------
    if (show_textures) {
        // float angle = 0;
        // SDL_FRect texture = { 0, 0, (float)window_width, (float)window_height };
        // SDL_FPoint center = {texture.w / 2, texture.h / 2};
        // SDL_RenderTextureRotated(renderer, texture_idea, NULL, &texture, angle, &center, SDL_FLIP_NONE);
    }
    




    // COLLECTION GAME STATE
    // ---------------
    if (gamestate == STATE_COLLECTION && show_textures) {

        SDL_FRect cardtexture = {0};
        SDL_FPoint center = {0};
        float sineh, angle;

        for (int i = 0; i < TOTAL_CARDS; i++) {

            if (show_textures) {

                sineh = 5 *  sin(2 * SDL_GetTicks()/1000.0f + i*0.1);
                angle = 2.5f * sin(SDL_GetTicks()/1000.0f + i*0.1);
                if (!cardsunlocked[i]) { sineh /= 2.0f; angle = 0.0f; }

                float xpos = CARD_SPACING*2 + (CARD_WIDTH+CARD_SPACING) * (i % 9);
                float ypos = CARD_SPACING + (floor((i/9)) + 0.1) * (CARD_HEIGHT+CARD_SPACING) - state_timer*5;

                cardtexture = (SDL_FRect) {  
                    (window_width/2) + (xpos - 1190/2) * window_scale_x,
                    ypos * window_scale_y,
                    CARD_WIDTH*window_scale_x, 
                    CARD_HEIGHT*window_scale_y
                };

                center = (SDL_FPoint) {cardtexture.w / 2, cardtexture.h / 2};
                if(cardsunlocked[i]) {
                SDL_RenderTextureRotated(renderer, cards.cardtexture[i], NULL, &cardtexture, (double)angle, &center, SDL_FLIP_NONE);
                } else {
                SDL_RenderTextureRotated(renderer, gamebuttons.buttontexture[BUTTON_DECK], NULL, &cardtexture, (double)angle, &center, SDL_FLIP_NONE);
                }

                if (!cardsunlocked[i]) {
                    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 200);
                    SDL_RenderFillRect(renderer, &cardtexture);

                    SDL_RenderTextureRotated(renderer, texture_dim1, NULL, &cardtexture, (double)angle, &center, SDL_FLIP_NONE);

                    int width = 50, height = 50;
                    cardtexture = (SDL_FRect) {
                        (window_width/2) + (xpos - 1190/2 + CARD_WIDTH/2 - width/2) * window_scale_x,
                        (ypos + CARD_HEIGHT/2 - height/2) * window_scale_y,
                        width*window_scale_x, 
                        height*window_scale_y
                    };
                    center = (SDL_FPoint) {cardtexture.w / 2, cardtexture.h / 2};
                    SDL_RenderTextureRotated(renderer, texture_cardlock, NULL, &cardtexture, (double)angle, &center, SDL_FLIP_NONE);

                }
            }
        }
    }

    // DRAFT GAMESTATE OTHER THINGS
    // ----------------------------
    if (gamestate == STATE_DRAFT && show_textures) {
        // SDL_FRect texture = {0};
        // SDL_FPoint center = {0};
        // float halflifefunc = 2000 * pow(0.65, (state_timer/10.0f) / 0.05f);
        // float sineh; double angle;

        // sineh = sin(state_timer);
        // texture = (SDL_FRect) {  
        //     window_width/2 - 500/2*window_scale_x,
        //     halflifefunc*window_scale_y + window_height - 500*window_scale_y - (500/2)*window_scale_y - (sineh*window_scale_y), 
        //     500*window_scale_x, 
        //     500*window_scale_y,
        // };
        // center = (SDL_FPoint) {texture.w / 2, texture.h / 2};
        // SDL_RenderTextureRotated(renderer, cards.cardtexture[5], NULL, &texture, 0, &center, SDL_FLIP_NONE);

    }





    // DECK GAME STATE
    // ---------------
    if (gamestate == STATE_DECK && show_textures) {

        SDL_FRect cardtexture = {0};
        SDL_FPoint center = {0};
        float sineh, angle;

        int num = -1;
        for (int i = 0; i < MAX_CARDS; i++) {
            if (!indeck.isActive[i]) continue;
            num += 1;

            if (show_textures) {

                sineh = 5 *  sin(2 * indeck.zoneTime[i]);
                angle = 2.5f * sin(indeck.zoneTime[i]);

                float xpos = CARD_SPACING*2 + (CARD_WIDTH+CARD_SPACING) * (num % 9);
                float ypos = CARD_SPACING + (floor((num/9)) + 0.1) * (CARD_HEIGHT+CARD_SPACING) - state_timer*5;

                cardtexture = (SDL_FRect) {  
                    (window_width/2) + (xpos - 1190/2) * window_scale_x,
                    ypos * window_scale_y,
                    CARD_WIDTH*window_scale_x, 
                    CARD_HEIGHT*window_scale_y
                };

                center = (SDL_FPoint) {cardtexture.w / 2, cardtexture.h / 2};
                SDL_RenderTextureRotated(renderer, cards.cardtexture[indeck.ID[i]], NULL, &cardtexture, (double)angle, &center, SDL_FLIP_NONE);
                if (indeck.rarity[i] > 0) SDL_RenderTextureRotated(renderer, texture_rarity[indeck.rarity[i]], NULL, &cardtexture, (double)angle, &center, SDL_FLIP_NONE);

                if (!indeck.isSellable[i]) {
                    int width = 50, height = 50;
                    cardtexture = (SDL_FRect) { 
                        (window_width/2) + (xpos - 1190/2) * window_scale_x,
                        ypos * window_scale_y,
                        width*window_scale_x, 
                        height*window_scale_y
                    };
                    SDL_RenderTextureRotated(renderer, texture_event, NULL, &cardtexture, (double)angle, &center, SDL_FLIP_NONE);
                }
                
            }
        }
    }

    // ZONES TEXTURES / HITBOXES
    // -------------------------
    for (int i = 0; i < MAX_ZONES; i++) {
        if (!playzones.isActive[i] && !menuzone.isActive[i] && !draftzones.isActive[i]) continue;

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        SDL_FRect zonetexture = {0};
        SDL_FRect zonehitbox = {0};
        SDL_FPoint center = {0};
        float angle = 0;

        if (gamestate == STATE_PLAY) {
            if (show_textures) {
                zonetexture = (SDL_FRect){ playzones.x[i], playzones.y[i], playzones.w[i], playzones.h[i] };
                center = (SDL_FPoint){ zonetexture.w / 2, zonetexture.h / 2 };
                SDL_RenderTextureRotated(renderer, playzones.zonetexture[i], NULL, &zonetexture, angle, &center, SDL_FLIP_NONE);
            }
            if (show_hitboxes) {
                zonehitbox = (SDL_FRect){playzones.x[i], playzones.y[i], playzones.w[i], playzones.h[i]};
                SDL_RenderRect(renderer, &zonehitbox);
            }
        }

        if (gamestate == STATE_MENU) {
            if (show_textures) {
                zonetexture = (SDL_FRect){ menuzone.x[i], menuzone.y[i], menuzone.w[i], menuzone.h[i] };
                center = (SDL_FPoint){ zonetexture.w / 2, zonetexture.h / 2 };
                SDL_RenderTextureRotated(renderer, menuzone.zonetexture[i], NULL, &zonetexture, angle, &center, SDL_FLIP_NONE);
            }
            if (show_hitboxes) {
                zonehitbox = (SDL_FRect){menuzone.x[i], menuzone.y[i], menuzone.w[i], menuzone.h[i]};
                SDL_RenderRect(renderer, &zonehitbox);
            }
        }

        if (gamestate == STATE_DRAFT) {
            if (show_textures) {
                zonetexture = (SDL_FRect){ draftzones.x[i], draftzones.y[i], draftzones.w[i], draftzones.h[i] };
                center = (SDL_FPoint){ zonetexture.w / 2, zonetexture.h / 2 };
                SDL_RenderTextureRotated(renderer, draftzones.zonetexture[i], NULL, &zonetexture, angle, &center, SDL_FLIP_NONE);
            }
            if (show_hitboxes) {
                zonehitbox = (SDL_FRect){draftzones.x[i], draftzones.y[i], draftzones.w[i], draftzones.h[i]};
                SDL_RenderRect(renderer, &zonehitbox);
            }
        }

    }

    // BUTTONS TEXTURES / HITBOXES
    // ---------------------------
    for (int i = 0; i < MAX_BUTTONS; i++) {
        if (!gamebuttons.isActive[i] && !menubuttons.isActive[i] && !settingbuttons.isActive[i] && !draftbuttons.isActive[i] && !deckbuttons.isActive[i] && !collectionbuttons.isActive[i]) continue;
        
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        SDL_FRect buttontexture = {0};
        SDL_FRect buttonhitbox = {0};
        SDL_FPoint center = {0};
        float sineh, sinea, angle, button_grow_w, button_grow_h;

        if (gamestate == STATE_PLAY) {
            if (show_textures) {
                
                sineh = 5 * sin(2.0f * (gamebuttons.clickTime[i]));
                sinea = 2.5 * sin(SDL_GetTicks() / 1000.0f + i*0.2);
                angle = sinea;

                button_grow_w = (gamebuttons.isHover[i]+gamebuttons.isPressed[i]) * CARD_GROW * gamebuttons.w[i];
                button_grow_h = (gamebuttons.isHover[i]+gamebuttons.isPressed[i]) * CARD_GROW * gamebuttons.h[i];

                buttontexture = (SDL_FRect) {
                    gamebuttons.x[i] - (button_grow_w/2), 
                    gamebuttons.y[i] - (button_grow_h/2), 
                    gamebuttons.w[i] + (button_grow_w*window_scale_x), 
                    gamebuttons.h[i] + (button_grow_h*window_scale_y)
                };

                center = (SDL_FPoint) {buttontexture.w / 2, buttontexture.h / 2};
                SDL_RenderTextureRotated(renderer, gamebuttons.buttontexture[i], NULL, &buttontexture, angle, &center, SDL_FLIP_NONE);
            }
            if (show_hitboxes) {
                if (gamebuttons.isPressed[i]) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                buttonhitbox = (SDL_FRect) {gamebuttons.x[i], gamebuttons.y[i], gamebuttons.w[i], gamebuttons.h[i]};
                SDL_RenderRect(renderer, &buttonhitbox);
            }
        }
        
        if (gamestate == STATE_MENU) {
            if (show_textures) {
                sineh = 5 * sin(2.0f * menubuttons.clickTime[i]);
                sinea = sin(SDL_GetTicks() / 1000.0f + i*0.2);
                // if (menubuttons.isPressed[i]) sinea = 0;
                angle = sinea;

                button_grow_w = (menubuttons.isHover[i]+menubuttons.isPressed[i]) * CARD_GROW * menubuttons.w[i];
                button_grow_h = (menubuttons.isHover[i]+menubuttons.isPressed[i]) * CARD_GROW * menubuttons.h[i];

                buttontexture = (SDL_FRect) {
                    menubuttons.x[i] - (button_grow_w/2), 
                    menubuttons.y[i] - (button_grow_h/2), 
                    menubuttons.w[i] + (button_grow_w*window_scale_x), 
                    menubuttons.h[i] + (button_grow_h*window_scale_y)
                };

                center = (SDL_FPoint) {buttontexture.w / 2, buttontexture.h / 2};
                SDL_RenderTextureRotated(renderer, menubuttons.buttontexture[i], NULL, &buttontexture, angle, &center, SDL_FLIP_NONE);
            }
            if (show_hitboxes) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                if (menubuttons.isPressed[i]) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                buttonhitbox = (SDL_FRect) {menubuttons.x[i], menubuttons.y[i], menubuttons.w[i], menubuttons.h[i]};
                SDL_RenderRect(renderer, &buttonhitbox);
            }
        }
        
        if (gamestate == STATE_SETTINGS) {
            if (show_textures) {
                sineh = 5 * sin(2.0f * settingbuttons.clickTime[i]);
                sinea = sin(SDL_GetTicks() / 1000.0f + i*0.2);
                // if (settingbuttons.isPressed[i]) sinea = 0;
                angle = sinea;

                button_grow_w = settingbuttons.isPressed[i] * CARD_GROW * settingbuttons.w[i];
                button_grow_h = settingbuttons.isPressed[i] * CARD_GROW * settingbuttons.h[i];

                buttontexture = (SDL_FRect) {
                    settingbuttons.x[i] - (button_grow_w/2), 
                    settingbuttons.y[i] - (button_grow_h/2), 
                    settingbuttons.w[i] + (button_grow_w*window_scale_x), 
                    settingbuttons.h[i] + (button_grow_h*window_scale_y)
                };

                center = (SDL_FPoint) {buttontexture.w / 2, buttontexture.h / 2};
                SDL_RenderTextureRotated(renderer, settingbuttons.buttontexture[i], NULL, &buttontexture, angle, &center, SDL_FLIP_NONE);
            }
            if (show_hitboxes) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                if (settingbuttons.isPressed[i]) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                buttonhitbox = (SDL_FRect) {settingbuttons.x[i], settingbuttons.y[i], settingbuttons.w[i], settingbuttons.h[i]};
                SDL_RenderRect(renderer, &buttonhitbox);
            }
        }
        
        if (gamestate == STATE_DRAFT) {
            if (show_textures) {
                sineh = 5 * sin(2.0f * draftbuttons.clickTime[i]);
                sinea = sin(SDL_GetTicks() / 1000.0f + i*0.2);
                // if (draftbuttons.isPressed[i]) sinea = 0;
                angle = sinea;

                button_grow_w = (draftbuttons.isHover[i]+draftbuttons.isPressed[i]) * CARD_GROW * draftbuttons.w[i];
                button_grow_h = (draftbuttons.isHover[i]+draftbuttons.isPressed[i]) * CARD_GROW * draftbuttons.h[i];

                buttontexture = (SDL_FRect) {
                    draftbuttons.x[i] - (button_grow_w/2), 
                    draftbuttons.y[i] - (button_grow_h/2), 
                    draftbuttons.w[i] + (button_grow_w*window_scale_x), 
                    draftbuttons.h[i] + (button_grow_h*window_scale_y)
                };

                center = (SDL_FPoint) {buttontexture.w / 2, buttontexture.h / 2};
                SDL_RenderTextureRotated(renderer, draftbuttons.buttontexture[i], NULL, &buttontexture, angle, &center, SDL_FLIP_NONE);
            }
            if (show_hitboxes) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                if (draftbuttons.isPressed[i]) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                buttonhitbox = (SDL_FRect) {draftbuttons.x[i], draftbuttons.y[i], draftbuttons.w[i], draftbuttons.h[i]};
                SDL_RenderRect(renderer, &buttonhitbox);
            }
        }

        if (gamestate == STATE_DECK) {
            if (show_textures) {
                sineh = 5 * sin(2.0f * deckbuttons.clickTime[i]);
                sinea = sin(SDL_GetTicks() / 1000.0f + i*0.2);
                // if (deckbuttons.isPressed[i]) sinea = 0;
                angle = sinea;

                button_grow_w = (deckbuttons.isHover[i]+deckbuttons.isPressed[i]) * CARD_GROW * deckbuttons.w[i];
                button_grow_h = (deckbuttons.isHover[i]+deckbuttons.isPressed[i]) * CARD_GROW * deckbuttons.h[i];

                buttontexture = (SDL_FRect) {
                    deckbuttons.x[i] - (button_grow_w/2), 
                    deckbuttons.y[i] - (button_grow_h/2), 
                    deckbuttons.w[i] + (button_grow_w*window_scale_x), 
                    deckbuttons.h[i] + (button_grow_h*window_scale_y)
                };

                center = (SDL_FPoint) {buttontexture.w / 2, buttontexture.h / 2};
                SDL_RenderTextureRotated(renderer, deckbuttons.buttontexture[i], NULL, &buttontexture, angle, &center, SDL_FLIP_NONE);
            }
            if (show_hitboxes) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                if (deckbuttons.isPressed[i]) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                buttonhitbox = (SDL_FRect) {deckbuttons.x[i], deckbuttons.y[i], deckbuttons.w[i], deckbuttons.h[i]};
                SDL_RenderRect(renderer, &buttonhitbox);
            }
        }

        if (gamestate == STATE_COLLECTION) {
            if (show_textures) {
                sineh = 5 * sin(2.0f * collectionbuttons.clickTime[i]);
                sinea = sin(SDL_GetTicks() / 1000.0f + i*0.2);
                // if (collectionbuttons.isPressed[i]) sinea = 0;
                angle = sinea;

                button_grow_w = (collectionbuttons.isHover[i]+collectionbuttons.isPressed[i]) * CARD_GROW * collectionbuttons.w[i];
                button_grow_h = (collectionbuttons.isHover[i]+collectionbuttons.isPressed[i]) * CARD_GROW * collectionbuttons.h[i];

                buttontexture = (SDL_FRect) {
                    collectionbuttons.x[i] - (button_grow_w/2), 
                    collectionbuttons.y[i] - (button_grow_h/2), 
                    collectionbuttons.w[i] + (button_grow_w*window_scale_x), 
                    collectionbuttons.h[i] + (button_grow_h*window_scale_y)
                };

                center = (SDL_FPoint) {buttontexture.w / 2, buttontexture.h / 2};
                SDL_RenderTextureRotated(renderer, collectionbuttons.buttontexture[i], NULL, &buttontexture, angle, &center, SDL_FLIP_NONE);
            }
            if (show_hitboxes) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                if (collectionbuttons.isPressed[i]) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
                buttonhitbox = (SDL_FRect) {collectionbuttons.x[i], collectionbuttons.y[i], collectionbuttons.w[i], collectionbuttons.h[i]};
                SDL_RenderRect(renderer, &buttonhitbox);
            }
        }

    }





    // SOME OTHER TRADE STATE TEXTURES / HITBOXES 
    // ----------------------------------
    if (gamestate == STATE_PLAY) {
        
        // MONEY
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_FRect moneyhitbox = {0, CARD_MARGIN*window_scale_y, (CARD_MARGIN+CARD_SPACING+CARD_WIDTH+CARD_MARGIN-15)*window_scale_x, 50*window_scale_y};
        SDL_RenderRect(renderer, &moneyhitbox);

        // RENDER TEXT
        SDL_Color color = {0, 0, 0, 255};

        SDL_Surface *textSurface = TTF_RenderText_Solid(fontBalatro, stringf("$%d + $%d", profileinfo.extraprice[profile], profileinfo.extrainflation[profile]*profileinfo.extracount[profile]), strlen(stringf("$%d + $%d", profileinfo.extraprice[profile], profileinfo.extraprice[profile]*profileinfo.extracount[profile])), color);
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FRect textQuad = { CARD_MARGIN*window_scale_x, (WINDOW_HEIGHT-CARD_HEIGHT-CARD_SPACING-CARD_MARGIN-12)*window_scale_y, (float)textSurface->w*window_scale_x, (float)textSurface->h*window_scale_y };
        SDL_RenderTexture(renderer, textTexture, NULL, &textQuad);
        SDL_DestroyTexture(textTexture);
        SDL_DestroySurface(textSurface);

        textSurface = TTF_RenderText_Solid(fontBalatro, "Round #", strlen("Round #"), color);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FRect textQuad2 = { 15*window_scale_x, (CARD_MARGIN+65+10)*window_scale_y, (float)textSurface->w*window_scale_x, (float)textSurface->h*window_scale_y };
        SDL_RenderTexture(renderer, textTexture, NULL, &textQuad2);
        SDL_DestroyTexture(textTexture);
        SDL_DestroySurface(textSurface);

        
        textSurface = TTF_RenderText_Solid(fontBalatro, stringf("money: $%d", profileinfo.money[profile]), strlen(stringf("money: $%d", profileinfo.money[profile])), color);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FRect textQuad3 = { 15*window_scale_x, (CARD_MARGIN+10)*window_scale_y, (float)textSurface->w*window_scale_x, (float)textSurface->h*window_scale_y };
        SDL_RenderTexture(renderer, textTexture, NULL, &textQuad3);
        SDL_DestroyTexture(textTexture);
        SDL_DestroySurface(textSurface);


        SDL_FRect moneyhitbox2 = {0, (CARD_MARGIN+65)*window_scale_y, (CARD_MARGIN+CARD_SPACING+CARD_WIDTH+CARD_MARGIN-15)*window_scale_x, 50*window_scale_y};
        SDL_RenderRect(renderer, &moneyhitbox2);

        SDL_FRect moneyhitbox5 = {15*window_scale_x, (CARD_MARGIN+260)*window_scale_y, (CARD_MARGIN+CARD_SPACING+CARD_WIDTH+CARD_MARGIN-15-15)*window_scale_x, 180*window_scale_y};
        SDL_RenderRect(renderer, &moneyhitbox5);

        render_charts();
        render_assistant();
        render_headline();
    }

    // OTHER MAIN MENU TEXTURES / HITBOXES 
    // ----------------------------------
    if (gamestate == STATE_MENU && show_textures) {
        SDL_Color color = {0, 0, 0, 255};
        SDL_Surface *textSurface = TTF_RenderText_Solid(fontBalatro, version, strlen(version), color);
        SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        SDL_FRect textQuad = { 15*window_scale_x, (WINDOW_HEIGHT-15-textSurface->h)*window_scale_y, (float)textSurface->w*window_scale_x, (float)textSurface->h*window_scale_y };
        SDL_RenderTexture(renderer, textTexture, NULL, &textQuad);
        SDL_DestroyTexture(textTexture);
        SDL_DestroySurface(textSurface);

        SDL_FRect texture = {0};
        SDL_FPoint center = {0};
        float halflifefunc = 2000 * pow(0.25, (SDL_GetTicks()/10000.0f) / 0.05f);
        float sineh, sinea;
        double angle;

        // card left
        // ---------
        sineh = 5 *  sin(2 * SDL_GetTicks() / 1000.0f);
        sinea = 2.5f * sin(SDL_GetTicks() / 1000.0f);
        angle = sinea - 28;
        texture = (SDL_FRect) {  
            200*window_scale_x - (CARD_HEIGHT/2/2), 
            (halflifefunc+600)*window_scale_y - (CARD_HEIGHT/2/2) - (sineh*window_scale_y), 
            CARD_WIDTH/2*window_scale_x, 
            CARD_HEIGHT/2*window_scale_y
        };
        center = (SDL_FPoint) {texture.w / 2, texture.h / 2};
        SDL_RenderTextureRotated(renderer, cards.cardtexture[5], NULL, &texture, angle, &center, SDL_FLIP_NONE);

        // card right
        // ----------
        sineh = 5 *  sin(2 * (SDL_GetTicks() / 1000.0f) - 0.5f);
        sinea = 2.5f * sin((SDL_GetTicks() / 1000.0f) - 0.32f);      
        angle = sinea + 18;
        texture = (SDL_FRect) {  
            1000*window_scale_x - (CARD_HEIGHT/2/1.8f), 
            (halflifefunc+450)*window_scale_y - (CARD_HEIGHT/2/1.8f) - (sineh*window_scale_y), 
            CARD_WIDTH/1.8f*window_scale_x, 
            CARD_HEIGHT/1.8f*window_scale_y
        };
        center = (SDL_FPoint) {texture.w / 2, texture.h / 2};
        SDL_RenderTextureRotated(renderer, cards.cardtexture[9], NULL, &texture, angle, &center, SDL_FLIP_NONE);

        // title logo
        // ----------
        sineh = sin(SDL_GetTicks() / 1000.0f);
        texture = (SDL_FRect) {  
            (window_width/2) - (1000/2)*window_scale_x, 
            (-halflifefunc+200)*window_scale_y - (200/2)*window_scale_x - (sineh*window_scale_y), 
            1000*window_scale_x, 
            200*window_scale_y,
        };
        center = (SDL_FPoint) {texture.w / 2, texture.h / 2};
        SDL_RenderTextureRotated(renderer, cards.cardtexture[5], NULL, &texture, 0, &center, SDL_FLIP_NONE);

        
        // SDL_FRect texture = {0};
        // SDL_FPoint center = {0};
        // float halflifefunc = 2000 * pow(0.65, (state_timer/10.0f) / 0.05f);
        // float sineh; double angle;

        // sineh = sin(2 * state_timer);
        // texture = (SDL_FRect) {  
        //     window_width/2 - 500/2*window_scale_x,
        //     halflifefunc*window_scale_y + window_height - 125*window_scale_y - (150/2)*window_scale_y - (sineh*window_scale_y), 
        //     500*window_scale_x, 
        //     150*window_scale_y,
        // };
        // center = (SDL_FPoint) {texture.w / 2, texture.h / 2};
        // SDL_RenderTextureRotated(renderer, cards.cardtexture[5], NULL, &texture, 0, &center, SDL_FLIP_NONE);
    }

    // OTHER SETTINGS TEXTURES / HITBOXES
    // ----------------------------------
    if (gamestate == STATE_SETTINGS && show_textures) {
        SDL_Color color = {0};
        SDL_Surface *textSurface = NULL;
        SDL_Texture *textTexture = {0};
        SDL_FRect textQuad = {0};

        color = {0, 0, 0, 255};

        // ASPECT RATIO TITLE
        textSurface = TTF_RenderText_Solid(fontBalatro, "ASPECT RATIO", strlen("ASPECT RATIO"), color);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        textQuad = (SDL_FRect) { ((WINDOW_WIDTH/2)-(textSurface->w/2))*window_scale_x, (CARD_SPACING+5+(40+CARD_SPACING)*1)*window_scale_y, (float)textSurface->w*window_scale_x, (float)textSurface->h*window_scale_y };
        SDL_RenderTexture(renderer, textTexture, NULL, &textQuad);
        SDL_DestroyTexture(textTexture);
        SDL_DestroySurface(textSurface);

        // CURRENT ASPECT RATIO
        textSurface = TTF_RenderText_Solid(fontBalatro, stringf("currently %d:%d", WIDTH_RATIO, HEIGHT_RATIO), strlen(stringf("currently %d:%d", WIDTH_RATIO, HEIGHT_RATIO)), color);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        textQuad = (SDL_FRect) { ((WINDOW_WIDTH/2)-(textSurface->w/2))*window_scale_x, (CARD_SPACING+5+(40+CARD_SPACING)*3)*window_scale_y, (float)textSurface->w*window_scale_x, (float)textSurface->h*window_scale_y };
        SDL_RenderTexture(renderer, textTexture, NULL, &textQuad);
        SDL_DestroyTexture(textTexture);
        SDL_DestroySurface(textSurface);

        // GAME SPEED TITLE
        textSurface = TTF_RenderText_Solid(fontBalatro, "GAME SPEED", strlen("GAME SPEED"), color);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        textQuad = (SDL_FRect) { ((WINDOW_WIDTH/2)-(textSurface->w/2))*window_scale_x, (CARD_SPACING+5+(40+CARD_SPACING)*5)*window_scale_y, (float)textSurface->w*window_scale_x, (float)textSurface->h*window_scale_y };
        SDL_RenderTexture(renderer, textTexture, NULL, &textQuad);
        SDL_DestroyTexture(textTexture);
        SDL_DestroySurface(textSurface);

        // GAME SPEED SELECTION
        textSurface = TTF_RenderText_Solid(fontBalatro, stringf("%d", (int)game_speed), strlen(stringf("%d", (int)game_speed)), color);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        textQuad = (SDL_FRect) { ((WINDOW_WIDTH/2)-(textSurface->w/2))*window_scale_x, (CARD_SPACING+5+(40+CARD_SPACING)*6)*window_scale_y, (float)textSurface->w*window_scale_x, (float)textSurface->h*window_scale_y };
        SDL_RenderTexture(renderer, textTexture, NULL, &textQuad);
        SDL_DestroyTexture(textTexture);
        SDL_DestroySurface(textSurface);

        // FRAMERATE TITLE
        textSurface = TTF_RenderText_Solid(fontBalatro, "FRAMERATE", strlen("FRAMERATE"), color);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        textQuad = (SDL_FRect) { ((WINDOW_WIDTH/2)-(textSurface->w/2))*window_scale_x, (CARD_SPACING+5+(40+CARD_SPACING)*9)*window_scale_y, (float)textSurface->w*window_scale_x, (float)textSurface->h*window_scale_y };
        SDL_RenderTexture(renderer, textTexture, NULL, &textQuad);
        SDL_DestroyTexture(textTexture);
        SDL_DestroySurface(textSurface);

        // CURRENT FRAMERATE
        textSurface = TTF_RenderText_Solid(fontBalatro, stringf("%.0f FPS", TARGET_FPS), strlen(stringf("%.0f FPS", TARGET_FPS)), color);
        textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        textQuad = (SDL_FRect) { ((WINDOW_WIDTH/2)-(textSurface->w/2))*window_scale_x, (CARD_SPACING+5+(40+CARD_SPACING)*10)*window_scale_y, (float)textSurface->w*window_scale_x, (float)textSurface->h*window_scale_y };
        SDL_RenderTexture(renderer, textTexture, NULL, &textQuad);
        SDL_DestroyTexture(textTexture);
        SDL_DestroySurface(textSurface);
        
        // SDL_Color color = {0, 0, 0, 255};
        // SDL_Surface *textSurface = TTF_RenderText_Solid(fontBalatro, version, strlen(version), color);
        // SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        // SDL_FRect textQuad = { 15*window_scale_x, (WINDOW_HEIGHT-15-textSurface->h)*window_scale_y, (float)textSurface->w*window_scale_x, (float)textSurface->h*window_scale_y };
        // SDL_RenderTexture(renderer, textTexture, NULL, &textQuad);
        // SDL_DestroyTexture(textTexture);
        // SDL_DestroySurface(textSurface);

        SDL_FRect texture = {0};
        SDL_FPoint center = {0};
        float halflifefunc = 2000 * pow(0.65, (state_timer/10.0f) / 0.05f);
        float sineh; double angle;

        sineh = sin(state_timer);
        texture = (SDL_FRect) {  
            window_width/2 - 500/2*window_scale_x,
            halflifefunc*window_scale_y + window_height - 125*window_scale_y - (150/2)*window_scale_y - (sineh*window_scale_y), 
            500*window_scale_x, 
            150*window_scale_y,
        };
        center = (SDL_FPoint) {texture.w / 2, texture.h / 2};
        SDL_RenderTextureRotated(renderer, cards.cardtexture[5], NULL, &texture, 0, &center, SDL_FLIP_NONE);

    }





    // CARDS TEXTURES / HITBOXES
    // --------------------------------
    for (int i = 0; i < MAX_CARDS; i++) {
        if (!inplay.isActive[i] && !infeature.isActive[i] && !indraft.isActive[i]) continue;
        if (inplay.isActive[i] && inplay.isDragging[i]) continue;
        if (indraft.isActive[i] && indraft.isDragging[i]) continue;

        SDL_FRect cardtexture = {0};
        SDL_FRect cardhitbox = {0};
        SDL_FPoint center = {0};
        float sineh, sinea, fanning, card_grow_w, card_grow_h;
        double angle;

        if (gamestate == STATE_PLAY) {
            if (show_textures) {

                card_grow_w = (inplay.isDragging[i]+inplay.isHover[i]) * CARD_GROW;
                card_grow_h = (inplay.isDragging[i]+inplay.isHover[i]) * CARD_GROW;

                sineh = 0, sinea = 0, fanning = 0;
                if (!inplay.isDragging[i]) {
                    sineh = 5 *  sin(2 * inplay.zoneTime[i]);
                    fanning = (inplay.zoneNum[i] - ((playzones.num_cards[inplay.zoneID[i]] + 1) / 2.0f)) * 2.5f;
                    sinea = 2.5f * sin(inplay.zoneTime[i]);
                }
                angle = inplay.vx[i]/60 + sinea + fanning;

                cardtexture = (SDL_FRect) {
                    inplay.x[i] - (inplay.w[i]/2) - (card_grow_w*inplay.w[i]/2), 
                    inplay.y[i] - (inplay.h[i]/2) - (card_grow_h*inplay.h[i]/2) - (sineh*window_scale_y), 
                    inplay.w[i] + (card_grow_w*inplay.w[i]*window_scale_x), 
                    inplay.h[i] + (card_grow_h*inplay.h[i]*window_scale_y)
                };

                center = (SDL_FPoint) {cardtexture.w / 2, cardtexture.h / 2};
                SDL_RenderTextureRotated(renderer, cards.cardtexture[inplay.ID[i]], NULL, &cardtexture, angle, &center, SDL_FLIP_NONE);
                if (inplay.rarity[i] > 0) SDL_RenderTextureRotated(renderer, texture_rarity[inplay.rarity[i]], NULL, &cardtexture, angle, &center, SDL_FLIP_NONE);

                if (!inplay.isSellable[i]) {
                    int width = 50, height = 50;
                    cardtexture = (SDL_FRect) {
                        inplay.x[i] - (inplay.w[i]/2) - (card_grow_w*inplay.w[i]/2), 
                        inplay.y[i] - (inplay.h[i]/2) - (card_grow_h*inplay.h[i]/2) - (sineh*window_scale_y),
                        (width + card_grow_w*width) * window_scale_x,
                        (height + card_grow_h*height) * window_scale_y
                    };
                    SDL_RenderTextureRotated(renderer, texture_event, NULL, &cardtexture, (double)angle, &center, SDL_FLIP_NONE);
                }
            }
            if (show_hitboxes) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                cardhitbox = (SDL_FRect) {inplay.x[i] - inplay.w[i]/2, inplay.y[i] - inplay.h[i]/2 , inplay.w[i], inplay.h[i]};
                SDL_RenderRect(renderer, &cardhitbox);
            }
        }

        if (gamestate == STATE_MENU) {
            if (show_textures) {
                
                card_grow_w = (infeature.isDragging[i]+infeature.isHover[i]) * CARD_GROW;
                card_grow_h = (infeature.isDragging[i]+infeature.isHover[i]) * CARD_GROW;

                sineh = 0, sinea = 0, fanning = 0;
                if (!infeature.isDragging[i]) {
                    sineh = 5 *  sin(2 * infeature.zoneTime[i]);
                    fanning = (infeature.zoneNum[i] - ((playzones.num_cards[infeature.zoneID[i]] + 1) / 2.0f)) * 2.5f;
                    sinea = 2.5f * sin(infeature.zoneTime[i]);
                }
                angle = infeature.vx[i]/60 + sinea + fanning;

                cardtexture = (SDL_FRect) {
                    infeature.x[i] - (infeature.w[i]/2) - (card_grow_w*infeature.w[i]/2), 
                    infeature.y[i] - (infeature.h[i]/2) - (card_grow_h*infeature.h[i]/2) - (sineh*window_scale_y), 
                    infeature.w[i] + (card_grow_w*infeature.w[i]*window_scale_x), 
                    infeature.h[i] + (card_grow_h*infeature.h[i]*window_scale_y)
                };

                center = (SDL_FPoint) {cardtexture.w / 2, cardtexture.h / 2};
                SDL_RenderTextureRotated(renderer, cards.cardtexture[infeature.ID[i]], NULL, &cardtexture, angle, &center, SDL_FLIP_NONE);
                if (infeature.rarity[i] > 0) SDL_RenderTextureRotated(renderer, texture_rarity[infeature.rarity[i]], NULL, &cardtexture, angle, &center, SDL_FLIP_NONE);

                if (!infeature.isSellable[i]) {
                    int width = 50, height = 50;
                    cardtexture = (SDL_FRect) {
                        infeature.x[i] - (infeature.w[i]/2) - (card_grow_w*infeature.w[i]/2), 
                        infeature.y[i] - (infeature.h[i]/2) - (card_grow_h*infeature.h[i]/2) - (sineh*window_scale_y),
                        (width + card_grow_w*width) * window_scale_x,
                        (height + card_grow_h*height) * window_scale_y
                    };
                    SDL_RenderTextureRotated(renderer, texture_event, NULL, &cardtexture, (double)angle, &center, SDL_FLIP_NONE);
                }
            }
            if (show_hitboxes) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                cardhitbox = (SDL_FRect) {infeature.x[i] - infeature.w[i]/2, infeature.y[i] - infeature.h[i]/2 , infeature.w[i], infeature.h[i]};
                SDL_RenderRect(renderer, &cardhitbox);
            }
        }

        if (gamestate == STATE_DRAFT) {
            if (show_textures) {
                
                card_grow_w = (indraft.isDragging[i]+indraft.isHover[i]) * CARD_GROW;
                card_grow_h = (indraft.isDragging[i]+indraft.isHover[i]) * CARD_GROW;

                sineh = 0, sinea = 0, fanning = 0;
                if (!indraft.isDragging[i]) {
                    sineh = 5 *  sin(2 * indraft.zoneTime[i]);
                    fanning = (indraft.zoneNum[i] - ((playzones.num_cards[indraft.zoneID[i]] + 1) / 2.0f)) * 2.5f;
                    sinea = 2.5f * sin(indraft.zoneTime[i]);
                }
                angle = indraft.vx[i]/60 + sinea + fanning;

                cardtexture = (SDL_FRect) {
                    indraft.x[i] - (indraft.w[i]/2) - (card_grow_w*indraft.w[i]/2), 
                    indraft.y[i] - (indraft.h[i]/2) - (card_grow_h*indraft.h[i]/2) - (sineh*window_scale_y), 
                    indraft.w[i] + (card_grow_w*indraft.w[i]*window_scale_x), 
                    indraft.h[i] + (card_grow_h*indraft.h[i]*window_scale_y)
                };

                center = (SDL_FPoint) {cardtexture.w / 2, cardtexture.h / 2};
                SDL_RenderTextureRotated(renderer, cards.cardtexture[indraft.ID[i]], NULL, &cardtexture, angle, &center, SDL_FLIP_NONE);
                if (indraft.rarity[i] > 0) SDL_RenderTextureRotated(renderer, texture_rarity[indraft.rarity[i]], NULL, &cardtexture, angle, &center, SDL_FLIP_NONE);

                if (!indraft.isSellable[i]) {
                    int width = 50, height = 50;
                    cardtexture = (SDL_FRect) {
                        indraft.x[i] - (indraft.w[i]/2) - (card_grow_w*indraft.w[i]/2), 
                        indraft.y[i] - (indraft.h[i]/2) - (card_grow_h*indraft.h[i]/2) - (sineh*window_scale_y),
                        (width + card_grow_w*width) * window_scale_x,
                        (height + card_grow_h*height) * window_scale_y
                    };
                    SDL_RenderTextureRotated(renderer, texture_event, NULL, &cardtexture, (double)angle, &center, SDL_FLIP_NONE);
                }
            }
            if (show_hitboxes) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                cardhitbox = (SDL_FRect) {indraft.x[i] - indraft.w[i]/2, indraft.y[i] - indraft.h[i]/2 , indraft.w[i], indraft.h[i]};
                SDL_RenderRect(renderer, &cardhitbox);
            }
        }
    }
    
    // TOP CARD TEXTURE (GAME ONLY)
    // ----------------------------
    for (int i = 0; i < MAX_CARDS; i++) {
        if (gamestate != STATE_PLAY && gamestate != STATE_DRAFT) break;
        if (!inplay.isDragging[i] && !indraft.isDragging[i]) continue;
        // if (!inplay.isDragging[i] && !infeature.isDragging[i] && abs(inplay.vx[i]) < 1 && abs(inplay.vy[i]) < 1) continue;

        SDL_FRect cardtexture = {0};
        SDL_FRect cardhitbox = {0};
        SDL_FPoint center = {0};
        float sineh, sinea, fanning, card_grow_w, card_grow_h;
        double angle;

        if (gamestate == STATE_PLAY) {
            if (show_textures) {

                card_grow_w = (inplay.isDragging[i]+inplay.isHover[i]) * CARD_GROW;
                card_grow_h = (inplay.isDragging[i]+inplay.isHover[i]) * CARD_GROW;

                sineh = 0, sinea = 0, fanning = 0;
                if (!inplay.isDragging[i]) {
                    sineh = 5 *  sin(2 * inplay.zoneTime[i]);
                    fanning = (inplay.zoneNum[i] - ((playzones.num_cards[inplay.zoneID[i]] + 1) / 2.0f)) * 2.5f;
                    sinea = 2.5f * sin(inplay.zoneTime[i]);
                }
                angle = inplay.vx[i]/60 + sinea + fanning;

                cardtexture = (SDL_FRect) {
                    inplay.x[i] - (inplay.w[i]/2) - (card_grow_w*inplay.w[i]/2), 
                    inplay.y[i] - (inplay.h[i]/2) - (card_grow_h*inplay.h[i]/2) - (sineh*window_scale_y), 
                    inplay.w[i] + (card_grow_w*inplay.w[i]*window_scale_x), 
                    inplay.h[i] + (card_grow_h*inplay.h[i]*window_scale_y)
                };

                center = (SDL_FPoint) {cardtexture.w / 2, cardtexture.h / 2};
                SDL_RenderTextureRotated(renderer, cards.cardtexture[inplay.ID[i]], NULL, &cardtexture, angle, &center, SDL_FLIP_NONE);
                if (inplay.rarity[i] > 0) SDL_RenderTextureRotated(renderer, texture_rarity[inplay.rarity[i]], NULL, &cardtexture, angle, &center, SDL_FLIP_NONE);

                if (!inplay.isSellable[i]) {
                    int width = 50, height = 50;
                    cardtexture = (SDL_FRect) {
                        inplay.x[i] - (inplay.w[i]/2) - (card_grow_w*inplay.w[i]/2), 
                        inplay.y[i] - (inplay.h[i]/2) - (card_grow_h*inplay.h[i]/2) - (sineh*window_scale_y),
                        (width + card_grow_w*width) * window_scale_x,
                        (height + card_grow_h*height) * window_scale_y
                    };
                    SDL_RenderTextureRotated(renderer, texture_event, NULL, &cardtexture, (double)angle, &center, SDL_FLIP_NONE);
                }
            }
            if (show_hitboxes) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                cardhitbox = (SDL_FRect) {inplay.x[i] - inplay.w[i]/2, inplay.y[i] - inplay.h[i]/2 , inplay.w[i], inplay.h[i]};
                SDL_RenderRect(renderer, &cardhitbox);
            }
        }
        
        if (gamestate == STATE_DRAFT) {
            if (show_textures) {
                
                card_grow_w = (indraft.isDragging[i]+indraft.isHover[i]) * CARD_GROW;
                card_grow_h = (indraft.isDragging[i]+indraft.isHover[i]) * CARD_GROW;

                sineh = 0, sinea = 0, fanning = 0;
                if (!indraft.isDragging[i]) {
                    sineh = 5 *  sin(2 * indraft.zoneTime[i]);
                    fanning = (indraft.zoneNum[i] - ((playzones.num_cards[indraft.zoneID[i]] + 1) / 2.0f)) * 2.5f;
                    sinea = 2.5f * sin(indraft.zoneTime[i]);
                }
                angle = indraft.vx[i]/60 + sinea + fanning;

                cardtexture = (SDL_FRect) {
                    indraft.x[i] - (indraft.w[i]/2) - (card_grow_w*indraft.w[i]/2), 
                    indraft.y[i] - (indraft.h[i]/2) - (card_grow_h*indraft.h[i]/2) - (sineh*window_scale_y), 
                    indraft.w[i] + (card_grow_w*indraft.w[i]*window_scale_x), 
                    indraft.h[i] + (card_grow_h*indraft.h[i]*window_scale_y)
                };

                center = (SDL_FPoint) {cardtexture.w / 2, cardtexture.h / 2};
                SDL_RenderTextureRotated(renderer, cards.cardtexture[indraft.ID[i]], NULL, &cardtexture, angle, &center, SDL_FLIP_NONE);
                if (indraft.rarity[i] > 0) SDL_RenderTextureRotated(renderer, texture_rarity[indraft.rarity[i]], NULL, &cardtexture, angle, &center, SDL_FLIP_NONE);

                if (!indraft.isSellable[i]) {
                    int width = 50, height = 50;
                    cardtexture = (SDL_FRect) {
                        indraft.x[i] - (indraft.w[i]/2) - (card_grow_w*indraft.w[i]/2), 
                        indraft.y[i] - (indraft.h[i]/2) - (card_grow_h*indraft.h[i]/2) - (sineh*window_scale_y),
                        (width + card_grow_w*width) * window_scale_x,
                        (height + card_grow_h*height) * window_scale_y
                    };
                    SDL_RenderTextureRotated(renderer, texture_event, NULL, &cardtexture, (double)angle, &center, SDL_FLIP_NONE);
                }
            }
            if (show_hitboxes) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                cardhitbox = (SDL_FRect) {indraft.x[i] - indraft.w[i]/2, indraft.y[i] - indraft.h[i]/2 , indraft.w[i], indraft.h[i]};
                SDL_RenderRect(renderer, &cardhitbox);
            }
        }
    }





    // ANIMATIONS
    // ----------
    for (int i = 0; i < MAX_ANIMATIONS; i++ ) {
        if (!anim.isActive[i] || !show_textures) continue;

        if (anim.ID[i] == ANIM_TRANSITION_1) {
            float cosine = (WINDOW_WIDTH+40)/4 * -cosf(anim.runtime[i]) + (WINDOW_WIDTH+40)/4;
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_FRect rect1 = { cosine*window_scale_x, 0, (WINDOW_WIDTH+40)/-2*window_scale_x, WINDOW_HEIGHT*window_scale_y};
            SDL_FRect rect2 = { (WINDOW_WIDTH-cosine)*window_scale_x, 0, (WINDOW_WIDTH+40)/2*window_scale_x, WINDOW_HEIGHT*window_scale_y};
            SDL_RenderFillRect(renderer, &rect1);
            SDL_RenderFillRect(renderer, &rect2);
        }

        if (anim.ID[i] == ANIM_TRANSITION_2) {
            float cosine = (WINDOW_WIDTH+40)/2 * -cosf(anim.runtime[i]) + (WINDOW_WIDTH+40)/2;
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_FRect rect = { cosine*window_scale_x, 0, -(WINDOW_WIDTH+40)*window_scale_x, WINDOW_HEIGHT*window_scale_y};
            SDL_RenderFillRect(renderer, &rect);
        }

        if (anim.ID[i] == ANIM_TRANSITION_3) {
            float cosine = (WINDOW_HEIGHT+40)/2 * -cosf(anim.runtime[i]) + (WINDOW_HEIGHT+40)/2;
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_FRect rect = { 0, cosine*window_scale_y, WINDOW_WIDTH*window_scale_x, -(WINDOW_HEIGHT+40)*window_scale_y};
            SDL_RenderFillRect(renderer, &rect);
        }

        if (anim.ID[i] == ANIM_PRESENT_CARD) {
            float cosine = WINDOW_HEIGHT/2 * -cosf(anim.runtime[i]);
            double spin = (CARD_WIDTH*1.25*window_scale_x) * sinf(anim.runtime[i] - 1);
            double angle = 0;

            SDL_FRect cardtexture = {  
                window_width/2 - (float)(spin/2), 
                (WINDOW_HEIGHT-cosine-CARD_HEIGHT-100)*window_scale_y, 
                (float)(spin), 
                (float)(CARD_HEIGHT*1.25*window_scale_x)
            };

            SDL_FPoint center = {cardtexture.w / 2, cardtexture.h / 2};
            if (spin > 0) {
                SDL_RenderTextureRotated(renderer, cards.cardtexture[(int)anim.value[i]], NULL, &cardtexture, angle, &center, SDL_FLIP_NONE);
            } else {
                SDL_RenderTextureRotated(renderer, gamebuttons.buttontexture[BUTTON_DECK], NULL, &cardtexture, angle, &center, SDL_FLIP_NONE);
            }
        }

    }

    // PAUSE OVERLAY
    // -------------
    if (pause) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 175);
        SDL_FRect rect = {0, 0, (float)window_width, (float)window_height};
        SDL_RenderFillRect(renderer, &rect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 175);
        SDL_FRect rect2 = {(float)(window_width/2 - 50/2), (float)(window_height/2 - 50/2), 50.0f, 50.0f};
        SDL_RenderFillRect(renderer, &rect2);
    }

    // CURSOR TEXTURES / HITBOX
    // ------------------------
    if (custom_cursor) {
        SDL_HideCursor();

        SDL_FRect cursorbox = {mousex, mousey, 30.0f * window_scale_x, 30.0f * window_scale_y};
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        if (show_textures) SDL_RenderTexture(renderer, gamebuttons.buttontexture[0], NULL, &cursorbox);
        if (show_hitboxes) SDL_RenderRect(renderer, &cursorbox);

    } else { SDL_ShowCursor(); }

}

// MAIN FUNCTION ====================================================================================================

int main() {

    adjust_window_size();

    SDL_Init(SDL_INIT_VIDEO);
    TTF_Init();
    window = SDL_CreateWindow(stringf("%s %s", title, version), (int)(WINDOW_WIDTH), (int)(WINDOW_HEIGHT), SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
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
        control_fps(TARGET_FPS);
    }

    sleep(0.5);
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();
    return 0;
}
