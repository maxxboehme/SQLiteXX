file(WRITE ${CMAKE_BINARY_DIR}/temp/CMakeLists.txt
   "cmake_minimum_required(VERSION 3.2)\n"
   "project(sqlite3 C)\n"
   "\n"
   "find_package(Threads REQUIRED)\n"
   "\n"
   "add_library(sqlite3 STATIC sqlite3.c sqlite3.h sqlite3ext.h)\n"
   "target_link_libraries(sqlite3 \${CMAKE_THREAD_LIBS_INIT} \${CMAKE_DL_LIBS})\n"
   "set_target_properties(sqlite3 PROPERTIES OUTPUT_NAME sqlite3)\n"
   "\n"
   "add_executable(sqlite sqlite3.c shell.c sqlite3.h sqlite3ext.h)\n"
   "target_link_libraries(sqlite \${CMAKE_THREAD_LIBS_INIT} \${CMAKE_DL_LIBS})\n"
   "\n"
   "add_definitions(-DSQLITE_ENABLE_RTREE)\n"
   "add_definitions(-DSQLITE_ENABLE_JSON1)\n"
   "install(TARGETS sqlite DESTINATION bin)\n"
   "install(TARGETS sqlite3 DESTINATION lib)\n"
   "install(FILES sqlite3.h DESTINATION include)\n"
)

ExternalProject_Add(
    sqlite
    # DEPENDS
    TMP_DIR           ${CMAKE_BINARY_DIR}/temp
    STAMP_DIR         ${CMAKE_BINARY_DIR}/stamp
    #--Download step--------------
    DOWNLOAD_DIR      ${download_dir}/sqlite
    URL               http://www.sqlite.org/2017/sqlite-autoconf-3160200.tar.gz
    URL_HASH          SHA1=64ca578ad44a94115b1db0406740e14288f74bb8
    #--Update/Patch step----------
    UPDATE_COMMAND    ${CMAKE_COMMAND} -E copy
                           ${CMAKE_BINARY_DIR}/temp/CMakeLists.txt
                           ${CMAKE_CURRENT_BINARY_DIR}/sqlite/CMakeLists.txt
    #--Configure step-------------
    SOURCE_DIR        ${CMAKE_CURRENT_BINARY_DIR}/sqlite
    CMAKE_ARGS        -DCMAKE_INSTALL_PREFIX:PATH=${DEPENDENCY_INSTALL_PREFIX}
                      -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    #--Build step-----------------
    BINARY_DIR        ${CMAKE_BINARY_DIR}/sqlite
    BUILD_COMMAND     ${CMAKE_COMMAND} --build .
    #--Install step----------------
    INSTALL_DIR       ${DEPENDENCY_INSTALL_PREFIX}
)

ExternalProject_Get_Property(sqlite install_dir)
ExternalProject_Get_Property(sqlite binary_dir)
set(SQLITE_INCLUDE_DIR ${install_dir}/include CACHE INTERNAL "")
set(SQLITE_LIBRARIES_DIR ${binary_dir}/${CMAKE_BUILD_TYPE} CACHE INTERNAL "")
