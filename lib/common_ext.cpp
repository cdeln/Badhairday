#include "common_ext.hpp"
#include <iostream>

void checkFBOStatus()
{
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);                                                
    if( status != GL_FRAMEBUFFER_COMPLETE ) {
        std::string msg;
        switch(status) {   
            case GL_FRAMEBUFFER_UNDEFINED: msg = "GL_FRAME_BUFFER_UNDEFINED"; break;                   
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: msg = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT"; break;     
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: msg = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"; break;
            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: msg = "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER"; break;   
            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: msg = "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER"; break;
            case GL_FRAMEBUFFER_UNSUPPORTED: msg = "GL_FRAMEBUFFER_UNSUPPORTED"; break;                
            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: msg = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE; break";
            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS : msg = "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS"; break;
            default: msg = "Unknown FBO error";                                                            
        }   
        throw gtl::Exception(msg);
    }
}

void initFBO(FBOstruct & fbo, int width, int height) {

    fbo.width = width;
    fbo.height = height;

    // create objects
    glGenFramebuffers(1, &fbo.fb); 
    glBindFramebuffer(GL_FRAMEBUFFER, fbo.fb);
    glGenTextures(1, &fbo.texid);
    glBindTexture(GL_TEXTURE_2D, fbo.texid);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fbo.texid, 0);

    // Renderbuffer
    // initialize depth renderbuffer
    glGenRenderbuffers(1, &fbo.rb);
    glBindRenderbuffer(GL_RENDERBUFFER, fbo.rb);
    glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, fbo.width, fbo.height );
    glFramebufferRenderbuffer( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo.rb );

    checkFBOStatus();

    fprintf(stderr, "Framebuffer object %d\n", fbo.fb);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void bindFBO(GLint fbo) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
}
void bindFBO(FBOstruct & fbo) {
    bindFBO(fbo.fb);
}

/*
void bindAttrib(gtl::shader::Shader & program, gtl::shader::attribute::Attribute * attrib) {
    if( attrib != NULL ) {
        GLint loc = glGetAttribLocation(program, attrib->name.c_str());
        glVertexAttribPointer(loc, attrib->size, attrib->type, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(loc);
    }
}

void drawModel(
        Model * m, 
        gtl::shader::Shader & program,
        gtl::shader::attribute::Attribute * vertex,
        gtl::shader::attribute::Attribute * normal,
        gtl::shader::attribute::Attribute * texCoord) {
    program.use();
    glBindVertexArray(m->vao);
    glBindBuffer(GL_ARRAY_BUFFER, m->vb);
    bindAttrib(program, vertex);
    glBindBuffer(GL_ARRAY_BUFFER, m->nb);
    bindAttrib(program, normal);
    glBindBuffer(GL_ARRAY_BUFFER, m->tb);
    bindAttrib(program, texCoord);
}
*/
