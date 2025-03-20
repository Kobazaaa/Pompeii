#--------------------------------------------------
#    COPY TEXTURES TO OUTPUT
#--------------------------------------------------

# Set texture directories
set(TEXTURES_SOURCE_DIR ${CMAKE_SOURCE_DIR}/textures)
set(TEXTURES_BINARY_DIR ${CMAKE_BINARY_DIR}/textures)
file(MAKE_DIRECTORY ${TEXTURES_BINARY_DIR})

# Texture Files
file(GLOB TEXTURES_SOURCES
  ${TEXTURES_SOURCE_DIR}/*.png
  ${TEXTURES_SOURCE_DIR}/*.jpeg
  ${TEXTURES_SOURCE_DIR}/*.jpg
  ${TEXTURES_SOURCE_DIR}/*.bmp
)

# Copy the textures
add_custom_target(CopyTextures ALL
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                        ${TEXTURES_SOURCE_DIR} ${TEXTURES_BINARY_DIR}
                COMMENT "Copying textures to output directory."
)
# Add CopyTextures as a dependency to project
add_dependencies(${PROJECT_NAME} CopyTextures)
