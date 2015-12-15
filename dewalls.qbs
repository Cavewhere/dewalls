import qbs 1.0

Project {
    name: "dewalls"

    StaticLibrary {
        name: "dewalls"
        Depends { name: "cpp" }
        Depends { name: "Qt"; submodules: ["core"] }

        cpp.includePaths: ["src"]

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: ["src"]
        }

        Properties {
            condition: qbs.targetOS.contains("osx") || qbs.targetOS.contains("linux")
            cpp.cxxFlags: [
                "-stdlib=libc++",
                "-std=c++11", //For c++11 support
                "-Werror" //Treat warnings as errors
            ]
        }

        Properties {
            condition: qbs.targetOS.contains("osx")



            cpp.dynamicLibraries: [
                "c++"
            ]
        }

        files: [
            "src/*.cpp",
            "src/*.h",
        ]
    }

    CppApplication {
        name: "dewalls-test"
        consoleApplication: true
        type: "application"

        Depends { name: "cpp" }
        Depends { name: "Qt"; submodules: ["core"] }
        Depends { name: "dewalls" }

        cpp.includePaths: ["src"]

        Properties {
            condition: qbs.targetOS.contains("osx") || qbs.targetOS.contains("linux")
            cpp.cxxFlags: [
                "-stdlib=libc++",
                "-std=c++11", //For c++11 support
                "-Werror" //Treat warnings as errors
            ]
        }

        Properties {
            condition: qbs.targetOS.contains("osx")

            cpp.dynamicLibraries: [
                "c++"
            ]
            // TEMP
            cpp.rpaths: [
                "/Users/andy/Qt/5.5/clang_64/lib"
            ]
        }

        files: [
            "test/*.cpp",
            "test/*.h",
            "test/dewalls-test.qrc",
        ]
    }
}
