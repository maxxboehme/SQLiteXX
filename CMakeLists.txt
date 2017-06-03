cmake_minimum_required(VERSION 3.2)
cmake_policy(SET CMP0048 NEW)
message(STATUS "CMake version: ${CMAKE_VERSION}")

set(SQLITEXX_VERSION_MAJOR 0)
set(SQLITEXX_VERSION_MINOR 1)
set(SQLITEXX_VERSION_PATCH 0)
set(SQLITEXX_VERSION ${SQLITEXX_VERSION_MAJOR}.${SQLITEXX_VERSION_MINOR}.${SQLITEXX_VERSION_PATCH})
project(SQLiteXX VERSION ${SQLITEXX_VERSION} LANGUAGES C CXX)
find_package(Threads REQUIRED)


set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake"
   ${CMAKE_MODULE_PATH})

# Setting up standard defaults, these will be passed down into external projects
include(GenerateExportHeader)
include(BuildType)
include(ExternalProject)
include(download_dir)

# Add the third party dependencies
add_subdirectory(thirdparty)

# Collecting header and source files for SQLiteXX
file(GLOB SQLITEXX_HEADERS "src/*.h")
file(GLOB SQLITEXX_SOURCES "src/*.cpp")

add_library(SQLiteXX ${SQLITEXX_HEADERS} ${SQLITEXX_SOURCES})
add_dependencies(SQLiteXX sqlite)
generate_export_header(SQLiteXX)
target_include_directories(SQLiteXX PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_include_directories(SQLiteXX PRIVATE ${SQLITE_INCLUDE_DIR})
target_link_libraries(SQLiteXX ${SQLITE_LIBRARIES})
target_link_libraries(SQLiteXX ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
target_compile_features(SQLiteXX PRIVATE cxx_nullptr)
target_compile_features(SQLiteXX PUBLIC cxx_rvalue_references cxx_noexcept cxx_variadic_templates cxx_strong_enums cxx_generic_lambdas)

# Setting for use in testing component
set(SQLITEXX_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src)
set(SQLITEXX_LIBRARIES $<TARGET_LINKER_FILE_DIR:SQLiteXX>/$<TARGET_LINKER_FILE_NAME:SQLiteXX>)

# === Creating CMake Package Files for SQLiteXX ===
include(CMakePackageConfigHelpers)

set(INCLUDE_INSTALL_DIR include/${PROJECT_NAME} CACHE INTERNAL "")
set(LIB_INSTALL_DIR lib CACHE INTERNAL "")
set(CONFIG_INSTALL_DIR ${LIB_INSTALL_DIR}/cmake/${PROJECT_NAME} CACHE INTERNAL "")

configure_package_config_file(
    SQLiteXXConfig.cmake.in
    ${CMAKE_BINARY_DIR}/SQLiteXX/SQLiteXXConfig.cmake
    INSTALL_DESTINATION ${CONFIG_INSTALL_DIR}
    PATH_VARS INCLUDE_INSTALL_DIR LIB_INSTALL_DIR)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake"
    VERSION ${SQLITEXX_VERSION}
    COMPATIBILITY SameMajorVersion)

install(
    TARGETS SQLiteXX
    EXPORT SQLiteXXTargets
    LIBRARY DESTINATION ${LIB_INSTALL_DIR}
    ARCHIVE DESTINATION ${LIB_INSTALL_DIR}
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION ${INCLUDE_INSTALL_DIR})
install(EXPORT SQLiteXXTargets
    FILE SQLiteXXTargets.cmake
    NAMESPACE SQLiteXX::
    DESTINATION ${CONFIG_INSTALL_DIR})
install(
    FILES
        ${CMAKE_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}/${PROJECT_NAME}ConfigVersion.cmake
    DESTINATION
        ${CONFIG_INSTALL_DIR})
install(
    FILES ${SQLITEXX_HEADERS}
    DESTINATION ${INCLUDE_INSTALL_DIR})


# === Setting Up SQLiteXX Tests ===
enable_testing()
include(CTest)
find_program(VALGRIND_COMMAND valgrind)
set(MEMCHECK_COMMAND ${VALGRIND_COMMAND})
set(MEMORYCHECK_COMMAND_OPTIONS "--trace-children=yes --leak-check=full")
add_subdirectory(tests)


# add a target to generate API documentation with Doxygen
find_package(Doxygen)
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})
set(doxy_main_page ${CMAKE_CURRENT_SOURCE_DIR}/README.md)

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


# === General CPack Variables ===
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "SQLiteXX")
set(CPACK_PACKAGE_VENDOR "Maxx Boehme")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_PACKAGE_VERSION ${SQLITEXX_VERSION})
set(CPACK_PACKAGE_VERSION_MAJOR ${SQLITEXX_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${SQLITEXX_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${SQLITEXX_VERSION_PATCH})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "SQLiteXX")

# DEB Package Variables
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsqlite3-dev")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Maxx Boehme")
set(CPACK_DEBIAN_ARCHITECTURE ${CMAKE_SYSTEM_PROCESSOR})

# This must always be after CPACK variables
include(CPack)
