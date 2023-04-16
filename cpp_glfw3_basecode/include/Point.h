#ifndef POINT_H
#define POINT_H

// Pull in the ShaderProgram if required
#ifndef SHADER_PROGRAM_HPP
    #include "ShaderProgram.hpp"
#endif

//#define GLEW_STATIC
#include "GLFW/glfw3.h"

// Include the GL Mathematics library
#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" // Needed for the perspective() method
#include "glm/gtc/type_ptr.hpp"         // Needed for the value_ptr() method



using glm::vec3;
using glm::vec4;
using glm::mat4;

// Class to draw a point in 3D space
class Point
{
    private:
        // ----- Static Properties -----

        static const int VERTEX_COMPONENTS  = 3;                                     // x/y/z
        static const int COLOUR_COMPONENTS  = 4;                                     // r/g/b/a
        static const int COMPONENT_COUNT    = VERTEX_COMPONENTS + COLOUR_COMPONENTS; // 7 components per vertex (e.g., x/y/z + r/g/b/a = 7)
        static const int VERTEX_COUNT       = 1;                                     // We're drawing a single vertex for a point

        // Our buffer is this size
        static const int BUFFER_SIZE_BYTES = COMPONENT_COUNT * VERTEX_COUNT * sizeof(GL_FLOAT);

        // Keep track of how many point instances we have.
        static int pointInstances;

        // Our point will be drawn using a ShaderProgram, which we'll accept as a pointer
        static ShaderProgram* pointShaderProgram;

        //Define our vertex shader source code
        //Note: The R" notation is for raw strings and preserves all spaces, indentation,
        //newlines etc. in utf-8|16|32 wchar_t format, but requires C++0x or C++11.
        static const char* vertexShaderSource;

        //Define our fragment shader source code
        static const char* fragmentShaderSource;

        static GLuint pointVaoId;          // The id of the Vertex Array Object  (VAO) containing our shader program details
        static GLuint pointVertexBufferId; // The id of the Vertex Buffer Object (VBO) containing our vertex data

        static float *pointDataArray;          // Pointer to an array of floats used to draw the point

        // ----- Per-Object Properties -----

        vec3  location;
        vec3 speed;
        vec4  colour;
        float pointSize;


        // ----- Private methods -----

        static void setupShaderProgram();

    public:
        // Default constructor
        Point();

        // Constructor
        Point(vec3 location, vec3 speed, vec4 colour, float pointSize);

        // Destructor
        ~Point();

        void setLocation(vec3 l);
        void setLocation(float x, float y, float z);

        void setColour(vec3 c);
        void setColour(vec4 c);
        void setColour(float r, float g, float b);
        void setColour(float r, float g, float b, float a);

        void setPointSize(float pointSize);


        // Method to draw the point - just takes a combined Model/View/Projection matrix
        // to pass to the shader as a uniform.
        void draw(mat4 mvpMatrix);

        // Static method to draw an array of Points - this is more efficient, but each point must the same size (i.e. the same glPointSize)
        static void draw(Point* pointArray, int numPoints, float pointSize, mat4 mvpMatrix);

        void update();
};

#endif // POINT_H
