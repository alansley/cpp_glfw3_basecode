#ifndef IMGUI_DEMO_SCENE_H
#define IMGUI_DEMO_SCENE_H

#include "imgui.h"
#include "backends/imgui_impl_opengl3.h"

class ImGuiDemoScene
{
public:
    // State
    inline static bool show_demo_window = true;
    inline static bool show_another_window = false;
    inline static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    static void draw()
    {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window) { ImGui::ShowDemoWindow(&show_demo_window); }

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                           // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");                // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);       // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);             // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit4("clear color", (float*)&clear_color);  // Edit 3 floats representing a color

            if (ImGui::Button("Button")) { counter++; }              // Buttons return true when clicked (most widgets return true when edited/activated)
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();        
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
};
	
#endif // IMGUI_DEMO_SCENE_H
