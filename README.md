# 🌋 Pompeii

**Pompeii** is a Vulkan-based Graphics Renderer written in C++, by Kobe Dereyne.
Pompeii is a learning project to explore the Vulkan API
during the Graphics Programming 2 course at Howest University - Digital Arts & Entertainment.
<br>
<br>
Link to repository: [GitHub Repo](https://github.com/Kobazaaa/Pompeii.git)

---

## 🍴 Resource Fetching

Resources like models and textures are being fetched from another repo.
<br>
Link to repository: [GitHub Repo](https://github.com/Kobazaaa/PompeiiResources.git)

---

## 🎮 Controls

### Movement
- **W**: Move forward
- **A**: Move left
- **S**: Move backward
- **D**: Move right
- **E**: Move (world) up
- **Q**: Move (world) down

### Look Around
- **LMB + DRAG**: Look around


### Window
- **F11**: Toggle Fullscreen
- **ESC**: Exit Fullscreen

---

## 📸 Screenshots

Sponza Scene with Transparent Objects (no mipmapping):
![Sponza Scene with Transparent Object](./images/SponzaScene.png)

Sponza Scene Hallway (mipmapping):
![Sponza Scene with Transparent Object 2](./images/SponzaScene2.png)

GBuffer Albedo Opacity:
![GBuffer Albedo Opacity](./images/GBuffer_-_Albedo_Opacity.png)

One side of a point light's Depth Buffer:
![Light_Depth_Map__2_](./images/Light_Depth_Map__2_.png)

HDR Output:
![HDR Output](./images/Render_Target.png)

Final Result:
![Final Result](./images/Swapchain_Image_0.png)

---

## 📁 Folder Structure

<pre>
Pompeii/ 
├── cmake/			# CMake Scripts.
├── images/			# images for the README.md, such as screenshots.
├── project/			# project code and assets.
|	├── shaders/
|	├── src/		# source code.
|	|    ├── commands/
|	|    ├── context/
|	|    ├── core/
|	|    ├── graphics/
|	|    |       ├── memory/
|	|    |       ├── passes/
|	|    |       └── pipeline/
|	|    ├── helper/
|	|    ├── presentation/
|	|    └── scene/
|	└── CmakeLists.txt	# Build configuration project.
├── .gitattributes		
├── .gitignore			
├── CMakeLists.txt		# Build configuration global.
├── CMakeSettings.json
├── LICENSE.txt
└── README.md			# ⭕ You are here :)
</pre>

---

## 🙏 Resources

Graphics Programming Teachers at Howest University - Digital Arts & Entertainment <br>
The Vulkan Tutorial at [vulkan-tutorial.com](https://vulkan-tutorial.com) <br>
Morgan McGuire's [Computer Graphics Archive](https://casual-effects.com/data) <br>
Khronos Groups's [glTF Sample Assets](https://github.com/KhronosGroup/glTF-Sample-Assets/tree/main) <br>
OpenGL's Tutorials at [LearnOpenGL](https://learnopengl.com) <br>
Extra repository storing all the models and textures used are at [Pompeii Resources Repository](https://github.com/Kobazaaa/PompeiiResources.git)



---

## 📚 Third-Party Libraries

This project uses the following third-party libraries:

### Vulkan

| Library                                | Purpose          |
|----------------------------------------|------------------|
| [Vulkan API](https://vulkan.lunarg.com) | Vulkan |
| [Vulkan Memory Allocator (VMA)](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git) | Vulkan Memory |

### Other

| Library                                | Purpose          |
|----------------------------------------|------------------|
| [GLFW](https://github.com/glfw/glfw.git) | Window & Input |
| [glm](https://github.com/g-truc/glm.git) | Math Library |
| [stb](https://github.com/nothings/stb.git) | Texture Loading |
| [Open Asset Import Library (Assimp)](https://github.com/assimp/assimp.git) | Model Loading |
