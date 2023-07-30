from conan import ConanFile
from conan.tools.files import copy
from conan.tools.cmake import CMake, CMakeToolchain, CMakeDeps, cmake_layout

import os

required_conan_version = ">=1.53.0"


class VoiceTuberConan(ConanFile):
    name = "VoiceTuber"
    description = "VTuber application which only requires your voice and microphone, no need for a webcam or other tracking nonsense."
    topics = "vtuber",
    url = homepage = "https://github.com/StellaSmith/VoiceTuber"
    license = "MIT"
    package_type = "application"
    settings = "os", "arch", "compiler", "build_type"

    options = {
        "fPIC": [True, False],
    }

    default_options = {
        "fPIC": True,

        "pulseaudio/*:shared": True,
        "sdl/*:shared": True,
        "sdl_ttf/*:shared": True,
        "libcurl/*:shared": True,
    }

    exports_sources = "CMakeLists.txt", "src/*", "3rd-party/*"

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["imgui_RES_DIR"] = os.path.join(self.dependencies["imgui"].package_folder, "res").replace("\\", "/")
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def requirements(self):
        self.requires("stb/cci.20220909")
        self.requires("libcurl/8.1.2")
        self.requires("glm/cci.20230113")
        self.requires("sdl/2.26.5")
        self.requires("sdl_ttf/2.20.1")
        self.requires("libuv/1.45.0")
        self.requires("cpptoml/0.1.1")
        self.requires("imgui/cci.20230105+1.89.2.docking")
        self.requires("pocketsphinx/5.0.1")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
        copy(self, pattern="LICENSE", src=os.path.join(self.source_folder), dst=os.path.join(self.package_folder, "licenses"))

    def package_info(self):
        self.cpp_info.libs = ["pocketsphinx"]

