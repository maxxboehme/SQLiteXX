
ExternalProject_Add(
    Catch
    # DEPENDS
    TMP_DIR           ${CMAKE_BINARY_DIR}/temp
    STAMP_DIR         ${CMAKE_BINARY_DIR}/stamp
    #--Download step--------------
    DOWNLOAD_DIR      ${download_dir}/Catch
    GIT_REPOSITORY    https://www.github.com/philsquared/Catch.git
    GIT_TAG           v1.7.0
    #--Update/Patch step----------
    UPDATE_COMMAND    ""
    #--Configure step-------------
    SOURCE_DIR        ${CMAKE_CURRENT_BINARY_DIR}/Catch
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX:PATH=${DEPENDENCY_INSTALL_PREFIX}
                      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    #--Build step-----------------
    BINARY_DIR        ${CMAKE_BINARY_DIR}/Catch
    BUILD_COMMAND     ${CMAKE_COMMAND} --build .
    #--Install step----------------
    INSTALL_DIR       ${DEPENDENCY_INSTALL_PREFIX}
)

ExternalProject_Get_Property(Catch install_dir)
set(CATCH_INCLUDE_DIR ${install_dir}/include CACHE INTERNAL "Path to include folder for Catch")
