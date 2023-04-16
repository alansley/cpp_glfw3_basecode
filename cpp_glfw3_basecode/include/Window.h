#ifndef WINDOW_H
#define WINDOW_H

#include <iostream>
#include <stdio.h>

// Include the GL Mathematics library
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/string_cast.hpp"

// Include GLFW3
#include "GLFW/glfw3.h"

// Include ImGui shenanigans
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include "Camera.h"
#include "../demo_scenes/DemoSceneGlobals.h"
#include "Utils.hpp"

// Save ourselves some typing...
using std::cout;
using std::endl;
using std::string;
using glm::vec3;
using glm::vec4;
using glm::mat4;

// Enums for whether to use `Hor+` or `Vert-` field of view calculations when resizing the window
enum FoVMode
{
    HorizontalPlus, // Default
    VerticalMinus
};

class Window
{
private:
    // Whether we should provide verbose output or not
	static const bool VERBOSE = true;

	// The actual GLFWwindow we'll draw to
	static GLFWwindow *glfwWindow;

    // Window size and projection settings
    static GLsizei windowWidth;
    static GLsizei windowHeight;
    static float aspectRatio;
    static FoVMode fovMode;                     // Whether we should use `Hor+` or `Vert-` projection matrix calculations when adjusting the window size
    static float horizFieldOfViewDegs;          // We adjust this if using a Hor+ FoVMode
    static float calculatedVertFieldOfViewRads; // We 
    static float nearClipDistance;
    static float farClipDistance;    

    // Matrices
    static mat4 projectionMatrix;      // The projection matrix is used to perform the 3D to 2D conversion i.e. it maps from eye space to clip space.
    static mat4 viewMatrix;            // The view matrix maps the world coordinate system into eye cordinates (i.e. world space to eye space)
    static mat4 orthoProjectionMatrix; // An orthographic (2D) projection matrix to use with the window

    // Misc
    static bool rightMouseButtonDown; // We'll only look around when the right mouse button is down
    static bool checkDemoChangeKeys;  // If we're showing included demos we'll check the keys to change demo

    // Camera. Params: location, rotation (degrees), window width, window height.
    static Camera *camera;

    // Place a breakpoint on the implementation of this callback to help track down any glfw issues
    static void glfwErrorCallback(int error, const char* description);

    // ImGui IO handler
    static ImGuiIO imguiIO;

    // FPS tracking vars
    static int framesDuringInterval;
    static double deltaTime, frameStartTimeSecs, frameEndTimeSecs, fpsReportIntervalSecs, fpsReportTimer, fps;
    static bool printFpsToConsole;

    static inline const float MIN_HORIZONTAL_FOV_DEGS = 10.0f;
    static inline const float MAX_HORIZONTAL_FOV_DEGS = 140.0f;

public:
    // Constructor/destructor
    Window(GLsizei width, GLsizei height, string windowTitle, bool checkDemoKeys);
    ~Window();

    // Initialisation method to set up our OpenGL rendering context
    static void initGL(GLFWwindow* window);

    // Window related methods
    static GLFWwindow* getGlfwWindow();
    static void displayWindowProperties(GLFWwindow* window);
    static GLsizei getWindowWidth()      { return windowWidth;          }
    static bool IsRightMouseButtonDown() { return rightMouseButtonDown; }
    
    // Callbacks
    static void resizeWindow(GLFWwindow *window, GLsizei newWidth, GLsizei newHeight);
    static void handleKeypress(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void handleDemoChangeKeys(GLFWwindow* window, int key, int scancode, int action, int mods); // Change demos w/ left/right cursor keys
    static void handleMouseMove(GLFWwindow *window, double mouseX, double mouseY);
    static void handleMouseButton(GLFWwindow *window, int button, int action, int mods);
    static void handleMouseWheelScroll(GLFWwindow* window, double xOffset, double yOffset);

    // Camera related methods
    static Camera* getCamera() { return camera; }
    static void moveCamera(double deltaTimeSecs);
    static void setCameraLocation(vec3 location);
    static void setCameraRotationEuler(vec3 rotationDegs);

    // Matrix getters
    static mat4 getViewMatrix();
    static mat4 getProjectionMatrix();
    static mat4 getViewProjectionMatrix();
    static mat4 getOrthoProjectionMatrix();

    // FPS-tracking related methods
    static void setFrameStartTime(double timeSecs) { frameStartTimeSecs = timeSecs; }
    static double getDeltaTime()                   { return deltaTime;              }
    static double getFPS()                         { return fps;                    }
    static void updateFpsDetails();

    // Field of View methods
    static float getHorizFoVDegs() { return horizFieldOfViewDegs; }

    static void toggleFoVMode()
    {
        if (fovMode == HorizontalPlus) { fovMode = VerticalMinus; } else { fovMode = HorizontalPlus; }
    }

    static string getFoVModeString()
	{
        switch (fovMode)
        {
        case HorizontalPlus:
            return "Hor+";
        case VerticalMinus:
            return "Vert-";
        default:
            return "Unknown!";
        }
    }
};

#endif // WINDOW_H
