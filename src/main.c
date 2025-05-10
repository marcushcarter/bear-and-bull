#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <stdio.h>
#include <stdbool.h>

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "uniform vec2 offset;\n"
    "void main() {\n"
    "   gl_Position = vec4(aPos + offset, 0.0, 1.0);\n"
    "}\0";

const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "   FragColor = vec4(1.0, 1.0, 1.0, 1.0);\n"
    "}\0";


SDL_Window* window;
SDL_GLContext glContext;
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
float window_scale = 1.0f;

#define SCREEN_WIDTH 1500
#define SCREEN_HEIGHT 1000

// Box vertices (2 triangles for a square)
float quad[] = {
    -0.1f, -0.1f,
     0.1f, -0.1f,
     0.1f,  0.1f,
    -0.1f, -0.1f,
     0.1f,  0.1f,
    -0.1f,  0.1f
};

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

int main() {
    SDL_Init(SDL_INIT_VIDEO);

    window = SDL_CreateWindow("Box Movement", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    glContext = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, glContext);
    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

    SDL_GetWindowSize(window, &window_width, &window_height);
    glViewport(0, 0, window_width, window_height);

    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    // Create shader program
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Set up VAO and VBO
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    float offsetX = 0.0f;
    float offsetY = 0.0f;

    running = true;

    const Uint8* sdlKeys = (Uint8*)SDL_GetKeyboardState(NULL);
    SDL_memcpy(currKeyState, sdlKeys, NUM_KEYS);
    SDL_memcpy(prevKeyState, sdlKeys, NUM_KEYS);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    while (running) {

        SDL_GetWindowSize(window, &window_width, &window_height);
        glViewport(0, 0, window_width, window_height);

        dt = get_delta_time();
        if (dt > 0.3) continue;

        // INPUTS FUNCTION

        while (SDL_PollEvent(&event) != 0) {
            if (event.type == SDL_EVENT_QUIT) running = false;
        }
        const Uint8* sdlKeys = (Uint8*)SDL_GetKeyboardState(NULL);
        SDL_memcpy(currKeyState, sdlKeys, NUM_KEYS);
        prevMouseState = currMouseState;
        currMouseState = SDL_GetMouseState(&mousex, &mousey);
        mousex = mousex/(window_width/2)-1;
        mousey = -mousey/(window_height/2)+1;
        
        // UPDATE

        // RENDER

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        GLint offsetLocation = glGetUniformLocation(shaderProgram, "offset");
        glUniform2f(offsetLocation, mousex, mousey);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glUseProgram(shaderProgram);
        offsetLocation = glGetUniformLocation(shaderProgram, "offset");
        glUniform2f(offsetLocation, 0, 0);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        SDL_GL_SwapWindow(window);

        // OTHER

        SDL_memcpy(prevKeyState, currKeyState, NUM_KEYS);
        control_fps(120.0f);
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    SDL_GL_DestroyContext(glContext);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
