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

FetchContent_MakeAvailable(GLFW GLM)

# Link libraries to the project
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Vulkan::Vulkan
    glfw
    glm::glm
)
