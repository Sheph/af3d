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

#include "JointPointToPoint.h"
#include "SceneObject.h"
#include "Scene.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(JointPointToPoint, Joint)
    JOINT_PARAM(JointPointToPoint, AProperty_ObjectA, "Object A", SceneObject, SceneObjectPtr())
    JOINT_PARAM(JointPointToPoint, AProperty_ObjectAPath, "Object A path", String, "")
    JOINT_PARAM(JointPointToPoint, AProperty_ObjectB, "Object B", SceneObject, SceneObjectPtr())
    JOINT_PARAM(JointPointToPoint, AProperty_ObjectBPath, "Object B path", String, "")
    JOINT_PARAM(JointPointToPoint, AProperty_CollideConnected, "Collide connected bodies", Bool, false)
    ACLASS_PROPERTY(JointPointToPoint, LocalPivotA, "local pivot A", "Local pivot A", Vec3f, btVector3(0.0f, 0.0f, 0.0f), Position, APropertyEditable)
    ACLASS_PROPERTY(JointPointToPoint, WorldPivotA, "world pivot A", "World pivot A", Vec3f, btVector3(0.0f, 0.0f, 0.0f), Position, APropertyEditable|APropertyTransient)
    ACLASS_PROPERTY(JointPointToPoint, LocalPivotB, "local pivot B", "Local pivot B", Vec3f, btVector3(0.0f, 0.0f, 0.0f), Position, APropertyEditable)
    ACLASS_PROPERTY(JointPointToPoint, WorldPivotB, "world pivot B", "World pivot B", Vec3f, btVector3(0.0f, 0.0f, 0.0f), Position, APropertyEditable|APropertyTransient)
    ACLASS_PROPERTY(JointPointToPoint, WorldPivotATransform, AProperty_WorldTransform, "World pivot A transform", Transform, btTransform::getIdentity(), Position, APropertyTransient)
    ACLASS_PROPERTY(JointPointToPoint, WorldPivotBTransform, "world pivot B transform", "World pivot B transform", Transform, btTransform::getIdentity(), Position, APropertyTransient)
    ACLASS_DEFINE_END(JointPointToPoint)

    JointPointToPoint::JointPointToPoint(const SceneObjectPtr& objectA, const SceneObjectPtr& objectB,
        bool collideConnected)
    : Joint(AClass_JointPointToPoint, objectA, objectB, collideConnected)
    {
    }

    const AClass& JointPointToPoint::staticKlass()
    {
        return AClass_JointPointToPoint;
    }

    AObjectPtr JointPointToPoint::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<JointPointToPoint>(
            SceneObject::fromObjectAndPath(propVals.get(AProperty_ObjectA).toObject<SceneObject>(), propVals.get(AProperty_ObjectAPath).toString()),
            SceneObject::fromObjectAndPath(propVals.get(AProperty_ObjectB).toObject<SceneObject>(), propVals.get(AProperty_ObjectBPath).toString()),
            propVals.get(AProperty_CollideConnected).toBool());
        obj->afterCreate(propVals);
        return obj;
    }

    void JointPointToPoint::render(bool drawA, PhysicsDebugDraw& dd, const btVector3& c, float sz)
    {
    }

    void JointPointToPoint::setPivotA(const btVector3& value)
    {
        pivotA_ = value;
        if (constraint_) {
            constraint_->setPivotA(objectA()->localCenter().inverse() * value);
        }
        setDirty();
    }

    void JointPointToPoint::setPivotB(const btVector3& value)
    {
        pivotB_ = value;
        if (constraint_) {
            constraint_->setPivotB(objectB()->localCenter().inverse() * value);
        }
        setDirty();
    }

    btVector3 JointPointToPoint::worldPivotA() const
    {
        auto objA = objectA();
        return objA ? objA->getWorldPoint(pivotA()) : (pos() + pivotA());
    }

    void JointPointToPoint::setWorldPivotA(const btVector3& value)
    {
        auto objA = objectA();
        if (objA) {
            setPivotA(objA->getLocalPoint(value));
        } else {
            setPos(value - pivotA());
        }
    }

    btVector3 JointPointToPoint::worldPivotB() const
    {
        auto objB = objectB();
        return objB ? objB->getWorldPoint(pivotB()) : (pos() + pivotB());
    }

    void JointPointToPoint::setWorldPivotB(const btVector3& value)
    {
        auto objB = objectB();
        if (objB) {
            setPivotB(objB->getLocalPoint(value));
        } else {
            setPos(value - pivotB());
        }
    }

    APropertyValue JointPointToPoint::propertyWorldPivotATransformGet(const std::string&) const
    {
        auto objA = objectA();
        if (objA) {
            return btTransform(objA->basis(), objA->getWorldPoint(pivotA()));
        } else {
            return toTransform(pos() + pivotA());
        }
    }

    void JointPointToPoint::propertyWorldPivotATransformSet(const std::string&, const APropertyValue& value)
    {
        setWorldPivotA(value.toTransform().getOrigin());
    }

    APropertyValue JointPointToPoint::propertyWorldPivotBTransformGet(const std::string&) const
    {
        auto objB = objectB();
        if (objB) {
            return btTransform(objB->basis(), objB->getWorldPoint(pivotB()));
        } else {
            return toTransform(pos() + pivotB());
        }
    }

    void JointPointToPoint::propertyWorldPivotBTransformSet(const std::string&, const APropertyValue& value)
    {
        setWorldPivotB(value.toTransform().getOrigin());
    }

    void JointPointToPoint::doRefresh(bool forceDelete)
    {
        auto objA = objectA();
        auto objB = objectB();
        if (objA && objB) {
            setPos(objA->pos());
        }

        if (forceDelete) {
            delete constraint_;
            constraint_ = nullptr;
            return;
        }

        if (constraint_) {
            constraint_->setEnabled(constraint_->getRigidBodyA().isInWorld() &&
                constraint_->getRigidBodyB().isInWorld());
        } else if (objA && objA->body() && objA->body()->isInWorld()) {
            if (objB && objB->body() && objB->body()->isInWorld()) {
                constraint_ = new btPoint2PointConstraint(*objA->body(), *objB->body(),
                    objA->localCenter().inverse() * pivotA_, objB->localCenter().inverse() * pivotB_);
            }
        }
    }

    void JointPointToPoint::doAdopt(bool withEdit)
    {
        if (withEdit) {
            editA_ = createTransformEdit(AProperty_WorldTransform, true, true);
            if (objectB()) {
                editB_ = createTransformEdit("world pivot B transform", true);
            }
        }
    }

    void JointPointToPoint::doAbandon()
    {
        if (editA_) {
            editA_->removeFromParent();
            editA_.reset();
        }
        if (editB_) {
            editB_->removeFromParent();
            editB_.reset();
        }
    }
}
