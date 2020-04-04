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

#include "RenderGizmoRotateComponent.h"
#include "MaterialManager.h"
#include "SceneObject.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(RenderGizmoRotateComponent, RenderComponent)
    ACLASS_DEFINE_END(RenderGizmoRotateComponent)

    RenderGizmoRotateComponent::RenderGizmoRotateComponent()
    : RenderComponent(AClass_RenderGizmoRotateComponent)
    {
        material_ = materialManager.createMaterial(MaterialTypeImm);
        material_->setBlendingParams(BlendingParams(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        material_->setDepthTest(false);
    }

    const AClass& RenderGizmoRotateComponent::staticKlass()
    {
        return AClass_RenderGizmoRotateComponent;
    }

    AObjectPtr RenderGizmoRotateComponent::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<RenderGizmoRotateComponent>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void RenderGizmoRotateComponent::update(float dt)
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

    void RenderGizmoRotateComponent::render(RenderList& rl, void* const* parts, size_t numParts)
    {
        auto sz = getSizes(rl.frustum());

        auto rop = rl.addGeometry(material_, GL_TRIANGLES);

        auto vForward = targetXf_.getBasis() * btVector3_forward;
        auto vUp = targetXf_.getBasis() * btVector3_up;
        auto vRight = targetXf_.getBasis() * btVector3_right;

        rop.addCircle(targetXf_.getOrigin(),
            rl.frustum().plane(Frustum::Plane::Far).normal * sz.radius1, Color(1.0f, 1.0f, 1.0f, alpha(RotateType::Trackball) * 0.25f));

        rop.addRing(targetXf_.getOrigin(), vRight * sz.width, sz.radius1, Color(1.0f, 0.0f, 0.0f, alpha(RotateType::PlaneX)));
        rop.addRing(targetXf_.getOrigin(), vUp * sz.width, sz.radius1, Color(0.0f, 1.0f, 0.0f, alpha(RotateType::PlaneY)));
        rop.addRing(targetXf_.getOrigin(), vForward * sz.width, sz.radius1, Color(0.0f, 0.0f, 1.0f, alpha(RotateType::PlaneZ)));

        rop.addRing(targetXf_.getOrigin(),
            rl.frustum().plane(Frustum::Plane::Far).normal * sz.width, sz.radius2, Color(1.0f, 1.0f, 1.0f, alpha(RotateType::PlaneCurrent)));
    }

    std::pair<AObjectPtr, float> RenderGizmoRotateComponent::testRay(const Frustum& frustum, const Ray& ray, void* part)
    {
        auto res = ray.testSphere(Sphere(targetXf_.getOrigin(), radius_));
        if (res.first) {
            return std::make_pair(sharedThis(), res.second);
        } else {
            return std::make_pair(AObjectPtr(), 0.0f);
        }
    }

    void RenderGizmoRotateComponent::debugDraw()
    {
    }

    RotateType RenderGizmoRotateComponent::testRay(const Frustum& frustum, const Ray& ray) const
    {
        return RotateType::None;
    }

    void RenderGizmoRotateComponent::onRegister()
    {
        targetXf_ = getTargetXf();
        prevAABB_ = calcAABB();
        cookie_ = manager()->addAABB(this, prevAABB_, nullptr);
        dirty_ = false;
    }

    void RenderGizmoRotateComponent::onUnregister()
    {
        manager()->removeAABB(cookie_);
    }

    AABB RenderGizmoRotateComponent::calcAABB()
    {
        auto sz = btVector3(radius_, radius_, radius_);

        return AABB(targetXf_.getOrigin() - sz, targetXf_.getOrigin() + sz);
    }

    btTransform RenderGizmoRotateComponent::getTargetXf() const
    {
        return target_ ? target_->propertyGet(AProperty_WorldTransform).toTransform() : btTransform::getIdentity();
    }

    RenderGizmoRotateComponent::Sizes RenderGizmoRotateComponent::getSizes(const Frustum& frustum) const
    {
        auto viewExt = frustum.getExtents(targetXf_.getOrigin());

        Sizes ret;

        ret.radius1 = viewportRadius_ * viewExt.y();
        ret.width = viewportWidth_ * viewExt.y();
        ret.radius2 = ret.radius1 + ret.width * 10.0f;

        return ret;
    }
}
