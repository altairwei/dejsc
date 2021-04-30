import os
import shutil

from conans import ConanFile, CMake, tools


class V8TestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    exports_sources = "dejsc/*"

    def _set_environment_vars(self):
        """set the environment variables, such that the google tooling is found (including the bundled python2)"""
        os.environ["PATH"] = os.path.join(self.source_folder, "depot_tools") + os.pathsep + os.environ["PATH"]
        os.environ["DEPOT_TOOLS_PATH"] = os.path.join(self.source_folder, "depot_tools")
        if tools.os_info.is_windows:
            os.environ["DEPOT_TOOLS_WIN_TOOLCHAIN"] = "0"
            os.environ["GYP_MSVS_VERSION"] = "2017" if str(self.settings.compiler.version) == "15" else "2019"

    def source(self):
        if not os.path.exists(os.path.join(self.source_folder, "depot_tools")):
            self.run("git clone --depth 1 https://chromium.googlesource.com/chromium/tools/depot_tools.git")
        self._set_environment_vars()
        v8_source_root = os.path.join(self.source_folder, "v8")
        if not os.path.exists(v8_source_root):
            self.run("gclient")
            self.run("fetch v8")
            with tools.chdir("v8"):
                self.run("git checkout {}".format(self.version))
                self.run("gclient sync")
        dejsc_folder = os.path.join(v8_source_root, "src", "dejsc")
        if os.path.exists(dejsc_folder):
            shutil.rmtree(dejsc_folder)
        shutil.copytree(
            os.path.join(self.source_folder, "dejsc"),
            os.path.join(dejsc_folder))

    def build(self):
        pass

    def imports(self):
        self.copy("*.dll", dst="bin", src="bin")
        self.copy("*.dylib*", dst="bin", src="lib")
        self.copy('*.so*', dst='bin', src='lib')
