#--------------------------------------------------
#    SET VARIABLES
#--------------------------------------------------
set(SOURCE_DIR			"${CMAKE_SOURCE_DIR}/project/src")
set(ENGINE_SOURCE_DIR	"${CMAKE_SOURCE_DIR}/project/src/ENGINE")
set(RENDERER_SOURCE_DIR	"${CMAKE_SOURCE_DIR}/project/src/RENDERER")
set(EDITOR_SOURCE_DIR	"${CMAKE_SOURCE_DIR}/project/src/EDITOR")
set(SHADER_DIR			"${CMAKE_SOURCE_DIR}/project/shaders")

#--------------------------------------------------
#    CREATE EXECUTABLE
#--------------------------------------------------
# Source files
set(SOURCES
	############
	#  SHARED  #
	############
	# main
	"${SOURCE_DIR}/main.cpp"

	############
	# RENDERER #
	############
	# commands
	"${RENDERER_SOURCE_DIR}/commands/CommandBuffer.h"				"${RENDERER_SOURCE_DIR}/commands/CommandBuffer.cpp"
	"${RENDERER_SOURCE_DIR}/commands/CommandPool.h"					"${RENDERER_SOURCE_DIR}/commands/CommandPool.cpp"

	# context
	"${RENDERER_SOURCE_DIR}/context/Context.h"
	"${RENDERER_SOURCE_DIR}/context/Instance.h"						"${RENDERER_SOURCE_DIR}/context/Instance.cpp"
	"${RENDERER_SOURCE_DIR}/context/Device.h"						"${RENDERER_SOURCE_DIR}/context/Device.cpp"
	"${RENDERER_SOURCE_DIR}/context/PhysicalDevice.h"				"${RENDERER_SOURCE_DIR}/context/PhysicalDevice.cpp"

	# core
	"${RENDERER_SOURCE_DIR}/core/Renderer.h"						"${RENDERER_SOURCE_DIR}/core/Renderer.cpp"
	"${RENDERER_SOURCE_DIR}/core/Window.h"							"${RENDERER_SOURCE_DIR}/core/Window.cpp"

	# datatypes
	"${RENDERER_SOURCE_DIR}/datatypes/EnvironmentMap.h"				"${RENDERER_SOURCE_DIR}/datatypes/EnvironmentMap.cpp"
	"${RENDERER_SOURCE_DIR}/datatypes/GPUCamera.h"
	"${RENDERER_SOURCE_DIR}/datatypes/Light.h"						"${RENDERER_SOURCE_DIR}/datatypes/Light.cpp"
	"${RENDERER_SOURCE_DIR}/datatypes/Material.h"					"${RENDERER_SOURCE_DIR}/datatypes/Material.cpp"
	"${RENDERER_SOURCE_DIR}/datatypes/Mesh.h"						"${RENDERER_SOURCE_DIR}/datatypes/Mesh.cpp"
	"${RENDERER_SOURCE_DIR}/datatypes/RenderingItems.h"
	"${RENDERER_SOURCE_DIR}/datatypes/Shapes.h"						"${RENDERER_SOURCE_DIR}/datatypes/Shapes.cpp"

	# graphics
	 # graphics/memory
	"${RENDERER_SOURCE_DIR}/graphics/memory/Buffer.h"				"${RENDERER_SOURCE_DIR}/graphics/memory/Buffer.cpp"
	"${RENDERER_SOURCE_DIR}/graphics/memory/GBuffer.h"				"${RENDERER_SOURCE_DIR}/graphics/memory/GBuffer.cpp"
	"${RENDERER_SOURCE_DIR}/graphics/memory/Image.h"				"${RENDERER_SOURCE_DIR}/graphics/memory/Image.cpp"
	"${RENDERER_SOURCE_DIR}/graphics/memory/Sampler.h"				"${RENDERER_SOURCE_DIR}/graphics/memory/Sampler.cpp"
	"${RENDERER_SOURCE_DIR}/graphics/memory/SyncManager.h"			"${RENDERER_SOURCE_DIR}/graphics/memory/SyncManager.cpp"
	 # graphics/passes
	"${RENDERER_SOURCE_DIR}/graphics/passes/BlitPass.h"				"${RENDERER_SOURCE_DIR}/graphics/passes/BlitPass.cpp"
	"${RENDERER_SOURCE_DIR}/graphics/passes/DepthPrePass.h"			"${RENDERER_SOURCE_DIR}/graphics/passes/DepthPrePass.cpp"
	"${RENDERER_SOURCE_DIR}/graphics/passes/ForwardPass.h"			"${RENDERER_SOURCE_DIR}/graphics/passes/ForwardPass.cpp"
	"${RENDERER_SOURCE_DIR}/graphics/passes/GeometryPass.h"			"${RENDERER_SOURCE_DIR}/graphics/passes/GeometryPass.cpp"
	"${RENDERER_SOURCE_DIR}/graphics/passes/LightingPass.h"			"${RENDERER_SOURCE_DIR}/graphics/passes/LightingPass.cpp"
	"${RENDERER_SOURCE_DIR}/graphics/passes/ShadowPass.h"			"${RENDERER_SOURCE_DIR}/graphics/passes/ShadowPass.cpp"
	"${RENDERER_SOURCE_DIR}/graphics/passes/UIPass.h"				"${RENDERER_SOURCE_DIR}/graphics/passes/UIPass.cpp"
	 # graphics/pipeline
	"${RENDERER_SOURCE_DIR}/graphics/pipeline/DescriptorPool.h"		"${RENDERER_SOURCE_DIR}/graphics/pipeline/DescriptorPool.cpp"
	"${RENDERER_SOURCE_DIR}/graphics/pipeline/DescriptorSet.h"		"${RENDERER_SOURCE_DIR}/graphics/pipeline/DescriptorSet.cpp"
	"${RENDERER_SOURCE_DIR}/graphics/pipeline/FrameBuffer.h"		"${RENDERER_SOURCE_DIR}/graphics/pipeline/FrameBuffer.cpp"
	"${RENDERER_SOURCE_DIR}/graphics/pipeline/Pipeline.h"			"${RENDERER_SOURCE_DIR}/graphics/pipeline/Pipeline.cpp"
	"${RENDERER_SOURCE_DIR}/graphics/pipeline/RenderPass.h"			"${RENDERER_SOURCE_DIR}/graphics/pipeline/RenderPass.cpp"
	"${RENDERER_SOURCE_DIR}/graphics/pipeline/Shader.h"				"${RENDERER_SOURCE_DIR}/graphics/pipeline/Shader.cpp"

	# helper
	"${RENDERER_SOURCE_DIR}/helper/Debugger.h"						"${RENDERER_SOURCE_DIR}/helper/Debugger.cpp"

	# presentation
	"${RENDERER_SOURCE_DIR}/presentation/SwapChain.h"				"${RENDERER_SOURCE_DIR}/presentation/SwapChain.cpp"

	############
	#  ENGINE  #
	############
	# helper
	"${ENGINE_SOURCE_DIR}/helper/ConsoleTextSettings.h"
	"${ENGINE_SOURCE_DIR}/helper/DeletionQueue.h"					"${ENGINE_SOURCE_DIR}/helper/DeletionQueue.cpp"
	"${ENGINE_SOURCE_DIR}/helper/ServiceLocator.h"
	"${ENGINE_SOURCE_DIR}/helper/Timer.h"							"${ENGINE_SOURCE_DIR}/helper/Timer.cpp"

	# managers
	"${ENGINE_SOURCE_DIR}/managers/AssetManager.h"					"${ENGINE_SOURCE_DIR}/managers/AssetManager.cpp"
	
	# scene
	"${ENGINE_SOURCE_DIR}/scene/Scene.h"							"${ENGINE_SOURCE_DIR}/scene/Scene.cpp"
	 # scene/base
	"${ENGINE_SOURCE_DIR}/scene/base/SceneManager.h"				"${ENGINE_SOURCE_DIR}/scene/base/SceneManager.cpp"
	 # scene/components
	"${ENGINE_SOURCE_DIR}/scene/components/Camera.h"				"${ENGINE_SOURCE_DIR}/scene/components/Camera.cpp"
	"${ENGINE_SOURCE_DIR}/scene/components/Component.h"				"${ENGINE_SOURCE_DIR}/scene/components/Component.cpp"
	"${ENGINE_SOURCE_DIR}/scene/components/LightComponent.h"		"${ENGINE_SOURCE_DIR}/scene/components/LightComponent.cpp"
	"${ENGINE_SOURCE_DIR}/scene/components/MeshFilter.h"			"${ENGINE_SOURCE_DIR}/scene/components/MeshFilter.cpp"
	"${ENGINE_SOURCE_DIR}/scene/components/MeshRenderer.h"			"${ENGINE_SOURCE_DIR}/scene/components/MeshRenderer.cpp"
	 # scene/sceneobject
	"${ENGINE_SOURCE_DIR}/scene/sceneobject/SceneObject.h"			"${ENGINE_SOURCE_DIR}/scene/sceneobject/SceneObject.cpp"
	"${ENGINE_SOURCE_DIR}/scene/sceneobject/Transform.h"			"${ENGINE_SOURCE_DIR}/scene/sceneobject/Transform.cpp"

	# systems
	"${ENGINE_SOURCE_DIR}/systems/ISystem.h"
	"${ENGINE_SOURCE_DIR}/systems/LightingSystem.h"					"${ENGINE_SOURCE_DIR}/systems/LightingSystem.cpp"
	"${ENGINE_SOURCE_DIR}/systems/RenderSystem.h"					"${ENGINE_SOURCE_DIR}/systems/RenderSystem.cpp"

	############
	#  EDITOR  #
	############
	"${EDITOR_SOURCE_DIR}/Editor.h"									"${EDITOR_SOURCE_DIR}/Editor.cpp"
	"${EDITOR_SOURCE_DIR}/EditorUI.h"								"${EDITOR_SOURCE_DIR}/EditorUI.cpp"
)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Set include directories
target_include_directories(${PROJECT_NAME} PRIVATE
	"${SOURCE_DIR}"

	"${RENDERER_SOURCE_DIR}"
	"${RENDERER_SOURCE_DIR}/commands"
	"${RENDERER_SOURCE_DIR}/context"
	"${RENDERER_SOURCE_DIR}/core"
	"${RENDERER_SOURCE_DIR}/datatypes"
	"${RENDERER_SOURCE_DIR}/graphics"
	"${RENDERER_SOURCE_DIR}/graphics/memory"
	"${RENDERER_SOURCE_DIR}/graphics/passes"
	"${RENDERER_SOURCE_DIR}/graphics/pipeline"
	"${RENDERER_SOURCE_DIR}/helper"
	"${RENDERER_SOURCE_DIR}/presentation"

	"${ENGINE_SOURCE_DIR}"
	"${ENGINE_SOURCE_DIR}/helper"
	"${ENGINE_SOURCE_DIR}/managers"
	"${ENGINE_SOURCE_DIR}/scene"
	"${ENGINE_SOURCE_DIR}/scene/base"
	"${ENGINE_SOURCE_DIR}/scene/components"
	"${ENGINE_SOURCE_DIR}/scene/sceneobject"
	"${ENGINE_SOURCE_DIR}/systems"

	"${EDITOR_SOURCE_DIR}"
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
include("../cmake/CompileShaders.cmake")
include("../cmake/CopyFolder.cmake")
copy_repo_to_exe("https://github.com/Kobazaaa/PompeiiResources.git")
