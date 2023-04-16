#include "Window.h"

#include <algorithm>

// Static declarations (without which we get 'unresolved external symbol...' errors).
GLFWwindow *Window::glfwWindow;
GLsizei     Window::windowWidth;
GLsizei     Window::windowHeight;
float       Window::aspectRatio;
float       Window::horizFieldOfViewDegs;          // We only ever set our field of view via the horizontal FoV value..
float       Window::calculatedVertFieldOfViewRads; // ..but we need the VERTICAL FoV to generate the projection matrix.
FoVMode  Window::fovMode;
float       Window::nearClipDistance;
float       Window::farClipDistance;
mat4        Window::projectionMatrix;
mat4        Window::viewMatrix;
mat4        Window::orthoProjectionMatrix;
bool        Window::rightMouseButtonDown;
Camera     *Window::camera;
ImGuiIO     Window::imguiIO;
int         Window::framesDuringInterval;
double      Window::deltaTime;
double      Window::frameStartTimeSecs;
double      Window::frameEndTimeSecs;
double      Window::fpsReportIntervalSecs;
double      Window::fpsReportTimer;
double      Window::fps;
bool        Window::printFpsToConsole;
bool        Window::checkDemoChangeKeys;

// Constructor
Window::Window(GLsizei width, GLsizei height, string windowTitle, bool checkDemoKeys)
{
    // Protect against a zero-sized window
    //if (width == 0)  { width = 1;  }
    //if (height == 0) { height = 1; }

    checkDemoChangeKeys = checkDemoKeys;

    // Window and projection settings
    windowWidth                   = width;
    windowHeight                  = height;
    aspectRatio                   = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

    horizFieldOfViewDegs          = 90.0f;
    fovMode                       = HorizontalPlus;
    float halfHorizFoVRads        = horizFieldOfViewDegs * Utils::DEGS_TO_RADS * 0.5f;
    float heightToWidthRatio      = static_cast<float>(windowHeight) / static_cast<float>(windowWidth);
    calculatedVertFieldOfViewRads = 2.0f * atan(tan(halfHorizFoVRads) * heightToWidthRatio);

    nearClipDistance              = 1.0f;
    farClipDistance               = 2000.0f;
    
    viewMatrix            = mat4(1.0f);

    // Misc
    rightMouseButtonDown = false; // We'll only look around when the right mouse button is down

    // Instantiate our camera at the origin looking down the negative Z axis (i.e. 'into' the screen)
    camera = new Camera(vec3(0.0f), vec3(0.0f), windowWidth, windowHeight);

    // Translate to our camera position
    viewMatrix = glm::translate(viewMatrix, -camera->getPosition());

    // ----- Initialise GLFW, specify window hints & open a context -----

    glfwSetErrorCallback(glfwErrorCallback);

    // IMPORTANT: glfwInit resets all window hints, so we must call glfwInit FIRST and THEN we supply window hints!
    if (!glfwInit())
    {
        cout << "glfwInit failed!" << endl;
        exit(1);
    }

    // If we want to use a a core profile (i.e. no legacy fixed-pipeline functionality) or if we want to
    // use forward compatible mode (i.e. only non-deprecated features of a given OpenGL version available)
    // then we MUST specify the MAJOR.MINOR context version we want to use FIRST!
    //
    // For further window hints, see: http://www.glfw.org/docs/latest/window.html#window_hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);    
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE); // Resizable windows are nice    
    glfwWindowHint(GLFW_SAMPLES, 8);         // Ask for 8x Anti-Aliasing       

    // Create a window. Params: width, height, title, *monitor, *share
    // Note: Pass a monitor to the monitor arg for full-screen (we pass NULL to get a window).
    glfwWindow = glfwCreateWindow(windowWidth, windowHeight, windowTitle.c_str(), NULL, NULL);
    if (!glfwWindow)
    {
        cout << "Failed to create window - bad context MAJOR.MINOR version?" << endl;
        Utils::getKeypressThenExit();
    }    

    // Make the current OpenGL context active
    glfwMakeContextCurrent(glfwWindow);

    // Calculate the projection matrix & orthographic projection matrix for the current window size
    resizeWindow(glfwWindow, width, height);

    // Setup ImGUI. Note: We can only do this when we have a current OpenGL context.
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    imguiIO = ImGui::GetIO();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls - uncomment as req'd
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls  - uncomment as req'd
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true);
    const char* glsl_version = "#version 430";
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Set up our OpenGL settings
    initGL(glfwWindow);

    // Setup our FPS tracking vars
	deltaTime             = 1.0 / 60.0; // We'll assume 60fps for very first frame as we have no data
	framesDuringInterval  = 0;	
	fpsReportIntervalSecs = 1.0;
	fpsReportTimer        = 0.0;	
	printFpsToConsole     = false;
}

// Destructor
Window::~Window() { delete camera; }

// Function to set up our OpenGL rendering context
void Window::initGL(GLFWwindow *window)
{
    // ---------- Setup OpenGL Options ----------
    glViewport( 0, 0, GLsizei(windowWidth), GLsizei(windowHeight) ); // Viewport is entire window
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);                         // Clear to black with full alpha
    glEnable(GL_DEPTH_TEST);                                                      // Enable depth testing
    glDepthFunc(GL_LEQUAL);                                                           // Specify depth testing function
    glClearDepth(1.0);                                                                // Clear the full extent of the depth buffer (default)
    glCullFace(GL_BACK);                                                         // Specify that if we cull faces, we cull the back-face..
    glDisable(GL_CULL_FACE);                                                      // ..but for now we'll disable back-face culling.    
    glFrontFace(GL_CCW);                                                         // Counter-clockwise winding indicates a forward facing polygon (default)
    glEnable(GL_BLEND);                                                           // Enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);                 // Set the blend function

    // ---------- Setup GLFW Callback Functions ----------
    glfwSetWindowSizeCallback(glfwWindow, resizeWindow);
    glfwSetKeyCallback(glfwWindow, handleKeypress);
    glfwSetCursorPosCallback(glfwWindow, handleMouseMove);
    glfwSetMouseButtonCallback(glfwWindow, handleMouseButton);
    glfwSetScrollCallback(glfwWindow, handleMouseWheelScroll);

    // ---------- Setup GLFW Options ----------
    glfwSwapInterval(1);                                                       // Swap buffers every frame (i.e. lock to VSync)
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);       // Do not hide the mouse cursor
    glfwSetWindowPos(window, 200, 200);                              // Push the top-left of the window out from the top-left corner of the screen
    glfwSetCursorPos(window, windowWidth / 2.0, windowHeight / 2.0); // Centre the mouse cursor on startup
}

// Callback function to resize the window and set the viewport to the correct size.
// Note: The projection matrix field of view (FoV) is set using either the `Hor+` or `Vert-` algorithms based on the current setting.
// See: https://en.wikipedia.org/wiki/Field_of_view_in_video_games
// Also: https://www.wsgf.org/article/screen-change
void Window::resizeWindow(GLFWwindow *window, GLsizei newWidth, GLsizei newHeight)
{
    // Protect against a zero-sized window
    if (newWidth  == 0) { newWidth  = 1; }
    if (newHeight == 0) { newHeight = 1; }

    if (VERBOSE)
    {
        cout << "New widow size is: " << newWidth << "x" << newHeight << " (Aspect ratio: " << aspectRatio << ")" << endl;
    }

    // Keep track of the new width and height of the window
    windowWidth  = newWidth;
    windowHeight = newHeight;

    // Calculate the new aspect ratio
    aspectRatio = static_cast<float>(windowWidth) / static_cast<float>(windowHeight);

    // Calculate the "Vert-" FoV required to maintain our fixed horizontal field of view
    float horizFoVRads = horizFieldOfViewDegs * Utils::DEGS_TO_RADS;
    float vertFoVRads = 2.0f * atan(tan(horizFoVRads / 2.0f) * (float)windowHeight / (float)windowWidth);
    float vertFoVDegs = vertFoVRads * Utils::RADS_TO_DEGS;

    // If we're using Vert- FoV behaviour then the horizontal FoV remains the same and the vertical FoV decreases as window width increases & vice versa
    if (fovMode == VerticalMinus)
    {
        // Nothing to do - we've calculated the `Vert-` FoV setting above

        if (VERBOSE)
        {
            cout << "Vert-: To maintain a horiz FoV of " << horizFieldOfViewDegs << " we must use a vert FoV of: " << vertFoVDegs << " degs" << endl;
        }        
    }
    // If we're using Hor+ FoV behaviour then the vertical FoV remains largely the same while the horiz FoV increases as window width increases & vice versa
    else if (fovMode == HorizontalPlus)
    {
        // Work out the horizontal FoV using our initial vert FoV..
        float horizFoVRads = 2.0f * atan(tan(calculatedVertFieldOfViewRads/ 2.0f) * aspectRatio);
        horizFieldOfViewDegs = horizFoVRads * Utils::RADS_TO_DEGS;

        // ..then recalculate the vert FoV (that we need for our glm::perspective method) from that.
        vertFoVRads = 2.0f * atan(tan(horizFoVRads / 2.0f) * (float)windowHeight / (float)windowWidth);
        vertFoVDegs = vertFoVRads * Utils::RADS_TO_DEGS;

        if (VERBOSE)
        {
            cout << "Hor+: To maintain a vert FoV of " << calculatedVertFieldOfViewRads * Utils::RADS_TO_DEGS << " we must use a horiz FoV of: " << horizFieldOfViewDegs << " degs" << endl;
        }
    }
    else
    {
        std::cerr << "Unknown FoV setting: " << fovMode << " - ignoring FoV change!" << endl;
        return;
    }

    // Recalculate the projection matrix and orthographic projection matrix.
    // Note: The orthographic matrix has (0,0) at the top-left and (width,height) at the bottom-right.
    projectionMatrix = glm::perspective(vertFoVRads, aspectRatio, nearClipDistance, farClipDistance);
    orthoProjectionMatrix = glm::ortho(0.0f, static_cast<float>(windowWidth), static_cast<float>(windowHeight), 0.0f, 0.1f, 100.0f);

    // Viewport is the entire window
    glViewport(0, 0, windowWidth, windowHeight);

    // Update the midpoint location in the camera class because it uses these values, too
    camera->updateWindowMidpoint(windowWidth, windowHeight);
}

// Define the static methods to move to the next & previous demo scenes
static void nextDemo()      { if (currentDemoScene < (demoSceneCount - 1)) { ++currentDemoScene; } }
static void previousDemo()  { if (currentDemoScene > 0) { --currentDemoScene; } }

// Callback function to handle keypresses
void Window::handleKeypress(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    //if (action == GLFW_PRESS || action == GLFW_REPEAT)
    {
        if      (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)                        { glfwSetWindowShouldClose(window, GL_TRUE); }
        else if (key == GLFW_KEY_LEFT   && action == GLFW_PRESS && checkDemoChangeKeys) { previousDemo();  }
        else if (key == GLFW_KEY_RIGHT  && action == GLFW_PRESS && checkDemoChangeKeys) { nextDemo();      }
        else if (key == GLFW_KEY_F      && action == GLFW_PRESS && checkDemoChangeKeys) { toggleFoVMode(); }
        else    camera->handleKeypress(key, action); // Note: GLFW_PRESS/REPEAT/RELEASE of other keys all get passed through to the camera       
    }
}

// Callback function to handle mouse movement
void Window::handleMouseMove(GLFWwindow *window, double mouseX, double mouseY)
{
    // Pass through the cursor position to ImGUI
    ImGui_ImplGlfw_CursorPosCallback(window, mouseX, mouseY);

    // We'll only look around when the right mouse button is down
    if (rightMouseButtonDown) { camera->handleMouseMove(window, mouseX, mouseY); }
}

// Callback function to handle mouse button presses
void Window::handleMouseButton(GLFWwindow *window, int button, int action, int mods)
{
    // Pass through the mouse button details to ImGUI
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

    // When the right mouse button is down (only) we'll use mouse movements to look around
    if (button == GLFW_MOUSE_BUTTON_2)
    {
        if (action == GLFW_PRESS)
        {
            glfwSetCursorPos(window, windowWidth / 2.0, windowHeight / 2.0);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            rightMouseButtonDown = true;
        }
        else // Action must be GLFW_RELEASE
        {
            // Display cursor when the user releases the LMB            
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            rightMouseButtonDown = false;
        }
    }
}

// Callback function to adjust the horizontal field of view when the mouse wheel is scrolled
void Window::handleMouseWheelScroll(GLFWwindow *window, double xOffset, double yOffset)
{
    // Adjust our horizontal field of view.
    // Note: Change this to `+=` to reverse the zoom direction. By using `-=` we zoom IN when we 'scroll up' (yOffset will be +1), and
    // we zoom OUT when we 'scroll down' (yOffset will be -1).
    // Also: Multiply the yOffset value to increase or decrease the zoom sensitivity.
    horizFieldOfViewDegs -= yOffset;

    // Clamp to a minimum and maximum horizontal field of view
    horizFieldOfViewDegs = std::ranges::clamp(horizFieldOfViewDegs, MIN_HORIZONTAL_FOV_DEGS, MAX_HORIZONTAL_FOV_DEGS);

    // Recalculate the vertical FoV (required for glm::perspective)
    float halfHorizFoVRads = horizFieldOfViewDegs * Utils::DEGS_TO_RADS * 0.5f;
    float heightToWidthRatio = (float)windowHeight / (float)windowWidth;
    calculatedVertFieldOfViewRads = 2.0f * atan(tan(halfHorizFoVRads) * heightToWidthRatio);

    // Call resize window which will recalculate the projection matrix for us
    resizeWindow(glfwWindow, windowWidth, windowHeight);
}

void Window::moveCamera(double deltaTimeSecs)
{
    camera->move(deltaTimeSecs);

    // Reset our View matrix
    viewMatrix = mat4(1.0f);

    // Perform camera rotation. Note: We must rotate and THEN transform or WSAD movement is applied to the global X/Y/Z axes!
    viewMatrix = glm::rotate(viewMatrix, camera->getXRotationRads(), Utils::X_AXIS);
    viewMatrix = glm::rotate(viewMatrix, camera->getYRotationRads(), Utils::Y_AXIS);

    // Translate to our camera position
    viewMatrix = glm::translate(viewMatrix, -camera->getPosition());    
}

void Window::setCameraLocation(vec3 location) { camera->setPosition(location); }

void Window::setCameraRotationEuler(vec3 rotationDegs) { camera->setRotationDegs(rotationDegs); }

mat4 Window::getViewMatrix()            { return Window::viewMatrix;                  }
mat4 Window::getProjectionMatrix()      { return projectionMatrix;                    }
mat4 Window::getViewProjectionMatrix()  { return mat4(projectionMatrix * viewMatrix); }
mat4 Window::getOrthoProjectionMatrix() { return orthoProjectionMatrix;               }
GLFWwindow* Window::getGlfwWindow()     { return glfwWindow;                          }

// Function to display details about our OpenGL rendering context
void Window::displayWindowProperties(GLFWwindow *window)
{
	cout << "----- GLFW Window Properties -----" << endl;

	// Get profile version
	int openGLProfileVersion = glfwGetWindowAttrib(window, GLFW_OPENGL_PROFILE);

    // Get a friendly string version of the profile
	string profileString = "";
	switch (openGLProfileVersion)
	{
		case 0:
			profileString = "Default (0)";
			break;
		case GLFW_OPENGL_CORE_PROFILE:
			profileString = "Core";
			break;
		case GLFW_OPENGL_COMPAT_PROFILE:
			profileString = "Compatibility";
			break;
		default:
			profileString = "Unknown profile enumeration: " + openGLProfileVersion;
			break;
	}

    // Get the context major and minor version
	int openGLMajorVersion = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
	int openGLMinorVersion = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);

    // Get whether we're in forward compatible mode
	int forwardCompatibleMode  = glfwGetWindowAttrib(window, GLFW_OPENGL_FORWARD_COMPAT);
	string forwardCompatibleModeString = forwardCompatibleMode ? "Yes" : "No";

    // Display our human readable profile/context/forward-compatability details
	cout << "OpenGL profile: " << profileString << " (Context: " << openGLMajorVersion << "." << openGLMinorVersion << ") -  Forward compatible mode: " << forwardCompatibleModeString << endl;
    cout << "Note: Values of 0 can mean 'System Default'." << endl;

    // Grab the refresh rate. Note: You can only call `glfwGetWindowMonitor(window)` if the window itself it fullscreen - otherwise
    // it returns null as the monitor. See: https://stackoverflow.com/questions/73598632/opengl-glfwgetvideomode-causes-seg-fault
    auto monitor = glfwGetPrimaryMonitor();
    auto videoMode = glfwGetVideoMode(monitor);
    auto refreshRate = videoMode->refreshRate;
    cout << "Refresh rate: " << refreshRate << " Hz" << endl;

    auto redBits   = videoMode->redBits;
    auto greenBits = videoMode->greenBits;
    auto blueBits  = videoMode->blueBits;
	cout << "RGB buffer bits                : " << redBits << "\t" << greenBits << "\t" << blueBits << std::endl;

    // TODO: Leaving this for now - it used to work in GLFW2 but causes errors now!
    /*
    int alphaBits = glfwGetWindowAttrib(window, GLFW_ALPHA_BITS);
    int depthBits = glfwGetWindowAttrib(window, GLFW_DEPTH_BITS);
    int stencilBits = glfwGetWindowAttrib(window, GLFW_STENCIL_BITS);
    int accumRedBits = glfwGetWindowAttrib(window, GLFW_ACCUM_RED_BITS);
    int accumGreenBits = glfwGetWindowAttrib(window, GLFW_ACCUM_BLUE_BITS);
    int accumBlueBits = glfwGetWindowAttrib(window, GLFW_ACCUM_GREEN_BITS);
    int accumAlphaBits = glfwGetWindowAttrib(window, GLFW_ACCUM_ALPHA_BITS);
	cout << "Alpha buffer bits              : " << alphaBits << std::endl;
	cout << "Depth buffer bits              : " << depthBits << std::endl;
	cout << "Stencil buffer bits            : " << stencilBits << std::endl;
	cout << "Accumulation buffer bits (RGBA): " << accumRedBits << "\t" << accumGreenBits << "\t" << accumBlueBits << "\t" << accumAlphaBits << endl;    

	// Check anti-aliasing
	// Note: Although this can come back as zero, anti-aliasing samples asked for via window hints can actually still be applied
	int antiAliasingSamples = glfwGetWindowAttrib(window, GLFW_SAMPLES);
	cout << "Anti-Aliasing Samples: " << antiAliasingSamples << endl;
    */

	cout << "----------------------------------" << endl << endl;
}

void Window::updateFpsDetails()
{
    ++framesDuringInterval;
    frameEndTimeSecs = glfwGetTime();
    deltaTime = frameEndTimeSecs - frameStartTimeSecs;

    fpsReportTimer += deltaTime;
    if (fpsReportTimer >= fpsReportIntervalSecs)
    {
        fps = static_cast<double>(framesDuringInterval) / fpsReportTimer;
        framesDuringInterval = 0;
        fpsReportTimer = 0.0;

        if (printFpsToConsole) { cout << "FPS: " << Window::fps << endl; }
    }    
}

// GLFW error callback. Put a breakpoint on the `fprintf` line to help track down any error origins, if required.
void Window::glfwErrorCallback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

