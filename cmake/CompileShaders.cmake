#--------------------------------------------------
#    SHADER COMPILATION
#--------------------------------------------------

# Check if Vulkan SDK has already been found, if not, shaders cannot be compiled
if(NOT Vulkan_FOUND)
    message(FATAL_ERROR "Vulkan not found yet, cannot compile shaders!")
endif()

# Find the glslc executable
find_program(GLSLC_EXECUTABLE glslc HINTS "${Vulkan_GLSLC_EXECUTABLE}")
if(NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "glslc not found! Make sure Vulkan SDK is installed and available in PATH.")
else()
    message("glslc.exe found at ${GLSLC_EXECUTABLE}")
endif()

# Set shader directories
set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})

# Shader Files
file(GLOB SHADER_SOURCES
  ${SHADER_SOURCE_DIR}/*.vert
  ${SHADER_SOURCE_DIR}/*.frag
  ${SHADER_SOURCE_DIR}/*.comp
)

# Compile the shaders
add_custom_target(CompileShaders ALL COMMENT "Compiling shaders to output directory.")
foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SHADER_OUTPUT_NAME "${SHADER_NAME}.spv")
    set(SHADER_OUTPUT_DIR "${SHADER_BINARY_DIR}/${SHADER_OUTPUT_NAME}")

    message("Compiling ${SHADER_NAME} to ${SHADER_OUTPUT_NAME}")

    add_custom_command(
        TARGET CompileShaders POST_BUILD
        COMMAND ${GLSLC_EXECUTABLE} -g ${SHADER} -o ${SHADER_OUTPUT_DIR} -I${SHADER_SOURCE_DIR}
        DEPENDS ${SHADER}
        COMMENT "Compiling ${SHADER_INPUT_NAME} to ${SHADER_OUTPUT_NAME}.spv"
        VERBATIM
    )
endforeach()

# Add CompileShaders as a dependency to project
add_dependencies(${PROJECT_NAME} CompileShaders)
