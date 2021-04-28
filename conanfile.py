import os

from conans import ConanFile, CMake, tools


class V8TestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake", "cmake_paths"
    requires = (
        "cli11/1.9.1",
        "v8/8.8.214@altairwei/testing"
    )

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.definitions["CONAN_INSTALL_MANUALLY"] = "ON"
        cmake.build()

    def imports(self):
        self.copy("*.dll", dst="bin", src="bin")
        self.copy("*.dylib*", dst="bin", src="lib")
        self.copy('*.so*', dst='bin', src='lib')
