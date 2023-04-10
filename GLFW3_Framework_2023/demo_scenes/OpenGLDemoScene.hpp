#ifndef OPENGL_DEMO_SCENE_HPP
#define OPENGL_DEMO_SCENE_HPP

#include "glad/glad.h"

#include "glm/gtc/type_ptr.hpp"    // Needed for the value_ptr() method
#include "glm/gtx/string_cast.hpp" // Needed to easily print glm stuff

#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"

#include "ShaderProgram.hpp"
#include "Grid.h"
#include "Model.h"
#include "Window.h"


class OpenGLDemoScene
{
private:
    // Properties used to draw a 3D model
    ShaderProgram* modelShaderProgram;
    GLuint modelVaoId, modelVertexBufferId, modelNormalBufferId;
    mat4 modelMVP;
    mat4 modelMMatrix = mat4(1.0f);
    mat3 normalMatrix;
    Model* model;
    vec3 modelRotationSpeed;

    // Elements required to load and draw a textured quad
    ShaderProgram* texQuadShaderProgram;
    GLuint texQuadVaoId, texQuadVertexBufferId, textureID1, textureID2;
    GLint texture1SamplerID, texture2SamplerID;	// Note: If you want to use separate sampling units for each texture 
    float quadSize = 50.0f;
    float texQuadVertices[20]; // 3 vertex positions + 2 texture coordinates = 5 floats. 4 verts per quad = 20 floats in total.

    // Pointers to two grids we draw by default just so we can see something is happening and display orientation when moving w/ the mouse & keys
    //Grid upperGrid, lowerGrid;
    // Instantiate our grids. Params: width, depth, level (i.e. location on Y-axis), number of grid lines
    Grid* upperGrid = new Grid(500.0f, 500.0f,  50.0f, 20);
    Grid* lowerGrid = new Grid(500.0f, 500.0f, -50.0f, 20);

    // ----- Methods -----
    
    // Setup the shader program to draw a 3D model
    void setupModelShaderProgram(Model* model)
    {
        // Setup the shader to draw our model
        modelShaderProgram = new ShaderProgram("Model Shader Program");
        modelShaderProgram->addShader(GL_VERTEX_SHADER, ShaderProgram::loadShaderFromFile("shaders/phong.vert"));
        modelShaderProgram->addShader(GL_FRAGMENT_SHADER, ShaderProgram::loadShaderFromFile("shaders/phong.frag"));
        //modelShaderProgram->addShader(GL_TESS_CONTROL_SHADER, ShaderProgram::loadShaderFromFile("shaders/tessellation_control_shader.glsl"));
        //modelShaderProgram->addShader(GL_TESS_EVALUATION_SHADER, ShaderProgram::loadShaderFromFile("shaders/tessellation_evaluation_shader.glsl"));

        modelShaderProgram->initialise();

        // Add shader attributes
        modelShaderProgram->bindAttribute("vertexPosition");
        modelShaderProgram->bindAttribute("vertexNormal");

        // Add shader uniforms
        modelShaderProgram->bindUniform("modelMatrix");
        modelShaderProgram->bindUniform("viewMatrix");
        modelShaderProgram->bindUniform("projectionMatrix");
        modelShaderProgram->bindUniform("normalMatrix");

        //modelShaderProgram->bindUniform("time");        // Number of seconds since starting (used for randomness within shaders)

        // Working in 3D so we have x/y/z components for the vertex position (we also use the same value for the number of normal components to use)
        constexpr int VERTEX_COMPONENTS = 3;

        // Get an Id for the Vertex Array Object (VAO) and bind to it
        glGenVertexArrays(1, &modelVaoId);
        glBindVertexArray(modelVaoId);

        // Generate a vertex buffer, fill it, and specify attributes
        glGenBuffers(1, &modelVertexBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, modelVertexBufferId);
        glBufferData(GL_ARRAY_BUFFER, model->getVertexDataSizeBytes(), model->getVertexData(), GL_STATIC_DRAW);

        glVertexAttribPointer(modelShaderProgram->attribute("vertexPosition"), // Vertex location attribute index
            VERTEX_COMPONENTS, // Number of location components per vertex
            GL_FLOAT, // Data type
            false, // Normalised?
            0,  // Stride
            0); // Offset

        // Generate a normal buffer, fill it, and specify attributes
        glGenBuffers(1, &modelNormalBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, modelNormalBufferId);
        glBufferData(GL_ARRAY_BUFFER, model->getNormalDataSizeBytes(), model->getNormalData(), GL_STATIC_DRAW);
			// Args: attribute location, num components, component data type, normalised?, stride, offset
			glVertexAttribPointer(modelShaderProgram->attribute("vertexNormal"), VERTEX_COMPONENTS, GL_FLOAT, false, 0, 0);
        // Unbind VBO
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Enable the vertex attributes
        glEnableVertexAttribArray(modelShaderProgram->attribute("vertexPosition"));
        glEnableVertexAttribArray(modelShaderProgram->attribute("vertexNormal"));

        // Unbind our Vertex Array object - all the buffer and attribute settings above will be associated with our VAO!
        glBindVertexArray(0);
    }

    // Parameter-less version for our demo scene
    void setupModelShaderProgram() { setupModelShaderProgram(model); }

    void drawModel(Model* model)
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        // Specify the shader program we're using
        modelShaderProgram->use();

        // Bind to our vertex array object
        glBindVertexArray(modelVaoId);

        // Rotate the model matrix
        auto currentTime = static_cast<float>(glfwGetTime());
        modelMMatrix = glm::rotate(mat4(1.0), currentTime * modelRotationSpeed.z, Utils::Z_AXIS);
        modelMMatrix = glm::rotate(modelMMatrix, currentTime * modelRotationSpeed.y, Utils::Y_AXIS);
        modelMMatrix = glm::rotate(modelMMatrix, currentTime * modelRotationSpeed.x, Utils::X_AXIS);

        // Provide the model. view, and projection matrix
        glUniformMatrix4fv(modelShaderProgram->uniform("modelMatrix"), 1, GL_FALSE, glm::value_ptr(modelMMatrix));
        glUniformMatrix4fv(modelShaderProgram->uniform("viewMatrix"), 1, GL_FALSE, glm::value_ptr(Window::getViewMatrix()));
        glUniformMatrix4fv(modelShaderProgram->uniform("projectionMatrix"), 1, GL_FALSE, glm::value_ptr(Window::getProjectionMatrix()));

        //glUniform1f(modelShaderProgram->uniform("time"), (GLfloat)glfwGetTime());

        // Calculate the normal matrix as the inverse transpose of a 3x3 of the Model matrix and provide it
        normalMatrix = glm::transpose(glm::inverse(mat3(modelMMatrix)));
        glUniformMatrix3fv(modelShaderProgram->uniform("normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));

        // Draw the model as triangles
        glDrawArrays(GL_TRIANGLES, 0, model->getNumVertices());

        // Unbind from our vertex array object and disable our shader program
        glBindVertexArray(0);
        modelShaderProgram->disable();
    }

    // Parameter-less version for our demo scene
    void drawModel() { drawModel(model); }

    // Method to load the C++/OpenGL textures and set up a shader program to draw them as a textured quad
    void setupTexturedQuad()
    {
        // Load textures!
        textureID1 = Utils::loadTexture("textures/opengl_logo.png");
        textureID2 = Utils::loadTexture("textures/cpp_logo.png");

        // ------------- Textured Quad shader ------------------------

        // Setup the shader to draw our model
        texQuadShaderProgram = new ShaderProgram("Textured Quad Shader Program");
        texQuadShaderProgram->addShader(GL_VERTEX_SHADER, ShaderProgram::loadShaderFromFile("shaders/textured_quad.vert"));
        texQuadShaderProgram->addShader(GL_FRAGMENT_SHADER, ShaderProgram::loadShaderFromFile("shaders/textured_quad.frag"));
        texQuadShaderProgram->initialise();

        // Add shader attributes
        texQuadShaderProgram->bindAttribute("position");
        texQuadShaderProgram->bindAttribute("texCoords");

        // Add shader uniforms
        texQuadShaderProgram->bindUniform("modelMatrix");
        texQuadShaderProgram->bindUniform("projectionMatrix");
        texQuadShaderProgram->bindUniform("textureMap");

        // x/y/z for each vertex plus s/t for the texture coordinates
        constexpr int VERTEX_COMPONENTS = 3;
        constexpr int TEXTURE_COMPONENTS = 2;
        constexpr int FLOATS_PER_VERTEX = VERTEX_COMPONENTS + TEXTURE_COMPONENTS;

        // Get an Id for the Vertex Array Object (VAO) and bind to it
        glGenVertexArrays(1, &texQuadVaoId);
        glBindVertexArray(texQuadVaoId);

        // ----- Location Vertex Buffer Object (VBO) -----

        // Generate a vertex buffer, fill it and specify attributes
        glGenBuffers(1, &texQuadVertexBufferId);
        glBindBuffer(GL_ARRAY_BUFFER, texQuadVertexBufferId);
        glBufferData(GL_ARRAY_BUFFER, sizeof(texQuadVertices), texQuadVertices, GL_STATIC_DRAW);
			// Args: attribute location, num components, component data type, normalised?, stride, offset
			glVertexAttribPointer(texQuadShaderProgram->attribute("position"), VERTEX_COMPONENTS, GL_FLOAT, false, sizeof(float) * FLOATS_PER_VERTEX, 0);
			glVertexAttribPointer(texQuadShaderProgram->attribute("texCoords"), TEXTURE_COMPONENTS, GL_FLOAT, false, sizeof(float) * FLOATS_PER_VERTEX, (void*)(sizeof(float) * VERTEX_COMPONENTS));

			// Enable the vertex attributes
			glEnableVertexAttribArray(texQuadShaderProgram->attribute("position"));
			glEnableVertexAttribArray(texQuadShaderProgram->attribute("texCoords"));

        // Unbind VBO & VAO - all the buffer and attribute settings above will be associated with our VAO!
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void drawTexturedQuad()
    {
        // ----- Draw spinning textured quad -----

        // Disable depth testing so this always gets overlaid on top of whatever has already been drawn
        glDisable(GL_DEPTH_TEST);

        // Specify we're using our shader program & bind to our vertex array object
        texQuadShaderProgram->use();
        glBindVertexArray(texQuadVaoId);

        // Translate model matrix to upper-right corner and rotate around Y-axis
        mat4 texQuadModelMatrix = mat4(1.0f);
        vec3 corner = vec3(Window::getWindowWidth() - quadSize, quadSize, -quadSize);
        texQuadModelMatrix = glm::translate(texQuadModelMatrix, corner);
        texQuadModelMatrix = glm::rotate(texQuadModelMatrix, -(GLfloat)glfwGetTime() * 2.0f, Utils::Y_AXIS);

        // Work out the normal of the quad
        vec3 modelRight = texQuadModelMatrix * vec4(Utils::X_AXIS, 0.0f);
        vec3 modelUp = texQuadModelMatrix * vec4(Utils::Y_AXIS, 0.0f);
        vec3 normal = glm::normalize(glm::cross(vec3(modelRight), vec3(modelUp)));
        //cout << glm::to_string(normal) << endl;
        float dotProduct = glm::dot(normal, Utils::Z_AXIS);
        //cout << dotProduct << endl;

        // Get the location of the "textureMap" uniform. Note: Don't use texQuadShaderProgram->uniform("textureMap") for this! We need it as an int!
        GLint textureMapLocation = glGetUniformLocation(texQuadShaderProgram->getProgramID(), "textureMap");

        // Pointing forward? Draw OpenGL logo texture...
        // Note: Just as an example I'm using sampler unit 0 for one texture and then sampler unit 1 for another - you don't have to do this.
        if (dotProduct < 0.0f)
        {
            // Spin the quad another 180 degrees otherwise the image is back-to-front (i.e. displays right-to-left when it should be left-to-right)
            texQuadModelMatrix = glm::rotate(texQuadModelMatrix, glm::pi<float>(), Utils::Y_AXIS);

            // Send the shader program the texture image unit we'll be using (0), then set that as active & bind the texture to it
            glUniform1i(textureMapLocation, 0);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textureID1);
        }
        else // ...otherwise draw the "C++" texture.
        {
            // Send the shader program the texture image unit we'll be using (1), then set that as active & bind the texture to it
            glUniform1i(textureMapLocation, 1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, textureID2);
        }

        // Provide the ModelView and Projection matrix uniforms
        glUniformMatrix4fv(texQuadShaderProgram->uniform("modelMatrix"), 1, GL_FALSE, glm::value_ptr(texQuadModelMatrix));
        glUniformMatrix4fv(texQuadShaderProgram->uniform("projectionMatrix"), 1, GL_FALSE, glm::value_ptr(Window::getOrthoProjectionMatrix()));

        // Draw the quad
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        // Disable the Vertex Array Object, disable the shader program, and re-enable depth testing
        glBindVertexArray(0);
        texQuadShaderProgram->disable();
        glEnable(GL_DEPTH_TEST);
    }

    void drawGUI()
    {
        // Data we'll use in our GUI
        string fpsString = "FPS: " + std::to_string(Window::getFPS());

        auto camera = Window::getCamera();

        string camRotDegsString = "Cam Rot (Degs): " + glm::to_string(camera->getRotationDegs());
        string camRotRadsString = "Cam Rot (Rads): " + glm::to_string(camera->getRotationRads());


        string sinXRotRads = "SinXRot: " + std::to_string(sin(camera->getRotationRads().x));
        string cosXRotRads = "CosXRot: " + std::to_string(cos(camera->getRotationRads().x));
        string sinYRotRads = "SinYRot: " + std::to_string(sin(camera->getRotationRads().y));
        string cosYRotRads = "CosYRot: " + std::to_string(cos(camera->getRotationRads().y));


        //auto camRot = Camera::getR

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Create a window with the given title & append into it
        ImGui::SetNextWindowPos(ImVec2(20, 20));
        ImGui::SetNextWindowSize(ImVec2(400, 270));
        ImGui::Begin("Details / Settings");
			ImGui::SeparatorText("Details");
		        ImGui::Text(fpsString.c_str());
		        ImGui::Text(camRotDegsString.c_str());
		        ImGui::Text(camRotRadsString.c_str());
		        ImGui::Text(sinXRotRads.c_str());
		        ImGui::Text(cosXRotRads.c_str());
		        ImGui::Text(sinYRotRads.c_str());
		        ImGui::Text(cosYRotRads.c_str());
			ImGui::SeparatorText("Sliders");
		        ImGui::SliderFloat("X Rot Speed", &modelRotationSpeed.x, -5.0f, 5.0f);
		        ImGui::SliderFloat("Y Rot Speed", &modelRotationSpeed.y, -5.0f, 5.0f);
		        ImGui::SliderFloat("Z Rot Speed", &modelRotationSpeed.z, -5.0f, 5.0f);
        ImGui::End();

        // Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void drawGrids()
    {
        lowerGrid->draw( Window::getViewProjectionMatrix() );
        upperGrid->draw( Window::getViewProjectionMatrix() );
    }

public:
    // Constructor
    OpenGLDemoScene()
    {
        // Load our cow model & scale it up.
       // Note: If we use `Model::DRAWING_AS_ELEMENTS` then while the vertex count is decreased the normals are per-FACE rather than per-VERTEX, so it gives
       // the models a 'faceted' look.
        model = new Model("models/cow.obj", Model::DRAWING_AS_ARRAYS);
        model->scale(4.0f);
        modelRotationSpeed = vec3(0.0f, 1.0f, 0.0f);

        // TODO: Surely there has to be a way to do this 'nicely' via some `new float[] { value1, value2 }` stuff - but C++ complains =/
        // Bottom-left
        texQuadVertices[0] = -quadSize;  // x
        texQuadVertices[1] = -quadSize;  // y
        texQuadVertices[2] = 0.0f;       // z
        texQuadVertices[3] = 0.0f;       // s
        texQuadVertices[4] = 0.0f;       // t

        // Bottom-right
        texQuadVertices[5] = quadSize;   // x
        texQuadVertices[6] = -quadSize;  // y
        texQuadVertices[7] = 0.0f;       // z
        texQuadVertices[8] = 1.0f;       // s
        texQuadVertices[9] = 0.0f;       // t

        // Top-left
        texQuadVertices[10] = -quadSize; // x
        texQuadVertices[11] = quadSize;  // y
        texQuadVertices[12] = 0.0f;      // z
        texQuadVertices[13] = 0.0f;      // s
        texQuadVertices[14] = 1.0f;      // t

        // Top-right
        texQuadVertices[15] = quadSize;  // x
        texQuadVertices[16] = quadSize;  // y
        texQuadVertices[17] = 0.0f;      // z
        texQuadVertices[18] = 1.0f;      // s
        texQuadVertices[19] = 1.0f;      // t
    }

    // Destructor
    ~OpenGLDemoScene()
    {
        delete upperGrid;
        delete lowerGrid;
        delete model;
        delete modelShaderProgram;
        delete texQuadShaderProgram;
    }

    // Method to call all setup functions we require
    void setup()
    {
        setupModelShaderProgram();
        setupTexturedQuad();
    }

    // Method to draw all the elements of our OpenGL demo scene
    void draw()
    {
        drawGrids();
        drawModel();
        drawTexturedQuad();
        drawGUI();
    }























   
};
	
#endif // OPENGL_DEMO_SCENE_HPP
