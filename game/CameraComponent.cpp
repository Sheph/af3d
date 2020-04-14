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
#include "InputManager.h"
#include "Logger.h"
#include "imgui.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(CameraComponent, PhasedComponent)
    ACLASS_DEFINE_END(CameraComponent)

    CameraComponent::CameraComponent(const btTransform& xf)
    : PhasedComponent(AClass_CameraComponent, phasePreRender),
      xf_(xf)
    {
    }

    const AClass& CameraComponent::staticKlass()
    {
        return AClass_CameraComponent;
    }

    AObjectPtr CameraComponent::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<CameraComponent>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void CameraComponent::preRender(float dt)
    {
        ImGuiIO& io = ImGui::GetIO();

        if (io.WantCaptureMouse || io.WantCaptureKeyboard) {
            return;
        }

        float moveSpeed = 5.0f;

        if (inputManager.keyboard().pressed(KI_LSHIFT)) {
            moveSpeed *= 3.0f;
        }

        if (inputManager.keyboard().pressed(KeySequence(KI_W)) || inputManager.keyboard().pressed(KeySequence(KM_SHIFT, KI_W))) {
            parent()->setPos(parent()->pos() + parent()->getForward() * dt * moveSpeed);
        }
        if (inputManager.keyboard().pressed(KeySequence(KI_S)) || inputManager.keyboard().pressed(KeySequence(KM_SHIFT, KI_S))) {
            parent()->setPos(parent()->pos() - parent()->getForward() * dt * moveSpeed);
        }
        if (inputManager.keyboard().pressed(KeySequence(KI_A)) || inputManager.keyboard().pressed(KeySequence(KM_SHIFT, KI_A))) {
            parent()->setPos(parent()->pos() - parent()->getRight() * dt * moveSpeed);
        }
        if (inputManager.keyboard().pressed(KeySequence(KI_D)) || inputManager.keyboard().pressed(KeySequence(KM_SHIFT, KI_D))) {
            parent()->setPos(parent()->pos() + parent()->getRight() * dt * moveSpeed);
        }
        if (inputManager.keyboard().pressed(KeySequence(KI_SPACE)) || inputManager.keyboard().pressed(KeySequence(KM_SHIFT, KI_SPACE))) {
            parent()->setPos(parent()->pos() + parent()->getUp() * dt * moveSpeed);
        }
        if (inputManager.keyboard().pressed(KeySequence(KI_C)) || inputManager.keyboard().pressed(KeySequence(KM_SHIFT, KI_C))) {
            parent()->setPos(parent()->pos() - parent()->getUp() * dt * moveSpeed);
        }

        if (!inputManager.mouse().pressed(false)) {
            mousePressed_ = false;
            return;
        }

        if (!mousePressed_) {
            mousePressed_ = true;
            mousePrevPos_ = inputManager.mouse().pos();
            return;
        }

        auto sz = viewport_.upperBound - viewport_.lowerBound;

        auto diff = (inputManager.mouse().pos() - mousePrevPos_) / Vector2f(sz.x(), sz.y());

        auto dir = quatRotate(btQuaternion(btVector3_up, btRadians(-diff.x() * 80.0f)), parent()->getForward());
        dir = quatRotate(btQuaternion(parent()->getRight(), btRadians(-diff.y() * 80.0f)), dir);

        parent()->setBasis(makeLookBasis(dir, btVector3_up));

        mousePrevPos_ = inputManager.mouse().pos();
    }

    const Frustum& CameraComponent::getFrustum() const
    {
        frustum_.setTransform(parent()->smoothTransform() * xf_);
        return frustum_;
    }

    Vector2f CameraComponent::screenToViewport(const Vector2f& pt) const
    {
        auto sz = viewport_.upperBound - viewport_.lowerBound;

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

        const auto& frustum = getFrustum();
        auto invM = frustum.viewProjMat().inverse();

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
    }

    void CameraComponent::onUnregister()
    {
    }
}
