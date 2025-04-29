#define GLAD_GL_IMPLEMENTATION
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
// #include <glm/glm.hpp>
// #include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "linmath.h"
#include "rainbow.h"
 
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <vector>
#include <iostream> 
#include <algorithm>

const float convergence_radius_squared = 4.0f;
// const int MAX_REPS = 100;

typedef struct SampleDimensions
{
    float xCenter;
    float yCenter;
    float zoomFactor;
} SampleDimensions;

// Vertex shader source
const char* vertexShaderSource = R"glsl(
    #version 330 core
    layout (location = 0) in vec2 aPos;
    layout (location = 1) in vec2 aTexCoord;
    out vec2 TexCoord;
    uniform mat4 projection;
    uniform mat4 model;
    void main()
    {
        TexCoord = aTexCoord;
        gl_Position = projection * model * vec4(aPos, 0.0, 1.0);
    }
)glsl";

// Fragment shader source
const char* fragmentShaderSource = R"glsl(
    #version 330 core
    in vec2 TexCoord;
    out vec4 FragColor;
    uniform sampler2D texture1;
    void main()
    {
        FragColor = texture(texture1, TexCoord);
    }
)glsl";

// Function to compile shaders
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

// Function to create shader program
unsigned int createShaderProgram(const char* vertexShader, const char* fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);
    
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

/**
 * @brief
 * For a complex number c = a + bi, count how many iterations it takes
 * until the magnitude of z_n = z^2_n-1 + c is larger than 2.
 * 
 * @param a real value of input complex number
 * @param b imaginary value of input complex number
 * @param maxIterations after how many interations to stop
 */
int iterateMandelbrot(float a, float b, int maxRepetitions)
{
    float tmp_a = a;
    float tmp_b = b;
    for (int i = 0; i < maxRepetitions; ++i) {
        float original_a = tmp_a;
        float original_b = tmp_b;
        tmp_a = original_a*original_a - original_b*original_b + a;
        tmp_b = 2*original_a*original_b + b;
        if (tmp_a*tmp_a + tmp_b*tmp_b > convergence_radius_squared) {
            return i;
        }
    }
    return maxRepetitions;

}

void makeMandelbrot(unsigned char* textureData, int nx, int ny, float aspectRatio, SampleDimensions sd, int maxRepetitions) {
    std::vector<float> r, g, b;
    for (int i = 0; i <= maxRepetitions; ++i) {
        float rr, gg, bb;
        intToRainbowRGB(i, maxRepetitions, rr, gg, bb);
        r.push_back(rr*256);
        g.push_back(gg*256);
        b.push_back(bb*256);
    }
    float xStart = sd.xCenter - sd.zoomFactor / 2;
    float yStart = sd.yCenter - sd.zoomFactor / 2 / aspectRatio;
    float dx = sd.zoomFactor;
    float dy = sd.zoomFactor / aspectRatio;
    for (int y = 0; y < ny; y++) {
        float imag_0 = yStart + (float) y / ny * dy;
        for (int x = 0; x < nx; x++) {
            float real_0 = xStart + (float) x / nx * dx;
            int nReps = iterateMandelbrot(real_0, imag_0, maxRepetitions);
            textureData[(y * nx + x) * 3 + 0] = r[nReps];
            textureData[(y * nx + x) * 3 + 1] = g[nReps];
            textureData[(y * nx + x) * 3 + 2] = b[nReps];
        }
    }
}


int main() {
    int maxRepetitions = 10;
    int defaultMaxRepetitions = maxRepetitions;
    int width = 800;
    int height = 600;
    float aspectRatio = (float) width / height;
    SampleDimensions sd;
    sd.xCenter = 0;
    sd.yCenter = 0;
    sd.zoomFactor = 1.f;
    SampleDimensions default_sd = sd;
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
    GLFWwindow* window = glfwCreateWindow(width, height, "Texture Example", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Create shader program
    unsigned int shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texWidth, texHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Generate some texture data
    unsigned char* textureData = new unsigned char[texWidth * texHeight * 3];
    makeMandelbrot(textureData, texWidth, texHeight, aspectRatio, sd, maxRepetitions);


    // Update entire texture
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texWidth, texHeight, GL_RGB, GL_UNSIGNED_BYTE, textureData);


    mat4x4 model, projection;
    mat4x4_identity(model);
    mat4x4_ortho(projection, 0.0f, (float) width, 0.0f, (float) height, 0.0f, 1.0f);

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        glfwGetWindowSize(window, &width, &height);
        const float aspectRatio = width / (float) height;
        
        bool update_vertices = true;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            sd.xCenter += 0.1f * sd.zoomFactor;
        } else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            sd.xCenter -= 0.1f * sd.zoomFactor;
        } else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            sd.yCenter += 0.1f * sd.zoomFactor;
        } else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            sd.yCenter -= 0.1f * sd.zoomFactor;
        } else if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) {
            sd.zoomFactor /= 1.5f;
        } else if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS) {
            sd.zoomFactor *= 1.5f;
        } else if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS) {
            maxRepetitions += 10;
        } else if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS) {
            maxRepetitions = std::max(10, maxRepetitions/10);
        } else if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
            sd = default_sd;
            maxRepetitions = defaultMaxRepetitions;
        } else {
            update_vertices = false;
        }

        if (update_vertices) {
            makeMandelbrot(textureData, texWidth, texHeight, aspectRatio, sd, maxRepetitions);
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texWidth, texHeight, GL_RGB, GL_UNSIGNED_BYTE, textureData);
        }


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
    delete[] textureData;
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}