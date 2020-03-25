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
#include "Settings.h"

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
        float moveSpeed = 5.0f;

        if (inputManager.keyboard().pressed(KI_LSHIFT)) {
            moveSpeed *= 3.0f;
        }

        if (inputManager.keyboard().pressed(KI_W)) {
            parent()->setPos(parent()->pos() + parent()->getForward() * dt * moveSpeed);
        }
        if (inputManager.keyboard().pressed(KI_S)) {
            parent()->setPos(parent()->pos() - parent()->getForward() * dt * moveSpeed);
        }
        if (inputManager.keyboard().pressed(KI_A)) {
            parent()->setPos(parent()->pos() - parent()->getRight() * dt * moveSpeed);
        }
        if (inputManager.keyboard().pressed(KI_D)) {
            parent()->setPos(parent()->pos() + parent()->getRight() * dt * moveSpeed);
        }
        if (inputManager.keyboard().pressed(KI_SPACE)) {
            parent()->setPos(parent()->pos() + parent()->getUp() * dt * moveSpeed);
        }
        if (inputManager.keyboard().pressed(KI_C)) {
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

        auto diff = (inputManager.mouse().pos() - mousePrevPos_) / Vector2f(settings.viewHeight, settings.viewHeight);

        auto dir = quatRotate(btQuaternion(btVector3_up, btRadians(-diff.x() * 80.0f)), parent()->getForward());
        dir = quatRotate(btQuaternion(parent()->getRight(), btRadians(-diff.y() * 80.0f)), dir);

        parent()->setBasis(makeLookBasis(dir, btVector3_up));

        mousePrevPos_ = inputManager.mouse().pos();
    }

    const Frustum& CameraComponent::getFrustum() const
    {
        frustum_.setTransform(parent()->transform() * xf_);
        return frustum_;
    }

    void CameraComponent::onRegister()
    {
    }

    void CameraComponent::onUnregister()
    {
    }
}
