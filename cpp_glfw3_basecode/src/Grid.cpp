#include "Grid.h"

// ----- Static declarations -----

ShaderProgram *Grid::gridShaderProgram;

// ----- Static initialisation -----

// How many grid instances currently exist
int Grid::gridInstances = 0;

//Define our vertex shader source code
//Note: The R" notation is for raw strings and preserves all spaces, indentation,
//newlines etc. in utf-8|16|32 wchar_t format, but requires C++0x or C++11.
const char *Grid::vertexShaderSource = R"(
#version 430

in vec4 vertexPosition; // Incoming vertex attribute

uniform mat4 mvpMatrix; // Combined Model/View/Projection matrix

void main(void)
{
    gl_Position = mvpMatrix * vertexPosition; // Project our geometry
}
)";

// Define our fragment shader source code
const char *Grid::fragmentShaderSource = R"(
#version 430

out vec4 fragColour; // Outgoing fragment colour

void main()
{
    fragColour = vec4(1.0); // Output in white at full opacity
}
)";

// Constructor
Grid::Grid(const float width, const float depth, const float height, const int numDivisions)
{
    // Calculate how many vertices our grid will consist of.
    // Multiplied by 2 because 2 verts per line, and times 2 again because our
    // grid is composed of -z to +z lines, as well as -x to +x lines. Add +4 to
    // the total for the final two lines to 'close off' the grid.
    numVerts = (numDivisions * 2 * 2) + 4;

    // So for this many vertices, we're going to be using this many floats...
    const int gridFloatCount = numVerts * VERTEX_COMPONENTS;

    // ...which we'll store in this float array!
    gridVertexArray = new float[gridFloatCount];

    // If this is the first grid we're creating then do the shader setup
    if (Grid::gridInstances == 0)
    {
        // ----- Grid shader program setup -----

        Grid::gridShaderProgram = new ShaderProgram("Grid Shader Program");
        Grid::gridShaderProgram->addShader(GL_VERTEX_SHADER,   Grid::vertexShaderSource);
        Grid::gridShaderProgram->addShader(GL_FRAGMENT_SHADER, Grid::fragmentShaderSource);
        Grid::gridShaderProgram->initialise();

        // ----- Grid shader attributes and uniforms -----

        // Add the shader attributes
        Grid::gridShaderProgram->bindAttribute("vertexPosition");

        // Add the shader uniforms
        Grid::gridShaderProgram->bindUniform("mvpMatrix");
    }

    // Increment our count of gridInstances so we can keep track of how many we have
    ++gridInstances;

    // For a grid of width and depth, the extent goes from -halfWidth to +halfWidth,
    // and -halfDepth to +halfDepth.
    const float halfWidth = width / 2.0f;
    const float halfDepth = depth / 2.0f;

    // How far we move our vertex locations each time through the loop
    const float xStep = width / static_cast<float>(numDivisions);
    const float zStep = depth / static_cast<float>(numDivisions);

    // Starting locations
    float xLoc = -halfWidth;
    float zLoc = -halfDepth;

    // Split the vertices into half for -z to +z lines, and half for -x to +x
    int halfNumVerts = numVerts / 2;

    // Index to keep track of the float value we're working on
    int index = 0;

    // Near to far lines
    // Note: Step by 2 because we're setting two vertices each time through the loop
    for (int loop = 0; loop < halfNumVerts; loop += 2)
    {
        // Far vertex of line
        gridVertexArray[index++] = xLoc;       // x
        gridVertexArray[index++] = height;     // y
        gridVertexArray[index++] = -halfDepth; // z

        // Near vertex of line
        gridVertexArray[index++] = xLoc;       // x
        gridVertexArray[index++] = height;     // y
        gridVertexArray[index++] = halfDepth;  // z

        // Step on the X-axis
        xLoc += xStep;
    }

    // Left to right lines
    // Note: Step by 2 because we're setting two vertices each time through the loop
    for (int loop = halfNumVerts; loop < numVerts; loop += 2)
    {
        // Left vertex
        gridVertexArray[index++] = -halfWidth; // x
        gridVertexArray[index++] = height;     // y
        gridVertexArray[index++] = zLoc;       // z

        // Right vertex
        gridVertexArray[index++] = halfWidth;  // x
        gridVertexArray[index++] = height;     // y
        gridVertexArray[index++] = zLoc;       // z

        // Step on the Z-axis
        zLoc += zStep;
    }

    // ----- Set up our Vertex Array Object (VAO) to hold the shader attributes -----
    // Note: The grid VAO cannot be static because we may have multiple grids of various sizes.

    // Get an Id for the Vertex Array Object (VAO) and bind to it
    glGenVertexArrays(1, &gridVaoId);
    glBindVertexArray(gridVaoId);

        // ----- Location Vertex Buffer Object (VBO) -----

        // Generate an id for the locationBuffer and bind to it
        glGenBuffers(1, &gridVertexBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, gridVertexBufferId);

        // Place the location data into the VBO...
        GLint gridArraySizeBytes = numVerts * VERTEX_COMPONENTS * sizeof(float);
        glBufferData(GL_ARRAY_BUFFER, gridArraySizeBytes, gridVertexArray, GL_STATIC_DRAW);

        // ...and specify the data format.
        glVertexAttribPointer(gridShaderProgram->attribute("vertexLocation"),  // Vertex location attribute index
                                                           VERTEX_COMPONENTS,  // Number of normal components per vertex
                                                                    GL_FLOAT,  // Data type
                                                                       false,  // Normalised?
                                                                           0,  // Stride
                                                                           0); // Offset
        // Unbind VBO
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Enable the vertex attribute at this location
        glEnableVertexAttribArray(gridShaderProgram->attribute("vertexLocation"));

    // Unbind our Vertex Array object - all the buffer and attribute settings above will be associated with our VAO!
    glBindVertexArray(0);
}

// Destructor
Grid::~Grid()
{
    delete[] gridVertexArray;    
    gridInstances--;

    // If this is the last grid we're getting rid of, clean up the shader program
    if (gridInstances == 0) { delete gridShaderProgram; }
}

// Method to draw the grid - takes a combined Model/View/Projection matrix
// to pass to the shader as a uniform.
void Grid::draw(glm::mat4 mvpMatrix)
{
    // Specify we're using our shader program
    gridShaderProgram->use();

        // Bind to our vertex array object
        glBindVertexArray(gridVaoId);

            // Provide the projection matrix uniform
            glUniformMatrix4fv(gridShaderProgram->uniform("mvpMatrix"), 1, GL_FALSE, glm::value_ptr(mvpMatrix) );

            // Draw the grid as lines
            glDrawArrays(GL_LINES, 0, numVerts * VERTEX_COMPONENTS);

        // Unbind from our vertex array object and disable our shader program
        glBindVertexArray(0);

    // Disable our shader program
    gridShaderProgram->disable();
}