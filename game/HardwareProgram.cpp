/*
 * Copyright (c) 2020, Stanislav Vorobiov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "HardwareProgram.h"
#include "Logger.h"
#include "af3d/Assert.h"

namespace af3d
{
    static const VariableTypeInfo tiInt(GL_INT, 1, 4);
    static const VariableTypeInfo tiFloat(GL_FLOAT, 1, 4);
    static const VariableTypeInfo tiFloatVec2(GL_FLOAT, 2, 8);
    static const VariableTypeInfo tiFloatVec3(GL_FLOAT, 3, 12);
    static const VariableTypeInfo tiFloatVec4(GL_FLOAT, 4, 16);
    static const VariableTypeInfo tiUnsignedInt8Vec4(GL_UNSIGNED_BYTE, 4, 4);
    static const VariableTypeInfo tiFloatMat4(GL_FLOAT, 16, 4 * 16);

    static const std::unordered_map<std::string, VertexAttribName> staticVertexAttribMap = {
        {"pos", VertexAttribName::Pos},
        {"texCoord", VertexAttribName::UV},
        {"normal", VertexAttribName::Normal},
        {"color", VertexAttribName::Color},
        {"tangent", VertexAttribName::Tangent},
        {"bitangent", VertexAttribName::Bitangent},
    };

    static const GLint staticVertexAttribLocations[static_cast<int>(VertexAttribName::Max) + 1] = {
        0,
        1,
        2,
        3,
        4,
        5
    };

    static const std::unordered_map<std::string, UniformName> staticUniformMap = {
        {"viewProj", UniformName::ViewProjMatrix},
        {"modelViewProj", UniformName::ModelViewProjMatrix},
        {"model", UniformName::ModelMatrix},
        {"prevStableMVP", UniformName::PrevStableMatrix},
        {"curStableMVP", UniformName::CurStableMatrix},
        {"eyePos", UniformName::EyePos},
        {"lightPos", UniformName::LightPos},
        {"lightColor", UniformName::LightColor},
        {"lightDir", UniformName::LightDir},
        {"lightCutoffCos", UniformName::LightCutoffCos},
        {"lightCutoffInnerCos", UniformName::LightCutoffInnerCos},
        {"lightPower", UniformName::LightPower},
        {"viewportSize", UniformName::ViewportSize},
        {"time", UniformName::Time},
        {"specularCMLevels", UniformName::SpecularCMLevels},
        {"dt", UniformName::Dt},
        {"realDt", UniformName::RealDt},
        {"mainColor", UniformName::MainColor},
        {"specularColor", UniformName::SpecularColor},
        {"shininess", UniformName::Shininess},
        {"gridPos", UniformName::GridPos},
        {"gridRight", UniformName::GridRight},
        {"gridUp", UniformName::GridUp},
        {"gridStep", UniformName::GridStep},
        {"gridXColor", UniformName::GridXColor},
        {"gridYColor", UniformName::GridYColor},
        {"roughness", UniformName::Roughness},
        {"mipLevel", UniformName::MipLevel},
        {"gaussianKernel[0]", UniformName::GaussianKernel},
        {"gaussianOffset[0]", UniformName::GaussianOffset},
        {"gaussianMSize", UniformName::GaussianMSize},
        {"gaussianDir", UniformName::GaussianDir},
        {"threshold", UniformName::Threshold},
        {"strength", UniformName::Strength}
    };

    static const std::unordered_map<std::string, SamplerName> staticSamplerMap = {
        {"texMain", SamplerName::Main},
        {"texNormal", SamplerName::Normal},
        {"texSpecular", SamplerName::Specular},
        {"texNoise", SamplerName::Noise},
        {"texRoughness", SamplerName::Roughness},
        {"texMetalness", SamplerName::Metalness},
        {"texIrradiance", SamplerName::Irradiance},
        {"texSpecularCM", SamplerName::SpecularCM},
        {"texSpecularLUT", SamplerName::SpecularLUT},
        {"texPrev", SamplerName::Prev}
    };

    GLint VariableInfo::sizeInBytes() const
    {
        return HardwareProgram::getTypeInfo(type).sizeInBytes * count;
    }

    HardwareProgram::HardwareProgram(HardwareResourceManager* mgr)
    : HardwareResource(mgr)
    {
    }

    HardwareProgram::~HardwareProgram()
    {
        GLuint id = id_;
        if (id != 0) {
            std::vector<GLuint> shaderIds;
            for (const auto& p : shaders_) {
                shaderIds.push_back(p.first);
            }
            cleanup([id, shaderIds](HardwareContext& ctx) {
                for (auto sId : shaderIds) {
                    ogl.DetachShader(id, sId);
                }
                ogl.DeleteProgram(id);
            });
        } else {
            cleanup();
        }
    }

    GLint HardwareProgram::getVertexAttribLocation(VertexAttribName name)
    {
        return staticVertexAttribLocations[static_cast<int>(name)];
    }

    const VariableTypeInfo& HardwareProgram::getTypeInfo(GLenum type)
    {
        switch (type) {
        case GL_INT: return tiInt;
        case GL_FLOAT: return tiFloat;
        case GL_FLOAT_VEC2: return tiFloatVec2;
        case GL_FLOAT_VEC3: return tiFloatVec3;
        case GL_FLOAT_VEC4: return tiFloatVec4;
        case GL_UNSIGNED_INT8_VEC4_NV: return tiUnsignedInt8Vec4;
        case GL_FLOAT_MAT4: return tiFloatMat4;
        default:
            runtime_assert(false);
            return tiFloat;
        }
    }

    void HardwareProgram::invalidate(HardwareContext& ctx)
    {
        shaders_.clear();
        id_ = 0;
        activeAttribs_.clear();
        activeUniforms_.clear();
        samplers_.resetAll();
    }

    GLuint HardwareProgram::id(HardwareContext& ctx) const
    {
        return id_;
    }

    void HardwareProgram::attachShader(const HardwareShaderPtr& shader, HardwareContext& ctx)
    {
        if (id_ == 0) {
            id_ = ogl.CreateProgram();
            btAssert(id_ != 0);
        }

        auto sId = shader->id(ctx);
        btAssert(sId != 0);

        ogl.AttachShader(id_, sId);
        shaders_.push_back(std::make_pair(sId, shader));
    }

    bool HardwareProgram::link(HardwareContext& ctx)
    {
        runtime_assert(id_ != 0);

        ogl.LinkProgram(id_);

        GLint tmp = 0;

        ogl.GetProgramiv(id_, GL_LINK_STATUS, &tmp);

        if (!tmp) {
            tmp = 0;
            ogl.GetProgramiv(id_, GL_INFO_LOG_LENGTH, &tmp);

            std::string buff(tmp, 0);

            ogl.GetProgramInfoLog(id_, buff.size(), nullptr, &buff[0]);

            LOG4CPLUS_ERROR(logger(), "Unable to link program - " << buff);

            return false;
        }

        if (!fillAttribs(ctx)) {
            return false;
        }

        if (!fillUniforms(ctx)) {
            return false;
        }

        return true;
    }

    bool HardwareProgram::fillAttribs(HardwareContext& ctx)
    {
        GLint cnt = 0;
        ogl.GetProgramiv(id_, GL_ACTIVE_ATTRIBUTES, &cnt);

        for (GLuint i = 0; i < static_cast<GLuint>(cnt); ++i) {
            GLint size = 0;
            GLenum type = 0;

            const GLsizei bufSize = 64;
            GLchar name[bufSize];
            GLsizei length = 0;

            ogl.GetActiveAttrib(id_, i, bufSize, &length, &size, &type, name);

            auto it = staticVertexAttribMap.find(name);
            if (it == staticVertexAttribMap.end()) {
                LOG4CPLUS_ERROR(logger(), "Bad vertex attribute name: " << name);
                return false;
            }

            if (size != 1) {
                LOG4CPLUS_ERROR(logger(), "Vertex attribute with size > 1: " << name);
                return false;
            }

            GLint location = ogl.GetAttribLocation(id_, name);
            if (location != getVertexAttribLocation(it->second)) {
                LOG4CPLUS_ERROR(logger(), "Bad vertex attribute location (" << location << ") for: " << name);
                return false;
            }

            activeAttribs_[it->second] = VariableInfo(type, size, location);
        }

        return true;
    }

    bool HardwareProgram::fillUniforms(HardwareContext& ctx)
    {
        GLint cnt = 0;
        ogl.GetProgramiv(id_, GL_ACTIVE_UNIFORMS, &cnt);

        std::array<GLint, static_cast<int>(SamplerName::Max) + 1> samplerLocations;

        for (GLuint i = 0; i < static_cast<GLuint>(cnt); ++i) {
            GLint size = 0;
            GLenum type = 0;

            const GLsizei bufSize = 64;
            GLchar name[bufSize];
            GLsizei length = 0;

            ogl.GetActiveUniform(id_, i, bufSize, &length, &size, &type, name);

            if ((type == GL_SAMPLER_2D) || (type == GL_SAMPLER_CUBE)) {
                auto it = staticSamplerMap.find(name);
                if (it == staticSamplerMap.end()) {
                    LOG4CPLUS_ERROR(logger(), "Bad sampler name: " << name);
                    return false;
                }

                samplers_.set(it->second);

                samplerLocations[static_cast<int>(it->second)] = ogl.GetUniformLocation(id_, name);

                continue;
            }

            auto it = staticUniformMap.find(name);
            if (it == staticUniformMap.end()) {
                LOG4CPLUS_ERROR(logger(), "Bad uniform name: " << name);
                return false;
            }

            GLint location = ogl.GetUniformLocation(id_, name);

            activeUniforms_[it->second] = VariableInfo(type, size, location);
        }

        ogl.UseProgram(id_);
        int texUnit = 0;
        for (int i = 0; i <= static_cast<int>(SamplerName::Max); ++i) {
            SamplerName sName = static_cast<SamplerName>(i);
            if (samplers_[sName]) {
                ogl.Uniform1i(samplerLocations[i], texUnit++);
            }
        }
        ogl.UseProgram(0);

        return true;
    }
}
