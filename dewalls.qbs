import qbs 1.0
import qbs.Probes

Project {
    name: "dewalls"

    Probes.ConanfileProbe {
        id: conan
        conanfilePath: project.sourceDirectory + "/conanfile.py"
        generators: "qbs"
        options: ({system_qt:"True", system_qbs:"True"})
    }

    references: conan.generatedFilesPath + "/conanbuildinfo.qbs"

    DynamicLibrary {
        name: "dewalls"

        Depends { name: "cpp" }
        Depends { name: "Qt"; submodules: ["core"] }
        Depends { name: "bundle" }

        Export {
            Depends { name: "cpp" }
            cpp.includePaths: ["src"]
            cpp.cxxFlags: {
                if(qbs.toolchain.contains("gcc")) {
                    return ["-Wno-attributes"] //Ignore-around to a g++ bug, https://gcc.gnu.org/bugzilla/show_bug.cgi?id=43407
                }
                return []
            }
        }

        Group {
            fileTagsFilter: ["dynamiclibrary"]
            condition: qbs.buildVariant == "release"
            qbs.install: qbs.targetOS.contains("windows")
        }

        Group {
            fileTagsFilter: ["bundle.content"]
            qbs.install: bundle.isBundle
            qbs.installSourceBase: product.buildDirectory
            qbs.installDir: "lib"
            qbs.installPrefix: ""
        }

        cpp.includePaths: ["src"]
        cpp.cxxLanguageVersion: "c++11"
        cpp.treatWarningsAsErrors: false

        Properties {
            condition: qbs.targetOS.contains("windows")
            cpp.defines: ["DEWALLS_LIB"]
        }

        Properties {
            condition: qbs.targetOS.contains("macos")
            cpp.sonamePrefix: qbs.installRoot + "/lib"
            cpp.cxxFlags: {
                var flags = [];

                if(qbs.buildVariant == "debug") {
                    flags.push("-fsanitize=address");
                    flags.push("-fno-omit-frame-pointer")
                }

                return flags;
            }

            cpp.driverFlags: {
                var flags = [];
                if(qbs.buildVariant == "debug") {
                    flags.push("-fsanitize=address")
                }
                return flags;
            }
        }

        Properties {
            condition: qbs.targetOS.contains("linux")
            cpp.cxxFlags: {
                var flags = []
                if(qbs.toolchain.contains("gcc")) {
                    flags.push("-Wno-attributes") //Ignore-around to a g++ bug, https://gcc.gnu.org/bugzilla/show_bug.cgi?id=43407
                }
                return flags
            }
        }

        files: [
            "conanfile.txt",
            "src/*.cpp",
            "src/*.h"
        ]
    }

    CppApplication {
        name: "dewalls-test"
        consoleApplication: true
        type: "application"

        Depends { name: "cpp" }
        Depends { name: "Qt"; submodules: ["core"] }
        Depends { name: "dewalls" }
        Depends { name: "catch2" }

        cpp.includePaths: ["src", "lib"]
        cpp.cxxLanguageVersion: "c++11"

        files: [
            "conanfile.py",
            "test/*.cpp",
            "test/*.h",
            "test/dewalls-test.qrc",
        ]
    }
}
