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

#include "HardwareFramebuffer.h"

namespace af3d
{
    HardwareFramebuffer::HardwareFramebuffer(HardwareResourceManager* mgr)
    : HardwareResource(mgr)
    {
    }

    HardwareFramebuffer::~HardwareFramebuffer()
    {
        GLuint id = id_;
        if (id != 0) {
            cleanup([id](HardwareContext& ctx) {
                ogl.DeleteFramebuffers(1, &id);
            });
        } else {
            cleanup();
        }
    }

    void HardwareFramebuffer::invalidate(HardwareContext& ctx)
    {
        for (size_t i = 0; i < attachments_.size(); ++i) {
            attachments_[i] = Attachment();
        }
        id_ = 0;
    }

    GLuint HardwareFramebuffer::id(HardwareContext& ctx) const
    {
        return id_;
    }

    void HardwareFramebuffer::attachTarget(AttachmentPoint attachmentPoint, const HardwareRenderTarget& target, HardwareContext& ctx)
    {
        if (attachments_[attachmentPoint].attached(target)) {
            return;
        }

        createFramebuffer();

        auto rId = target.texture()->id(ctx);
        btAssert(rId != 0);

        GLuint curFb = 0;
        ogl.GetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&curFb);

        ogl.BindFramebuffer(GL_FRAMEBUFFER, id_);
        GLenum textarget = HardwareTexture::glType(target.texture()->type());
        if (target.texture()->type() == TextureTypeCubeMap) {
            textarget = HardwareTexture::glCubeFace(target.cubeFace());
        }
        ogl.FramebufferTexture2D(GL_FRAMEBUFFER, glAttachmentPoint(attachmentPoint), textarget, rId, target.level());
        attachments_[attachmentPoint] = Attachment(target);

        ogl.BindFramebuffer(GL_FRAMEBUFFER, curFb);
    }

    void HardwareFramebuffer::attachRenderbuffer(AttachmentPoint attachmentPoint, const HardwareRenderbufferPtr& rb, HardwareContext& ctx)
    {
        if (attachments_[attachmentPoint].attached(rb)) {
            return;
        }

        createFramebuffer();

        auto rId = rb->id(ctx);
        btAssert(rId != 0);

        GLuint curFb = 0;
        ogl.GetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&curFb);

        ogl.BindFramebuffer(GL_FRAMEBUFFER, id_);
        ogl.FramebufferRenderbuffer(GL_FRAMEBUFFER, glAttachmentPoint(attachmentPoint), GL_RENDERBUFFER, rId);
        attachments_[attachmentPoint] = Attachment(rb);

        ogl.BindFramebuffer(GL_FRAMEBUFFER, curFb);
    }

    bool HardwareFramebuffer::checkStatus()
    {
        createFramebuffer();

        GLuint curFb = 0;
        ogl.GetIntegerv(GL_FRAMEBUFFER_BINDING, (GLint*)&curFb);

        ogl.BindFramebuffer(GL_FRAMEBUFFER, id_);
        GLenum status = ogl.CheckFramebufferStatus(GL_FRAMEBUFFER);

        ogl.BindFramebuffer(GL_FRAMEBUFFER, curFb);

        return status == GL_FRAMEBUFFER_COMPLETE;
    }

    GLenum HardwareFramebuffer::glAttachmentPoint(AttachmentPoint attachmentPoint)
    {
        switch (attachmentPoint) {
        case DepthAttachment: return GL_DEPTH_ATTACHMENT;
        case StencilAttachment: return GL_STENCIL_ATTACHMENT;
        default:
            btAssert(false);
        case ColorAttachment: return GL_COLOR_ATTACHMENT0;
        }
    }

    void HardwareFramebuffer::createFramebuffer()
    {
        if (id_ == 0) {
            ogl.GenFramebuffers(1, &id_);
            btAssert(id_ != 0);
        }
    }
}
