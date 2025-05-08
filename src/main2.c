// Includes
#include <SDL3/SDL.h>
#include <glad/glad.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <stdio.h>
#include <stdbool.h>

// Constants
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define MAX_CARDS 20

// Structures
typedef struct {
    int id;
    int power;
    GLuint texture;
} Card;

Card cardTypes[MAX_CARDS];
int totalCardTypes = 0;

bool isCardInPlay[MAX_CARDS];
int cardInPlay[MAX_CARDS];
int numCardsInPlay = 0;

// Shader sources
const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in vec2 aTexCoord;\n"
    "out vec2 TexCoord;\n"
    "uniform vec2 uPosition;\n"
    "uniform vec2 uScale;\n"
    "void main()\n"
    "{\n"
    "    vec2 scaled = aPos * uScale + uPosition;\n"
    "    gl_Position = vec4((scaled.x / 400.0 - 1.0), -(scaled.y / 300.0 - 1.0), 0.0, 1.0);\n"
    "    TexCoord = aTexCoord;\n"
    "}";

const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec2 TexCoord;\n"
    "uniform sampler2D uTexture;\n"
    "void main()\n"
    "{\n"
    "    FragColor = texture(uTexture, TexCoord);\n"
    "}";

// Shader program
GLuint shaderProgram;

// Quad VBO/VAO
GLuint VBO, VAO;

void createQuad() {
    float vertices[] = {
        // pos      // tex
        0.0f, 1.0f,  0.0f, 1.0f,
        1.0f, 1.0f,  1.0f, 1.0f,
        0.0f, 0.0f,  0.0f, 0.0f,

        1.0f, 1.0f,  1.0f, 1.0f,
        1.0f, 0.0f,  1.0f, 0.0f,
        0.0f, 0.0f,  0.0f, 0.0f,
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

GLuint compileShader(const char* src, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, NULL);
    glCompileShader(shader);
    return shader;
}

void setupShaders() {
    GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

GLuint loadTexture(const char* path) {
    int w, h, ch;
    unsigned char* data = stbi_load(path, &w, &h, &ch, STBI_rgb_alpha);
    if (!data) return 0;

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_image_free(data);

    return tex;
}

void loadCards() {
    const char* filenames[] = {"card1.png", "card2.png", "card3.png"};
    int powers[] = {5, 8, 3};
    for (int i = 0; i < 3; i++) {
        cardTypes[i].texture = loadTexture(filenames[i]);
        cardTypes[i].power = powers[i];
        cardTypes[i].id = i;
    }
    totalCardTypes = 3;
}

void drawCard(int id, float x, float y) {
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    glBindTexture(GL_TEXTURE_2D, cardTypes[id].texture);
    
    glUniform2f(glGetUniformLocation(shaderProgram, "uPosition"), x, y);
    glUniform2f(glGetUniformLocation(shaderProgram, "uScale"), 100.0f, 140.0f);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Cards", SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    SDL_GLContext glctx = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, glctx);
    SDL_GL_SetSwapInterval(1);

    gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    createQuad();
    setupShaders();
    loadCards();

    bool running = true;
    SDL_Event e;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
        }

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        for (int i = 0; i < totalCardTypes; i++) {
            drawCard(i, 150.0f * i + 50.0f, 200.0f);
        }

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DestroyContext(glctx);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
