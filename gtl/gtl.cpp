#include "gtl.hpp"

namespace gtl {

#define STRING_OPTION(X) case X: return #X; break
#define BOOL_OPTION(X) case X: return true; break


    namespace debug {

        bool messageTypeIsError(GLenum type) {
            switch(type) {
                BOOL_OPTION(GL_DEBUG_TYPE_ERROR);
            }
            return false; 
        }

        std::string sourceEnumString(GLenum source) {
            switch(source) {
                STRING_OPTION(GL_DEBUG_SOURCE_API);
                STRING_OPTION(GL_DEBUG_SOURCE_WINDOW_SYSTEM);
                STRING_OPTION(GL_DEBUG_SOURCE_SHADER_COMPILER);
                STRING_OPTION(GL_DEBUG_SOURCE_THIRD_PARTY);
                STRING_OPTION(GL_DEBUG_SOURCE_APPLICATION);
                STRING_OPTION(GL_DEBUG_SOURCE_OTHER);
            }
            return "UNKNOWN_SOURCE";
        }

        std::string typeEnumString(GLenum type) {
            switch(type) {
                STRING_OPTION(GL_DEBUG_TYPE_ERROR);
                STRING_OPTION(GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR);
                STRING_OPTION(GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR);
                STRING_OPTION(GL_DEBUG_TYPE_PORTABILITY);
                STRING_OPTION(GL_DEBUG_TYPE_PERFORMANCE);
                STRING_OPTION(GL_DEBUG_TYPE_MARKER);
                STRING_OPTION(GL_DEBUG_TYPE_PUSH_GROUP);
                STRING_OPTION(GL_DEBUG_TYPE_POP_GROUP);
                STRING_OPTION(GL_DEBUG_TYPE_OTHER);
            }
            return "UNKNOWN_TYPE";
        }

        std::string severityEnumString(GLenum severity) {
            switch(severity) {
                STRING_OPTION(GL_DEBUG_SEVERITY_HIGH);
                STRING_OPTION(GL_DEBUG_SEVERITY_MEDIUM);
                STRING_OPTION(GL_DEBUG_SEVERITY_LOW);
                STRING_OPTION(GL_DEBUG_SEVERITY_NOTIFICATION);
            }
            return "UNKNOWN_SEVERITY";
        }

        void messageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
            //std::cout << "source: " << source << ", type: " << type  << ", severity: " << severity << ", length: " << length << std::endl;
            if( messageTypeIsError(type) ) { 
                std::stringstream ss;
                ss  << "OpenGL error!\n\t"
                    << "Source: " << sourceEnumString(source) << "\n\t"
                    << "Type: " << typeEnumString(type) << "\n\t"
                    << "Severity: " << severityEnumString(severity) << "\n\t"
                    << "Message: " << message;
                throw Exception(ss.str());
            }
        }

        void init() {
            glDebugMessageCallback(messageCallback, NULL);
        }

    }

    namespace shader {
        // SHADER
        Shader::Shader() : initialized(false) {
            for(int i = 0; i < MAX_SHADERS; ++i) {
                hasShader[i] = false;
            }
        }

        void Shader::init(GLenum _matrixMode) { 
            matrixMode = _matrixMode;
            for(int i = 0; i < MAX_SHADERS; ++i) {
                if( hasShader[i] ) {
                    shader[i] = loadShader(i);
                }
            }
            //std::cout << shaderName[VERT] << ": Linking program" << std::endl;
            program = loadProgram();
            initialized = true;
        }

        void Shader::debug() {
            use();
            GLint count;
            GLint size;
            GLchar name[200];
            GLenum type;
            glGetProgramiv(program, GL_ACTIVE_ATTRIBUTES, &count);

            std::cout << "Num of attribs: " << count << std::endl;
            for(int i = 0; i < count; i++){
                glGetActiveAttrib(program, (GLuint)i, 200, NULL, &size, &type, name);

                printf("Shader name: %s, Name: %s, Type: %u \n", shaderName[VERT].c_str(), name, type);
            }
        }

        void Shader::use() const {
            if( ! initialized ) {
                throw Exception("Shader \"" + programName + "\" not initialized!");
            }
            glUseProgram( program );
        }

        /*
        void Shader::reset() {
            for(std::vector<attribute::uniform::Uniform * >::iterator uniform = uniforms.begin();
                    uniform != uniforms.end();
                    ++uniform) {
                (*uniform)->reset();
            }
        }

        void Shader::check() {
            for(auto uniform : uniforms ) {
                uniform->check();
            }
        }
        */

        Shader::operator GLuint() {
            return program;
        }

        GLuint Shader::loadShader(int shaderNum) {
            GLuint shader = glCreateShader(SHADER_TYPE[shaderNum]);
            const GLchar * c_source = source[shaderNum].c_str();
            glShaderSource(shader, 1, &c_source, nullptr);
            glCompileShader(shader);
            GLint success = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
            if( success == GL_FALSE ) {
                GLint logSize = 0; 
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logSize); 
                std::vector<GLchar> infoLog(logSize);
                glGetShaderInfoLog(shader, logSize, &logSize, &infoLog[0]);
                infoLog.resize(logSize);
                std::string infoStr(infoLog.begin(), infoLog.end());

                std::stringstream ss;
                ss << "Compile error from program: " << shaderName[shaderNum] << ", shader: " << SHADER_NAME[shaderNum] << std::endl << infoStr; 
                infoStr = ss.str();
                glDeleteShader(shader);
                throw Exception(infoStr);
            }
            return shader;
        }

        GLuint Shader::loadProgram() {
            GLuint program = glCreateProgram();
            for(int i = 0; i < MAX_SHADERS; ++i) {
                if( hasShader[i] ) {
                    glAttachShader(program, shader[i]);
                }
            }
            glLinkProgram(program);
            GLint success = 0;
            glGetProgramiv(program, GL_LINK_STATUS, &success);
            if( success == GL_FALSE ) {
                GLint logSize = 0; 
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logSize); 
                std::vector<GLchar> infoLog(logSize);
                glGetProgramInfoLog(program, logSize, &logSize, &infoLog[0]);
                infoLog.resize(logSize);
                std::string infoStr(infoLog.begin(), infoLog.end());

                std::stringstream ss;
                ss << "Link error from " << programName << "\n" << infoStr; 
                infoStr = ss.str();
                throw Exception(infoStr);
            }
            return program;
        }

        // SHADER LOCATION
        void ShaderLocation::init(const std::string & aname, const Shader * ashader) {
            name = aname;
            shader = ashader;
            glUseProgram(shader->program);
        }

        // ATTRIBUTE
        namespace attribute {
            void Attribute::init(const std::string & aname, const Shader * shader) { 
                ShaderLocation::init(aname, shader); 
                location = glGetAttribLocation(shader->program, name.c_str()); 
                //if (location < 0){
                // TODO
                //throw Exception(infoStr);
                //}
            }

            void Attribute::debug() {
                std::stringstream ss;
                ss << "Attribute error:\n name: " << name << " Location: " << location << 
                    "\n Shader name: " << shader->shaderName[VERT] << 
                    "\n Has shader: " << shader->hasShader[VERT] <<
                    "\n Has shader source: " << shader->source[VERT] <<
                    "\n Program: " << shader->program <<
                    "\n Initialized: " << shader->initialized;  
                std::string infoStr = ss.str();
                std::cout << infoStr;
            }
        }

        // UNIFORM
        namespace uniform {
            void Uniform::init(const std::string & name, const Shader * shader) {
                ShaderLocation::init(name, shader); 
                location = glGetUniformLocation(shader->program, name.c_str());
            }
            /*
            void Uniform::reset() {

            }
            void Uniform::check() {

            }
            */

            void Matrix::init(const std::string & name, const Shader * shader) {
                Uniform::init(name, shader);
                matrixMode = shader->matrixMode;
            }

            void Bool::operator=(const bool val) {
                glUniform1i(location, val);
            }
            void Int::operator=(const GLint val) {
                glUniform1i(location, val);
            }
            void Float::operator=(const GLfloat val) {
                glUniform1f(location, val);
            }

            void Vec2::operator=(const GLfloat * val) {
                glUniform2fv(location, 1, val);
            }
            void Vec2::operator=(const glm::vec2 & val) {
                glUniform2fv(location, 1, &val[0]);
            }

            void Vec3::operator=(const GLfloat * val) {
                glUniform3fv(location, 1, val);
            }
            void Vec3::operator=(const glm::vec3 & val) {
                glUniform3fv(location, 1, &val[0]);
            }

            void Vec4::operator=(const GLfloat * val) {
                glUniform4fv(location, 1, val);
            }
            void Vec4::operator=(const glm::vec4 & val) {
                glUniform4fv(location, 1, &val[0]);
            }


            void Mat2::operator=(const GLfloat * val) {
                glUniformMatrix2fv(location, 1, matrixMode, val);
            }
            void Mat2::operator=(const glm::mat2 & val) {
                glUniformMatrix2fv(location, 1, matrixMode, &val[0][0]);
            }

            void Mat3::operator=(const GLfloat * val) {
                glUniformMatrix3fv(location, 1, matrixMode, val);
            }
            void Mat3::operator=(const glm::mat3 & val) {
                glUniformMatrix3fv(location, 1, matrixMode, &val[0][0]);
            }

            void Mat4::operator=(const GLfloat * val) {
                glUniformMatrix4fv(location, 1, matrixMode, val);
            }
            void Mat4::operator=(const glm::mat4 & val) {
                glUniformMatrix4fv(location, 1, matrixMode, &val[0][0]);
            }

            void Mat4::init(const std::string & name, const Shader * shader) {
                Matrix::init(name, shader);
            }

            void Sampler2D::operator=(const GLint val) {
                glUniform1i(location, val);
            }
            void Sampler2D::operator=(const Texture<GL_TEXTURE_2D> & tex) {
                glUniform1i(location, tex.unit()); 
            } 

            void Sampler3D::operator=(const GLint val) {
                glUniform1i(location, val);
            }
            void Sampler3D::operator=(const Texture<GL_TEXTURE_3D> & tex) {
                glUniform1i(location, tex.unit()); 
            } 

            void Usampler3D::operator=(const GLint val) {
                glUniform1i(location, val);
            }
            void Usampler3D::operator=(const Texture<GL_TEXTURE_3D> & tex) {
                glUniform1i(location, tex.unit()); 
            } 

            void Sampler2Darray::operator=(const GLint val) {
                glUniform1i(location, val);
            }
            void Sampler2Darray::operator=(const Texture<GL_TEXTURE_2D_ARRAY> & tex) {
                glUniform1i(location, tex.unit()); 
            } 

            void Image2D::operator=(const GLint val) {
                glUniform1i(location, val);
            }
            void Image2D::operator=(const Texture<GL_TEXTURE_2D> & tex) {
                glUniform1i(location, tex.unit());
            }
            void Image3D::operator=(const GLint val) {
                glUniform1i(location, val);
            }
            void Image3D::operator=(const Texture<GL_TEXTURE_3D> & tex) {
                glUniform1i(location, tex.unit());
            }
        }

    }
}
