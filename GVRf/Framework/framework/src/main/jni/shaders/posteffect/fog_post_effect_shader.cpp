//
// Created by j.reynolds on 6/27/2017.
//


/***************************************************************************
 * Renders a post effect which creates a fog effect throughout the scene
 ***************************************************************************/

#include "fog_post_effect_shader.h"

#include "gl/gl_program.h"
#include "objects/post_effect_data.h"
#include "objects/textures/render_texture.h"
#include "util/gvr_log.h"

namespace gvr {
    static const char VERTEX_SHADER[] = "attribute vec3 a_position;\n"
            "attribute vec2 a_texcoord;\n"
            "varying vec2 v_tex_coord;\n"
            "varying float distance;\n"
            "void main() {\n"
            "  distance = length(a_position);\n"
            "  v_tex_coord = a_texcoord.xy;\n"
            "  gl_Position = vec4(a_position.xyz, 1);\n"
            "}\n";

    static const char FRAGMENT_SHADER[] = "precision highp float;\n"
            "uniform sampler2D u_texture;\n"
            "varying vec2 v_tex_coord;\n"
            "varying float distance;\n"
            "const float b = 0.5f;\n"
            "void main() {\n"
            "  vec3 color = texture2D(u_texture, v_tex_coord).rgb;\n"
            "  float fogAmount = 1.0 - exp( -distance*b );\n"
            "  vec3  fogColor  = vec3(0.5,0.6,0.7);\n"
            "  gl_FragColor = vec4(mix( color, fogColor, fogAmount), 1.0f);\n"
            "}\n";

    FogPostEffectShader::FogPostEffectShader() :
            program_(0), a_position_(0), a_tex_coord_(0), u_texture_(0) {
        program_ = new GLProgram(VERTEX_SHADER, FRAGMENT_SHADER);
        a_position_ = glGetAttribLocation(program_->id(), "a_position");
        a_tex_coord_ = glGetAttribLocation(program_->id(), "a_texcoord");
        u_texture_ = glGetUniformLocation(program_->id(), "u_texture");
        vaoID_ = 0;
    }

    FogPostEffectShader::~FogPostEffectShader() {
        delete program_;
        if (vaoID_ != 0) {
            GL(glDeleteVertexArrays(1, &vaoID_));
        }
    }

    void FogPostEffectShader::render(
            RenderTexture* render_texture,
            PostEffectData* post_effect_data,
            std::vector<glm::vec3>& vertices, std::vector<glm::vec2>& tex_coords,
            std::vector<unsigned short>& triangles) {

        glUseProgram(program_->id());

        GLuint tmpID;

        if(vaoID_ == 0)
        {
            glGenVertexArrays(1, &vaoID_);
            glBindVertexArray(vaoID_);

            glGenBuffers(1, &tmpID);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tmpID);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short)*triangles.size(), &triangles[0], GL_STATIC_DRAW);

            if (vertices.size())
            {
                glGenBuffers(1, &tmpID);
                glBindBuffer(GL_ARRAY_BUFFER, tmpID);
                glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3)*vertices.size(), &vertices[0], GL_STATIC_DRAW);
                glEnableVertexAttribArray(a_position_);
                glVertexAttribPointer(a_position_, 3, GL_FLOAT, 0, 0, 0);
            }

            if (tex_coords.size())
            {
                glGenBuffers(1, &tmpID);
                glBindBuffer(GL_ARRAY_BUFFER, tmpID);
                glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*tex_coords.size(), &tex_coords[0], GL_STATIC_DRAW);
                glEnableVertexAttribArray(a_tex_coord_);
                glVertexAttribPointer(a_tex_coord_, 2, GL_FLOAT, 0, 0, 0);
            }
        }

        glActiveTexture (GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, render_texture->getId());
        glUniform1i(u_texture_, 0);

        glBindVertexArray(vaoID_);
        glDrawElements(GL_TRIANGLES, triangles.size(), GL_UNSIGNED_SHORT, 0);
        glBindVertexArray(0);

        checkGLError("FogPostEffectShader::render");
    }
}
