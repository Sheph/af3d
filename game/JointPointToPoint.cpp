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

namespace af3d
{
    ACLASS_DEFINE_BEGIN(JointPointToPoint, Joint)
    JOINT_PARAM(JointPointToPoint, "object A", "Object A", SceneObject, SceneObjectPtr())
    JOINT_PARAM(JointPointToPoint, "object B", "Object B", SceneObject, SceneObjectPtr())
    JOINT_PARAM(JointPointToPoint, "collide connected", "Collide connected bodies", Bool, false)
    ACLASS_PROPERTY(JointPointToPoint, PivotA, "pivot A", "Pivot A", Vec3f, btVector3(0.0f, 0.0f, 0.0f), Position, APropertyEditable)
    ACLASS_PROPERTY(JointPointToPoint, PivotB, "pivot B", "Pivot B", Vec3f, btVector3(0.0f, 0.0f, 0.0f), Position, APropertyEditable)
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
        auto obj = std::make_shared<JointPointToPoint>(propVals.get("object A").toObject<SceneObject>(),
            propVals.get("object B").toObject<SceneObject>(),
            propVals.get("collide connected").toBool());
        obj->afterCreate(propVals);
        return obj;
    }

    void JointPointToPoint::setPivotA(const btVector3& value)
    {
        pivotA_ = value;
        if (constraint_) {
            constraint_->setPivotA(value);
        }
        setDirty();
    }

    void JointPointToPoint::setPivotB(const btVector3& value)
    {
        pivotB_ = value;
        if (constraint_) {
            constraint_->setPivotB(value);
        }
        setDirty();
    }

    btVector3 JointPointToPoint::doGetPos() const
    {
        return objectA()->getWorldPoint(constraint_->getPivotInA());
    }

    void JointPointToPoint::doSetPos(const btVector3& pos)
    {
        setPivotA(objectA()->getLocalPoint(pos));
        if (hasBodyB()) {
            setPivotB(objectB()->getLocalPoint(pos));
        }
    }

    void JointPointToPoint::doRefresh(bool forceDelete)
    {
        if (forceDelete) {
            delete constraint_;
            constraint_ = nullptr;
            return;
        }

        if (constraint_) {
            constraint_->setEnabled(constraint_->getRigidBodyA().isInWorld() &&
                (!hasBodyB() || constraint_->getRigidBodyB().isInWorld()));
        } else {
            auto objA = objectA();
            if (objA && objA->body() && objA->body()->isInWorld()) {
                if (hasBodyB()) {
                    auto objB = objectB();
                    if (objB && objB->body() && objB->body()->isInWorld()) {
                        constraint_ = new btPoint2PointConstraint(*objA->body(), *objB->body(), pivotA_, pivotB_);
                    }
                } else {
                    constraint_ = new btPoint2PointConstraint(*objA->body(), pivotA_);
                }
            }
        }
    }
}
