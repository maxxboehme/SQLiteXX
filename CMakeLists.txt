cmake_minimum_required(VERSION 2.8)
project(SQLiteXX CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
   ${CMAKE_MODULE_PATH})

# Setting up standard defaults, these will be passed down into external projects
include(BuildType)
include(ExternalProject)
include(download_dir)

# Add the third party dependencies
add_subdirectory(thirdparty)

file(GLOB HEADERS "src/*.h")
file(GLOB SOURCES "src/*.cpp")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${SQLITE_INCLUDE_DIR})

link_directories(${SQLITE_LIBRARIES_DIR})

add_executable(SQLiteXX ${SOURCES})
add_dependencies(SQLiteXX sqlite)
target_link_libraries(SQLiteXX sqlite3)
target_compile_features(SQLiteXX PUBLIC cxx_range_for cxx_noexcept)
set(SQLITECXX_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src)

enable_testing()
include(CTest)
find_program( MEMORYCHECK_COMMAND valgrind )
set(MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --leak-check=full")
add_subdirectory(tests)

# add a target to generate API documentation with Doxygen
find_package(Doxygen)
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})
set(doxy_main_page ${CMAKE_CURRENT_SOURCE_DIR}/ReadMe.md)

if(BUILD_DOCUMENTATION)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build documentation.")
    endif()

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)
endif()

install(FILES ${HEADERS} DESTINATION include/SQLiteXX)
