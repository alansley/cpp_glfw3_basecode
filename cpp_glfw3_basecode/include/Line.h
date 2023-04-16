#ifndef LINE_H
#define LINE_H

// Include the GL Mathematics library
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> // Needed for the perspective() method
#include <glm/gtc/type_ptr.hpp>         // Needed for the value_ptr() method

// Pull in the ShaderProgram if required
#ifndef SHADER_PROGRAM_HPP
    #include "ShaderProgram.hpp"
#endif

using glm::vec3;
using glm::vec4;
using glm::mat4;

// Class to draw a line in 3D space
class Line
{
    private:
        // ----- Static Properties -----

        static const int VERTEX_COMPONENTS  = 3;                                     // x, y and z
        static const int COLOUR_COMPONENTS  = 4;                                     // r, g, b and a
        static const int COMPONENT_COUNT    = VERTEX_COMPONENTS + COLOUR_COMPONENTS; // 7 components per vertex
        static const int VERTEX_COUNT       = 2;                                     // We're drawing a single vertex for a line

        // Our buffer is this size
        static const int BUFFER_SIZE_BYTES = COMPONENT_COUNT * VERTEX_COUNT * sizeof(GL_FLOAT);

        // Keep track of how many line instances we have.
        static int lineInstances;

        // Our line will be drawn using a ShaderProgram, which we'll accept as a lineer
        static ShaderProgram* lineShaderProgram;

        //Define our vertex shader source code
        //Note: The R" notation is for raw strings and preserves all spaces, indentation,
        //newlines etc. in utf-8|16|32 wchar_t format, but requires C++0x or C++11.
        static const char* vertexShaderSource;

        //Define our fragment shader source code
        static const char* fragmentShaderSource;

        static GLuint lineVaoId;          // The id of the Vertex Array Object  (VAO) containing our shader program details
        static GLuint lineVertexBufferId; // The id of the Vertex Buffer Object (VBO) containing our vertex data

        static float *lineDataArray;          // lineer to an array of floats used to draw the line

        // ----- Per-Object Properties -----

        vec3  p1Location;
        vec3  p2Location;
        vec4  colour;
        float lineWidth;

        // Private methods

        static void setupShaderProgram();

    public:
        // Default constructor
        Line();

        // Constructor
        Line(vec3 p1Location, vec3 p2Location, vec4 colour, float lineWidth);

        // Destructor
        ~Line();

        void setP1Location(float x, float y, float z);
        void setP2Location(float x, float y, float z);

        void setColour(float r, float g, float b);
        void setColour(float r, float g, float b, float a);

        void setlineWidth(float lineWdith);


        // Method to draw the line - just takes a combined Model/View/Projection matrix
        // to pass to the shader as a uniform.
        void draw(mat4 mvpMatrix);
};

#endif // LINE_H
