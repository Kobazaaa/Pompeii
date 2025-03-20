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

# Fetch tinyobjloader
FetchContent_Declare(
   tinyobj
   GIT_REPOSITORY https://github.com/tinyobjloader/tinyobjloader.git
   GIT_TAG v1.0.6
   GIT_SHALLOW TRUE
   GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(GLFW GLM stb tinyobj)

# Link libraries to the project
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Vulkan::Vulkan
    glfw
    glm::glm
)
target_include_directories(${PROJECT_NAME} PRIVATE ${stb_SOURCE_DIR} ${tinyobj_SOURCE_DIR})
