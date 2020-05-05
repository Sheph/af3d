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

#include "CameraComponent.h"
#include "SceneObject.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(CameraComponent, PhasedComponent)
    ACLASS_DEFINE_END(CameraComponent)

    CameraComponent::CameraComponent(const CameraPtr& camera)
    : PhasedComponent(AClass_CameraComponent, phasePreRender, 1),
      camera_(camera)
    {
    }

    const AClass& CameraComponent::staticKlass()
    {
        return AClass_CameraComponent;
    }

    AObjectPtr CameraComponent::create(const APropertyValueMap& propVals)
    {
        return AObjectPtr();
    }

    void CameraComponent::preRender(float dt)
    {
        camera_->setTransform(parent()->smoothTransform());
    }

    Vector2f CameraComponent::screenToViewport(const Vector2f& pt) const
    {
        auto sz = camera_->viewport().upperBound - camera_->viewport().lowerBound;

        return Vector2f(pt.x() / sz.x(), (sz.y() - pt.y()) / sz.y());
    }

    Ray CameraComponent::screenPointToRay(const Vector2f& pt) const
    {
        return viewportPointToRay(screenToViewport(pt));
    }

    Ray CameraComponent::viewportPointToRay(const Vector2f& pt) const
    {
        float nx = (2.0f * pt.x()) - 1.0f;
        float ny = (2.0f * pt.y()) - 1.0f;

        Vector4f near(nx, ny, -1.0f, 1.0f);
        // Use midPoint rather than far point to avoid issues with infinite projection.
        Vector4f mid(nx, ny, 0.0f, 1.0f);

        auto invM = camera_->frustum().viewProjMat().inverse();

        auto pos4 = invM * near;
        auto target4 = invM * mid;

        auto pos = toVector3(pos4 / pos4.w());
        auto target = toVector3(target4 / target4.w());

        auto dir = target - pos;
        btZeroNormalize(dir);

        return Ray(pos, dir);
    }

    void CameraComponent::onRegister()
    {
        camera_->setTransform(parent()->smoothTransform());
    }

    void CameraComponent::onUnregister()
    {
    }
}
