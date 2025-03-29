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
	"${SOURCE_DIR}/main.cpp"

	"${SOURCE_DIR}/Window.cpp"				"${SOURCE_DIR}/Window.h"
	"${SOURCE_DIR}/Instance.cpp"			"${SOURCE_DIR}/Instance.h"
	"${SOURCE_DIR}/Debugger.cpp"			"${SOURCE_DIR}/Debugger.h"
	"${SOURCE_DIR}/PhysicalDevice.cpp"		"${SOURCE_DIR}/PhysicalDevice.h"
)

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})



#--------------------------------------------------
#    LIBRARIES
#--------------------------------------------------
include("../cmake/FetchAndIncludeLibraries.cmake")



#--------------------------------------------------
#    RESOURCES
#--------------------------------------------------
include("../cmake/CompileShaders.cmake")				# Shaders

include("../cmake/CopyFolder.cmake")
copy_folder_to_exe("${TEXTURE_DIR}" copy_directory)		# Textures
copy_folder_to_exe("${MODEL_DIR}" copy_directory)		# Models
