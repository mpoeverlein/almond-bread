#define GLAD_GL_IMPLEMENTATION
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <vector>
#include <iostream> 
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include "linmath.h" 


std::string loadShaderFile(const char* filePath) {
    std::ifstream file;
    std::stringstream buffer;
    
    file.open(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << filePath << std::endl;
        return "";
    }
    
    buffer << file.rdbuf();
    file.close();
    return buffer.str();
}

unsigned int compileShader(unsigned int type, const char* source) {
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, nullptr);
    glCompileShader(id);
    
    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(id, 512, nullptr, infoLog);
        std::cerr << "Shader compilation error:\n" << infoLog << std::endl;
    }
    return id;
}

unsigned int createShaderProgram(unsigned int vs, unsigned int fs) {
    unsigned int program = glCreateProgram();
    
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    
    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader program linking error:\n" << infoLog << std::endl;
    }
    
    glDeleteShader(vs);
    glDeleteShader(fs);
    
    return program;
}

int maxRepetitions = 10;
int defaultMaxRepetitions = maxRepetitions;
int width = 800;
int height = 600;
float aspectRatio = (float) width / height;
float xCenter = 0;
float yCenter = 0;
float zoomFactor = 1.f;
float defaultXCenter = xCenter;
float defaultYCenter = yCenter;
float defaultZoomFactor = zoomFactor;
int currentFuncIndex = 0; // or 1, etc.
const int lengthFuncIndices = 4;

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        float moveSpeed = 0.1f /  zoomFactor; // Slower pan when zoomed in
        switch (key) {
            case GLFW_KEY_W: yCenter += moveSpeed; break;
            case GLFW_KEY_S: yCenter -= moveSpeed; break;
            case GLFW_KEY_A: xCenter -= moveSpeed; break;
            case GLFW_KEY_D: xCenter += moveSpeed; break;
            case GLFW_KEY_Q: zoomFactor *= 1.1f; break; // Zoom in
            case GLFW_KEY_E: zoomFactor /= 1.1f; break; // Zoom out
            case GLFW_KEY_M: maxRepetitions += 10; break;
            case GLFW_KEY_N: maxRepetitions = std::max(10,maxRepetitions-10); break;
            case GLFW_KEY_C: currentFuncIndex = (currentFuncIndex + 1) % lengthFuncIndices; break;
            case GLFW_KEY_UP: yCenter += moveSpeed; break;
            case GLFW_KEY_DOWN: yCenter -= moveSpeed; break;
            case GLFW_KEY_LEFT: xCenter -= moveSpeed; break;
            case GLFW_KEY_RIGHT: xCenter += moveSpeed; break;
            case GLFW_KEY_COMMA: zoomFactor *= 1.1f; break; // Zoom in
            case GLFW_KEY_PERIOD: zoomFactor /= 1.1f; break; // Zoom out
            case GLFW_KEY_R: zoomFactor = defaultZoomFactor; xCenter = defaultXCenter; yCenter = defaultYCenter; maxRepetitions = defaultMaxRepetitions; break;
        }
    }
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    zoomFactor *= (yoffset > 0) ? 1.1f : 0.9f;
}


int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create window
    GLFWwindow* window = glfwCreateWindow(width, height, "Mandelbrot Explorer", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    std::string vertSource = loadShaderFile("shaders/texture.vert");
    std::string fragSource = loadShaderFile("shaders/texture.frag");
    
    if (vertSource.empty() || fragSource.empty()) {
        // Handle error (shader failed to load)
        return -1;
    }
    
    unsigned int vertexShader = compileShader(GL_VERTEX_SHADER, vertSource.c_str());
    unsigned int fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragSource.c_str());
    unsigned int shaderProgram = createShaderProgram(vertexShader, fragmentShader);

    GLint zoomLoc = glGetUniformLocation(shaderProgram, "zoom");
    GLint centerLoc = glGetUniformLocation(shaderProgram, "center");
    GLint nRepsLoc = glGetUniformLocation(shaderProgram, "maxRepetitions");
    GLint aspectLoc = glGetUniformLocation(shaderProgram, "aspectRatio");

    // Set up vertex data and buffers
    float vertices[] = {
        // positions     // texture coords
        0.0f, 0.0f,  0.0f, 0.0f,  // bottom-left
        (float) width, 0.0f,  1.0f, 0.0f,  // bottom-right
        (float) width, (float) height,  1.0f, 1.0f,  // top-right
        0.0f, (float) height,  0.0f, 1.0f   // top-left
    };

    unsigned int indices[] = {
        0, 1, 2,  // first triangle
        0, 2, 3   // second triangle
    };

    unsigned int VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Create texture
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Allocate texture storage
    const int texWidth = width;
    const int texHeight = height;
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, texWidth, texHeight, 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);


    mat4x4 model, projection;
    mat4x4_identity(model);
    mat4x4_ortho(projection, 0.0f, (float) width, 0.0f, (float) height, 0.0f, 1.0f);

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        glfwGetWindowSize(window, &width, &height);
        aspectRatio = width / (float) height;

        glUniform1f(zoomLoc, zoomFactor);
        glUniform2f(centerLoc, xCenter, yCenter);
        glUniform1i(nRepsLoc, maxRepetitions);
        glUniform1f(aspectLoc, aspectRatio);
        glUniform1i(glGetUniformLocation(shaderProgram, "colorFuncIndex"), currentFuncIndex);
        // std::cout << "ZOOM FACTOR " << zoomFactor << "X" << xCenter << "Y" << yCenter << "\n";


        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Activate shader
        glUseProgram(shaderProgram);
        
        // Set uniforms
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
        
        // Bind texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
        
        // Draw quad
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}