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
        LOG4CPLUS_INFO(logger(), "OpenGL vendor: " << ogl.GetString(GL_VENDOR));
        LOG4CPLUS_INFO(logger(), "OpenGL renderer: " << ogl.GetString(GL_RENDERER));
        LOG4CPLUS_INFO(logger(), "OpenGL version: " << ogl.GetString(GL_VERSION));

        importer_.SetIOHandler(new AssimpIOSystem());

        ogl.PixelStorei(GL_UNPACK_ALIGNMENT, 1);
        ogl.PixelStorei(GL_PACK_ALIGNMENT, 1);
        ogl.Enable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        GLint sampleBuffers = 0;
        GLint samples = 0;

        ogl.GetIntegerv(GL_SAMPLE_BUFFERS, &sampleBuffers);
        ogl.GetIntegerv(GL_SAMPLES, &samples);

        LOG4CPLUS_INFO(logger(), "sample_buffers = " << sampleBuffers << ", samples = " << samples);
        LOG4CPLUS_INFO(logger(), "texture filter: " << (settings.trilinearFilter ? "trilinear" : "bilinear"));
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

    void HardwareContext::deleteTexture(GLuint texId)
    {
        for (auto& texUnit : texUnits_) {
            for (auto& tId : texUnit.texIds) {
                if (tId == texId) {
                    tId = 0;
                }
            }
        }
        ogl.DeleteTextures(1, &texId);
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
            sampler->setParameterInt(GL_TEXTURE_WRAP_R, params.texWrapW, *this);
            it = samplers_.emplace(params, sampler).first;
        }
        auto samplerId = it->second->id(*this);
        if (samplerId != texUnits_[activeTexUnit_].samplerId) {
            texUnits_[activeTexUnit_].samplerId = samplerId;
            ogl.BindSampler(activeTexUnit_, samplerId);
        }
    }

    bool HardwareContext::setMRT(const HardwareMRT& mrt)
    {
        GLuint oldFbId = 0;
        ogl.GetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&oldFbId);

        Vector2u sz = mrt.getSize();

        HardwareFramebufferPtr fb;

        auto fbIter = framebuffers_.find(sz);
        if (fbIter != framebuffers_.end()) {
            for (int i = static_cast<int>(AttachmentPoint::Color0); i <= static_cast<int>(AttachmentPoint::Max); ++i) {
                AttachmentPoint p = static_cast<AttachmentPoint>(i);
                fbIter->second.fb->attach(p, mrt.attachment(p), *this);
            }
            const auto& dt = mrt.attachment(AttachmentPoint::Depth);
            if (!dt  && !fbIter->second.targetDepth) {
                auto rb = hwManager.createRenderbuffer(sz.x(), sz.y());
                rb->allocate(GL_DEPTH_COMPONENT24, *this);
                fbIter->second.targetDepth = HardwareRenderTarget(rb);
            }
            fbIter->second.fb->attach(AttachmentPoint::Depth, (dt ? dt : fbIter->second.targetDepth), *this);
        }

        if ((fbIter == framebuffers_.end()) && !sz.isZero()) {
            LOG4CPLUS_DEBUG(logger(), "hwContext: new framebuffer for " << sz);
            FramebufferState fbs;
            fbs.fb = hwManager.createFramebuffer();
            for (int i = static_cast<int>(AttachmentPoint::Color0); i <= static_cast<int>(AttachmentPoint::Max); ++i) {
                AttachmentPoint p = static_cast<AttachmentPoint>(i);
                fbs.fb->attach(p, mrt.attachment(p), *this);
            }
            const auto& dt = mrt.attachment(AttachmentPoint::Depth);
            if (!dt) {
                auto rb = hwManager.createRenderbuffer(sz.x(), sz.y());
                rb->allocate(GL_DEPTH_COMPONENT24, *this);
                fbs.targetDepth = HardwareRenderTarget(rb);
            }
            fbs.fb->attach(AttachmentPoint::Depth, (dt ? dt : fbs.targetDepth), *this);
            if (!fbs.fb->checkStatus()) {
                LOG4CPLUS_ERROR(logger(), "hwContext: framebuffer not complete, wtf ???");
            }
            fbIter = framebuffers_.emplace(sz, fbs).first;
        }

        if (fbIter != framebuffers_.end()) {
            if (currentFbId_ == 0) {
                defaultFbId_ = oldFbId;
            }

            GLuint fbId = fbIter->second.fb->id(*this);
            if (fbId != currentFbId_) {
                currentFbId_ = fbId;
                ogl.BindFramebuffer(GL_FRAMEBUFFER, fbId);
            } else {
                ogl.BindFramebuffer(GL_FRAMEBUFFER, oldFbId);
            }
        } else if (currentFbId_ != 0) {
            currentFbId_ = 0;
            ogl.BindFramebuffer(GL_FRAMEBUFFER, defaultFbId_);
            defaultFbId_ = 0;
        } else {
            ogl.BindFramebuffer(GL_FRAMEBUFFER, oldFbId);
        }

        return !sz.isZero();
    }
}
