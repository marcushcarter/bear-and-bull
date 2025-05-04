#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <stdio.h>
#include <math.h>

// Vertex Shader (wiggle + twist effect)
const char* vertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec3 ourColor;\n"
"uniform float time;\n"
"void main() {\n"
"    // Apply twisting effect along the z-axis based on y position and time\n"
"    float twistFactor = 0.5; // Controls the twist intensity\n"
"    float twistAmount = twistFactor * aPos.y + time; // Twisting effect increases with time\n"
"    vec3 twistedPos = aPos;\n"
"    twistedPos.z += sin(twistAmount) * 0.2; // Apply a sine-based twist to the z-coordinate\n"
"    // Apply wiggle effect on the y-coordinate\n"
"    float frequency = 10.0;\n"
"    float amplitude = 0.05;\n"
"    twistedPos.y += sin(twistedPos.x * frequency + time) * amplitude;\n"
"    gl_Position = vec4(twistedPos, 1.0);\n"
"    ourColor = aColor;\n"
"}\n";

// Fragment Shader (outputs color)
const char* fragmentShaderSource = "#version 330 core\n"
"in vec3 ourColor;\n"
"out vec4 FragColor;\n"
"void main() {\n"
"    FragColor = vec4(ourColor, 1.0);\n"
"}\n";

// Vertex Shader for screen quad
const char* screenVertexShaderSource = "#version 330 core\n"
"layout (location = 0) in vec2 aPos;\n"
"layout (location = 1) in vec2 aTexCoord;\n"
"out vec2 TexCoord;\n"
"void main() {\n"
"    TexCoord = aTexCoord;\n"
"    gl_Position = vec4(aPos.xy, 0.0, 1.0);\n"
"}\n";

// Pixelation Fragment Shader
const char* pixelFragmentShaderSource = "#version 330 core\n"
"in vec2 TexCoord;\n"
"out vec4 FragColor;\n"
"uniform sampler2D screenTexture;\n"
"uniform vec2 resolution;\n"
"uniform float pixelSize;\n"
"void main() {\n"
"    vec2 uv = TexCoord;\n"
"    vec2 block = pixelSize / resolution;\n"
"    vec2 pixelUV = floor(uv / block) * block;\n"
"    FragColor = texture(screenTexture, pixelUV);\n"
"}\n";

// Error checker
void checkCompileErrors(GLuint shader, const char* type) {
    int success;
    char infoLog[512];
    if (strcmp(type, "PROGRAM") == 0) {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 512, NULL, infoLog);
            printf("ERROR::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
        }
    } else {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            printf("ERROR::SHADER::%s::COMPILATION_FAILED\n%s\n", type, infoLog);
        }
    }
}

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window* window = SDL_CreateWindow("Pixelated Twisting Triangle", 800, 600, SDL_WINDOW_OPENGL);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        printf("Failed to initialize GLAD\n");
        return -1;
    }

    // Triangle vertices
    float triangleVertices[] = {
        // pos         // color
         0.0f,  0.5f, 0.0f,   1, 0, 0,
        -0.5f, -0.5f, 0.0f,   0, 1, 0,
         0.5f, -0.5f, 0.0f,   0, 0, 1
    };

    // Quad for fullscreen render
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,
         1.0f, -1.0f,   1.0f, 0.0f,

        -1.0f,  1.0f,   0.0f, 1.0f,
         1.0f, -1.0f,   1.0f, 0.0f,
         1.0f,  1.0f,   1.0f, 1.0f
    };

    GLuint triangleVAO, triangleVBO;
    glGenVertexArrays(1, &triangleVAO);
    glGenBuffers(1, &triangleVBO);
    glBindVertexArray(triangleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Compile shaders
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vShader);
    checkCompileErrors(vShader, "VERTEX");

    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fShader);
    checkCompileErrors(fShader, "FRAGMENT");

    GLuint triangleProgram = glCreateProgram();
    glAttachShader(triangleProgram, vShader);
    glAttachShader(triangleProgram, fShader);
    glLinkProgram(triangleProgram);
    checkCompileErrors(triangleProgram, "PROGRAM");

    GLuint screenVShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(screenVShader, 1, &screenVertexShaderSource, NULL);
    glCompileShader(screenVShader);
    checkCompileErrors(screenVShader, "VERTEX");

    GLuint screenFShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(screenFShader, 1, &pixelFragmentShaderSource, NULL);
    glCompileShader(screenFShader);
    checkCompileErrors(screenFShader, "FRAGMENT");

    GLuint screenProgram = glCreateProgram();
    glAttachShader(screenProgram, screenVShader);
    glAttachShader(screenProgram, screenFShader);
    glLinkProgram(screenProgram);
    checkCompileErrors(screenProgram, "PROGRAM");

    glDeleteShader(vShader);
    glDeleteShader(fShader);
    glDeleteShader(screenVShader);
    glDeleteShader(screenFShader);

    // Framebuffer setup
    GLuint fbo, fboTexture, rbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &fboTexture);
    glBindTexture(GL_TEXTURE_2D, fboTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // no smoothing
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboTexture, 0);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 800, 600);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        printf("Framebuffer is not complete!\n");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLint timeLoc = glGetUniformLocation(triangleProgram, "time");
    GLint resolutionLoc = glGetUniformLocation(screenProgram, "resolution");
    GLint pixelSizeLoc = glGetUniformLocation(screenProgram, "pixelSize");

    int running = 1;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = 0;
        }

        float time = SDL_GetTicks() / 1000.0f;

        // Render to FBO
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glViewport(0, 0, 800, 600);
        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(triangleProgram);
        glUniform1f(timeLoc, time);
        glBindVertexArray(triangleVAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        // Render to screen with pixelation
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.08, 0.08, 0.08, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(screenProgram);
        glUniform2f(resolutionLoc, 800.0f, 600.0f);
        glUniform1f(pixelSizeLoc, 6.0f);
        glBindTexture(GL_TEXTURE_2D, fboTexture);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        SDL_GL_SwapWindow(window);
    }

    SDL_DestroyWindow(window);
    SDL_GL_DestroyContext(gl_context);
    SDL_Quit();
    return 0;
}
