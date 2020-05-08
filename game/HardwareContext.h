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

#ifndef _HARDWARE_CONTEXT_H_
#define _HARDWARE_CONTEXT_H_

#include "HardwareSampler.h"
#include "HardwareProgram.h"
#include "HardwareFramebuffer.h"
#include "HardwareRenderTarget.h"
#include "assimp/Importer.hpp"
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <sstream>
#include <map>

namespace af3d
{
    struct SamplerParams
    {
        SamplerParams() = default;
        SamplerParams(GLenum texMinFilter,
            GLenum texWrapU,
            GLenum texWrapV,
            GLenum texWrapW,
            GLenum texMagFilter)
        : texMinFilter(texMinFilter),
          texWrapU(texWrapU),
          texWrapV(texWrapV),
          texWrapW(texWrapW),
          texMagFilter(texMagFilter) {}
        SamplerParams(GLenum texWrapU,
            GLenum texWrapV,
            GLenum texWrapW,
            GLenum texMagFilter)
        : texWrapU(texWrapU),
          texWrapV(texWrapV),
          texMagFilter(texMagFilter) {}
        explicit SamplerParams(GLenum texMinFilter, GLenum texMagFilter = GL_LINEAR)
        : texMinFilter(texMinFilter),
          texMagFilter(texMagFilter) {}

        inline bool operator<(const SamplerParams& other) const
        {
            if (texMinFilter != other.texMinFilter) {
                return texMinFilter < other.texMinFilter;
            }
            if (texWrapU != other.texWrapU) {
                return texWrapU < other.texWrapU;
            }
            if (texWrapV != other.texWrapV) {
                return texWrapV < other.texWrapV;
            }
            if (texWrapW != other.texWrapW) {
                return texWrapW < other.texWrapW;
            }
            return texMagFilter < other.texMagFilter;
        }

        inline std::string toString() const
        {
            std::ostringstream os;
            if (texMinFilter) {
                os << *texMinFilter;
            } else {
                os << 0;
            }
            os << "|" << texWrapU << "|" << texWrapV << "|" << texWrapV << "|" << texMagFilter;
            return os.str();
        }

        boost::optional<GLenum> texMinFilter;
        GLenum texWrapU = GL_REPEAT;
        GLenum texWrapV = GL_REPEAT;
        GLenum texWrapW = GL_REPEAT;
        GLenum texMagFilter = GL_LINEAR;
    };

    struct HardwareTextureBinding
    {
        HardwareTextureBinding() = default;
        HardwareTextureBinding(const HardwareTexturePtr& tex,
            const SamplerParams& params)
        : tex(tex),
          params(params) {}

        inline bool operator<(const HardwareTextureBinding& other) const
        {
            if (tex != other.tex) {
                return tex < other.tex;
            }
            return params < other.params;
        }

        HardwareTexturePtr tex;
        SamplerParams params;
    };

    class HardwareContext : boost::noncopyable
    {
    public:
        HardwareContext();
        ~HardwareContext() = default;

        inline Assimp::Importer& importer() { return importer_; }

        void setActiveTextureUnit(int unit);

        void bindTexture(TextureType texType, GLuint texId);

        void bindSampler(int unit, const SamplerParams& params);

        void setRenderTarget(const HardwareRenderTarget& target);

    private:
        struct TextureUnit
        {
            std::array<GLuint, TextureTypeMax + 1> texIds;
            GLuint samplerId = 0;
        };

        using SamplerMap = std::map<SamplerParams, HardwareSamplerPtr>;
        using FramebufferList = std::list<HardwareFramebufferPtr>;

        Assimp::Importer importer_;
        SamplerMap samplers_;
        FramebufferList framebuffers_;
        std::array<TextureUnit, static_cast<int>(SamplerName::Max) + 1> texUnits_;
        int activeTexUnit_ = 0;

        GLuint defaultFbId_ = 0;
        GLuint currentFbId_ = 0;
    };
}

#endif
