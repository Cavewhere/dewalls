cmake_minimum_required(VERSION 3.1)
project(PackageTest CXX)

option(SYSTEM_QT "Use Qt found on the system instead of Conan" Off)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

message("Using system qt:${SYSTEM_QT}")

add_executable(example example.cpp)

if(${SYSTEM_QT})
#    set(QT5_DIR $ENV{QTDIR})
    find_package(Qt5 COMPONENTS Core REQUIRED)
    target_link_libraries(example Qt5::Core)
endif()

target_link_libraries(example ${CONAN_LIBS})

# CTest is a testing tool that can be used to test your project.
# enable_testing()
# add_test(NAME example
#          WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/bin
#          COMMAND example)
