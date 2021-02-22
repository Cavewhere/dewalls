from conans import ConanFile, CMake, tools
#from conan.tools.qbs import Qbs
#from conan.tools.qbs import QbsToolchain

import os, sys
#from qbstoolchain import QbsToolchain

class DewallsConan(ConanFile):
    name = "dewalls"
    license = "MIT"
    author = "Andy Edwards Philip Schuchardt vpicaver@gmail.com"
    url = "https://github.com/Cavewhere/conan-cave-software"
    description = "A C++/Qt parser for Walls .wpj and .srv cave survey data format"
    topics = ("gis")
    settings = "os", "compiler", "build_type", "arch"
    requires = "catch2/2.13.4", "QbsToolchainProfile/1.0@cave-software/dev"
    options = {"system_qt": [True, False],
               "system_qbs": [True, False]}
    default_options = {"system_qt": False,
                      "system_qbs": False }

    def requirements(self):
        # Or add a new requirement!
        if not self.options.system_qt:
           self.requires("qt/5.15.2")

    def set_version(self):
        git = tools.Git()
        refStr = git.run("ls-remote -h https://github.com/Cavewhere/dewalls.git")
        refs = refStr.split("\n")

        sha = "unknown-sha"
        for ref in refs:
            print("ref:{0}".format(ref))
            if "refs/heads/master" in ref:
                sha = ref.split()[0]

        self.version = "{0}".format(sha)

    def configure(self):
        if not self.options.system_qt:
            self.options["qt"].shared = True

#    def config_options(self):
#        if self.settings.os == "Windows":
#            del self.options.fPIC

    def source(self):
        self.run("git clone  https://github.com/Cavewhere/dewalls.git --branch master .")
        self.run("git pull --tags")


    def build_requirements(self):
        if not self.options.system_qbs:
            self.build_requires("Qbs/1.17.0@cave-software/dev")

    def build(self):
        from qbstoolchain import QbsToolchain

        #print(', '.join("ENV %s: %s" % item for item in vars(self.deps_env_info["qt"]).items()))
        def qmake_exe(self):
            if self.settings.os == "Windows":
                return "qmake.exe"
            else:
                return "qmake"

        def settings_dir(self):
            return '%s/conan_qbs_toolchain_settings_dir' % self.install_folder

        tc = QbsToolchain(self)

        def path_to_qmake(self):
            if self.options.system_qt:
                return "{0}/bin".format(os.getenv("QTDIR"))
            else:
                return self.deps_cpp_info["qt"].bin_paths[0]

        qmake_path = "{0}{1}{2}".format(path_to_qmake(self), os.path.sep, qmake_exe(self))
        project_file = "dewalls.qbs"
        qbs_build_variant = str(self.settings.build_type).lower()

        self.run("qbs setup-qt --settings-dir {0} {1} qt-profile"
        .format(self.build_folder, qmake_path))

        self.run("qbs config --settings-dir {0} profiles.qt-profile.baseProfile conan"
        .format(self.build_folder)) #MSVC2019-x64")

        self.run("qbs build --no-install --settings-dir {0} --build-directory {0} --file {1} config:{2} profile:qt-profile"
        .format(self.build_folder, project_file, qbs_build_variant))

        self.run("qbs run -p dewalls-test --settings-dir {0} config:{1} profile:qt-profile"
        .format(self.build_folder, qbs_build_variant))

    def package(self):
        self.copy("*.h", dst="include", src="src")
        self.copy("*dewalls.lib", dst="lib", keep_path=False)
        self.copy("*.dll", dst="bin", keep_path=False)
        self.copy("*.so", dst="lib", keep_path=False)
        self.copy("*.dylib", dst="lib", keep_path=False)
        self.copy("*.a", dst="lib", keep_path=False)

    def package_info(self):
        self.cpp_info.libs = ["dewalls"]
