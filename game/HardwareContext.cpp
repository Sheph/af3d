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
#include "Logger.h"

namespace af3d
{
    HardwareContext::HardwareContext()
    {
        importer_.SetIOHandler(new AssimpIOSystem());

        ogl.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
    }

    void HardwareContext::setActiveTextureUnit(int unit)
    {
        btAssert(unit < static_cast<int>(texUnits_.size()));
        activeTexUnit_ = unit;
        ogl.ActiveTexture(GL_TEXTURE0 + unit);
    }

    void HardwareContext::bindTexture(TextureType texType, GLuint texId)
    {
        if (texId != texUnits_[activeTexUnit_].texIds[texType]) {
            texUnits_[activeTexUnit_].texIds[texType] = texId;
            ogl.BindTexture(HardwareTexture::glType(texType), texId);
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

    void HardwareContext::setRenderTarget(const HardwareTexturePtr& tex)
    {
        HardwareFramebufferPtr fb;

        for (auto it = framebuffers_.begin(); it != framebuffers_.end();) {
            const auto& res = (*it)->attachment(HardwareFramebuffer::ColorAttachment, *this);
            if (res == tex) {
                btAssert(tex);
                fb = *it;
                ++it;
            } else if (res.use_count() == 1) {
                LOG4CPLUS_DEBUG(logger(), "hwContext: framebuffer " << res.get() << " is done");
                framebuffers_.erase(it++);
            } else {
                ++it;
            }
        }

        if (!fb && tex) {
            fb = hwManager.createFramebuffer();
            fb->attachTexture(HardwareFramebuffer::ColorAttachment, tex, *this);
            auto rb = hwManager.createRenderbuffer(tex->width(), tex->height());
            rb->allocate(GL_DEPTH24_STENCIL8, *this);
            fb->attachRenderbuffer(HardwareFramebuffer::DepthAttachment, rb, *this);
            fb->attachRenderbuffer(HardwareFramebuffer::StencilAttachment, rb, *this);
            if (!fb->checkStatus()) {
                LOG4CPLUS_ERROR(logger(), "hwContext: framebuffer not complete, wtf ???");
            }
            framebuffers_.push_back(fb);
        }

        if (fb) {
            if (currentFbId_ == 0) {
                ogl.GetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&defaultFbId_);
            }

            GLuint fbId = fb->id(*this);
            if (fbId != currentFbId_) {
                currentFbId_ = fbId;
                ogl.BindFramebuffer(GL_FRAMEBUFFER, fbId);
            }
        } else if (currentFbId_ != 0) {
            currentFbId_ = 0;
            ogl.BindFramebuffer(GL_FRAMEBUFFER, defaultFbId_);
            defaultFbId_ = 0;
        }
    }
}
