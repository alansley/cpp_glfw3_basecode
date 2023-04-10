#include "Point.h"

// ----- Static declarations -----

ShaderProgram *Point::pointShaderProgram;
GLuint Point::pointVaoId;
GLuint Point::pointVertexBufferId;
float *Point::pointDataArray;

// ----- Static initialisation -----

// How many point instances currently exist
int Point::pointInstances = 0;

//Define our vertex shader source code
//Note: The R" notation is for raw strings and preserves all spaces, indentation,
//newlines etc. in utf-8|16|32 wchar_t format, but requires C++0x or C++11.
const char *Point::vertexShaderSource = R"(
#version 330
in vec4 vertexLocation; // Incoming vertex attribute
in vec4 vertexColour;   // Incoming vertex attribute
out vec4 fragColour;
uniform mat4 mvpMatrix; // Combined Model/View/Projection matrix
void main(void)
{
    fragColour = vertexColour;
    gl_Position = mvpMatrix * vertexLocation; // Project our geometry
}
)";

// Define our fragment shader source code
const char *Point::fragmentShaderSource = R"(
#version 330
in vec4 fragColour;
out vec4 outputColour; // Outgoing fragment colour
void main()
{
    outputColour = fragColour;
}
)";

// Method to set up the shader program to draw points
void Point::setupShaderProgram()
{
    Point::pointDataArray = new float[VERTEX_COMPONENTS + COLOUR_COMPONENTS];

    // ----- point shader program setup -----

    Point::pointShaderProgram = new ShaderProgram("PointShaderProgram");

    Point::pointShaderProgram->addShader(GL_VERTEX_SHADER, Point::vertexShaderSource);
    Point::pointShaderProgram->addShader(GL_FRAGMENT_SHADER, Point::fragmentShaderSource);

    Point::pointShaderProgram->initialise();

    // ----- point shader attributes and uniforms -----

    // Add the shader attributes
    Point::pointShaderProgram->bindAttribute("vertexLocation");
    Point::pointShaderProgram->bindAttribute("vertexColour");

    // Add the shader uniforms
    Point::pointShaderProgram->bindUniform("mvpMatrix");

    // ----- Set up our Vertex Array Object (VAO) to hold the shader attributes -----
    // Note: The point VAO cannot be static because we may have multiple points of various sizes.

    // Get an Id for the Vertex Array Object (VAO) and bind to it
    glGenVertexArrays(1, &pointVaoId);
    glBindVertexArray(Point::pointVaoId);

        // ----- Location Vertex Buffer Object (VBO) -----

        // Generate a Vertex Buffer Object to store the point data
        glGenBuffers(1, &pointVertexBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, Point::pointVertexBufferId);

        // Note: We don't actually put any data into the VBO just yet, we do that in the draw() method

        // Specify the attribute pointer for the vertex location
        glVertexAttribPointer(Point::pointShaderProgram->attribute("vertexLocation"), // Vertex location attribute index
                                                                   VERTEX_COMPONENTS, // Number of normal components per vertex
                                                                            GL_FLOAT, // Data type
                                                                               false, // Normalised?
                                                   COMPONENT_COUNT * sizeof(GLfloat), // Stride
                                                                                  0); // Offset

        glVertexAttribPointer(Point::pointShaderProgram->attribute("vertexColour"),   // Vertex location attribute index
                                                                 COLOUR_COMPONENTS,   // Number of normal components per vertex
                                                                          GL_FLOAT,   // Data type
                                                                             true,    // Normalised?
                                                 COMPONENT_COUNT * sizeof(GLfloat),   // Stride - Doesn't matter for a Point, it's only 1 vertex!
                                   (GLvoid*) (VERTEX_COMPONENTS * sizeof(GLfloat)));  // Offset

        // Unbind VBO
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Enable the vertex attributes
        glEnableVertexAttribArray(pointShaderProgram->attribute("vertexLocation"));
        glEnableVertexAttribArray(pointShaderProgram->attribute("vertexColour"));

    // Unbind our Vertex Array object - all the buffer and attribute settings above will be associated with our VAO!
    glBindVertexArray(0);
}

/*
class Foo {
public:
  Foo(char x, int y) {}
  Foo(int y) : Foo('a', y) {}
};
*/

// Default constructor
Point::Point()
{
    // If this is the first point we're creating then do the shader setup
    if (Point::pointInstances == 0)
    {
        setupShaderProgram();
    }

    // Increment our count of pointInstances so we can keep track of how many we have
    pointInstances++;
}

// Three parameter constructor
Point::Point(vec3 loc, vec3 spd, vec4 col, float ps = 1.0f) : location(loc), speed(spd), colour(col), pointSize(ps)
{
    // Initialisation / assignment of location, colour and pointsize happen in the above initialisation list

    // If this is the first point we're creating then do the shader setup
    if (Point::pointInstances == 0)
    {
        setupShaderProgram();
    }
}

// Destructor
Point::~Point()
{
    // Decrement out count of point instances
    pointInstances--;

    // If this is the last point we're getting rid of, clean up the shader program
    if (pointInstances == 0)
    {
        delete[] pointDataArray;
        delete pointShaderProgram;
    }
}

void Point::setLocation(vec3 l)
{
    location.x = l.x;
    location.y = l.y;
    location.z = l.z;
}

void Point::setLocation(float x, float y, float z)
{
    location.x = x;
    location.y = y;
    location.z = z;
}

void Point::setColour(vec3 c)
{
    colour.r = c.r;
    colour.g = c.g;
    colour.b = c.b;
    colour.a = 1.0f;
}

void Point::setColour(vec4 c)
{
    colour.r = c.r;
    colour.g = c.g;
    colour.b = c.b;
    colour.a = c.a;
}

void Point::setColour(float r, float g, float b)
{
    colour.r = r;
    colour.g = g;
    colour.b = b;
    colour.a = 1.0f;
}

void Point::setColour(float r, float g, float b, float a)
{
    colour.r = r;
    colour.g = g;
    colour.b = b;
    colour.a = a;
}

void Point::setPointSize(float ps)
{
    pointSize = ps;
}

// Method to draw a single point - takes a combined Model/View/Projection matrix
// to pass to the shader as a uniform.
void Point::draw(mat4 mvpMatrix)
{
    // Specify we're using our shader program
    Point::pointShaderProgram->use();

        // Bind to our vertex buffer object
        glBindVertexArray(Point::pointVaoId);

            // Bind to our Vertex Buffer Object
            glBindBuffer(GL_ARRAY_BUFFER, Point::pointVertexBufferId);

            // Transfer the data for this particular point into the data array...
            Point::pointDataArray[0] = location.x;
            Point::pointDataArray[1] = location.y;
            Point::pointDataArray[2] = location.z;
            Point::pointDataArray[3] = colour.r;
            Point::pointDataArray[4] = colour.g;
            Point::pointDataArray[5] = colour.b;
            Point::pointDataArray[6] = colour.a;

            // ...and push it to the graphics card
            glBufferData(GL_ARRAY_BUFFER, Point::BUFFER_SIZE_BYTES, Point::pointDataArray, GL_DYNAMIC_DRAW);

            // Provide the projection matrix uniform
            glUniformMatrix4fv(Point::pointShaderProgram->uniform("mvpMatrix"), 1, GL_FALSE, glm::value_ptr(mvpMatrix) );

            // Save all point related attributes
            //glPushAttrib(GL_POINT_BIT);  ------------------------------------------------------------------------------------- FIX THIS - GL_POINT_BIT not longer a thing?!?!?!

                // Set the point size for this particular point
                glPointSize(pointSize);

                // Draw the point
                glDrawArrays(GL_POINTS, 0, Point::VERTEX_COUNT);

            // Restore all point related attributes
            //glPopAttrib();

        // Unbind from our vertex array objext and disable our shader program
        glBindVertexArray(0);

    // Disable our shader program
    Point::pointShaderProgram->disable();
}

// Static method to draw an array of Points - takes a combined Model/View/Projection matrix
// to pass to the shader as a uniform. This is vastly more effecient than drawing points
// individually, but as all points are drawn in a single call they must all have the same glPointSize.
void Point::draw(Point* pointArray, int numPoints, float pointSize, mat4 mvpMatrix)
{
    // Specify we're using our shader program
    Point::pointShaderProgram->use();

        // Bind to our vertex buffer object
        glBindVertexArray(Point::pointVaoId);

            // Bind to our Vertex Buffer Object
            glBindBuffer(GL_ARRAY_BUFFER, Point::pointVertexBufferId);

            // Work out how many floats we're using in total and create an array of that size
            int numFloats = Point::COMPONENT_COUNT * numPoints;
            float *combinedPointData = new float[numFloats];

            // Copy the data into the combinedPointData array.
            // Note: We start on -1 so we can use the more efficient prefix increment operator which
            // will immediately push the value to start on zero.
            int pointNumber = 0;
            for (int loop = -1; loop < numFloats; /* no iterator change - occurs within loop */ )
            {
                // Transfer the data for this particular point into the data array...
                combinedPointData[++loop] = pointArray[pointNumber].location.x;
                combinedPointData[++loop] = pointArray[pointNumber].location.y;
                combinedPointData[++loop] = pointArray[pointNumber].location.z;
                combinedPointData[++loop] = pointArray[pointNumber].colour.r;
                combinedPointData[++loop] = pointArray[pointNumber].colour.g;
                combinedPointData[++loop] = pointArray[pointNumber].colour.b;
                combinedPointData[++loop] = pointArray[pointNumber].colour.a;

                // Move on to the next point
                ++pointNumber;
            }

            // Push our combined point data to the graphics card
            glBufferData(GL_ARRAY_BUFFER, Point::BUFFER_SIZE_BYTES * numPoints, combinedPointData, GL_DYNAMIC_DRAW);

            // Free our combined point data
            delete[] combinedPointData;

            // Provide the projection matrix uniform
            glUniformMatrix4fv(Point::pointShaderProgram->uniform("mvpMatrix"), 1, GL_FALSE, glm::value_ptr(mvpMatrix) );

            // Save all point related attributes
            //glPushAttrib(GL_POINT_BIT); ------------------------------------------------------------------------------------- FIX THIS GL_POINT_BIT no longer a thing?!?!

                // Set the point size to draw all the points
                glPointSize(pointSize);

                // Draw the points
                glDrawArrays(GL_POINTS, 0, numPoints);

            // Restore all point related attributes
            //glPopAttrib();

        // Unbind from our vertex array objext and disable our shader program
        glBindVertexArray(0);

    // Disable our shader program
    Point::pointShaderProgram->disable();
}

void Point::update()
{
    location += speed;



    if (location.x > 1000.0f)
    {
        location.x = -1000.0f;
    }

    if (location.y < -500.0f)
    {
        location.y = 500.0f;
    }

}
