#!/usr/bin/env python3
import sys
import os
import yaml

def create_makefile(programs, build_location):
    program_paths = []
    makefile = ""
    for program_name, shaders in programs.items():
        for shader_type, shader_name in shaders.items():
            shaders[shader_type] = os.path.abspath(shader_name)
        program_fn = program_name + ".hpp"
        program_path = os.path.abspath(os.path.join(build_location, program_fn))
        program_paths.append(program_path)
        dep = program_path + ": " + " ".join(shaders.values())
        makefile += dep + "\n\t" + os.path.abspath("./sbt.py") 
        makefile += " " + program_path 
        makefile += " --program " + program_name 
        for shader_type, shader_name in shaders.items():
            makefile += " --" + shader_type + " " + shader_name
        makefile += "\n"
    makefile = "all: " + " ".join(program_paths) + "\n" + makefile
    return makefile

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Invalid number of args, usage: ./sbt <program_definition_file> <build_dir>",file=sys.stderr)
        sys.exit(1)

    programs_file = sys.argv[1]
    build_dir = sys.argv[2]

    programs_dir = os.path.join(build_dir, "programs")
    with open(programs_file, "r") as f:
        programs = yaml.load(f)
    makefile = create_makefile(programs, programs_dir) 
    with open(os.path.join(build_dir, "makefile"), "w") as f:
        f.write(makefile)

    # Create utility include header 
    with open(os.path.join(programs_dir, "programs.hpp"), "w") as f:
        s = ""
        for program in programs:
            s += "#include \"" + program + ".hpp\"\n"
        f.write(s)
