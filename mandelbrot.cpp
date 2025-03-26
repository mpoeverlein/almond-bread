#define GLAD_GL_IMPLEMENTATION
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
 
#include "linmath.h"
#include "rainbow.h"
#include "shaders.h"
 
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <vector>
#include <iostream> 
#include <algorithm> 

// for any complex number with magnitude larger than 2 the iteration will not converge
const float convergence_radius_squared = 4.0f;
// after how many interations to stop. this is a global value for now, but maybe a local adaptivtiy is possible
const int nIterations = 500;
// how much space between graph and edge of window
const float margin = 0.0;
// initial range from (boundary - i boundary) to (boundary + i boundary)
const float boundary = 1.1;

// float real_0 = -0.6f;
// float imaginary_0 = 0.0f;
// float zoom_factor = 1.0f;

// float real_0 = -1.21235;
// float imaginary_0 = 0.318563;
// float zoom_factor = 1/0.0190757;

float real_0 = -1.21235;
float imaginary_0 = 0.318989;
float zoom_factor = 1/1.41981e-05;

typedef struct Vertex
{
    vec2 true_position; // needed for future use
    vec2 position;
    vec3 color;
} Vertex;

typedef struct SampleDimensions
{
    float xStart;
    float xEnd;
    float dx;
    float yStart;
    float yEnd;
    float dy;
} SampleDimensions;

 
static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}
 
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
}

SampleDimensions createDimensions(int xSteps, int ySteps)
{
    SampleDimensions s;
    s.xStart = real_0 - (boundary / zoom_factor);
    s.xEnd = real_0 + (boundary / zoom_factor);
    s.dx = (s.xEnd - s.xStart) / xSteps;
    
    s.yStart = imaginary_0 - (boundary / zoom_factor);
    s.yEnd = imaginary_0 + (boundary / zoom_factor);
    s.dy = (s.yEnd - s.yStart) / ySteps;
    return s;
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
int iterateMandelbrot(float a, float b, int maxIterations)
{
    float tmp_a = a;
    float tmp_b = b;
    for (int i = 0; i < maxIterations; ++i) {
        float original_a = tmp_a;
        float original_b = tmp_b;
        tmp_a = original_a*original_a - original_b*original_b + a;
        tmp_b = 2*original_a*original_b + b;
        if (tmp_a*tmp_a + tmp_b*tmp_b > convergence_radius_squared) {
            return i;
        }
    }
    return maxIterations;

}

void populateVector(std::vector<float>& vec, float start, float delta)
{
    for (size_t j = 0; j < vec.size(); ++j) {
        vec[j] = j*delta + start;
    }
}

void calculatePlotValues(std::vector<float>& plotValues, std::vector<float>& input, float start, float end, float margin)
{
    for (size_t i = 0; i < plotValues.size(); ++i) {
        plotValues[i] = (input[i]-start)/(end - start) * 2 * (1-margin) - (1-margin);
    }
}

void createRGBVectors(int nIterations, std::vector<float>& r, std::vector<float>& g, std::vector<float>& b)
{
    float rr, gg, bb;
    for (int i = 0; i <= nIterations; ++i) {
        intToRainbowRGB(i, nIterations, rr, gg, bb);
        r[i] = rr; // r.push_back(rr);
        g[i] = gg; //g.push_back(gg);
        b[i] = bb; // b.push_back(bb);
    }
}

template <typename Callable>
void createRGBVectors(int nIterations, 
    std::vector<float>& r, 
    std::vector<float>& g, 
    std::vector<float>& b,
    Callable colorFunction
)
{
    float rr, gg, bb;
    for (int i = 0; i <= nIterations; ++i) {
        colorFunction(i, nIterations, rr, gg, bb);
        r[i] = rr; // r.push_back(rr);
        g[i] = gg; //g.push_back(gg);
        b[i] = bb; // b.push_back(bb);
    }
}

/**
 * @brief
 * Calculate color values of all pixels in the window
 * 
 * @param width window width
 * @param height window height
 * @param real_0 real value of window center
 * @param imaginary_0 imaginary value of window center
 * @param zoom_factor If 1, window width accounts for real value length of 2.2
 */
std::vector<Vertex> createVertices(int width, int height)
{
    float x;
    int xSteps = width;
    float y;
    int ySteps = height;

    SampleDimensions dimensions = createDimensions(xSteps, ySteps);

    std::vector<float> xInput(xSteps), yInput(ySteps);
    populateVector(xInput, dimensions.xStart, dimensions.dx);
    populateVector(yInput, dimensions.yStart, dimensions.dy);

    std::vector<float> r(nIterations+1), g(nIterations+1), b(nIterations+1);
    createRGBVectors(nIterations, r, g, b, intToRainbowRGB);
    // createRGBVectors(nIterations, r, g, b, intToBWRGB);
    

    std::vector<float> yPlotValues(ySteps), xPlotValues(xSteps);
    calculatePlotValues(yPlotValues, yInput, dimensions.yStart, dimensions.yEnd, margin);
    calculatePlotValues(xPlotValues, xInput, dimensions.xStart, dimensions.xEnd, margin);
    float yPlotValue, xPlotValue;

    std::vector<Vertex> vertices(ySteps*xSteps);
    for (size_t j = 0; j < yInput.size(); j++) {
        y = yInput[j];
        yPlotValue = yPlotValues[j];
        for (size_t i = 0; i < xInput.size(); i++) {
            xPlotValue = xPlotValues[i];
            x = xInput[i];
            int myIterations = iterateMandelbrot(x, y, nIterations);

            Vertex current_vertex;
            current_vertex.true_position[0] = x;
            current_vertex.true_position[1] = y;
            current_vertex.position[0] = xPlotValue;
            current_vertex.position[1] = yPlotValue;
            current_vertex.color[0] = r[myIterations];
            current_vertex.color[1] = g[myIterations];
            current_vertex.color[2] = b[myIterations]; 
            vertices[j*xSteps + i] = current_vertex;
        }
    }

    return vertices;
}

void updateVertices(std::vector<Vertex> &vertices, int width, int height)
{
    float x;
    int xSteps = width;

    float y;
    int ySteps = height;
    SampleDimensions dimensions = createDimensions(xSteps, ySteps);

    std::vector<float> xInput(xSteps), yInput(ySteps);
    populateVector(xInput, dimensions.xStart, dimensions.dx);
    populateVector(yInput, dimensions.yStart, dimensions.dy);

    std::vector<float> r(nIterations+1), g(nIterations+1), b(nIterations+1);
    createRGBVectors(nIterations, r, g, b, intToRainbowRGB);

    std::vector<float> yPlotValues(ySteps), xPlotValues(xSteps);
    calculatePlotValues(yPlotValues, yInput, dimensions.yStart, dimensions.yEnd, margin);
    calculatePlotValues(xPlotValues, xInput, dimensions.xStart, dimensions.xEnd, margin);
    float yPlotValue, xPlotValue;

    for (size_t j = 0; j < yInput.size(); j++) {
        y = yInput[j];
        yPlotValue = yPlotValues[j];
        for (size_t i = 0; i < xInput.size(); i++) {
            x = xInput[i];
            xPlotValue = xPlotValues[i];
            int myIterations = iterateMandelbrot(x, y, nIterations);

            Vertex& current_vertex = vertices[j*xSteps + i];
            current_vertex.true_position[0] = x;
            current_vertex.true_position[1] = y;
            current_vertex.position[0] = xPlotValue;
            current_vertex.position[1] = yPlotValue;
            current_vertex.color[0] = r[myIterations];
            current_vertex.color[1] = g[myIterations];
            current_vertex.color[2] = b[myIterations]; 
        }
    }
}

 
int main(void)
{
    int width = 1000;
    int height = 1000;
     
    glfwSetErrorCallback(error_callback);
 
    if (!glfwInit())
    {
        exit(EXIT_FAILURE);
    }
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
 
    GLFWwindow* window = glfwCreateWindow(width, height, "OpenGL Mandelbrot", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
    glfwGetWindowSize(window, &width, &height);
    std::vector<Vertex> vertices = createVertices(width, height);
 
    glfwSetKeyCallback(window, key_callback);
 
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);
  
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
        glfwGetWindowSize(window, &width, &height);
        const float ratio = width / (float) height;
        if (width * height != vertices.size()) {
            vertices = createVertices(width, height);
        }

        bool update_vertices = true;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            real_0 += 0.1f / zoom_factor;
        } else if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            real_0 -= 0.1f / zoom_factor;
        } else if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            imaginary_0 += 0.1f / zoom_factor;
        } else if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            imaginary_0 -= 0.1f / zoom_factor;
        } else if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS) {
            zoom_factor *= 1.5f;
        } else if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS) {
            zoom_factor /= 1.5f;
        } else {
            update_vertices = false;
        }

        if (update_vertices) {
            std::cout << real_0 << " " << imaginary_0 << " " << boundary/zoom_factor << "\n";
            updateVertices(vertices, width, height);
        }

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
 
        glClear(GL_COLOR_BUFFER_BIT);
        glPointSize(10);
 
        mat4x4 m, p, mvp;
        mat4x4_identity(m);
        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) &m);
        glBindVertexArray(vertex_array);
        glDrawArrays(GL_POINTS, 0, vertices.size());
        // std::cout << vertices[0].true_position[0] << "\t" << vertices[0].position[0] << "\n";
 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vertex_array);
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteProgram(program);
 
    glfwDestroyWindow(window);
 
    glfwTerminate();
    exit(EXIT_SUCCESS);
}
