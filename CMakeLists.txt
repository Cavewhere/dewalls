cmake_minimum_required(VERSION 3.12)

find_program(GIT_EXEC git REQUIRED)

execute_process(
    COMMAND ${GIT_EXEC} describe --tags
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_TAG
    )
string(REGEX REPLACE "\n$" "" GIT_TAG "${GIT_TAG}")
string(REGEX REPLACE "([0-9]+)\.([0-9]+)\.([0-9]+).*" "\\1.\\2.\\3" PERIOD_VERSION "${GIT_TAG}")

# set the project name
project(dewalls VERSION ${PERIOD_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Qt5 COMPONENTS Core REQUIRED)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_AUTOUIC ON)

if(CMAKE_VERSION VERSION_LESS "3.7.0")
    set(CMAKE_INCLUDE_CURRENT_DIR ON)
endif()

if(CONAN_EXPORTED) # in conan local cache
    # standard conan installation, deps will be defined in conanfile.py
    # and not necessary to call conan again, conan is already running
    include(${PROJECT_BINARY_DIR}/conanbuildinfo.cmake)
    conan_basic_setup()
    conan_define_targets()
else() #in user space
    # CONAN download proj
    # Download automatically, you can also just copy the conan.cmake file
    if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
       message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
       file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
                      "${CMAKE_BINARY_DIR}/conan.cmake")
    endif()

    include(${CMAKE_BINARY_DIR}/conan.cmake)

    conan_cmake_run(REQUIRES catch2/2.13.4
        BASIC_SETUP CMAKE_TARGETS
        BUILD missing)
endif()


file(GLOB sources RELATIVE ${PROJECT_SOURCE_DIR} "src/*.h" "src/*.cpp")

add_library(dewalls SHARED
    ${sources}
)
target_compile_definitions(dewalls PRIVATE "DEWALLS_LIB")
target_compile_definitions(dewalls PUBLIC "DEWALLS_DYLIB")
target_include_directories(dewalls PUBLIC "src")
target_link_libraries(dewalls Qt5::Core)

file(GLOB test_sources RELATIVE ${PROJECT_SOURCE_DIR}
    "test/*.h"
    "test/*.cpp"
    "test/*.qrc"
    )

add_executable(dewalls-test ${test_sources})
target_link_libraries(dewalls-test dewalls CONAN_PKG::catch2)
