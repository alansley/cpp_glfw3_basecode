#ifndef GRID_H
#define GRID_H

// Include the GL Mathematics library
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"         // Needed for the value_ptr() method

// Pull in the ShaderProgram if required
#ifndef SHADER_PROGRAM_HPP
    #include "ShaderProgram.hpp"
#endif

// Class to draw a grid in 3D space
class Grid
{
    private:
        // ----- Static Properties -----

        // Each vertex has an X, Y and Z component
        static const int VERTEX_COMPONENTS = 3;

        // Keep track of how many grid instances we have.
        static int gridInstances;

        // Our grid will be drawn using a ShaderProgram, which we'll accept as a pointer
        static ShaderProgram* gridShaderProgram;

        //Define our vertex shader source code
        //Note: The R" notation is for raw strings and preserves all spaces, indentation,
        //newlines etc. in utf-8|16|32 wchar_t format, but requires C++0x or C++11.
        static const char* vertexShaderSource;

        //Define our fragment shader source code
        static const char* fragmentShaderSource;

        // ----- Non-Static Properties -----

        GLuint gridVaoId;          // The id of the Vertex Array Object  (VAO) containing our shader program details
        GLuint gridVertexBufferId; // The id of the Vertex Buffer Object (VBO) containing our vertex data

        int numVerts;              // How many vertices in this grid?

        float *gridVertexArray;    // Pointer to an array of floats used to draw the grid

    public:
        // Constructor
        // Note: width is along +/- x-axis, depth is along +/- z-axis, height is the location on
        // the y-axis, numDivisions is how many lines to draw across each axis
        Grid(float width,  float depth,  float height,  int numDivisions);

        // Destructor
        ~Grid();

        // Method to draw the grid - just takes a combined Model/View/Projection matrix
        // to pass to the shader as a uniform.
        void draw(glm::mat4 mvpMatrix);
};

#endif // GRID_H
