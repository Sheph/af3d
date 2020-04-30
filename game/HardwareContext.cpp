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

#include "HardwareContext.h"
#include "Settings.h"
#include "HardwareResourceManager.h"
#include "AssimpIOSystem.h"

namespace af3d
{
    HardwareContext::HardwareContext()
    {
        importer_.SetIOHandler(new AssimpIOSystem());
    }

    void HardwareContext::setActiveTextureUnit(int unit)
    {
        btAssert(unit < static_cast<int>(texUnits_.size()));
        activeTexUnit_ = unit;
        ogl.ActiveTexture(GL_TEXTURE0 + unit);
    }

    void HardwareContext::bindTexture(GLuint texId)
    {
        if (texId != texUnits_[activeTexUnit_].texId) {
            texUnits_[activeTexUnit_].texId = texId;
            ogl.BindTexture(GL_TEXTURE_2D, texId);
        }
    }

    void HardwareContext::bindSampler(int unit, const SamplerParams& params)
    {
        auto it = samplers_.find(params);
        if (it == samplers_.end()) {
            auto sampler = hwManager.createSampler();
            sampler->setParameterInt(GL_TEXTURE_MAG_FILTER, params.texMagFilter, *this);
            if (params.texMinFilter) {
                sampler->setParameterInt(GL_TEXTURE_MIN_FILTER, *params.texMinFilter, *this);
            } else if (settings.trilinearFilter) {
                sampler->setParameterInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR, *this);
            } else {
                sampler->setParameterInt(GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST, *this);
            }
            sampler->setParameterInt(GL_TEXTURE_WRAP_S, params.texWrapU, *this);
            sampler->setParameterInt(GL_TEXTURE_WRAP_T, params.texWrapV, *this);
            it = samplers_.emplace(params, sampler).first;
        }
        auto samplerId = it->second->id(*this);
        if (samplerId != texUnits_[activeTexUnit_].samplerId) {
            texUnits_[activeTexUnit_].samplerId = samplerId;
            ogl.BindSampler(activeTexUnit_, samplerId);
        }
    }
}
