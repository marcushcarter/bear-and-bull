#include <SDL3/SDL.h>
#include <math.h>

// Convert degrees to radians
static inline float deg_to_rad(float degrees) {
    return degrees * (M_PI / 180.0f);
}

// Fast, filled rotated rectangle using SDL_RenderGeometry
void DrawRotatedRect(int x, int y, int width, int height, float angleDeg,
                     SDL_Renderer *renderer, SDL_FColor color) {
                        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    // Set blending and color
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    // Convert angle
    float angleRad = deg_to_rad(angleDeg);
    float cos_a = cosf(angleRad);
    float sin_a = sinf(angleRad);

    // Half extents
    float hw = width / 2.0f;
    float hh = height / 2.0f;

    // Corners of the rectangle before rotation (relative to center)
    float local[4][2] = {
        {-hw, -hh},  // top-left
        { hw, -hh},  // top-right
        { hw,  hh},  // bottom-right
        {-hw,  hh},  // bottom-left
    };

    // Final vertex positions (rotated and translated)
    SDL_Vertex verts[4];
    for (int i = 0; i < 4; ++i) {
        float rx = local[i][0] * cos_a - local[i][1] * sin_a;
        float ry = local[i][0] * sin_a + local[i][1] * cos_a;

        verts[i].position.x = x + rx;
        verts[i].position.y = y + ry;
        verts[i].color = color;
        verts[i].tex_coord.x = 0;
        verts[i].tex_coord.y = 0;
    }

    // Two triangles: (0, 1, 2) and (2, 3, 0)
    int indices[6] = {0, 1, 2, 2, 3, 0};

    SDL_RenderGeometry(renderer, NULL, verts, 4, indices, 6);
}


int main(void) {
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow("Transparent Triangle", 800, 600, SDL_WINDOW_RESIZABLE);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    int running = 1;
    SDL_Event e;

    float angle = 0;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) {
                running = 0;
            }
        }

        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255); // white background
        SDL_RenderClear(renderer);

        angle+=1;

        SDL_FColor semiTransparentBlack = {0, 0, 120, 128};
        DrawRotatedRect(400, 300, 200, 100, angle, renderer, semiTransparentBlack);



        SDL_RenderPresent(renderer);
        SDL_Delay(16);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
