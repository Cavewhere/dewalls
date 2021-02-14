cmake_minimum_required(VERSION 3.12)

enable_testing()
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

#add_test(NAME run-dewalls-test
#    COMMAND ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/dewalls-test
#    WORKING_DIRECTORY ${PROJECT_BINARY_DIR})

#string(REPLACE ";" "\\;" escaped_path "$ENV{PATH}")
#set_tests_properties(run-dewalls-test PROPERTIES
#    ENVIRONMENT "PATH=D:\\docs\\Projects\\cavewhere\\cavewhere\\build-dewalls-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug\\lib;D:\\docs\\Projects\\cavewhere\\cavewhere\\build-dewalls-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug\\bin;D:\\Qt\\5.15.2\\msvc2019_64\\lib;D:\\Qt\\5.15.2\\msvc2019_64\\bin;D:\\docs\\Projects\\cavewhere\\cavewhere\\build-dewalls-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug;C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\Common7\\IDE\\\\Extensions\\Microsoft\\IntelliCode\\CLI;C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\VC\\Tools\\MSVC\\14.27.29110\\bin\\HostX64\\x64;C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\Common7\\IDE\\VC\\VCPackages;C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\Common7\\IDE\\CommonExtensions\\Microsoft\\TestWindow;C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\Common7\\IDE\\CommonExtensions\\Microsoft\\TeamFoundation\\Team Explorer;C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\MSBuild\\Current\\bin\\Roslyn;C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\Team Tools\\Performance Tools\\x64;C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\Team Tools\\Performance Tools;C:\\Program Files (x86)\\Microsoft Visual Studio\\Shared\\Common\\VSPerfCollectionTools\\vs2019\\\\x64;C:\\Program Files (x86)\\Microsoft Visual Studio\\Shared\\Common\\VSPerfCollectionTools\\vs2019\\;C:\\Program Files (x86)\\Windows Kits\\10\\bin\\10.0.18362.0\\x64;C:\\Program Files (x86)\\Windows Kits\\10\\bin\\x64;C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\\\MSBuild\\Current\\Bin;C:\\Windows\\Microsoft.NET\\Framework64\\v4.0.30319;C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\Common7\\IDE\\;C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\Common7\\Tools\\;C:\\Program Files (x86)\\VMware\\VMware Player\\bin\\;C:\\Users\\vpica\\AppData\\Roaming\\ActiveState\\bin;C:\\Windows\\system32;C:\\Windows;C:\\Windows\\System32\\Wbem;C:\\Windows\\System32\\WindowsPowerShell\\v1.0\\;C:\\Windows\\System32\\OpenSSH\\;C:\\Program Files\\NVIDIA Corporation\\NVIDIA NvDLISR;C:\\Program Files\\Git\\cmd;C:\\Program Files (x86)\\Windows Kits\\10\\Windows Performance Toolkit\\;C:\\Program Files (x86)\\Common Files\\Acronis\\VirtualFile\\;C:\\Program Files (x86)\\Common Files\\Acronis\\VirtualFile64\\;C:\\Program Files (x86)\\Common Files\\Acronis\\FileProtector\\;C:\\Program Files (x86)\\Common Files\\Acronis\\FileProtector64\\;C:\\Program Files (x86)\\Common Files\\Acronis\\SnapAPI\\;C:\\Program Files (x86)\\NVIDIA Corporation\\PhysX\\Common;C:\\Strawberry\\c\\bin;C:\\Strawberry\\perl\\site\\bin;C:\\Strawberry\\perl\\bin;C:\\Program Files\\CMake\\bin;C:\\Users\\vpica\\AppData\\Local\\Microsoft\\WindowsApps;C:\\Program Files\\OpenSSL\\bin;C:\\Users\\vpica\\AppData\\Local\\Packages\\PythonSoftwareFoundation.Python.3.9_qbz5n2kfra8p0\\LocalCache\\local-packages\\Python39\\Scripts\\;;C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\CMake\\bin;C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\Common7\\IDE\\CommonExtensions\\Microsoft\\CMake\\Ninja"
#)

#set_property(TEST run-dewalls-test APPEND PROPERTY
#        ENVIRONMENT "PATH=D:/docs/Projects/cavewhere/cavewhere/build-dewalls-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug/lib;D:/docs/Projects/cavewhere/cavewhere/build-dewalls-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug/bin;D:/Qt/5.15.2/msvc2019_64/lib;D:/Qt/5.15.2/msvc2019_64/bin;D:/docs/Projects/cavewhere/cavewhere/build-dewalls-Desktop_Qt_5_15_2_MSVC2019_64bit-Debug;C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/IDE//Extensions/Microsoft/IntelliCode/CLI;C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/VC/Tools/MSVC/14.27.29110/bin/HostX64/x64;C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/IDE/VC/VCPackages;C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/IDE/CommonExtensions/Microsoft/TestWindow;C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/IDE/CommonExtensions/Microsoft/TeamFoundation/Team Explorer;C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/MSBuild/Current/bin/Roslyn;C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Team Tools/Performance Tools/x64;C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Team Tools/Performance Tools;C:/Program Files (x86)/Microsoft Visual Studio/Shared/Common/VSPerfCollectionTools/vs2019//x64;C:/Program Files (x86)/Microsoft Visual Studio/Shared/Common/VSPerfCollectionTools/vs2019/;C:/Program Files (x86)/Windows Kits/10/bin/10.0.18362.0/x64;C:/Program Files (x86)/Windows Kits/10/bin/x64;C:/Program Files (x86)/Microsoft Visual Studio/2019/Community//MSBuild/Current/Bin;C:/Windows/Microsoft.NET/Framework64/v4.0.30319;C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/IDE/;C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/Tools/;C:/Program Files (x86)/VMware/VMware Player/bin/;C:/Users/vpica/AppData/Roaming/ActiveState/bin;C:/Windows/system32;C:/Windows;C:/Windows/System32/Wbem;C:/Windows/System32/WindowsPowerShell/v1.0/;C:/Windows/System32/OpenSSH/;C:/Program Files/NVIDIA Corporation/NVIDIA NvDLISR;C:/Program Files/Git/cmd;C:/Program Files (x86)/Windows Kits/10/Windows Performance Toolkit/;C:/Program Files (x86)/Common Files/Acronis/VirtualFile/;C:/Program Files (x86)/Common Files/Acronis/VirtualFile64/;C:/Program Files (x86)/Common Files/Acronis/FileProtector/;C:/Program Files (x86)/Common Files/Acronis/FileProtector64/;C:/Program Files (x86)/Common Files/Acronis/SnapAPI/;C:/Program Files (x86)/NVIDIA Corporation/PhysX/Common;C:/Strawberry/c/bin;C:/Strawberry/perl/site/bin;C:/Strawberry/perl/bin;C:/Program Files/CMake/bin;C:/Users/vpica/AppData/Local/Microsoft/WindowsApps;C:/Program Files/OpenSSL/bin;C:/Users/vpica/AppData/Local/Packages/PythonSoftwareFoundation.Python.3.9_qbz5n2kfra8p0/LocalCache/local-packages/Python39/Scripts/;;C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin;C:/Program Files (x86)/Microsoft Visual Studio/2019/Community/Common7/IDE/CommonExtensions/Microsoft/CMake/Ninja")
