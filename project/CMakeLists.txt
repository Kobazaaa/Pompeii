#--------------------------------------------------
#    CREATE LIBRARY
#--------------------------------------------------
# Source files
set(SOURCES
	# commands
	"${SOURCE_DIR}/commands/CommandBuffer.cpp"
	"${SOURCE_DIR}/commands/CommandPool.cpp"

	# context
	"${SOURCE_DIR}/context/Instance.cpp"
	"${SOURCE_DIR}/context/Device.cpp"
	"${SOURCE_DIR}/context/PhysicalDevice.cpp"

	# core
	"${SOURCE_DIR}/core/Renderer.cpp"

	# datatypes
	"${SOURCE_DIR}/datatypes/EnvironmentMap.cpp"
	"${SOURCE_DIR}/datatypes/Light.cpp"
	"${SOURCE_DIR}/datatypes/Material.cpp"
	"${SOURCE_DIR}/datatypes/Mesh.cpp"
	"${SOURCE_DIR}/datatypes/Shapes.cpp"

	# graphics
	 # graphics/memory
	"${SOURCE_DIR}/graphics/memory/Buffer.cpp"
	"${SOURCE_DIR}/graphics/memory/GBuffer.cpp"
	"${SOURCE_DIR}/graphics/memory/Image.cpp"
	"${SOURCE_DIR}/graphics/memory/Sampler.cpp"
	"${SOURCE_DIR}/graphics/memory/SyncManager.cpp"
	 # graphics/passes
	"${SOURCE_DIR}/graphics/passes/BlitPass.cpp"
	"${SOURCE_DIR}/graphics/passes/DepthPrePass.cpp"
	"${SOURCE_DIR}/graphics/passes/ForwardPass.cpp"
	"${SOURCE_DIR}/graphics/passes/GeometryPass.cpp"
	"${SOURCE_DIR}/graphics/passes/LightingPass.cpp"
	"${SOURCE_DIR}/graphics/passes/ShadowPass.cpp"
	"${SOURCE_DIR}/graphics/passes/UIPass.cpp"
	 # graphics/pipeline
	"${SOURCE_DIR}/graphics/pipeline/DescriptorPool.cpp"
	"${SOURCE_DIR}/graphics/pipeline/DescriptorSet.cpp"
	"${SOURCE_DIR}/graphics/pipeline/FrameBuffer.cpp"
	"${SOURCE_DIR}/graphics/pipeline/Pipeline.cpp"
	"${SOURCE_DIR}/graphics/pipeline/RenderPass.cpp"
	"${SOURCE_DIR}/graphics/pipeline/Shader.cpp"

	# helper
	"${SOURCE_DIR}/helper/Debugger.cpp"
	"${SOURCE_DIR}/helper/DeletionQueue.cpp"

	# presentation
	"${SOURCE_DIR}/presentation/SwapChain.cpp"
)

# Create the library
add_library(${PROJECT_NAME} STATIC ${SOURCES})

# Set include directories
target_include_directories(${PROJECT_NAME} PUBLIC
	"${SOURCE_DIR}/"
	"${SOURCE_DIR}/commands"
	"${SOURCE_DIR}/context"
	"${SOURCE_DIR}/graphics"
	"${SOURCE_DIR}/graphics/memory"
	"${SOURCE_DIR}/graphics/passes"
	"${SOURCE_DIR}/graphics/pipeline"
	"${SOURCE_DIR}/presentation"

	"${SOURCE_DIR}/core"
	"${SOURCE_DIR}/datatypes"
	"${SOURCE_DIR}/helper"
)

# Max Warning Level & Warnings as Errors
target_compile_options(${PROJECT_NAME} INTERFACE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

#--------------------------------------------------
#    LIBRARIES
#--------------------------------------------------
include("../cmake/FetchAndIncludeLibraries.cmake")

#--------------------------------------------------
#    RESOURCES
#--------------------------------------------------
include("../cmake/CompileShaders.cmake")
include("../cmake/CopyFolder.cmake")
copy_folder("${CMAKE_CURRENT_BINARY_DIR}/shaders" "${CMAKE_BINARY_DIR}/project/shaders" copy_directory)
copy_repo_to_folder("https://github.com/Kobazaaa/PompeiiResources.git" "${CMAKE_BINARY_DIR}/project/")
