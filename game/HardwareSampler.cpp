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

#include "HardwareSampler.h"

namespace af3d
{
    HardwareSampler::HardwareSampler(HardwareResourceManager* mgr)
    : HardwareResource(mgr)
    {
    }

    HardwareSampler::~HardwareSampler()
    {
        GLuint id = id_;
        if (id != 0) {
            cleanup([id](HardwareContext& ctx) {
                ogl.DeleteSamplers(1, &id);
            });
        } else {
            cleanup();
        }
    }

    void HardwareSampler::doInvalidate(HardwareContext& ctx)
    {
        id_ = 0;
    }

    GLuint HardwareSampler::id(HardwareContext& ctx) const
    {
        return id_;
    }

    void HardwareSampler::setParameterFloat(GLenum pname, GLfloat param, HardwareContext& ctx)
    {
        createSampler();
        ogl.SamplerParameterf(id_, pname, param);
    }

    void HardwareSampler::setParameterInt(GLenum pname, GLint param, HardwareContext& ctx)
    {
        createSampler();
        ogl.SamplerParameteri(id_, pname, param);
    }

    void HardwareSampler::createSampler()
    {
        if (id_ == 0) {
            ogl.GenSamplers(1, &id_);
            btAssert(id_ != 0);
            setValid();
        }
    }
}
