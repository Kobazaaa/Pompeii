#--------------------------------------------------
#    SET VARIABLES
#--------------------------------------------------
set(SOURCE_DIR	"${CMAKE_SOURCE_DIR}/project/src")
set(TEXTURE_DIR "${CMAKE_SOURCE_DIR}/project/textures")
set(MODEL_DIR	"${CMAKE_SOURCE_DIR}/project/models")
set(SHADER_DIR	"${CMAKE_SOURCE_DIR}/project/shaders")



#--------------------------------------------------
#    CREATE EXECUTABLE
#--------------------------------------------------
# Source files
set(SOURCES
	# main
	"${SOURCE_DIR}/main.cpp"

	# commands
	"${SOURCE_DIR}/commands/CommandBuffer.h"				"${SOURCE_DIR}/commands/CommandBuffer.cpp"
	"${SOURCE_DIR}/commands/CommandPool.h"					"${SOURCE_DIR}/commands/CommandPool.cpp"

	# context
	"${SOURCE_DIR}/context/Context.h"
	"${SOURCE_DIR}/context/Instance.h"						"${SOURCE_DIR}/context/Instance.cpp"
	"${SOURCE_DIR}/context/Device.h"						"${SOURCE_DIR}/context/Device.cpp"
	"${SOURCE_DIR}/context/PhysicalDevice.h"				"${SOURCE_DIR}/context/PhysicalDevice.cpp"

	# core
	"${SOURCE_DIR}/core/Renderer.h"							"${SOURCE_DIR}/core/Renderer.cpp"
	"${SOURCE_DIR}/core/Window.h"							"${SOURCE_DIR}/core/Window.cpp"

	# graphics
	 # graphics/memory
	"${SOURCE_DIR}/graphics/memory/Buffer.h"				"${SOURCE_DIR}/graphics/memory/Buffer.cpp"
	"${SOURCE_DIR}/graphics/memory/GBuffer.h"				"${SOURCE_DIR}/graphics/memory/GBuffer.cpp"
	"${SOURCE_DIR}/graphics/memory/Image.h"					"${SOURCE_DIR}/graphics/memory/Image.cpp"
	"${SOURCE_DIR}/graphics/memory/Sampler.h"				"${SOURCE_DIR}/graphics/memory/Sampler.cpp"
	"${SOURCE_DIR}/graphics/memory/SyncManager.h"			"${SOURCE_DIR}/graphics/memory/SyncManager.cpp"
	 # graphics/passes
	"${SOURCE_DIR}/graphics/passes/DepthPrePass.h"			"${SOURCE_DIR}/graphics/passes/DepthPrePass.cpp"
	"${SOURCE_DIR}/graphics/passes/ForwardPass.h"			"${SOURCE_DIR}/graphics/passes/ForwardPass.cpp"
	"${SOURCE_DIR}/graphics/passes/GeometryPass.h"			"${SOURCE_DIR}/graphics/passes/GeometryPass.cpp"
	"${SOURCE_DIR}/graphics/passes/ShadowPass.h"			"${SOURCE_DIR}/graphics/passes/ShadowPass.cpp"
	 # graphics/pipeline
	"${SOURCE_DIR}/graphics/pipeline/DescriptorPool.h"		"${SOURCE_DIR}/graphics/pipeline/DescriptorPool.cpp"
	"${SOURCE_DIR}/graphics/pipeline/DescriptorSet.h"		"${SOURCE_DIR}/graphics/pipeline/DescriptorSet.cpp"
	"${SOURCE_DIR}/graphics/pipeline/FrameBuffer.h"			"${SOURCE_DIR}/graphics/pipeline/FrameBuffer.cpp"
	"${SOURCE_DIR}/graphics/pipeline/GraphicsPipeline.h"	"${SOURCE_DIR}/graphics/pipeline/GraphicsPipeline.cpp"
	"${SOURCE_DIR}/graphics/pipeline/RenderPass.h"			"${SOURCE_DIR}/graphics/pipeline/RenderPass.cpp"
	"${SOURCE_DIR}/graphics/pipeline/Shader.h"				"${SOURCE_DIR}/graphics/pipeline/Shader.cpp"

	# helper
	"${SOURCE_DIR}/helper/ConsoleTextSettings.h"
	"${SOURCE_DIR}/helper/Debugger.h"						"${SOURCE_DIR}/helper/Debugger.cpp"
	"${SOURCE_DIR}/helper/DeletionQueue.h"					"${SOURCE_DIR}/helper/DeletionQueue.cpp"
	"${SOURCE_DIR}/helper/Timer.h"							"${SOURCE_DIR}/helper/Timer.cpp"

	# presentation
	"${SOURCE_DIR}/presentation/SwapChain.h"				"${SOURCE_DIR}/presentation/SwapChain.cpp"

	# scene
	"${SOURCE_DIR}/scene/Camera.h"							"${SOURCE_DIR}/scene/Camera.cpp"
	"${SOURCE_DIR}/scene/DirectionalLight.h"				"${SOURCE_DIR}/scene/DirectionalLight.cpp"
	"${SOURCE_DIR}/scene/Material.h"						"${SOURCE_DIR}/scene/Material.cpp"
	"${SOURCE_DIR}/scene/Model.h"							"${SOURCE_DIR}/scene/Model.cpp"
	"${SOURCE_DIR}/scene/Scene.h"							"${SOURCE_DIR}/scene/Scene.cpp"

)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Set include directories
target_include_directories(${PROJECT_NAME} PRIVATE
	"${SOURCE_DIR}"
	"${SOURCE_DIR}/commands"
	"${SOURCE_DIR}/context"
	"${SOURCE_DIR}/core"
	"${SOURCE_DIR}/graphics"
	"${SOURCE_DIR}/graphics/memory"
	"${SOURCE_DIR}/graphics/passes"
	"${SOURCE_DIR}/graphics/pipeline"
	"${SOURCE_DIR}/helper"
	"${SOURCE_DIR}/presentation"
	"${SOURCE_DIR}/scene"
)

# Max Warning Level & Warnings as Errors
target_compile_options(${PROJECT_NAME} PRIVATE
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
include("../cmake/CompileShaders.cmake")				# Shaders

include("../cmake/CopyFolder.cmake")
copy_repo_to_exe("https://github.com/Kobazaaa/PompeiiResources.git")
