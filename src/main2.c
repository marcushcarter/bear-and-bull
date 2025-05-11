#include <SDL3/SDL.h>
#include <glad/glad.h>

#include <shaders.h>

#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

SDL_Window* window;
SDL_GLContext gl_context;
SDL_Event event;
bool running;

#define NUM_KEYS 256
Uint8 currKeyState[NUM_KEYS];
Uint8 prevKeyState[NUM_KEYS];
Uint32 currMouseState;
Uint32 prevMouseState;
float mousex, mousey, mousexndc, mouseyndc;

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

// MAIN FUNCTION ----------------------------------------------------------------------------------------------------

float quadVertices[] = {
    // positions         // colors
     -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // top left
     0.5f,   -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // top right
     0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,   // bottom left 
     -0.5f,  0.5f, 0.0f,  0.0f, 0.0f, 0.0f   // bottom right 
};

float quadIndices[] = {
    0, 1, 2,
    2, 3, 4
};
int main() {

    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    window = SDL_CreateWindow("OpenGL Window", 1500, 1000, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    gl_context = SDL_GL_CreateContext(window);
    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

    const Uint8* sdlKeys = (Uint8*)SDL_GetKeyboardState(NULL);
    SDL_memcpy(currKeyState, sdlKeys, NUM_KEYS);
    SDL_memcpy(prevKeyState, sdlKeys, NUM_KEYS);










    // compile the shaders
    // -------------------
    
    Shader ourShader("shaders/shader.vert", "shaders/shader.frag"); // you can name your shader files however you like








    // steup vertex data and buffers and configure vertex attributes
    // -------------------------------------------------------------

    unsigned int VBO, VAO, EBO; // initialize the buffers (Vertex Buffer, Vertex Array, Element Buffer)
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO); // sets any buffer calls we make to the GL_ARRAY_BUFFER
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW); // copies vertices to the currently bound buffer (GL_ARRAY_BUFFER)

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), quadIndices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture coord attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);








    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);


    running = true;
    while (running) {

        dt = get_delta_time();
        if (dt > 0.3) continue;



        // inputs

        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_EVENT_QUIT) running = false;
        }
        const Uint8* sdlKeys = (Uint8*)SDL_GetKeyboardState(NULL);
        SDL_memcpy(currKeyState, sdlKeys, NUM_KEYS);
        prevMouseState = currMouseState;
        currMouseState = SDL_GetMouseState(&mousex, &mousey);
        mousexndc = mousex/1500/2-1;
        mouseyndc = mousey/1000/2+1;

        int window_width, window_height;
        SDL_GetWindowSize(window, &window_width, &window_height);
        glViewport(0, 0, window_width, window_height);

        // updates
        // ------

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        
        ourShader.use();
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        SDL_GL_SwapWindow(window);

        // other
        // ------

        SDL_memcpy(prevKeyState, currKeyState, NUM_KEYS);
        control_fps(120.0f);
    }

    SDL_DestroyWindow(window);
    SDL_GL_DestroyContext(gl_context);
    SDL_Quit();
    return 0;
}
