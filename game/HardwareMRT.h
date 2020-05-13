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

#ifndef _HARDWARE_MRT_H_
#define _HARDWARE_MRT_H_

#include "HardwareRenderTarget.h"
#include "af3d/EnumSet.h"
#include "af3d/Vector2.h"

namespace af3d
{
    enum class AttachmentPoint
    {
        Depth = 0,
        Stencil,
        Color0,
        Color1,
        Color2,
        Color3,
        Color4,
        Color5,
        Max = Color5
    };

    inline GLenum glAttachmentPoint(AttachmentPoint attachmentPoint)
    {
        switch (attachmentPoint) {
        case AttachmentPoint::Depth: return GL_DEPTH_ATTACHMENT;
        case AttachmentPoint::Stencil: return GL_STENCIL_ATTACHMENT;
        case AttachmentPoint::Color1: return GL_COLOR_ATTACHMENT1;
        case AttachmentPoint::Color2: return GL_COLOR_ATTACHMENT2;
        case AttachmentPoint::Color3: return GL_COLOR_ATTACHMENT3;
        case AttachmentPoint::Color4: return GL_COLOR_ATTACHMENT4;
        case AttachmentPoint::Color5: return GL_COLOR_ATTACHMENT5;
        default:
            btAssert(false);
        case AttachmentPoint::Color0: return GL_COLOR_ATTACHMENT0;
        }
    }

    using AttachmentPoints = EnumSet<AttachmentPoint>;

    using AttachmentColors = std::array<Color, static_cast<int>(AttachmentPoint::Max) + 1>;

    class HardwareMRT
    {
    public:
        HardwareMRT() = default;
        ~HardwareMRT() = default;

        inline Vector2u getSize() const
        {
            const auto& ct = attachment(AttachmentPoint::Color0);
            if (ct) {
                return Vector2u(ct.fullWidth(), ct.fullHeight());
            }
            const auto& dt = attachment(AttachmentPoint::Depth);
            if (dt) {
                return Vector2u(dt.fullWidth(), dt.fullHeight());
            }
            const auto& st = attachment(AttachmentPoint::Stencil);
            return Vector2u(st.fullWidth(), st.fullHeight());
        }

        inline AttachmentPoints getDrawBuffers() const
        {
            AttachmentPoints buffs;
            for (int i = 0; i <= static_cast<int>(AttachmentPoint::Max); ++i) {
                AttachmentPoint p = static_cast<AttachmentPoint>(i);
                if (attachment(p)) {
                    buffs.set(p);
                }
            }
            return buffs;
        }

        inline const HardwareRenderTarget& attachment(AttachmentPoint attachmentPoint) const { return attachments_[static_cast<int>(attachmentPoint)]; }
        inline HardwareRenderTarget& attachment(AttachmentPoint attachmentPoint) { return attachments_[static_cast<int>(attachmentPoint)]; }

    private:
        std::array<HardwareRenderTarget, static_cast<int>(AttachmentPoint::Max) + 1> attachments_;
    };
}

#endif
