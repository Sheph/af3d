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

#include "JointConeTwist.h"
#include "SceneObject.h"
#include "Scene.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(JointConeTwist, Joint)
    JOINT_PARAM(JointConeTwist, AProperty_ObjectA, "Object A", SceneObject, SceneObjectPtr())
    JOINT_PARAM(JointConeTwist, AProperty_ObjectB, "Object B", SceneObject, SceneObjectPtr())
    JOINT_PARAM(JointConeTwist, AProperty_CollideConnected, "Collide connected bodies", Bool, false)
    ACLASS_PROPERTY(JointConeTwist, LocalFrameA, "local frame A", "Local frame A", Transform, btTransform::getIdentity(), Position, APropertyEditable)
    ACLASS_PROPERTY(JointConeTwist, WorldFrameA, AProperty_WorldTransform, "World frame A", Transform, btTransform::getIdentity(), Position, APropertyEditable|APropertyTransient)
    ACLASS_PROPERTY(JointConeTwist, LocalFrameB, "local frame B", "Local frame B", Transform, btTransform::getIdentity(), Position, APropertyEditable)
    ACLASS_PROPERTY(JointConeTwist, WorldFrameB, "world frame B", "World frame B", Transform, btTransform::getIdentity(), Position, APropertyEditable|APropertyTransient)
    ACLASS_DEFINE_END(JointConeTwist)

    JointConeTwist::JointConeTwist(const SceneObjectPtr& objectA, const SceneObjectPtr& objectB,
        bool collideConnected)
    : Joint(AClass_JointConeTwist, objectA, objectB, collideConnected)
    {
    }

    const AClass& JointConeTwist::staticKlass()
    {
        return AClass_JointConeTwist;
    }

    AObjectPtr JointConeTwist::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<JointConeTwist>(propVals.get(AProperty_ObjectA).toObject<SceneObject>(),
            propVals.get(AProperty_ObjectB).toObject<SceneObject>(),
            propVals.get(AProperty_CollideConnected).toBool());
        obj->afterCreate(propVals);
        return obj;
    }

    void JointConeTwist::setFrameA(const btTransform& value)
    {
        frameA_ = value;
        if (constraint_) {
            constraint_->setFrames(value, constraint_->getBFrame());
        }
        setDirty();
    }

    void JointConeTwist::setFrameB(const btTransform& value)
    {
        frameB_ = value;
        if (constraint_) {
            constraint_->setFrames(constraint_->getAFrame(), value);
        }
        setDirty();
    }

    btTransform JointConeTwist::worldFrameA() const
    {
        auto objA = objectA();
        return objA ? (objA->transform() * frameA()) : (toTransform(pos()) * frameA());
    }

    void JointConeTwist::setWorldFrameA(const btTransform& value)
    {
        auto objA = objectA();
        if (objA) {
            setFrameA(objA->transform().inverse() * value);
        } else {
            setPos(value.getOrigin() - frameA().getOrigin());
        }
    }

    btTransform JointConeTwist::worldFrameB() const
    {
        auto objB = objectB();
        return objB ? (objB->transform() * frameB()) : (toTransform(pos()) * frameB());
    }

    void JointConeTwist::setWorldFrameB(const btTransform& value)
    {
        auto objB = objectB();
        if (objB) {
            setFrameB(objB->transform().inverse() * value);
        } else {
            setPos(value.getOrigin() - frameB().getOrigin());
        }
    }

    void JointConeTwist::doRefresh(bool forceDelete)
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
                (!hasBodyB() || constraint_->getRigidBodyB().isInWorld()));
        } else {
            if (objA && objA->body() && objA->body()->isInWorld()) {
                if (hasBodyB()) {
                    if (objB && objB->body() && objB->body()->isInWorld()) {
                        constraint_ = new btConeTwistConstraint(*objA->body(), *objB->body(), frameA_, frameB_);
                    }
                } else {
                    constraint_ = new btConeTwistConstraint(*objA->body(), frameA_);
                }
            }
        }
    }

    void JointConeTwist::doAdopt(bool withEdit)
    {
        if (withEdit) {
            editA_ = createPointEdit(AProperty_WorldTransform, true);
            editB_ = createPointEdit("world frame B");
        }
    }

    void JointConeTwist::doAbandon()
    {
        if (editA_) {
            editA_->removeFromParent();
            editA_.reset();
            editB_->removeFromParent();
            editB_.reset();
        }
    }
}