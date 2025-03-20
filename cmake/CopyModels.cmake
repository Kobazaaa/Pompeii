#--------------------------------------------------
#    COPY MODELS TO OUTPUT
#--------------------------------------------------

# Set models directories
set(MODELS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/models)
set(MODELS_BINARY_DIR ${CMAKE_BINARY_DIR}/models)
file(MAKE_DIRECTORY ${MODELS_BINARY_DIR})

# Model Files
file(GLOB MODELS_SOURCES
  ${MODELS_SOURCE_DIR}/*.obj
)

# Copy the models
add_custom_target(CopyModels ALL
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                        ${MODELS_SOURCE_DIR} ${MODELS_BINARY_DIR}
                COMMENT "Copying models to output directory."
)
# Add CopyModels as a dependency to project
add_dependencies(${PROJECT_NAME} CopyModels)
