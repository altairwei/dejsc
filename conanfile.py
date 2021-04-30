import os
import shutil

from conans import ConanFile, CMake, tools


class V8TestConan(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    exports_sources = "dejsc/*", "BUILD.gn"

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
                self.run("git checkout {}".format("8.8.214"))
                self.run("gclient sync")
        dejsc_folder = os.path.join(v8_source_root, "src", "dejsc")
        if os.path.exists(dejsc_folder):
            shutil.rmtree(dejsc_folder)
        shutil.copytree(
            os.path.join(self.source_folder, "dejsc"),
            os.path.join(dejsc_folder))
        shutil.copy(
            os.path.join(self.source_folder, "BUILD.gn"),
            os.path.join(v8_source_root, "BUILD.gn"))

    def build_requirements(self):
        if not tools.which("ninja"):
            self.build_requires("ninja/1.10.0")
        if self.settings.os != "Windows":
            if not tools.which("bison"):
                self.build_requires("bison/3.5.3")
            if not tools.which("gperf"):
                self.build_requires("gperf/3.1")
            if not tools.which("flex"):
                self.build_requires("flex/2.6.4")

    def _install_system_requirements_linux(self):
        """some extra script must be executed on linux"""
        self.output.info("Calling v8/build/install-build-deps.sh")
        os.environ["PATH"] += os.pathsep + os.path.join(self.source_folder, "depot_tools")
        sh_script = self.source_folder + "/v8/build/install-build-deps.sh"
        self.run("chmod +x " + sh_script)
        cmd = sh_script + " --unsupported --no-arm --no-nacl --no-backwards-compatible --no-chromeos-fonts --no-prompt "
        cmd = cmd + ("--syms" if str(self.settings.build_type) == "Debug" else "--no-syms")
        cmd = "export DEBIAN_FRONTEND=noninteractive && " + cmd
        self.run(cmd)

    def _gen_arguments(self):
        # Refer to v8/infra/mb/mb_config.pyl
        is_debug = "true" if str(self.settings.build_type) == "Debug" else "false"
        gen_arguments = [
            "is_debug = " + is_debug,
            #"enable_iterator_debugging = " + is_debug, # TODO: make it configurable

            "target_cpu = " + ('"x64"' if str(self.settings.arch) == "x86_64" else '"x86"'),
            "is_component_build = false",
            "is_chrome_branded = false",
            "treat_warnings_as_errors = false",
            "is_clang = false", # Do not use clang and libc++ shipped with v8
            "use_custom_libcxx = false",
            "use_custom_libcxx_for_host = false",
            "use_glib = false",
            "use_sysroot = false",

            # V8 specific settings
            "v8_monolithic = true",
            "v8_static_library = true",
            "v8_use_external_startup_data = false",
            #"v8_enable_backtrace = false",
        ]

        return gen_arguments

    def build(self):
        v8_source_root = os.path.join(self.source_folder, "v8")
        self._set_environment_vars()

        if tools.os_info.is_linux:
            self._install_system_requirements_linux()

        with tools.chdir(v8_source_root):
            args = self._gen_arguments()
            args_gn_file = os.path.join(self.build_folder, "args.gn")
            with open(args_gn_file, "w") as f:
                f.write("\n".join(args))

            generator_call = "gn gen {folder}".format(folder=self.build_folder)

            print(generator_call)
            self.run(generator_call)
            self.run("ninja -C {folder} dejsc".format(folder=self.build_folder))

    def imports(self):
        self.copy("*.dll", dst="bin", src="bin")
        self.copy("*.dylib*", dst="bin", src="lib")
        self.copy('*.so*', dst='bin', src='lib')

    def package(self):
        if self.settings.os == "Windows":
            self.copy(pattern="dejsc.exe", dst="bin", keep_path=False)
        else:
            self.copy(pattern="dejsc", dst="bin", keep_path=False)