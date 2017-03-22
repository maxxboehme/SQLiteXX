cmake_minimum_required(VERSION 2.8)
project(SQLiteXX C CXX)

set(SQLITEXX_VERSION_MAJOR 0)
set(SQLITEXX_VERSION_MINOR 1)
set(SQLITEXX_VERSION_PATCH 0)
set(SQLITEXX_VERSION
    ${SQLITEXX_VERSION_MAJOR}.${SQLITEXX_VERSION_MINOR}.${SQLITEXX_VERSION_PATCH})

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
   ${CMAKE_MODULE_PATH})

# Setting up standard defaults, these will be passed down into external projects
include(GenerateExportHeader)
include(BuildType)
include(ExternalProject)
include(download_dir)

# Add the third party dependencies
add_subdirectory(thirdparty)

file(GLOB HEADERS "src/*.h")
file(GLOB SOURCES "src/*.cpp")

add_library(SQLiteXX ${SOURCES})
add_dependencies(SQLiteXX sqlite)
generate_export_header(SQLiteXX)
target_include_directories(SQLiteXX PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(SQLiteXX PUBLIC ${SQLITE_INCLUDE_DIR})
target_link_libraries(SQLiteXX ${SQLITE_LIBRARIES})
target_compile_features(SQLiteXX PUBLIC cxx_range_for cxx_noexcept)

set(SQLITEXX_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src)

include(CMakePackageConfigHelpers)
set(INCLUDE_INSTALL_DIR include/SQLiteXX CACHE INTERNAL "")
set(LIB_INSTALL_DIR lib/SQLiteXX CACHE INTERNAL "")
set(SYSCONFIG_INSTALL_DIR etc/SQLiteXX CACHE INTERNAL "")
configure_package_config_file(SQLiteXXConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/SQLiteXX/SQLiteXXConfig.cmake
    INSTALL_DESTINATION ${LIB_INSTALL_DIR}/SQLiteXX.cmake
    PATH_VARS INCLUDE_INSTALL_DIR SYSCONFIG_INSTALL_DIR)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/SQLiteXX/SQLiteXXConfigVersion.cmake"
    VERSION ${SQLITEXX_VERSION}
    COMPATIBILITY SameMajorVersion)


enable_testing()
include(CTest)
find_program(VALGRIND_COMMAND valgrind)
set(MEMCHECK_COMMAND ${VALGRIND_COMMAND})
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

install(FILES ${HEADERS} DESTINATION ${INCLUDE_INSTALL_DIR})

# General CPack variables
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "SQLiteXX")
set(CPACK_PACKAGE_VENDOR "Maxx Boehme")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION ${SQLITEXX_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${SQLITEXX_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${SQLITEXX_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${SQLITEXX_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "SQLiteXX")

# DEB package veraiables
#set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "sqlite")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Maxx Boehme")
set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

# This must always be after CPACK variables
include(CPack)
