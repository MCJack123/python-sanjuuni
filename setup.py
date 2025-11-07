from setuptools import setup, Command, Extension
from setuptools.command.build import SubCommand, build
import regex
import os
import platform

class build_clcpp(Command, SubCommand):
    def initialize_options(self): pass
    def finalize_options(self): pass

    def get_source_files(self):
        return ["sanjuuni.submodule/src/cl-pixel.cpp"]
    
    def get_outputs(self):
        return ["sanjuuni.submodule/src/cl-pixel-cl.cpp"]
    
    def get_output_mapping(self):
        return {"sanjuuni.submodule/src/cl-pixel-cl.cpp": "sanjuuni.submodule/src/cl-pixel.cpp"}

    def run(self):
        with open("sanjuuni.submodule/src/cc-pixel-cl.cpp", "w") as file:
            file.write("// Generated automatically; do not edit!\n#include <string>\nnamespace OpenCL {std::string get_opencl_c_code() { return ")
            text = ""
            with open("sanjuuni.submodule/src/cc-pixel.cpp", "r") as infile: text = infile.read()
            text = regex.sub("#ifndef OPENCV.*?#endif\n", "", text, flags=regex.S)
            text = regex.sub("\\\\", "\\\\\\\\", text)
            text = regex.sub('"', '\\\\"', text)
            text = regex.sub("^", '"', text, flags=regex.M)
            text = regex.sub("$", '\\\\n"', text, flags=regex.M)
            file.write(text)
            file.write(";}}")


class BuildCommand(build):
    def run(self):
        self.run_command('build_clcpp')
        build.run(self)

if os.environ.get('CIBUILDWHEEL', '0') == '1' and platform.system() == "Windows":
    triplet = "x64-windows" if (platform.machine() == "x86_64" or platform.machine() == "amd64" or platform.machine() == "AMD64") else "arm64-windows"
    setup(
        cmdclass={"build_clcpp": build_clcpp, "build": BuildCommand},
        ext_modules=[Extension(
            name="sanjuuni.__init__",
            sources=["sanjuunimodule.cpp", "sanjuuni.submodule/src/cc-pixel-cl.cpp", "sanjuuni.submodule/src/cc-pixel.cpp", "sanjuuni.submodule/src/generator.cpp", "sanjuuni.submodule/src/octree.cpp", "sanjuuni.submodule/src/quantize.cpp"],
            include_dirs=["sanjuuni.submodule/src", os.environ.get("VCPKG_INSTALLATION_ROOT") + "\\installed\\" + triplet + "\\include"],
            libraries=[os.environ.get("VCPKG_INSTALLATION_ROOT") + "\\installed\\" + triplet + "\\lib\\OpenCL"],
            depends=["sanjuuni.submodule/src/sanjuuni.hpp", "sanjuuni.submodule/src/opencl.hpp"],
            extra_compile_args=["-DNO_POCO=1", "-DHAS_OPENCL=1", "-DCL_API_CALL=__cdecl", "-DCL_API_ENTRY=__declspec(dllimport)"] # https://github.com/pypa/setuptools/issues/4810
        )]
    )
else:
    setup(
        cmdclass={"build_clcpp": build_clcpp, "build": BuildCommand},
        ext_modules=[Extension(
            name="sanjuuni.__init__",
            sources=["sanjuunimodule.cpp", "sanjuuni.submodule/src/cc-pixel-cl.cpp", "sanjuuni.submodule/src/cc-pixel.cpp", "sanjuuni.submodule/src/generator.cpp", "sanjuuni.submodule/src/octree.cpp", "sanjuuni.submodule/src/quantize.cpp"],
            include_dirs=["sanjuuni.submodule/src"],
            depends=["sanjuuni.submodule/src/sanjuuni.hpp", "sanjuuni.submodule/src/opencl.hpp"],
            extra_compile_args=["-DNO_POCO=1"] # https://github.com/pypa/setuptools/issues/4810
        )]
    )
