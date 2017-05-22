cmake_minimum_required(VERSION 2.8)
project(SQLiteXXInstallTest C CXX)

find_package(SQLiteXX 0.1.0 REQUIRED)

file(GLOB SOURCES "src/*.cpp")

add_executable(SQLiteXXInstallTest ${SOURCES})
target_include_directories(SQLiteXX PRIVATE ${SQLITEXX_INCLUDES})
target_link_libraries(SQLiteXX ${SQLITEXX_LIBRARIES})
target_compile_features(SQLiteXXInstallTest PUBLIC cxx_range_for cxx_noexcept)

enable_testing()
include(CTest)

add_test(InstallTest SQLiteXXInstallTest)
