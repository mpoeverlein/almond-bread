#define GLAD_GL_IMPLEMENTATION
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
 
#include <linmath.h>
 
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <vector>
#include <iostream>
#include <algorithm> 

typedef struct Vertex
{
    vec2 position;
    vec3 color;
} Vertex;

static const char* vertex_shader_text =
"#version 330\n"
"uniform mat4 MVP;\n"
"in vec3 vCol;\n"
"in vec2 vPos;\n"
"out vec3 color;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    color = vCol;\n"
"}\n";
 
static const char* fragment_shader_text =
"#version 330\n"
"in vec3 color;\n"
"out vec4 fragment;\n"
"void main()\n"
"{\n"
"    fragment = vec4(color, 1.0);\n"
"}\n";
 
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}
 
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

std::vector<float> logisticMap(std::vector<float>& arr, float r, int nRepetitions)
{
    for (size_t i = 0; i < arr.size(); ++i) {
        for (size_t j = 0; j < nRepetitions; j++) {
            arr[i] = r * arr[i] * (1-arr[i]);
	}
    }
    std::vector<float> output;
    for (size_t i = 0; i < arr.size(); ++i) {
	if (std::find(output.begin(), output.end(), arr[i]) == output.end()) {
	    output.push_back(arr[i]);
	}
    }
    return output;
}

int countIterationsElement(float start, float r, int maxRepetitions)
{
    std::vector<float> trajectory;
    float new_element;
    trajectory.push_back(start);
    for (int i = 0; i < maxRepetitions; ++i) {
        new_element = r * trajectory.back() * (1-trajectory.back());
	if (std::find(trajectory.begin(), trajectory.end(), new_element) == trajectory.end()) {
	    trajectory.push_back(new_element);
	} else {
	    return i;
	}
    }
    return maxRepetitions;
}


std::vector<int> countIterations(std::vector<float>& arr, float r, int maxRepetitions)
{
    // every second item counts how many repetitions were necessary to get to the stabilized values
    std::vector<int> output;
    for (size_t i = 0; i < arr.size(); ++i) {
	output.push_back(countIterationsElement(arr[i], r, maxRepetitions));
    }
    return output;
}

float logisticMap(float a, float r, int nRepetitions)
{
    return r * a * (1-a);
}

std::vector<float> copyArray(std::vector<float>& arr)
{
    return arr;
}

// Function written by Deepseek
void intToRainbowRGB(int value, int max_N, float& r, float& g, float& b) {
    // Ensure the value is within the range [0, max_N]
    value = std::max(0, std::min(value, max_N));

    // Normalize the value to a float between 0 and 1
    float normalized = static_cast<float>(value) / max_N;

    // Break the rainbow into 6 segments
    float segmentSize = 1.0f / 6.0f;
    int segment = static_cast<int>(normalized / segmentSize);
    float segmentPosition = (normalized - segment * segmentSize) / segmentSize;

    // Interpolate between colors in each segment
    switch (segment) {
        case 0: // Red to Yellow
            r = 1.0f;
            g = segmentPosition;
            b = 0.0f;
            break;
        case 1: // Yellow to Green
            r = 1.0f - segmentPosition;
            g = 1.0f;
            b = 0.0f;
            break;
        case 2: // Green to Cyan
            r = 0.0f;
            g = 1.0f;
            b = segmentPosition;
            break;
        case 3: // Cyan to Blue
            r = 0.0f;
            g = 1.0f - segmentPosition;
            b = 1.0f;
            break;
        case 4: // Blue to Magenta
            r = segmentPosition;
            g = 0.0f;
            b = 1.0f;
            break;
        case 5: // Magenta to Red
            r = 1.0f;
            g = 0.0f;
            b = 1.0f - segmentPosition;
            break;
        default: // Should never happen
            r = 0.0f;
            g = 0.0f;
            b = 0.0f;
            break;
    }
}

std::vector<Vertex> createVertices(int width, int height)
{

    float margin = 0.1; // how much space between graph and edge of window
    int xSteps = 100;
    float xStart = 0.0;
    float xEnd = 1.0;
    float dx = (xEnd-xStart) / xSteps;
    
    int rSteps = 100;
    float rStart = 0.0;
    float rEnd = 4.0;
    float dr = (rEnd - rStart) / rSteps;
    float r;

    int nIterations = 2000;

    std::vector<float> xInput(xSteps);
    std::vector<float> xInputCopy(xSteps);
    for (size_t j = 0; j < xSteps; ++j) {
            xInput[j] = j*dx;
    }

    std::vector<Vertex> vertices(rSteps*xSteps);
    for (size_t j = 0; j < rSteps; ++j) {
        r = j*dr;
        float x = (r-rStart)/(rEnd - rStart) * 2 * (1-margin) - (1-margin);
        xInputCopy = copyArray(xInput);
        // std::vector<float> yValues = logisticMap(xInputCopy, r, nIterations);
        std::vector<int> colorValues = countIterations(xInputCopy, r, nIterations);

        // for (size_t i = 0; i < yValues.size(); ++i) {
        for (size_t i = 0; i < colorValues.size(); ++i) {
            // float y = (yValues[i]-xStart)/(xEnd-xStart) * 2 * (1-margin) - (1-margin);
            float y = (xInputCopy[i]-xStart)/(xEnd-xStart) * 2 * (1-margin) - (1-margin);
            // vertices[j*xSteps + i] = {x, y, 0, 0, 1-std::exp(-0.5*yValues.size())};
            // vertices[j*xSteps + i] = {x, y, 1, 1, 1};
	    float colorValue = float(colorValues[i]) / nIterations;
	    // float colorValue = 1-std::exp(-0.1*colorValues[i]);
	    //std::cout << colorValues[i] << "\n";
	    //std::cout << colorValue << "\n";
	    float r, g, b;
	    intToRainbowRGB(colorValues[i], nIterations, r, g, b);
            // vertices[j*xSteps + i] = {x, y, colorValue, colorValue, colorValue};
	    std::cout << r << "\t" << g << "\t" << b << "\n";
        Vertex current_vertex;

        current_vertex.position[0] = x;
        current_vertex.position[1] = y;
        current_vertex.color[0] = r;
        current_vertex.color[1] = g;
        current_vertex.color[2] = b;

        vertices[j*xSteps + i] = current_vertex;
        }
    }

    return vertices;

}

 
int main(void)
{
    std::vector<Vertex> vertices = createVertices(1000, 1000);
     
    glfwSetErrorCallback(error_callback);
 
    if (!glfwInit())
    {
        exit(EXIT_FAILURE);
    }
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
 
    GLFWwindow* window = glfwCreateWindow(1000, 1000, "OpenGL Triangle", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
 
    glfwSetKeyCallback(window, key_callback);
 
    glfwMakeContextCurrent(window);
    
    // gladLoadGL(glfwGetProcAddress);
    gladLoadGL();
    glfwSwapInterval(1);
 
    // NOTE: OpenGL error checks have been omitted for brevity
 
    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
 
    const GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShader(vertex_shader);
 
    const GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShader(fragment_shader);
 
    const GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);
 
    const GLint mvp_location = glGetUniformLocation(program, "MVP");
    const GLint vpos_location = glGetAttribLocation(program, "vPos");
    const GLint vcol_location = glGetAttribLocation(program, "vCol");
 
    GLuint vertex_array;
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);
    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location, 2, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*) offsetof(Vertex, position));
    glEnableVertexAttribArray(vcol_location);
    glVertexAttribPointer(vcol_location, 3, GL_FLOAT, GL_FALSE,
                          sizeof(Vertex), (void*) offsetof(Vertex, color));
 
    while (!glfwWindowShouldClose(window))
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio = width / (float) height;
 
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
        glPointSize( 1.0 );
        glPointSize( 5.0 );
        glPointSize( 10.0 );
 
        mat4x4 m, p, mvp;
        mat4x4_identity(m);
        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &m);
        glBindVertexArray(vertex_array);
        glDrawArrays(GL_POINTS, 0, vertices.size());
 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
 
    glfwDestroyWindow(window);
 
    glfwTerminate();
    exit(EXIT_SUCCESS);
}