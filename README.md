# cpp_glfw3_basecode
Cross-platform C++ GLFW3 Basecode with [GLAD](https://glad.dav1d.de/), [GLM](https://github.com/g-truc/glm), [STB](https://github.com/nothings/stb), and [Dear ImGui](https://github.com/ocornut/imgui) pre-integrated for rapid OpenGL experimentation.

In terms of custom functionality, there is also:
- A `Window` class with a mouse/keyboard and window event handlers and a custom 3D camera to look and move around,
- A `ShaderProgram` class to easily load & combine vertex/fragment shaders as well as tesselation shaders,
- Simple `Point`, `Line`, `Quad` and `TexturedQuad` classes for basic drawing, as well as a `Grid` class for orientation, and
- A basic WaveFront .OBJ 3D model loader.

This project comes configured to build in Visual Studio 2022 on the Windows platform and via a Code::Blocks project on Linux.

This project is provided under a MIT license (see the top-level LICENSE file), however the integrated libraries have their own licenses - see the `cpp_glfw3_basecode/libs` folder and its contained library subfolders for further details.

TODO:
- Run a clean-up sweep through every file in the codebase,
- Create a YouTube video demonstration and link it in this readme,
- Create additional demo scenes with further functionality (i.e., use the tessalation shaders, scissor testing, geometry shaders, bump mapping, displacement mapping, stochastic sampling etc.),
- Integrate a better 3D model loader rather than the simple .OBJ file loader I wrote.
