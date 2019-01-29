#include "texture.hpp"
#include <iostream>

namespace gtl {

template<GLenum TextureTarget>
void Texture<TextureTarget>::init() {
    glGenTextures(1, &texID);
    bind();
    glTexParameteri(TextureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(TextureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    //glTexParameteri(TextureTarget, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(TextureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(TextureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    unbind();
}

template<GLenum TextureTarget> 
void Texture<TextureTarget>::bind() {
    glBindTexture(TextureTarget, texID); 
}
            
template<GLenum TextureTarget>
void Texture<TextureTarget>::bind(GLuint textureUnit) {
    texUnit = textureUnit;
    glActiveTexture(GL_TEXTURE0 + texUnit);
    bind();
}

template<GLenum TextureTarget>
void Texture<TextureTarget>::unbind() {
    glBindTexture(TextureTarget, 0);
}

template<GLenum TextureTarget>
void Texture<TextureTarget>::parameter(GLenum paramKey, GLenum paramVal) {
    glTexParameteri(TextureTarget, paramKey, paramVal);
}

template<GLenum TextureTarget>
GLuint Texture<TextureTarget>::unit() const {
    return texUnit;
}

template<GLenum TextureTarget>
GLuint Texture<TextureTarget>::id() const {
    return texID;
}

template<GLenum Target>
void Texture<Target>::format(GLenum format) {
    if( _allocated ) {
        throw Exception("Trying to set texture format on allocated texture, the texture will not be reallocated so set format before allocating!");
    }
    _format = format;
}

template<GLenum Target>
GLenum Texture<Target>::format() const {
    return _format;
}

// Function template specializations
template<> template<>
void Texture<GL_TEXTURE_1D>::allocate<GLsizei>(GLsizei width) {
    bind();
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32F, width, 0, GL_RGBA, format(), nullptr);
}

template<> template<>
void Texture<GL_TEXTURE_2D>::allocate<GLsizei, GLsizei>(GLsizei width, GLsizei height) {
    bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, format(), nullptr);
}

template<> template<>
void Texture<GL_TEXTURE_3D>::allocate<GLsizei, GLsizei, GLsizei>(GLsizei width, GLsizei height, GLsizei depth) {
    bind();
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, width, height, depth, 0, GL_RGBA, format(), nullptr);
}

template<> template<>
void Texture<GL_TEXTURE_1D_ARRAY>::allocate<GLsizei, GLsizei>(GLsizei width, GLsizei depth) {
    bind();
    glTexImage2D(GL_TEXTURE_1D_ARRAY, 0, GL_RGBA32F, width, depth, 0, GL_RGBA, format(), nullptr);
}

template<> template<>
void Texture<GL_TEXTURE_2D_ARRAY>::allocate<GLsizei, GLsizei, GLsizei>(GLsizei width, GLsizei height, GLsizei depth) {
    bind();
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA32F, width, height, depth, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
}


// Class template instantiations
template class Texture<GL_TEXTURE_1D>;
template class Texture<GL_TEXTURE_2D>;
template class Texture<GL_TEXTURE_3D>;
template class Texture<GL_TEXTURE_1D_ARRAY>;
template class Texture<GL_TEXTURE_2D_ARRAY>;

}
