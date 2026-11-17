cmake_minimum_required(VERSION 3.27)
project(Pompeii LANGUAGES CXX)

option(POMPEII_WARNINGS_AS_ERRORS "Treat warnings as errors in Pompeii" ON)

# Third party libs
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/FetchAndIncludeLibraries.cmake")

# Library
file(GLOB_RECURSE POMPEII_SOURCES CONFIGURE_DEPENDS	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
add_library(pompeii STATIC ${POMPEII_SOURCES})
add_library(pompeii::pompeii ALIAS pompeii)

target_compile_features(pompeii PUBLIC cxx_std_20)
set_target_properties(pompeii PROPERTIES CXX_EXTENSIONS OFF)

target_include_directories(pompeii
	PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(pompeii
	PUBLIC  glm::glm
	PRIVATE Vulkan::Vulkan
)

target_include_directories(pompeii SYSTEM PRIVATE
	${vma_SOURCE_DIR}/include
	${stb_SOURCE_DIR}
)

# Warnings as errors
if(MSVC)
	set(POMPEII_WARNINGS /W4)
	if(POMPEII_WARNINGS_AS_ERRORS)
		list(APPEND POMPEII_WARNINGS /WX)
	endif()
else()
	set(POMPEII_WARNINGS -Wall -Wextra -Wpedantic)
	if(POMPEII_WARNINGS_AS_ERRORS)
		list(APPEND POMPEII_WARNINGS -Werror)
	endif()
endif()
target_compile_options(pompeii PRIVATE ${POMPEII_WARNINGS})

# Shaders
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompileShaders.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/CopyFolder.cmake")
copy_folder("${CMAKE_CURRENT_BINARY_DIR}/shaders" "${CMAKE_BINARY_DIR}/shaders" copy_directory)
copy_repo_to_folder("https://github.com/Kobazaaa/PompeiiResources.git" "${CMAKE_BINARY_DIR}/")