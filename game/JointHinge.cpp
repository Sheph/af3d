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

#include "JointHinge.h"
#include "RenderJointComponent.h"
#include "SceneObject.h"
#include "Scene.h"
#include "PhysicsDebugDraw.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(JointHinge, Joint)
    JOINT_PARAM(JointHinge, AProperty_ObjectA, "Object A", SceneObject, SceneObjectPtr())
    JOINT_PARAM(JointHinge, AProperty_ObjectAPath, "Object A path", String, "")
    JOINT_PARAM(JointHinge, AProperty_ObjectB, "Object B", SceneObject, SceneObjectPtr())
    JOINT_PARAM(JointHinge, AProperty_ObjectBPath, "Object B path", String, "")
    JOINT_PARAM(JointHinge, AProperty_CollideConnected, "Collide connected bodies", Bool, false)
    ACLASS_PROPERTY(JointHinge, LocalFrameA, AProperty_LocalFrameA, "Local frame A", Transform, btTransform::getIdentity(), Position, APropertyEditable)
    ACLASS_PROPERTY(JointHinge, WorldFrameA, AProperty_WorldTransform, "World frame A", Transform, btTransform::getIdentity(), Position, APropertyEditable|APropertyTransient)
    ACLASS_PROPERTY(JointHinge, LocalFrameB, AProperty_LocalFrameB, "Local frame B", Transform, btTransform::getIdentity(), Position, APropertyEditable)
    ACLASS_PROPERTY(JointHinge, WorldFrameB, AProperty_WorldFrameB, "World frame B", Transform, btTransform::getIdentity(), Position, APropertyEditable|APropertyTransient)
    ACLASS_PROPERTY(JointHinge, LowerLimit, "lower limit", "Lower limit", FloatRadian, SIMD_HALF_PI, Position, APropertyEditable)
    ACLASS_PROPERTY(JointHinge, UpperLimit, "upper limit", "Upper limit", FloatRadian, -SIMD_HALF_PI, Position, APropertyEditable)
    ACLASS_PROPERTY(JointHinge, Softness, AProperty_Softness, "Describes % of limits where movement is free", UFloat, 0.9f, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointHinge, BiasFactor, AProperty_BiasFactor, "Strength with which constraint resists zeroth order limit violation", UFloat, 0.3f, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointHinge, RelaxationFactor, AProperty_RelaxationFactor, "The lower the value, the less the constraint will fight velocities which violate the angular limits", UFloat, 1.0f, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointHinge, MotorEnabled, AProperty_MotorEnabled, "Motor is enabled", Bool, false, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointHinge, MaxMotorImpulse, AProperty_MaxMotorImpulse, "Max motor impulse", Float, -1.0f, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointHinge, MotorVelocity, "motor velocity", "Motor velocity", FloatRadian, 0.0f, Physics, APropertyEditable)
    ACLASS_DEFINE_END(JointHinge)

    JointHinge::JointHinge(const SceneObjectPtr& objectA, const SceneObjectPtr& objectB,
        bool collideConnected)
    : Joint(AClass_JointHinge, objectA, objectB, collideConnected)
    {
    }

    const AClass& JointHinge::staticKlass()
    {
        return AClass_JointHinge;
    }

    AObjectPtr JointHinge::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<JointHinge>(
            SceneObject::fromObjectAndPath(propVals.get(AProperty_ObjectA).toObject<SceneObject>(), propVals.get(AProperty_ObjectAPath).toString()),
            SceneObject::fromObjectAndPath(propVals.get(AProperty_ObjectB).toObject<SceneObject>(), propVals.get(AProperty_ObjectBPath).toString()),
            propVals.get(AProperty_CollideConnected).toBool());
        obj->afterCreate(propVals);
        return obj;
    }

    void JointHinge::render(bool drawA, PhysicsDebugDraw& dd, const btVector3& c, float sz)
    {
        btTransform trOrig = drawA ? worldFrameA() : worldFrameB();
        btTransform tr = trOrig * fixup();

        btVector3& center = tr.getOrigin();
        btVector3 normal = tr.getBasis().getColumn(2);
        btVector3 axis = tr.getBasis().getColumn(0);

        if (drawA) {
            if (lowerLimit_ <= upperLimit_) {
                dd.drawArc(center, normal, axis, sz, sz, lowerLimit_, upperLimit_, c, true);
            } else {
                dd.drawArc(center, normal, axis, sz, sz, 0.0f, SIMD_2_PI, c, false);
            }
        } else {
            dd.drawArc(center, normal, axis, sz, sz, 0.0f, 0.0f, c, true);
        }
    }

    void JointHinge::setFrameA(const btTransform& value)
    {
        frameA_ = value;
        if (constraint_) {
            constraint_->setFrames(objectA()->localCenter().inverse() * frameAWithFixup(), constraint_->getBFrame());
        }
        setDirty();
    }

    void JointHinge::setFrameB(const btTransform& value)
    {
        frameB_ = value;
        if (constraint_) {
            constraint_->setFrames(constraint_->getAFrame(), objectB()->localCenter().inverse() * frameBWithFixup());
        }
        setDirty();
    }

    void JointHinge::setLowerLimit(float value)
    {
        lowerLimit_ = value;
        if (constraint_) {
            constraint_->setLimit(value, constraint_->getUpperLimit(),
                constraint_->getLimitSoftness(), constraint_->getLimitBiasFactor(), constraint_->getLimitRelaxationFactor());
        }
        setDirty();
    }

    void JointHinge::setUpperLimit(float value)
    {
        upperLimit_ = value;
        if (constraint_) {
            constraint_->setLimit(constraint_->getLowerLimit(), value,
                constraint_->getLimitSoftness(), constraint_->getLimitBiasFactor(), constraint_->getLimitRelaxationFactor());
        }
        setDirty();
    }

    void JointHinge::setSoftness(float value)
    {
        softness_ = value;
        if (constraint_) {
            constraint_->setLimit(constraint_->getLowerLimit(), constraint_->getUpperLimit(),
                value, constraint_->getLimitBiasFactor(), constraint_->getLimitRelaxationFactor());
        }
        setDirty();
    }

    void JointHinge::setBiasFactor(float value)
    {
        biasFactor_ = value;
        if (constraint_) {
            constraint_->setLimit(constraint_->getLowerLimit(), constraint_->getUpperLimit(),
                constraint_->getLimitSoftness(), value, constraint_->getLimitRelaxationFactor());
        }
        setDirty();
    }

    void JointHinge::setRelaxationFactor(float value)
    {
        relaxationFactor_ = value;
        if (constraint_) {
            constraint_->setLimit(constraint_->getLowerLimit(), constraint_->getUpperLimit(),
                constraint_->getLimitSoftness(), constraint_->getLimitBiasFactor(), value);
        }
        setDirty();
    }

    btTransform JointHinge::worldFrameA() const
    {
        auto objA = objectA();
        return objA ? (objA->transform() * frameA()) : (toTransform(pos()) * frameA());
    }

    void JointHinge::setWorldFrameA(const btTransform& value)
    {
        auto objA = objectA();
        if (objA) {
            setFrameA(objA->transform().inverse() * value);
        } else {
            setPos(value.getOrigin() - frameA().getOrigin());
        }
    }

    btTransform JointHinge::worldFrameB() const
    {
        auto objB = objectB();
        return objB ? (objB->transform() * frameB()) : (toTransform(pos()) * frameB());
    }

    void JointHinge::setWorldFrameB(const btTransform& value)
    {
        auto objB = objectB();
        if (objB) {
            setFrameB(objB->transform().inverse() * value);
        } else {
            setPos(value.getOrigin() - frameB().getOrigin());
        }
    }

    void JointHinge::enableMotor(bool value)
    {
        motorEnabled_ = value;
        if (constraint_) {
            constraint_->enableMotor(value);
        }
        setDirty();
    }

    void JointHinge::setMaxMotorImpulse(float value)
    {
        maxMotorImpulse_ = value;
        if (constraint_) {
            constraint_->setMaxMotorImpulse(value);
        }
        setDirty();
    }

    void JointHinge::setMotorVelocity(float value)
    {
        motorVelocity_ = value;
        if (constraint_) {
            constraint_->setMotorTargetVelocity(value);
        }
        setDirty();
    }

    void JointHinge::doRefresh(bool forceDelete)
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
        } else {
            if (objA && objA->body() && objA->body()->isInWorld()) {
                if (objB && objB->body() && objB->body()->isInWorld()) {
                    constraint_ = new btHingeConstraint(*objA->body(), *objB->body(),
                        objA->localCenter().inverse() * frameAWithFixup(), objB->localCenter().inverse() * frameBWithFixup(), true);
                }
            }
            if (constraint_) {
                constraint_->setLimit(lowerLimit_, upperLimit_, softness_, biasFactor_, relaxationFactor_);
                constraint_->enableMotor(motorEnabled_);
                constraint_->setMaxMotorImpulse(maxMotorImpulse_);
                constraint_->setMotorTargetVelocity(motorVelocity_);
            }
        }
    }

    void JointHinge::doAdopt(bool withEdit)
    {
        if (withEdit) {
            editA_ = createTransformEdit(AProperty_WorldTransform, false, true);
            auto jc = std::make_shared<RenderJointComponent>();
            jc->setJoint(shared_from_this());
            jc->setIsA(true);
            editA_->addComponent(jc);

            if (objectB()) {
                editB_ = createTransformEdit(AProperty_WorldFrameB, false);
                jc = std::make_shared<RenderJointComponent>();
                jc->setJoint(shared_from_this());
                jc->setIsA(false);
                editB_->addComponent(jc);
            }
        }
    }

    void JointHinge::doAbandon()
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

    const btTransform& JointHinge::fixup()
    {
        static const btTransform xf(btQuaternion(btVector3_up, SIMD_HALF_PI) * btQuaternion(btVector3_right, SIMD_HALF_PI), btVector3_zero);
        return xf;
    }

    btTransform JointHinge::frameAWithFixup() const
    {
        return frameA_ * fixup();
    }

    btTransform JointHinge::frameBWithFixup() const
    {
        return frameB_ * fixup();
    }
}
