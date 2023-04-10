#include "Line.h"

// ----- Static declarations -----

ShaderProgram *Line::lineShaderProgram;
GLuint Line::lineVaoId;
GLuint Line::lineVertexBufferId;
float *Line::lineDataArray;

// ----- Static initialisation -----

// How many line instances currently exist
int Line::lineInstances = 0;

// Define our vertex shader source code
// Note: The R" notation is for raw strings and preserves all spaces, indentation,
// newlines etc. in utf-8|16|32 wchar_t format, but requires C++0x or C++11.
const char *Line::vertexShaderSource = R"(
#version 430

in vec3 vertexLocation; // Incoming vertex attribute
in vec4 vertexColour;   // Incoming vertex attribute

out vec4 fragColour;

uniform mat4 mvpMatrix; // Combined Model/View/Projection matrix

void main(void)
{
    fragColour = vertexColour;
    gl_Position = mvpMatrix * vec4(vertexLocation, 1.0); // Project our geometry
}
)";

// Define our fragment shader source code
const char *Line::fragmentShaderSource = R"(
#version 430

in vec4 fragColour;

out vec4 outputColour; // Outgoing fragment colour

void main()
{
    outputColour = fragColour;
}
)";

void Line::setupShaderProgram()
{
    Line::lineDataArray = new float[VERTEX_COMPONENTS + COLOUR_COMPONENTS];

    // ----- line shader program setup -----

    Line::lineShaderProgram = new ShaderProgram("Line Shader");
    Line::lineShaderProgram->addShader(GL_VERTEX_SHADER,   Line::vertexShaderSource);
    Line::lineShaderProgram->addShader(GL_FRAGMENT_SHADER, Line::fragmentShaderSource);
    Line::lineShaderProgram->initialise();

    // ----- line shader attributes and uniforms -----

    // Add the shader attributes
    Line::lineShaderProgram->bindAttribute("vertexLocation");
    Line::lineShaderProgram->bindAttribute("vertexColour");

    // Add the shader uniforms
    Line::lineShaderProgram->bindUniform("mvpMatrix");

    // ----- Set up our Vertex Array Object (VAO) to hold the shader attributes -----
    // Note: The line VAO cannot be static because we may have multiple lines of various sizes.

    // Get an Id for the Vertex Array Object (VAO) and bind to it
    glGenVertexArrays(1, &lineVaoId);
    glBindVertexArray(Line::lineVaoId);

        // ----- Location Vertex Buffer Object (VBO) -----

        // Generate an id for the locationBuffer and bind to it
        glGenBuffers(1, &lineVertexBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, Line::lineVertexBufferId);

        // Specify the attribute lineer for the vertex location
        glVertexAttribPointer(Line::lineShaderProgram->attribute("vertexLocation"), // Vertex location attribute index
                                                                 VERTEX_COMPONENTS, // Number of normal components per vertex
                                                                          GL_FLOAT, // Data type
                                                                             false, // Normalised?
                                                 COMPONENT_COUNT * sizeof(GLfloat), // Stride
                                                                                0); // Offset

        glVertexAttribPointer(Line::lineShaderProgram->attribute("vertexColour"),   // Vertex location attribute index
                                                               COLOUR_COMPONENTS,   // Number of normal components per vertex
                                                                        GL_FLOAT,   // Data type
                                                                            true,   // Normalised?
                                               COMPONENT_COUNT * sizeof(GLfloat),   // Stride - Doesn't matter for a line, it's only 1 vertex!
                                 (GLvoid*) (VERTEX_COMPONENTS * sizeof(GLfloat)));  // Offset



        // Unbind VBO
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Enable the vertex attributes
        glEnableVertexAttribArray( lineShaderProgram->attribute("vertexLocation") );
        glEnableVertexAttribArray( lineShaderProgram->attribute("vertexColour")   );

    // Unbind our Vertex Array object - all the buffer and attribute settings above will be associated with our VAO!
    glBindVertexArray(0);
}

// Default constructor
Line::Line()
{
    // If this is the first line we're creating then do the shader setup
    if (Line::lineInstances == 0)
    {
        setupShaderProgram();
    }

    // Increment our count of lineInstances so we can keep track of how many we have
    lineInstances++;
}


// Three parameter constructor
Line::Line(vec3 p1Loc, vec3 p2Loc, vec4 col, float lw = 1.0f) : p1Location(p1Loc), p2Location(p2Loc), colour(col), lineWidth(lw)
{
    // Initialisation / assignment of location, colour and line width happen in the above initialisation list

    // If this is the first line we're creating then do the shader setup
    if (Line::lineInstances == 0)
    {
        setupShaderProgram();
    }
}

// Destructor
Line::~Line()
{
    // Decrement out count of line instances
    lineInstances--;

    // If this is the last line we're getting rid of, clean up the shader program
    if (lineInstances == 0)
    {
        delete[] lineDataArray;
        delete lineShaderProgram;
    }
}

void Line::setP1Location(float x, float y, float z)
{
    p1Location.x = x;
    p1Location.y = y;
    p1Location.z = z;
}

void Line::setP2Location(float x, float y, float z)
{
    p2Location.x = x;
    p2Location.y = y;
    p2Location.z = z;
}

void Line::setColour(float r, float g, float b)
{
    colour.r = r;
    colour.g = g;
    colour.b = b;
    colour.a = 1.0f;
}

void Line::setColour(float r, float g, float b, float a)
{
    colour.r = r;
    colour.g = g;
    colour.b = b;
    colour.a = a;
}

void Line::setlineWidth(float lw)
{
    lineWidth = lw;
}

// Method to draw the line - takes a combined Model/View/Projection matrix
// to pass to the shader as a uniform.
void Line::draw(mat4 mvpMatrix)
{
    // Specify we're using our shader program
    Line::lineShaderProgram->use();

        // Bind to our vertex buffer object
        glBindVertexArray(Line::lineVaoId);

            glBindBuffer(GL_ARRAY_BUFFER, Line::lineVertexBufferId);

            // Transfer the data for this particular line into the data array...
            Line::lineDataArray[0]  = p1Location.x;
            Line::lineDataArray[1]  = p1Location.y;
            Line::lineDataArray[2]  = p1Location.z;
            Line::lineDataArray[3]  = colour.r;
            Line::lineDataArray[4]  = colour.g;
            Line::lineDataArray[5]  = colour.b;
            Line::lineDataArray[6]  = colour.a;
            Line::lineDataArray[7]  = p1Location.x;
            Line::lineDataArray[8]  = p1Location.y;
            Line::lineDataArray[9]  = p1Location.z;
            Line::lineDataArray[10] = colour.r;
            Line::lineDataArray[11] = colour.g;
            Line::lineDataArray[12] = colour.b;
            Line::lineDataArray[13] = colour.a;

            // ...and push it to the graphics card
            glBufferData(GL_ARRAY_BUFFER, Line::BUFFER_SIZE_BYTES, Line::lineDataArray, GL_STATIC_DRAW);

            // Provide the projection matrix uniform
            glUniformMatrix4fv(Line::lineShaderProgram->uniform("mvpMatrix"), 1, GL_FALSE, glm::value_ptr(mvpMatrix) );

            // Save all line related attributes
            //glPushAttrib(GL_LINE_BIT);  ---------------------------------------------------------------------------------------------------- FIX THIS! GL_LINE_BIT not declared anywhere? WTF?

                // Set the line size for this particular line
                glLineWidth(lineWidth);

                // Draw the line as lines
                glDrawArrays(GL_LINES, 0, Line::VERTEX_COUNT);

            // Restore all line related attributes
            //glPopAttrib();

        // Unbind from our vertex array objext and disable our shader program
        glBindVertexArray(0);

    // Disable our shader program
    Line::lineShaderProgram->disable();
}
