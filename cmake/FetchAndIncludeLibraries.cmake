find_package(Vulkan REQUIRED)
if(Vulkan_FOUND)
    MESSAGE("Vulkan Found!")
endif()

include(FetchContent)

# Fetch GLFW
FetchContent_Declare(
   GLFW
   GIT_REPOSITORY https://github.com/glfw/glfw.git
   GIT_TAG 3.4
   GIT_SHALLOW TRUE
   GIT_PROGRESS TRUE
)

# Fetch GLM
FetchContent_Declare(
   GLM
   GIT_REPOSITORY https://github.com/g-truc/glm.git
   GIT_TAG 1.0.1
   GIT_SHALLOW TRUE
   GIT_PROGRESS TRUE
)

# Fetch stb
FetchContent_Declare(
   stb
   GIT_REPOSITORY https://github.com/nothings/stb.git
   GIT_TAG master
   GIT_SHALLOW TRUE
   GIT_PROGRESS TRUE
)

# Fetch assimp
FetchContent_Declare(
   assimp
   GIT_REPOSITORY https://github.com/assimp/assimp.git
   GIT_TAG v5.4.3
   GIT_SHALLOW TRUE
   GIT_PROGRESS TRUE
)

# Fetch VMA
FetchContent_Declare(
   vma
   GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
   GIT_TAG v3.2.1
   GIT_SHALLOW TRUE
   GIT_PROGRESS TRUE
)

# Fetch ImGui
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.92.1-docking
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
# Fetch ImGui FileDialog
FetchContent_Declare(
    igfd
    GIT_REPOSITORY https://github.com/aiekick/ImGuiFileDialog.git
    GIT_TAG v0.6.7
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(GLFW GLM stb assimp vma imgui igfd)

# Add the ImGui sources to project
target_sources(${PROJECT_NAME} PRIVATE
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp
  ${igfd_SOURCE_DIR}/ImGuiFileDialog.cpp
)
# don't treat warnings as errors for ImGuiFileDialog
set_source_files_properties(${igfd_SOURCE_DIR}/ImGuiFileDialog.cpp PROPERTIES COMPILE_OPTIONS "-WX-")

# Link libraries to the project
target_link_libraries(${PROJECT_NAME} PRIVATE
    Vulkan::Vulkan
    glfw
    glm::glm
    assimp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${stb_SOURCE_DIR}
    ${assimp_SOURCE_DIR}/include
    ${vma_SOURCE_DIR}/include
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${igfd_SOURCE_DIR}
)
