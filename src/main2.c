#include <SDL3/SDL.h>
#include <stdlib.h>
#include <time.h>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600
#define MAX_SECONDS 60

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Stock Simulator", WINDOW_WIDTH, WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    srand((unsigned int)time(NULL));

    // Chart parameters
    float chartX = 50, chartY = 50, chartWidth = 700, chartHeight = 500;
    float price = 100.0f, priceMin = 50.0f, priceMax = 200.0f;

    SDL_FPoint points[MAX_SECONDS];
    int currentSecond = 0;
    Uint32 startTime = SDL_GetTicks();
    Uint32 lastUpdate = startTime;

    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT)
                running = false;
        }

        Uint32 now = SDL_GetTicks();
        Uint32 elapsedTime = now - startTime;

        // Update price every second
        if (elapsedTime >= (currentSecond + 1) * 1000 && currentSecond < MAX_SECONDS) {
            float delta = ((rand() % 2001) - 1000) / 100.0f; // -10.0 to +10.0
            price += delta;
            if (price < priceMin) price = priceMin;
            if (price > priceMax) price = priceMax;

            float normX = (float)currentSecond / (MAX_SECONDS - 1);
            float x = chartX + normX * chartWidth;
            float yNorm = (price - priceMin) / (priceMax - priceMin);
            float y = chartY + chartHeight * (1.0f - yNorm);

            points[currentSecond].x = x;
            points[currentSecond].y = y;

            currentSecond++;
        }

        // Render
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // white background
        SDL_RenderClear(renderer);

        // Draw chart background
        SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255);
        SDL_FRect chartRect = { chartX, chartY, chartWidth, chartHeight };
        SDL_RenderRect(renderer, &chartRect);

        // Draw stock line
        if (currentSecond >= 2) {
            SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255); // red line
            SDL_RenderLines(renderer, points, currentSecond);
        }

        SDL_RenderPresent(renderer);

        // Cap FPS
        SDL_Delay(16);

        // Stop after 60 seconds
        if (elapsedTime >= 60000) running = false;
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
