
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
