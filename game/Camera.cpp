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

#include "Camera.h"
#include "Settings.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(Camera, AObject)
    ACLASS_DEFINE_END(Camera)

    Camera::Camera()
    : AObject(AClass_Camera)
    {
    }

    const AClass& Camera::staticKlass()
    {
        return AClass_Camera;
    }

    AObjectPtr Camera::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<Camera>();
        obj->propertiesSet(propVals);
        return obj;
    }

    const AABB2i& Camera::viewport() const
    {
        if (const auto& ct = renderTarget(AttachmentPoint::Color0)) {
            viewport_ = AABB2i(Vector2i_zero, Vector2i(ct.width(), ct.height()));
        } else if (const auto& dt = renderTarget(AttachmentPoint::Depth)) {
            viewport_ = AABB2i(Vector2i_zero, Vector2i(dt.width(), dt.height()));
        } else if (const auto& st = renderTarget(AttachmentPoint::Stencil)) {
            viewport_ = AABB2i(Vector2i_zero, Vector2i(st.width(), st.height()));
        }
        return viewport_;
    }

    void Camera::setViewport(const AABB2i& value)
    {
        viewport_ = value;
    }

    HardwareMRT Camera::getHardwareMRT() const
    {
        HardwareMRT mrt;
        for (int i = 0; i <= static_cast<int>(AttachmentPoint::Max); ++i) {
            AttachmentPoint p = static_cast<AttachmentPoint>(i);
            mrt.attachment(p) = renderTarget(p).toHardware();
        }
        return mrt;
    }
}
