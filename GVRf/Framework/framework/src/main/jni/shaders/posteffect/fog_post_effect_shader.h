//
// Created by j.reynolds on 6/27/2017.
//

#ifndef FRAMEWORK_FOG_POST_EFFECT_SHADER_H
#define FRAMEWORK_FOG_POST_EFFECT_SHADER_H


#include <memory>
#include <vector>

#include "gl/gl_headers.h"
#include "glm/glm.hpp"

#include "objects/hybrid_object.h"


namespace gvr {
    class GLProgram;
    class RenderTexture;
    class PostEffectData;

    class FogPostEffectShader: public HybridObject {
    public:
        FogPostEffectShader();
        virtual ~FogPostEffectShader();

        void render(RenderTexture* render_texture,
                    PostEffectData* post_effect_data,
                    std::vector<glm::vec3>& vertices,
                    std::vector<glm::vec2>& tex_coords,
                    std::vector<unsigned short>& triangles);

    private:
        FogPostEffectShader(
                const FogPostEffectShader& fog_post_effect_shader);
        FogPostEffectShader(
                FogPostEffectShader&& fog_post_effect_shader);
        FogPostEffectShader& operator=(
                const FogPostEffectShader& fog_post_effect_shader);
        FogPostEffectShader& operator=(
                FogPostEffectShader&& fog_post_effect_shader);

    private:
        GLProgram* program_;
        GLuint a_position_;
        GLuint a_tex_coord_;
        GLuint u_texture_;
        // add vertex array object
        GLuint vaoID_;
    };

}


#endif //FRAMEWORK_FOG_POST_EFFECT_SHADER_H
