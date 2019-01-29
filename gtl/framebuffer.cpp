#include "framebuffer.hpp"
#include <iostream>
#include "common.h"

namespace gtl {

    template<GLenum TextureTarget>
        void Framebuffer<TextureTarget>::init() {
            glGenFramebuffers(1, &fboID);
            texture.init();
        }

    template<GLenum TextureTarget>
        void Framebuffer<TextureTarget>::bind() {
            glBindFramebuffer(GL_FRAMEBUFFER, fboID);
        }


    template<GLenum TextureTarget>
        void Framebuffer<TextureTarget>::unbind() {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

    template<GLenum TextureTarget>
        GLuint Framebuffer<TextureTarget>::id() {
            return fboID;
        }

    // Class template instantiations
    template class Framebuffer<GL_TEXTURE_1D>;
    template class Framebuffer<GL_TEXTURE_2D>;
    template class Framebuffer<GL_TEXTURE_3D>;
    template class Framebuffer<GL_TEXTURE_1D_ARRAY>;
    template class Framebuffer<GL_TEXTURE_2D_ARRAY>;

    // Function template specializations
    template<> template<> void Framebuffer<GL_TEXTURE_1D>::allocate <GLsizei> (GLsizei width);
    template<> template<> void Framebuffer<GL_TEXTURE_2D>::allocate <GLsizei, GLsizei> (GLsizei width, GLsizei height);
    template<> template<> void Framebuffer<GL_TEXTURE_3D>::allocate <GLsizei, GLsizei, GLsizei> (GLsizei width, GLsizei height, GLsizei depth);
    template<> template<> void Framebuffer<GL_TEXTURE_1D_ARRAY>::allocate <GLsizei, GLsizei> (GLsizei width, GLsizei depth);

    template<> template<> void Framebuffer<GL_TEXTURE_2D_ARRAY>::allocate
        (GLsizei width, GLsizei height, GLsizei depth) {
            bind();
            texture.allocate(width, height, depth);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture.id(), 0);
        }

    template<> template<> void Framebuffer<GL_TEXTURE_3D>::allocate
        (GLsizei width, GLsizei height, GLsizei depth) {
            bind();
            texture.allocate(width, height, depth);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture.id(), 0);
        }

}
