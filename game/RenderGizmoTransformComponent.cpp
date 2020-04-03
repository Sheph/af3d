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

#include "RenderGizmoTransformComponent.h"
#include "MaterialManager.h"
#include "SceneObject.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(RenderGizmoTransformComponent, RenderComponent)
    ACLASS_DEFINE_END(RenderGizmoTransformComponent)

    RenderGizmoTransformComponent::RenderGizmoTransformComponent()
    : RenderComponent(AClass_RenderGizmoTransformComponent)
    {
        material_ = materialManager.createMaterial(MaterialTypeImm);
        material_->setBlendingParams(BlendingParams(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        material_->setDepthTest(false);
        //material_->setCullFaceMode(0);
    }

    const AClass& RenderGizmoTransformComponent::staticKlass()
    {
        return AClass_RenderGizmoTransformComponent;
    }

    AObjectPtr RenderGizmoTransformComponent::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<RenderGizmoTransformComponent>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void RenderGizmoTransformComponent::update(float dt)
    {
        btTransform newXf = getTargetXf();

        if ((newXf == targetXf_) && !dirty_) {
            return;
        }

        AABB aabb = calcAABB();

        btVector3 displacement = newXf.getOrigin() - targetXf_.getOrigin();

        manager()->moveAABB(cookie_, prevAABB_, aabb, displacement);

        targetXf_ = newXf;
        prevAABB_ = aabb;
        dirty_ = false;
    }

    void RenderGizmoTransformComponent::render(RenderList& rl, void* const* parts, size_t numParts)
    {
        auto viewExt = rl.frustum().getExtents(targetXf_.getOrigin());

        float radius = viewportRadius_ * viewExt.y();
        float length = viewportLength_ * viewExt.y();

        auto rop = rl.addGeometry(material_, GL_TRIANGLES);

        auto vForward = targetXf_.getBasis() * btVector3_forward;
        auto vUp = targetXf_.getBasis() * btVector3_up;
        auto vRight = targetXf_.getBasis() * btVector3_right;

        rop.addLine(targetXf_.getOrigin(), vForward * length, vUp * radius, Color(1.0f, 0.0f, 0.0f, 0.8f));
        rop.addLine(targetXf_.getOrigin(), vRight * length, vUp * radius, Color(0.0f, 1.0f, 0.0f, 0.8f));
        rop.addLine(targetXf_.getOrigin(), vUp * length, vForward * radius, Color(0.0f, 0.0f, 1.0f, 0.8f));
    }

    std::pair<AObjectPtr, float> RenderGizmoTransformComponent::testRay(const Frustum& frustum, const Ray& ray, void* part)
    {
        auto res = ray.testSphere(Sphere(targetXf_.getOrigin(), radius_));
        if (res.first) {
            return std::make_pair(sharedThis(), res.second);
        } else {
            return std::make_pair(AObjectPtr(), 0.0f);
        }
    }

    void RenderGizmoTransformComponent::debugDraw()
    {
    }

    void RenderGizmoTransformComponent::onRegister()
    {
        targetXf_ = getTargetXf();
        prevAABB_ = calcAABB();
        cookie_ = manager()->addAABB(this, prevAABB_, nullptr);
        dirty_ = false;
    }

    void RenderGizmoTransformComponent::onUnregister()
    {
        manager()->removeAABB(cookie_);
    }

    AABB RenderGizmoTransformComponent::calcAABB()
    {
        auto sz = btVector3(radius_, radius_, radius_);

        return AABB(targetXf_.getOrigin() - sz, targetXf_.getOrigin() + sz);
    }

    btTransform RenderGizmoTransformComponent::getTargetXf() const
    {
        return target_ ? target_->propertyGet(AProperty_WorldTransform).toTransform() : btTransform::getIdentity();
    }
}
