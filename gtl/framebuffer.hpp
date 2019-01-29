#ifndef __GTL_FRAMEBUFFER__
#define __GTL_FRAMEBUFFER__

#include "gl_include.h"
#include "texture.hpp"

namespace gtl {
    
    template <GLenum TextureTarget>
    class Framebuffer {
        private:
            GLuint fboID; 
        public:
            Texture<TextureTarget> texture;

            void init();
            void bind();
            void unbind();

            GLuint id();

            template <typename First, typename ...Rest>
            void allocate(First first, Rest... rest);

    };

}

#endif
