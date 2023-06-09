# C++ GLFW3 Basecode
Cross-platform C++ GLFW3 Basecode with [GLAD](https://glad.dav1d.de/), [GLM](https://github.com/g-truc/glm), [STB](https://github.com/nothings/stb), and [Dear ImGui](https://github.com/ocornut/imgui) pre-integrated for rapid OpenGL prototyping and experimentation.

[Main](https://github.com/alansley/cpp_glfw3_basecode/blob/main/cpp_glfw3_basecode/Main.cpp) is kept as small as possible so you can add as much code there as you want if you're just experimenting, or alternatively you might want to add your code to a 'demo-scene'-type class ([examples](https://github.com/alansley/cpp_glfw3_basecode/blob/main/cpp_glfw3_basecode/demo_scenes/OpenGLDemoScene.hpp) [provided](https://github.com/alansley/cpp_glfw3_basecode/blob/main/cpp_glfw3_basecode/demo_scenes/ImGuiDemoScene.hpp)) as a quick and easy way to separate out your new functionality.

![Screenshot of cpp_glf3_basecode](https://github.com/alansley/cpp_glfw3_basecode/blob/main/Screenshot_2023-04-16.png?raw=true)

In terms of custom functionality, this basecode also provides:
- A `Window` class with a mouse/keyboard and window event handlers and a custom 3D camera to look and move around,
- A `ShaderProgram` class to easily load & combine vertex/fragment shaders as well as tesselation shaders,
- Simple `Point`, `Line`, `Quad` and `TexturedQuad` classes for basic drawing, as well as a `Grid` class for orientation, 
- Selectable Hor+ and Vert- Field of View (FoV) handling, and
- A basic WaveFront .OBJ 3D model loader.

If you make any neat demo scenes or adjustments and would like to see them included please feel free to raise a [pull request](https://github.com/alansley/cpp_glfw3_basecode/pulls), or if you have any problems with the code you're welcome to raise an [issue](https://github.com/alansley/cpp_glfw3_basecode/issues).

## Build
This project comes configured to build in Visual Studio 2022 on the Windows platform and via a Code::Blocks project in Linux.

Simply download a zip of the code & extract it or clone the repo, then:
- For **Windows**: Open the `VisualStudio2022_Windows\cpp_glfw3_basecode.sln` file,

or

- For **Linux**: Open the `CodeBlocks_Linux/cpp_glfw3_basecode.cbp` file.

## License
This project is provided under a MIT license (see the top-level LICENSE file), however the integrated libraries have their own licenses - see the `libs` folder and its contained library subfolders for further details.

## TODO:
- Run a clean-up sweep through every file in the codebase fixing up any warnings and applying linting/best-practice where appropriate,
- Modify Point/Line/Quad/TexturedQuad classes to provide both 2D and 3D versions,
- Create a YouTube video demonstration and link it in this readme,
- Create additional demo scenes with further functionality (i.e., screen-space effect shaders, use the tessalation shaders, scissor testing, geometry shaders, bump / displacement mapping, stochastic sampling etc.),
- Integrate a better 3D model loader rather than the simple .OBJ file loader I wrote, (probably use TinyObjLoader - https://github.com/tinyobjloader) and
- Create a Code::Blocks project for Windows (the Linux one now exists, but there's no harm in adding a Windows version for those who want something lighter than VS2022).

## MAYBE:
- Add a Light struct and adjust the ambient/diffuse/specular shaders to use it so you can pass in an array of lights? (Possibly no need - TinyObjLoader takes lights as part of the OBJs)
