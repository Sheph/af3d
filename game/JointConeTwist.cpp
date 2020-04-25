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
#include "RenderJointComponent.h"
#include "SceneObject.h"
#include "Scene.h"
#include "PhysicsDebugDraw.h"

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
    ACLASS_PROPERTY(JointConeTwist, Swing1, "swing1", "Swing 1", FloatRadian, 0.0f, Position, APropertyEditable)
    ACLASS_PROPERTY(JointConeTwist, Swing2, "swing2", "Swing 2", FloatRadian, 0.0f, Position, APropertyEditable)
    ACLASS_PROPERTY(JointConeTwist, Twist, "twist", "Twist", FloatRadian, 0.0f, Position, APropertyEditable)
    ACLASS_PROPERTY(JointConeTwist, Softness, "softness", "Describes % of limits where movement is free", FloatPercentage, 1.0f, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointConeTwist, BiasFactor, "bias factor", "Strength with which constraint resists zeroth order limit violation", FloatPercentage, 0.3f, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointConeTwist, RelaxationFactor, "relaxation factor", "The lower the value, the less the constraint will fight velocities which violate the angular limits", FloatPercentage, 1.0f, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointConeTwist, Damping, "damping", "Damping", Float, 0.01f, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointConeTwist, FixThreshold, "fix threshold", "Fix threshold", Float, 0.05f, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointConeTwist, MotorEnabled, "motor enabled", "Motor is enabled", Bool, false, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointConeTwist, MaxMotorImpulse, "max motor impulse", "Max motor impulse", Float, -1.0f, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointConeTwist, MotorTarget, "motor target", "Motor target", Quaternion, btQuaternion::getIdentity(), Physics, APropertyEditable)
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

    void JointConeTwist::render(bool drawA, PhysicsDebugDraw& dd, const btVector3& c, float sz)
    {
        btTransform tr = (drawA ? worldFrameA() : worldFrameB()) * fixup();

        const float length = sz;
        static int nSegments = 8 * 4;
        float fAngleInRadians = 2.0f * 3.1415926f * (float)(nSegments - 1) / float(nSegments);
        btVector3 pPrev = getPointForAngle(fAngleInRadians, length);
        pPrev = tr * pPrev;
        for (int i = 0; i < nSegments; i++) {
            fAngleInRadians = 2.0f * 3.1415926f * (float)i / float(nSegments);
            btVector3 pCur = getPointForAngle(fAngleInRadians, length);
            pCur = tr * pCur;
            dd.drawLine(pPrev, pCur, c);

            if (i % (nSegments / 8) == 0) {
                dd.drawLine(tr.getOrigin(), pCur, c);
            }

            pPrev = pCur;
        }
    }

    void JointConeTwist::setFrameA(const btTransform& value)
    {
        frameA_ = value;
        if (constraint_) {
            constraint_->setFrames(objectA()->localCenter().inverse() * frameAWithFixup(), constraint_->getBFrame());
        }
        setDirty();
    }

    void JointConeTwist::setFrameB(const btTransform& value)
    {
        frameB_ = value;
        if (constraint_) {
            constraint_->setFrames(constraint_->getAFrame(), objectB()->localCenter().inverse() * frameBWithFixup());
        }
        setDirty();
    }

    void JointConeTwist::setSwing1(float value)
    {
        swing1_ = value;
        if (constraint_) {
            constraint_->setLimit(5, value);
        }
        setDirty();
    }

    void JointConeTwist::setSwing2(float value)
    {
        swing2_ = value;
        if (constraint_) {
            constraint_->setLimit(4, value);
        }
        setDirty();
    }

    void JointConeTwist::setTwist(float value)
    {
        twist_ = value;
        if (constraint_) {
            constraint_->setLimit(3, value);
        }
        setDirty();
    }

    void JointConeTwist::setSoftness(float value)
    {
        softness_ = value;
        if (constraint_) {
            constraint_->setLimit(constraint_->getSwingSpan1(), constraint_->getSwingSpan2(),
                constraint_->getTwistSpan(), value,
                constraint_->getBiasFactor(), constraint_->getRelaxationFactor());
        }
        setDirty();
    }

    void JointConeTwist::setBiasFactor(float value)
    {
        biasFactor_ = value;
        if (constraint_) {
            constraint_->setLimit(constraint_->getSwingSpan1(), constraint_->getSwingSpan2(),
                constraint_->getTwistSpan(), constraint_->getLimitSoftness(),
                value, constraint_->getRelaxationFactor());
        }
        setDirty();
    }

    void JointConeTwist::setRelaxationFactor(float value)
    {
        relaxationFactor_ = value;
        if (constraint_) {
            constraint_->setLimit(constraint_->getSwingSpan1(), constraint_->getSwingSpan2(),
                constraint_->getTwistSpan(), constraint_->getLimitSoftness(),
                constraint_->getBiasFactor(), value);
        }
        setDirty();
    }

    void JointConeTwist::setDamping(float value)
    {
        damping_ = value;
        if (constraint_) {
            constraint_->setDamping(value);
        }
        setDirty();
    }

    void JointConeTwist::setFixThreshold(float value)
    {
        fixThreshold_ = value;
        if (constraint_) {
            constraint_->setFixThresh(value);
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

    void JointConeTwist::enableMotor(bool value)
    {
        motorEnabled_ = value;
        if (constraint_) {
            constraint_->enableMotor(value);
        }
        setDirty();
    }

    void JointConeTwist::setMaxMotorImpulse(float value)
    {
        maxMotorImpulse_ = value;
        if (constraint_) {
            constraint_->setMaxMotorImpulse(value);
        }
        setDirty();
    }

    void JointConeTwist::setMotorTarget(const btQuaternion& value)
    {
        motorTarget_ = value;
        if (constraint_) {
            auto q = fixup().getRotation();
            constraint_->setMotorTargetInConstraintSpace(q.inverse() * value.inverse() * q);
        }
        setDirty();
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
                constraint_->getRigidBodyB().isInWorld());
        } else {
            if (objA && objA->body() && objA->body()->isInWorld()) {
                if (objB && objB->body() && objB->body()->isInWorld()) {
                    constraint_ = new btConeTwistConstraint(*objA->body(), *objB->body(),
                        objA->localCenter().inverse() * frameAWithFixup(), objB->localCenter().inverse() * frameBWithFixup());
                }
            }
            if (constraint_) {
                constraint_->setLimit(swing1_, swing2_, twist_, softness_, biasFactor_, relaxationFactor_);
                constraint_->setDamping(damping_);
                constraint_->setFixThresh(fixThreshold_);
                constraint_->enableMotor(motorEnabled_);
                constraint_->setMaxMotorImpulse(maxMotorImpulse_);
                auto q = fixup().getRotation();
                constraint_->setMotorTargetInConstraintSpace(q.inverse() * motorTarget_.inverse() * q);
            }
        }
    }

    void JointConeTwist::doAdopt(bool withEdit)
    {
        if (withEdit) {
            editA_ = createTransformEdit(AProperty_WorldTransform, false, true);
            auto jc = std::make_shared<RenderJointComponent>();
            jc->setJoint(shared_from_this());
            jc->setIsA(true);
            editA_->addComponent(jc);

            if (objectB()) {
                editB_ = createTransformEdit("world frame B", false);
                jc = std::make_shared<RenderJointComponent>();
                jc->setJoint(shared_from_this());
                jc->setIsA(false);
                editB_->addComponent(jc);
            }
        }
    }

    void JointConeTwist::doAbandon()
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

    // From btConeTwistConstraint::GetPointForAngle
    btVector3 JointConeTwist::getPointForAngle(float fAngleInRadians, float fLength) const
    {
        btScalar xEllipse = btCos(fAngleInRadians);
        btScalar yEllipse = btSin(fAngleInRadians);
        btScalar swingLimit = swing1_;
        if (fabs(xEllipse) > SIMD_EPSILON) {
            btScalar surfaceSlope2 = (yEllipse * yEllipse) / (xEllipse * xEllipse);
            btScalar norm = 1 / (swing2_ * swing2_);
            norm += surfaceSlope2 / (swing1_ * swing1_);
            btScalar swingLimit2 = (1 + surfaceSlope2) / norm;
            swingLimit = std::sqrt(swingLimit2);
        }
        btVector3 vSwingAxis(0, xEllipse, -yEllipse);
        btQuaternion qSwing(vSwingAxis, swingLimit);
        btVector3 vPointInConstraintSpace(fLength, 0, 0);
        return quatRotate(qSwing, vPointInConstraintSpace);
    }

    const btTransform& JointConeTwist::fixup()
    {
        static const btTransform xf(btQuaternion(btVector3_up, SIMD_HALF_PI) * btQuaternion(btVector3_right, SIMD_HALF_PI), btVector3_zero);
        return xf;
    }

    btTransform JointConeTwist::frameAWithFixup() const
    {
        return frameA_ * fixup();
    }

    btTransform JointConeTwist::frameBWithFixup() const
    {
        return frameB_ * fixup();
    }
}
