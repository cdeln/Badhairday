#ifndef __GTL_TEXTURE__
#define __GTL_TEXTURE__

#include "gl_include.h"
#include "exception.hpp"

namespace gtl {

    enum class TextureType : GLenum {
        TEXTURE_1D = GL_TEXTURE_1D,
        TEXTURE_2D = GL_TEXTURE_2D,
        TEXTURE_3D = GL_TEXTURE_3D,
        TEXTURE_1D_ARRAY = GL_TEXTURE_1D_ARRAY,
        TEXTURE_2D_ARRAY = GL_TEXTURE_2D_ARRAY
    };

    template<GLenum Target> 
    class Texture {
        private:
            GLenum _format = GL_UNSIGNED_BYTE;
            bool _allocated = false;
        public:
            void init();
            GLuint texID; 
            GLuint texUnit;
            void bind();
            void bind(GLuint textureUnit);
            void unbind();
            void parameter(GLenum paramKey, GLenum paramVal); 
            void format(GLenum format);
            GLenum format() const;

            GLuint id() const;
            GLuint unit() const;

            template <typename First, typename ...Rest>
            void allocate(First first, Rest... rest);
    };

}

#endif
