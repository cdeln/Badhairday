#ifndef __GTL__
#define __GTL__

#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <sstream>
#include <vector>
#include "gl_include.h" 
#include "exception.hpp"
#include "texture.hpp"

namespace gtl {

    namespace debug {
        void init();
    }

    // https://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
    /*
       std::string readFile(const std::string & fileName) {
       std::ifstream ifs(fileName);
       std::string str;
       ifs.seekg(0, std::ios::end);
       str.reserve(ifs.tellg());
       ifs.seekg(0, std::ios::beg);
       str.assign(
       (std::istreambuf_iterator<char>(ifs)),
       std::istreambuf_iterator<char>());
       return str;
       }
       */

    namespace shader {

        //    enum class ShaderType : int {
        const int VERT = 0;
        const int FRAG = 1;
        const int GEOM = 2;
        const int TESC = 3;
        const int TESE = 4;
        const int COMP = 5;
        const int MAX_SHADERS = 6;

        const GLenum SHADER_TYPE[MAX_SHADERS] = {
            GL_VERTEX_SHADER,
            GL_FRAGMENT_SHADER,
            GL_GEOMETRY_SHADER,
            GL_TESS_CONTROL_SHADER,
            GL_TESS_EVALUATION_SHADER,
            GL_COMPUTE_SHADER
        };

        const std::string SHADER_NAME[MAX_SHADERS] = {
            "Vertex",
            "Fragment",
            "Geometry",
            "Tesselation control",
            "Tesselation evaluation",
            "Compute",
        };

        // Forward declarations
        /*
        struct Shader;

        namespace attribute {
            namespace uniform {
                struct Uniform; 
            }
        }
        */

        struct Shader {
            std::string programName;
            std::string shaderName[MAX_SHADERS];
            std::string source[MAX_SHADERS];
            bool hasShader[MAX_SHADERS];
            GLuint shader[MAX_SHADERS];
            GLuint program;
            GLenum matrixMode;
            bool initialized;
            // TODO
            //std::vector< attribute::uniform::Uniform * > uniforms;

            Shader();
            GLuint loadShader(int shaderNum);
            GLuint loadProgram();
            void init(GLenum matrixMode = GL_FALSE);
            void use() const;
            // TODO
            //void reset();
            //void check();
            operator GLuint();
            void debug();        
        };

        struct ShaderLocation {
            std::string name;
            GLint location;
            const Shader * shader;
            void init(const std::string & aname, const Shader * program);
        };

        namespace attribute {

            struct Attribute : ShaderLocation {
                //int size;
                //GLenum type;
                //Attribute() {};
                //Attribute(int size, GLenum type) : size(size), type(type) {}
                void init(const std::string & aname, const Shader * shader);
                void debug();
            };

            struct Int : Attribute {};
            struct Float : Attribute {};
            struct Vec2 : Attribute {};
            struct Vec3 : Attribute {
                //Vec3() : Attribute(3, GL_FLOAT) {} 
            };
            struct Vec4 : Attribute {};

        }

        namespace uniform {

            struct Uniform : ShaderLocation {
                void init(const std::string & aname, const Shader * program);
                // TODO
                //void reset();
                //void check();
            };

            struct Scalar : Uniform { };
            struct Vector : Uniform {
                virtual void operator=(const GLfloat * val) = 0;
            };
            struct Matrix : Uniform {
                GLenum matrixMode;
                virtual void operator=(const GLfloat * val) = 0;
                void init(const std::string & name, const Shader * shader);
            };


            struct Bool : Scalar {
                void operator=(const bool val);
            };
            struct Int : Scalar {
                void operator=(const GLint val);
            };
            struct Float : Scalar {
                void operator=(const GLfloat val);
            };

            struct Vec2 : Vector { 
                void operator=(const GLfloat * val);
                void operator=(const glm::vec2 & val);
            }; 
            struct Vec3 : Vector { 
                void operator=(const GLfloat * val);
                void operator=(const glm::vec3 & val);
            }; 
            struct Vec4 : Uniform {
                void operator=(const GLfloat * val);
                void operator=(const glm::vec4 & val);
            }; 


            struct Mat2 : Matrix { 
                void operator=(const GLfloat * val);
                void operator=(const glm::mat2 & val); 
            };
            struct Mat3 : Matrix {
                void operator=(const GLfloat * val);
                void operator=(const glm::mat4 & val); 
                void operator=(const glm::mat3 & val); 
            };
            struct Mat4 : Matrix {
                void operator=(const GLfloat * val);
                void operator=(const glm::mat4 & val); 
                void init(const std::string & name, const Shader * shader); 
            };

            struct Sampler2D : Uniform {
                void operator=(const GLint val); 
                void operator=(const Texture<GL_TEXTURE_2D> & tex);
            };
            struct Sampler3D : Uniform {
                void operator=(const GLint val); 
                void operator=(const Texture<GL_TEXTURE_3D> & tex);
            };
            struct Usampler3D : Uniform {
                void operator=(const GLint val); 
                void operator=(const Texture<GL_TEXTURE_3D> & tex);
            };
            struct Sampler2Darray : Uniform {
                void operator=(const GLint val); 
                void operator=(const Texture<GL_TEXTURE_2D_ARRAY> & tex);
            };

            struct Image2D : Uniform {
                void operator=(const GLint val); 
                void operator=(const Texture<GL_TEXTURE_2D> & tex);
            };
            struct Image3D : Uniform {
                void operator=(const GLint val); 
                void operator=(const Texture<GL_TEXTURE_3D> & tex);
            };

        }

    }

    }

#endif
