#ifndef __COMMON_EXT_TPP__
#define __COMMON_EXT_TPP__

#include <iostream>

/*
GLuint getFBO(const GLuint & x) {
    return x;
}

GLuint getFBO(const int & x) {
    return static_cast<GLuint>(x);
}

GLuint getFBO(const FBOstruct & fbo) {
    return fbo.fb;
}

template<class FBO>
void bindFBO(const FBO & fbo) {
    glBindFramebuffer(GL_FRAMEBUFFER, getFBO(fbo));
}
*/

template <int texUnit, class First, class ... Rest>
void bindTextures(First & fbo, Rest & ... rest) {
    glActiveTexture(GL_TEXTURE0 + texUnit);
    fbo.bindTexture();
    bindTextures<texUnit + 1, Rest ...>(rest...);
    std::cout << "bindTextures, texUnit = " << texUnit << std::endl;
}

template <int texUnit, class First>
void bindTextures(First & fbo) { 
    glActiveTexture(GL_TEXTURE0 + texUnit);
    std::cout << "bindTextures, texUnit = " << texUnit << std::endl;
}

#endif
