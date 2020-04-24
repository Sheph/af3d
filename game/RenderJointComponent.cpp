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

#include "RenderJointComponent.h"
#include "SceneObject.h"
#include "Scene.h"
#include "Settings.h"
#include "MaterialManager.h"
#include "PhysicsDebugDraw.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(RenderJointComponent, RenderComponent)
    ACLASS_DEFINE_END(RenderJointComponent)

    RenderJointComponent::RenderJointComponent()
    : RenderComponent(AClass_RenderJointComponent)
    {
    }

    const AClass& RenderJointComponent::staticKlass()
    {
        return AClass_RenderJointComponent;
    }

    AObjectPtr RenderJointComponent::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<RenderJointComponent>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void RenderJointComponent::update(float dt)
    {
        AABB aabb = calcAABB();
        if (aabb == prevAABB_) {
            return;
        }

        btVector3 displacement = parent()->transform().getOrigin() - prevParentXf_.getOrigin();

        manager()->moveAABB(cookie_, prevAABB_, aabb, displacement);

        prevParentXf_ = parent()->transform();
        prevAABB_ = aabb;
    }

    void RenderJointComponent::render(RenderList& rl, void* const* parts, size_t numParts)
    {
        auto emJoint = scene()->workspace()->emJoint();

        bool show = emJoint->active() || scene()->workspace()->emObject()->active() ||
            scene()->workspace()->emCollision()->active();

        if (!show) {
            return;
        }

        PhysicsDebugDraw dd;
        dd.setRenderList(&rl);

        Color c;
        if (emJoint->active()) {
            if (emJoint->isSelected(joint_)) {
                c = settings.editor.jointMarkerColorSelected;
            } else if (emJoint->isHovered(joint_)) {
                c = settings.editor.jointMarkerColorHovered;
            } else {
                c = settings.editor.jointMarkerColorInactive;
            }
        } else {
            c = settings.editor.jointMarkerColorOff;
        }

        dd.setAlpha(c.w());

        auto viewExt = rl.frustum().getExtents(parent()->pos());
        auto sz = viewportSize_ * viewExt.y();

        joint_->render(isA_, dd, toVector3(c), sz);

        dd.flushLines();
    }

    std::pair<AObjectPtr, float> RenderJointComponent::testRay(const Frustum& frustum, const Ray& ray, void* part)
    {
        auto res = ray.testAABB(prevAABB_);
        if (res.first) {
            return std::make_pair(sharedThis(), res.second);
        } else {
            return std::make_pair(AObjectPtr(), 0.0f);
        }
    }

    void RenderJointComponent::onRegister()
    {
        prevParentXf_ = parent()->transform();
        prevAABB_ = calcAABB();
        cookie_ = manager()->addAABB(this, prevAABB_, nullptr);
    }

    void RenderJointComponent::onUnregister()
    {
        manager()->removeAABB(cookie_);
    }

    AABB RenderJointComponent::calcAABB()
    {
        auto sz = btVector3(radius_, radius_, radius_);

        return AABB(parent()->pos() - sz, parent()->pos() + sz);
    }
}
