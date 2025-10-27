#--------------------------------------------------
#    COPY SOURCE TO DESTINATION
#--------------------------------------------------
function(copy_folder COPY_SRC_DIR COPY_DST_DIR COPY_CMD)

    # Make sure the destination exists!
    file(MAKE_DIRECTORY ${COPY_DST_DIR})

    # Setup target name
    string(RANDOM LENGTH 15 RAND_STRING)
    set(COPY_TARGET "CopyFolder_${RAND_STRING}")

    # Copy the folder
    add_custom_target(${COPY_TARGET} ALL
                    COMMAND ${CMAKE_COMMAND} -E ${COPY_CMD}
                            ${COPY_SRC_DIR} ${COPY_DST_DIR}
                    COMMENT "Copying folder from ${COPY_SRC_DIR} to ${COPY_DST_DIR}."
                    VERBATIM
    )
    # Add COPY_TARGET as a dependency to project
    add_dependencies(${PROJECT_NAME} ${COPY_TARGET})

endfunction()

#--------------------------------------------------
#    COPY REPO TO DESTINATION
#--------------------------------------------------
include(FetchContent)
function(copy_repo_to_folder REPO_URL OUTPUT_DIR)

    FetchContent_Declare(
        repo
        GIT_REPOSITORY ${REPO_URL}
        GIT_TAG main
    )
    FetchContent_Populate(repo)

    copy_folder(${repo_SOURCE_DIR}/ ${OUTPUT_DIR} copy_directory)

endfunction()
