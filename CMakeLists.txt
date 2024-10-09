cmake_minimum_required(VERSION 3.10)
project(dewalls)

# Find Qt packages
find_package(Qt6 COMPONENTS Core REQUIRED)
find_package(Catch2 REQUIRED)

# Include the export header module
include(GenerateExportHeader)

# Add dynamic library
add_library(dewalls SHARED)

set_target_properties(dewalls PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# Use glob to collect source files
file(GLOB DEWALLS_SOURCES "src/*.cpp" "src/*.h")
target_sources(dewalls PRIVATE ${DEWALLS_SOURCES})

# Generate export header
generate_export_header(dewalls
    BASE_NAME DEWALLS
    EXPORT_MACRO_NAME DEWALLS_LIB_EXPORT
    EXPORT_FILE_NAME dewallsexport.h
    STATIC_DEFINE DEWALLS_BUILT_AS_STATIC
)

# Specify include directories
target_include_directories(dewalls PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>  # for the export header
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

# Add dependencies
target_link_libraries(dewalls PUBLIC Qt6::Core)

# Compiler flags
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(dewalls PRIVATE "-Wno-attributes")
endif()

# Add console application
add_executable(dewalls-test)

set_target_properties(dewalls-test PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# Use glob to collect source files for the test application
file(GLOB DEWALLS_TEST_SOURCES "test/*.cpp" "test/*.h" "test/dewalls-test.qrc")
target_sources(dewalls-test PRIVATE ${DEWALLS_TEST_SOURCES})

# Specify include directories for the test application
target_include_directories(dewalls-test PRIVATE src lib)

# Add dependencies for the test application
target_link_libraries(dewalls-test
    PRIVATE
    dewalls
    Catch2::Catch2
    Catch2::Catch2WithMain)

# Set C++11 standard
set_property(TARGET dewalls PROPERTY CXX_STANDARD 17)
set_property(TARGET dewalls-test PROPERTY CXX_STANDARD 17)
