from setuptools import setup, Command
from setuptools.command.build import SubCommand, build
import regex

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
            text = regex.sub("#ifndef OPENCV.*?#endif\n", "", text)
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

setup(cmdclass={"build_clcpp": build_clcpp, "build": BuildCommand})
