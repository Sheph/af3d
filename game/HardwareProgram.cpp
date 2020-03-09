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

    void HardwareProgram::invalidate(HardwareContext& ctx)
    {
        shaders_.clear();
        id_ = 0;
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

            ogl.GetProgramInfoLog(id_, buff.size(), NULL, &buff[0]);

            LOG4CPLUS_ERROR(logger(), "Unable to link program - " << buff);

            return false;
        }

        return true;
    }
}
