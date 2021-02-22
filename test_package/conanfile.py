import os

from conans import ConanFile, CMake, tools


class DewallsTestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake"
    options = {"system_qt": [True, False] }
    default_options = {"system_qt": False }

    def configure(self):
        self.options["dewalls"].system_qt = self.options.system_qt

    def build(self):
        cmake = CMake(self)

        if self.options.system_qt:
            qt_dir = os.getenv("QTDIR")
            cmake.definitions["Qt5_DIR"] = "{0}/lib/cmake/Qt5".format(qt_dir)

        # Current dir is "test_package/build/<build_id>" and CMakeLists.txt is
        # in "test_package"
        cmake.definitions["SYSTEM_QT"] = self.options.system_qt
        cmake.configure()
        cmake.build()

    def imports(self):
        self.copy("*.dll", dst="bin", src="bin")
        self.copy("*.dylib*", dst="bin", src="lib")
        self.copy('*.so*', dst='bin', src='lib')

    def test(self):
        if not tools.cross_building(self):
            os.chdir("bin")
            if self.options.system_qt:
                qt_bin_dir = "{0}/bin".format(os.getenv("QTDIR"))
                os.environ["PATH"] += os.pathsep + qt_bin_dir
            self.run(".%sexample" % os.sep)
