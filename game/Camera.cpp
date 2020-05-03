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

    Vector2f Camera::screenToViewport(const Vector2f& pt) const
    {
        auto sz = viewport().upperBound - viewport().lowerBound;

        return Vector2f(pt.x() / sz.x(), (sz.y() - pt.y()) / sz.y());
    }

    Ray Camera::screenPointToRay(const Vector2f& pt) const
    {
        return viewportPointToRay(screenToViewport(pt));
    }

    Ray Camera::viewportPointToRay(const Vector2f& pt) const
    {
        float nx = (2.0f * pt.x()) - 1.0f;
        float ny = (2.0f * pt.y()) - 1.0f;

        Vector4f near(nx, ny, -1.0f, 1.0f);
        // Use midPoint rather than far point to avoid issues with infinite projection.
        Vector4f mid(nx, ny, 0.0f, 1.0f);

        auto invM = frustum().viewProjMat().inverse();

        auto pos4 = invM * near;
        auto target4 = invM * mid;

        auto pos = toVector3(pos4 / pos4.w());
        auto target = toVector3(target4 / target4.w());

        auto dir = target - pos;
        btZeroNormalize(dir);

        return Ray(pos, dir);
    }
}
