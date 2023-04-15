#include <iostream>
#include <ctime>    // Required to get the time to seed the RNG
#include <cstdlib>  // Required for `srand` and `exit` calls

// IMPORTANT: Always pull in GLAD _before_ GLFW or we get a 'gl.h already imported' error
#ifndef __glad_h_
    #include "glad/glad.h"
#endif

#include "Window.h"

// Include the STB image loader.
// IMPORTANT: We must place this stb include along with the `STB_IMAGE_IMPLEMENTATION` definition AFTER all other
// headers/classes which themselves may include stb_image.h or we'll get build errors.
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

// Example scenes are kept separate to keep main clean
#include "demo_scenes/OpenGLDemoScene.hpp"
#include "demo_scenes/ImGuiDemoScene.hpp"

// Our demo scene controlling variables are declared extern in the `DemoSceneGlobals` header so we can access them from multiple classes easily
#include "demo_scenes/DemoSceneGlobals.h"
bool showDemoScenes = true;
int  currentDemoScene = 0;
int  demoSceneCount = 2;

// Our Window class holds a `glfwWindow`, a Camera, and all mouse/key & window handling callbacks
Window *window;

int main()
{
    // Seed the random number generator with the current time
    srand(static_cast<unsigned>(time(nullptr)));

    // Instantiate our window. Params: width, height, window title
    const string windowTitle = "r3d GLFW3 Basecode | Use left/right cursor to change demo scenes, WSAD to move, and RMB + mouse to look in 3D scenes.";
    window = new Window(1280, 720, windowTitle, showDemoScenes);

    // Load OpenGL extensions. NOTE: This must be called after we create our window and have a valid OpenGL context.
    const int gladLoaded = gladLoadGL();
    if (!gladLoaded)
    {
        cout << "GLAD failed to load OpenGL extensions!" << endl;
        return 1;
    }    

    // Move the camera back a little. Note: The negative Z-Axis runs INTO the screen so the positive Z-Axis runs OUT FROM the screen (unlike Unity etc.)
    Window::setCameraLocation(vec3(0.0, 0.0, 50.0));
    
    // Create objects for our demo scenes if we should
    OpenGLDemoScene* openGLDemoScene = nullptr;
    ImGuiDemoScene* imguiDemoScene   = nullptr;
    if (showDemoScenes)
    {
        openGLDemoScene = new OpenGLDemoScene();
        openGLDemoScene->setup();
        imguiDemoScene = new ImGuiDemoScene();
    }    

    // ----- Main game-loop -----
    double currentTime;
    bool exitMainLoop = false;
    while (!glfwWindowShouldClose(Window::getGlfwWindow()) && !exitMainLoop)
    {
        // Get frame details and input
        currentTime = glfwGetTime();
        Window::setFrameStartTime(currentTime);
        glfwPollEvents();
        Window::moveCamera( Window::getDeltaTime() );

        // ----- Draw stuff -----
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Remove this block out and add your own things as required! 
        if (showDemoScenes)
        {
            switch (currentDemoScene)
            {
            case 0:
                openGLDemoScene->draw();
                break;
            case 1:
                ImGuiDemoScene::draw();
                break;
            default:
                cout << "Asked to draw demo scenes but no matching scene found - aborting!" << endl;
                exitMainLoop = true; // Note: We exit the main loop rather than just calling `exit` so that we tear-down & free our resources
                break;
            }
        }

        // ----- End of drawing / show stuff -----
        glfwSwapBuffers( Window::getGlfwWindow() );        
        Window::updateFpsDetails();       
    }

    // ----- Post game-loop teardown -----

    // Clean up ImGUI
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Destroy the window and shutdown GLFW
    glfwDestroyWindow( Window::getGlfwWindow() );
    glfwTerminate();

    // Free our pointers and exit
    if (showDemoScenes)
    {
        delete openGLDemoScene;
        delete imguiDemoScene;
    }
    delete window;
    return 0;
}