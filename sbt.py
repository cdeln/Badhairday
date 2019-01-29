#!/usr/bin/env python3

"""
Shader Built Tool (SBT)
Builds GLSL shader into C++ class
"""

import sys
import os
import re
import argparse

BASE_SHADER = "Shader"
SHADER_TYPES = ["VERT", "FRAG", "GEOM", "TESC", "TESE", "COMP", "MIXED"]
INCLUDES = ["\"gtl/gtl.hpp\""]
NAMESPACES = ["gtl", "shader"]
SHADER_INIT_ARGS = "matrixMode"
SHADER_INIT_DEF = "GLenum matrixMode = GL_FALSE"

class ShaderLocation:
    def __init__(self, atype, aname):
        self.type = atype.lower().title()
        self.name = aname

class Attribute(ShaderLocation):
    def __init__(self, atype, aname):
        super().__init__(atype, aname)
    def __str__(self):
        return "attribute::" + self.type + " " + self.name
    def __eq__(self, other):
        return self.type == other.type and self.name == other.name
    def __hash__(self):
        return hash(self.type + self.name)

class Uniform(ShaderLocation):
    def __init__(self, atype, aname):
        super().__init__(atype, aname)
    def __str__(self):
        return "uniform::" + self.type + " " + self.name
    def __eq__(self, other):
        return self.type == other.type and self.name == other.name
    def __hash__(self):
        return hash(self.type + self.name)

class Define:
    def __init__(self, name, what): 
        self.name = name
        self.what = what
    def __str__(self):
        return "#define " + self.name + " " + self.what

class Shader:
    def __init__(self, name, fields, defines, source, shader_names): 
        self.name = name
        self.fields = fields
        self.defines = defines
        self.source = source
        self.shader_names = shader_names
    def __str__(self):
        source_code = ""
        # begin header guards
        source_code += "#ifndef __" + self.name + "__\n"
        source_code += "#define __" + self.name + "__\n"
        # includes
        for include in INCLUDES:
            source_code += "#include " + include + "\n"
        # defines, skip for now (pollutes namespace)
        #for define in self.defines:
        #    source_code += str(define) + "\n"
        # begin namespace
        for namespace in NAMESPACES: 
            source_code += "namespace " + namespace + "{\n" 
        # begin struct
        source_code += "\tstruct " + self.name + " : " + BASE_SHADER + " {\n"
        # fields
        for x in self.fields:
            source_code += "\t\t" + str(x) + ";\n"
        # begin constructor
        source_code += "\t\t" + self.name + "() : " + BASE_SHADER + "() {\n"
        source_code += "\t\t\tprogramName = \"" + self.name + "\";\n" 
        for key,val in self.source.items():
            source_code += "\t\t\thasShader["+key+"] = true;\n"
            source_code += "\t\t\tshaderName["+key+"] = \"" + self.shader_names[key] + "\";\n"
            source_code += "\t\t\tsource["+key+"] = \n"
            for line in val.split("\n"):
                source_code += "\t\t\t\t\""+line+"\\n\"\n"
            source_code += "\t\t\t;\n"
        source_code += "\t\t}\n"
        # end constructor
        # init function
        source_code += "\t\tvoid init(" + SHADER_INIT_DEF + ") {\n"
        source_code += "\t\t\t" + BASE_SHADER + "::init(" + SHADER_INIT_ARGS + ");\n"
        for field in self.fields:
            #if isinstance(field, Uniform):
            source_code += "\t\t\t" + field.name + ".init(\"" + field.name + "\", this);\n"
        source_code += "\t\t}\n"
        # end struct
        source_code += "\t};\n"
        # end namespace
        for namespace in NAMESPACES:
            source_code += "}\n"
        # end header guards
        source_code += "#endif"
        return source_code

def flatten(l):
    return [item for sublist in l for item in sublist]

def parse_lines(shader_source):
    lines = shader_source.split(";")
    lines = flatten([x.split("\n") for x in lines])
    lines = [re.sub(r"[^\S ]+", "", x) for x in lines]
    lines = [re.sub(r"[ ]+", " ", x) for x in lines if x]
    return lines

def parse_field_name(field_name):
    index = field_name.find("[")
    if index != -1:
        return field_name[:index]
    return field_name

def find_qualifier(tokens):
    for index, token in enumerate(tokens):
        if token in ["in", "out", "uniform"]:
            return index
    return -1

def parse_tokens(tokens):
    if len(tokens) > 0:
        if tokens[0] == "layout":
            index = find_qualifier(tokens)
            if index != -1:
                tokens = tokens[index:]
    if len(tokens) == 3:
        token_type = tokens[0]
        if token_type in ["in", "uniform"]:
            field_type = tokens[1]
            field_name = parse_field_name(tokens[2])
            if token_type == "in":
                return Attribute(field_type, field_name)
            elif token_type == "uniform":
                return Uniform(field_type, field_name)
        elif token_type == "#define":
            return Define(tokens[1], tokens[2])

def parse_shader(shader_source):
    fields = []
    preamble = []
    lines = parse_lines(shader_source)
    for line in lines:
        tokens = line.split()
        parsed = parse_tokens( tokens )
        if parsed:
            if isinstance(parsed, Attribute) or isinstance(parsed, Uniform):
                fields.append( parsed ) 
            else:
                preamble.append( parsed ) 
    return fields, preamble

def get_closing_bracket(source, index):
    next_bracket_begin = source.find("{", index + 1) 
    next_bracket_end = source.find("}", index + 1)
    if next_bracket_end == -1:
        raise Exception("Tried to parse shader source but could not find closing bracket")
    if next_bracket_begin != -1:
        if next_bracket_begin < next_bracket_end:
            index = get_closing_bracket(source, next_bracket_begin)
            return get_closing_bracket(source, index)
        else:
            return next_bracket_end
    else:
        return next_bracket_end

def parse_sub_shaders(shader_file):
    sub_shaders = {}
    with open(shader_file, "r") as f:
        shader_source = f.read()
        sub_shader_begin = 0
        sub_shader_end = 0
        while sub_shader_end != -1: 
            sub_shader_begin = shader_source.find("@", sub_shader_end)
            sub_shader_end = shader_source.find("@", sub_shader_begin+1)
            if sub_shader_begin == -1:
                raise Exception("Tried to parse sub-shader, but found not sub-shader token \"@\"")
            sub_shader_source = shader_source[sub_shader_begin:sub_shader_end]
            #newline = sub_shader_source.find("\n")
            bracket_begin = sub_shader_source.find("{")
            bracket_end = get_closing_bracket(sub_shader_source, bracket_begin)
            #sub_shader_type = sub_shader_source[1:newline].upper()
            #sub_shader_source = sub_shader_source[newline:]
            sub_shader_type = sub_shader_source[1:bracket_begin].strip().upper()
            sub_shader_source = sub_shader_source[bracket_begin+1:bracket_end]
            sub_shaders[sub_shader_type] = sub_shader_source
    return sub_shaders

def create_program(program_name, shader_source):
    shader_names = {}
    shader_source = {}
    shader_fields = set() 
    shader_defines = set() 
    for shader_type, shader_file in shader_files.items():
        if shader_file:
            shader_type = shader_type.upper()
            if not shader_type in SHADER_TYPES:
                raise Exception("Building program " + program_name + " failed, invalid shader type " + shader_type)
            if shader_type == "MIXED":
                sub_shaders = parse_sub_shaders(shader_file)
                for sub_shader_type, sub_shader_source in sub_shaders.items():
                    shader_names[sub_shader_type] = shader_file 
                    shader_source[sub_shader_type] = sub_shader_source 
                    new_shader_fields, new_shader_defines = parse_shader(shader_source[sub_shader_type])
                    for new_field in new_shader_fields:
                        shader_fields.add(new_field)
                    for new_define in new_shader_defines:
                        shader_defines.add(new_define)
                    #shader_fields += new_shader_fields
                    #shader_defines += new_shader_defines
            else:
                shader_names[shader_type] = shader_file 
                with open(shader_file, "r") as f:
                    shader_source[shader_type] = f.read()
                    new_shader_fields, new_shader_defines = parse_shader(shader_source[shader_type])
                    for new_field in new_shader_fields:
                        shader_fields.add(new_field)
                    for new_define in new_shader_defines:
                        shader_defines.add(new_define)
                    #shader_fields += new_shader_fields
                    #shader_defines += new_shader_defines
    return Shader( program_name, shader_fields, shader_defines, shader_source, shader_names )

if __name__ == '__main__':
#    if len(sys.argv) != 3:
#        print("""Invalid number of args, usage: ./sbt <program_definition_file> <program_output_dir>""",
#              file=sys.stderr)
    #if len(sys.argv) < 3:
    #    print("Not enough input args, usage: ./sbt.py <program_name> <shader_input_files>", file=sys.stderr)
    #    sys.exit(1)


    parser = argparse.ArgumentParser()
    parser.add_argument("output", help="Output file")
    parser.add_argument("--program", help="Program name")
    parser.add_argument("--vert", help="Vertex shader")
    parser.add_argument("--frag", help="Fragment shader")
    parser.add_argument("--geom", help="Geometry shader")
    parser.add_argument("--tesc", help="Tesselation control shader")
    parser.add_argument("--tese", help="Tesselation evaluation shader")
    parser.add_argument("--comp", help="Compute shader")
    parser.add_argument("--mixed", help="SBT mixed format shader")
    args = parser.parse_args()

    #if not args.vert:
    #    print("Must provide vertex shader source", file=sys.stderr)
    #    sys.exit(1)
    #if not args.frag:
    #    print("Must provide fragment shader source", file=sys.stderr)
    #    sys.exit(2)

    shader_files = dict(args.__dict__)
    del shader_files["program"]
    del shader_files["output"]
    program = create_program(args.program, shader_files)

    with open(args.output, "w") as f:
        f.write(str(program))
