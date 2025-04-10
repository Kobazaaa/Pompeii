# 🌋 Pompeii

**Pompeii** is a Vulkan-based Graphics Renderer written in C++.
Pompeii is a learning project to explore the Vulkan API
during the Graphics Programming 2 course at Howest University - Digital Arts & Entertainment

---

## 🌟 Features

### Vulkan Abstraction

I abstracted the Vulkan API, making classes (and builders) for most Vulkan Objects, such as:

- VkInstance
- VkCommandPool
- VkBuffer
- VkSwapchainKHR
- VkPipeline
- ...


### Rendering Opaque & Transparent Meshes

Two Vulkan pipelines have been set up: one for rendering opaque meshes and another for rendering transparent meshes.

When rendering a model or scene:

1. The vertex and index buffers are bound.
2. The opaque pipeline is bound, and all opaque meshes are drawn.
3. After rendering the opaque meshes, the transparent pipeline is bound, and the transparent meshes are drawn.


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

![Sponza Scene with Transparent Object](./images/SponzaScene.png)

---

## 📁 Folder Structure

<pre>
Pompeii/ 
├── cmake/			# CMake Scripts.
├── images/			# images for the README.md, such as screenshots.
└── project/			# project code and assets.
	├── models/
	├── shaders/
	├── src/		# source code.
	├── textures/
	└── CmakeLists.txt	# Build configuration project.
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
