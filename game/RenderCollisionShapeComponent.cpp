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

#include "RenderCollisionShapeComponent.h"
#include "SceneObject.h"
#include "Scene.h"
#include "Settings.h"
#include "MaterialManager.h"
#include "PhysicsDebugDraw.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(RenderCollisionShapeComponent, RenderComponent)
    ACLASS_DEFINE_END(RenderCollisionShapeComponent)

    RenderCollisionShapeComponent::RenderCollisionShapeComponent()
    : RenderComponent(AClass_RenderCollisionShapeComponent)
    {
    }

    const AClass& RenderCollisionShapeComponent::staticKlass()
    {
        return AClass_RenderCollisionShapeComponent;
    }

    AObjectPtr RenderCollisionShapeComponent::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<RenderCollisionShapeComponent>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void RenderCollisionShapeComponent::update(float dt)
    {
        AABB aabb;
        shape_->shape()->getAabb(shape_->worldTransform(),
            aabb.lowerBound, aabb.upperBound);
        if (aabb == prevAABB_) {
            return;
        }

        btVector3 displacement = parent()->transform().getOrigin() - prevParentXf_.getOrigin();

        manager()->moveAABB(cookie_, prevAABB_, aabb, displacement);

        prevParentXf_ = parent()->transform();
        prevAABB_ = aabb;
    }

    void RenderCollisionShapeComponent::render(RenderList& rl, void* const* parts, size_t numParts)
    {
        auto w = scene()->workspace();
        auto em = w->emCollision();
        if (em->active() || (settings.editor.collisionColorOff.w() > 0.0f)) {
            PhysicsDebugDraw dd;
            dd.setRenderList(&rl);

            float alpha = 0.0f;

            for (int i = 0; i < shape_->shape()->getNumChildShapes(); ++i) {
                auto s = CollisionShape::fromShape(shape_->shape()->getChildShape(i));

                Color c;
                if (em->active()) {
                    if (em->isSelected(s->sharedThis())) {
                        c = settings.editor.collisionColorSelected;
                    } else if (em->isHovered(s->sharedThis())) {
                        c = settings.editor.collisionColorHovered;
                    } else {
                        c = settings.editor.collisionColorInactive;
                    }
                } else {
                    c = settings.editor.collisionColorOff;
                }

                dd.setAlpha(c.w());
                alpha = btMax(alpha, c.w());

                dd.drawTransform(s->worldTransform(), 0.1f);
                s->render(dd, toVector3(c));
            }

            dd.setAlpha(alpha);
            dd.drawTransform(parent()->worldCenter(), 0.1f);

            dd.flushLines();
        }
    }

    std::pair<AObjectPtr, float> RenderCollisionShapeComponent::testRay(const Frustum& frustum, const Ray& ray, void* part)
    {
        auto res = ray.testAABB(prevAABB_);
        if (res.first) {
            return std::make_pair(sharedThis(), res.second);
        } else {
            return std::make_pair(AObjectPtr(), 0.0f);
        }
    }

    void RenderCollisionShapeComponent::onRegister()
    {
        prevParentXf_ = parent()->transform();
        shape_->shape()->getAabb(shape_->worldTransform(),
            prevAABB_.lowerBound, prevAABB_.upperBound);
        cookie_ = manager()->addAABB(this, prevAABB_, nullptr);
    }

    void RenderCollisionShapeComponent::onUnregister()
    {
        manager()->removeAABB(cookie_);
    }
}
