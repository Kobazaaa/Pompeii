#--------------------------------------------------
#    CREATE EXECUTABLE
#--------------------------------------------------
# Source files
set(SOURCES
	"./src/main.cpp")

# Create the executable
add_executable(${PROJECT_NAME} ${SOURCES})

#--------------------------------------------------
#    LIBRARIES
#--------------------------------------------------
include("../cmake/FetchAndIncludeLibraries.cmake")

#--------------------------------------------------
#    SHADERS
#--------------------------------------------------
include("../cmake/CompileShaders.cmake")
