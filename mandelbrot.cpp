#define GLAD_GL_IMPLEMENTATION
#include <glad/glad.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
 
#include "linmath.h"
#include "rainbow.h"
 
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <vector>
#include <iostream> 
#include <algorithm> 

const float convergence_radius_squared = 4.0f;

typedef struct Vertex
{
    vec2 true_position; // needed for future use
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

std::vector<float> copyArray(std::vector<float>& arr)
{
    return arr;
}


std::vector<Vertex> createVertices(int width, int height, float real_0, float imaginary_0, float zoom_factor)
{
    /*
    width: window in int
    real_0: center of screen
    */

    float margin = 0.0; // how much space between graph and edge of window
    float x;
    int xSteps = width;
    float xStart = real_0 - (1.1 / zoom_factor);
    // std::cout << xStart << "\n";
    float xEnd = real_0 + (1.1 / zoom_factor);
    float dx = (xEnd - xStart) / xSteps;
    
    float y;
    int ySteps = height;
    float yStart = (imaginary_0 - (1.1 / zoom_factor) * (float(width) / float(height)));
    float yEnd = (imaginary_0 + (1.1 / zoom_factor) * (float(width) / float(height)));
    float dy = (yEnd - yStart) / ySteps;

    // std::cout << xStart << " " << xEnd << "\t" << yStart << " " << yEnd << "\n";
    std::cout << "ratio" << (yEnd-yStart) / (xEnd-xStart) << "\n";

    int nIterations = 100;

    std::vector<float> xInput(xSteps);
    for (size_t j = 0; j < xSteps; ++j) {
            xInput[j] = j*dx + xStart;
    }
    std::vector<float> yInput(ySteps);
    for (size_t j = 0; j < ySteps; ++j) {
            yInput[j] = j*dy + yStart;
    }
    std::vector<float> r, g, b;
    for (int i = 0; i <= nIterations; ++i) {
        float rr, gg, bb;
        intToRainbowRGB(i, nIterations, rr, gg, bb);
        r.push_back(rr);
        g.push_back(gg);
        b.push_back(bb);
    }

    std::vector<Vertex> vertices(ySteps*xSteps);
    for (size_t j = 0; j < yInput.size(); ++j) {
        y = yInput[j];
        float yPlotValue = (y-yStart)/(yEnd - yStart) * 2 * (1-margin) - (1-margin);
        for (size_t i = 0; i < xInput.size(); ++i) {
            x = xInput[i];
            float xPlotValue = (x-xStart)/(xEnd - xStart) * 2 * (1-margin) - (1-margin);
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

 
int main(void)
{
    int width = 1000;
    int height = 1000;
    float real_0 = -0.6;
    float imaginary_0 = 0.0f;
    float zoom_factor = 1.0f;
    std::vector<Vertex> vertices = createVertices(width, height, real_0, imaginary_0, zoom_factor);
     
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
 
    glfwSetKeyCallback(window, key_callback);
 
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glfwSwapInterval(1);
 
    // NOTE: OpenGL error checks have been omitted for brevity
 
    GLuint vertex_buffer;
    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    // std::vector<Vertex> vertices = createVisuals(1000, 1000);
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
        glfwGetFramebufferSize(window, &width, &height);
        const float ratio = width / (float) height;
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

        // std::cout << update_vertices << "\n";

        if (update_vertices) {
        vertices = createVertices(width, height, real_0, imaginary_0, zoom_factor);
        }

        glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
 
        glViewport(0, 0, width, height);
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
