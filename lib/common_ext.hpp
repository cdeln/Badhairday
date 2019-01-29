#ifndef __COMMON_EXT__
#define __COMMON_EXT__

#include "common.h"
#include "gl_include.h" 
#include "gtl.hpp"

void checkFBOStatus();

void initFBO(FBOstruct & fbo, int width, int height);
void bindFBO(FBOstruct & fbo);
void bindFBO(GLint fbo);

/*
void drawModel(
        Model * m,
        gtl::shader::Shader & program,
        gtl::shader::attribute::Attribute * vertex,
        gtl::shader::attribute::Attribute * normal,
        gtl::shader::attribute::Attribute * texCoord);
*/

#include "common_ext.tpp"

#endif
