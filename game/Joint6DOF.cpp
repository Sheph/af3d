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

#include "Joint6DOF.h"
#include "RenderJointComponent.h"
#include "SceneObject.h"
#include "Scene.h"
#include "PhysicsDebugDraw.h"

bool matrixToEulerXYZ(const btMatrix3x3& mat, btVector3& xyz);

namespace af3d
{
    #define ANGULAR_PROPS(x, X) \
        ACLASS_PROPERTY(Joint6DOF, LowerAngular##X##Limit, "lower ang " #x " limit", "Lower angular " #x " limit", FloatRadian, 1.0f, Position, APropertyEditable) \
        ACLASS_PROPERTY(Joint6DOF, UpperAngular##X##Limit, "upper ang " #x " limit", "Upper angular " #x " limit", FloatRadian, -1.0f, Position, APropertyEditable) \
        ACLASS_PROPERTY(Joint6DOF, Angular##X##Softness, "ang " #x " softness", "Angular " #x " softness", UFloat, 0.5f, Physics, APropertyEditable) \
        ACLASS_PROPERTY(Joint6DOF, Angular##X##Damping, "ang " #x " damping", "Angular " #x " damping", UFloat, 1.0f, Physics, APropertyEditable) \
        ACLASS_PROPERTY(Joint6DOF, Angular##X##Restitution, "ang " #x " restitution", "Angular " #x " restitution", UFloat, 0.0f, Physics, APropertyEditable) \
        ACLASS_PROPERTY(Joint6DOF, Angular##X##MotorEnabled, "ang " #x " motor enabled", "Angular " #x " motor is enabled", Bool, false, Physics, APropertyEditable) \
        ACLASS_PROPERTY(Joint6DOF, Angular##X##MotorVelocity, "ang " #x " motor velocity", "Angular " #x " motor velocity", FloatRadian, 0.0f, Position, APropertyEditable) \
        ACLASS_PROPERTY(Joint6DOF, MaxAngular##X##MotorForce, "ang " #x " max motor force", "Max angular " #x " motor force", Float, 6.0f, Position, APropertyEditable) \
        ACLASS_PROPERTY(Joint6DOF, MaxAngular##X##LimitForce, "ang " #x " max limit force", "Max angular " #x " limit force", Float, 300.0f, Position, APropertyEditable)

    ACLASS_DEFINE_BEGIN(Joint6DOF, Joint)
    JOINT_PARAM(Joint6DOF, AProperty_ObjectA, "Object A", SceneObject, SceneObjectPtr())
    JOINT_PARAM(Joint6DOF, AProperty_ObjectAPath, "Object A path", String, "")
    JOINT_PARAM(Joint6DOF, AProperty_ObjectB, "Object B", SceneObject, SceneObjectPtr())
    JOINT_PARAM(Joint6DOF, AProperty_ObjectBPath, "Object B path", String, "")
    JOINT_PARAM(Joint6DOF, AProperty_CollideConnected, "Collide connected bodies", Bool, false)
    ACLASS_PROPERTY(Joint6DOF, LocalFrameA, AProperty_LocalFrameA, "Local frame A", Transform, btTransform::getIdentity(), Position, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, WorldFrameA, AProperty_WorldTransform, "World frame A", Transform, btTransform::getIdentity(), Position, APropertyEditable|APropertyTransient)
    ACLASS_PROPERTY(Joint6DOF, LocalFrameB, AProperty_LocalFrameB, "Local frame B", Transform, btTransform::getIdentity(), Position, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, WorldFrameB, AProperty_WorldFrameB, "World frame B", Transform, btTransform::getIdentity(), Position, APropertyEditable|APropertyTransient)
    ACLASS_PROPERTY(Joint6DOF, LowerLinearLimit, "lower lin limit", "Lower linear limit", Vec3f, btVector3(0.0f, 0.0f, 0.0f), Position, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, UpperLinearLimit, "upper lin limit", "Upper linear limit", Vec3f, btVector3(0.0f, 0.0f, 0.0f), Position, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, LinearSoftness, "lin softness", "Linear softness", UFloat, 0.7f, Physics, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, LinearDamping, "lin damping", "Linear damping", UFloat, 1.0f, Physics, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, LinearRestitution, "lin restitution", "Linear restitution", UFloat, 0.5f, Physics, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, LinearXMotorEnabled, "lin x motor enabled", "Linear x motor is enabled", Bool, false, Physics, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, LinearYMotorEnabled, "lin y motor enabled", "Linear y motor is enabled", Bool, false, Physics, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, LinearZMotorEnabled, "lin z motor enabled", "Linear z motor is enabled", Bool, false, Physics, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, LinearMotorVelocity, "lin motor velocity", "Linear motor velocity", Vec3f, btVector3(0.0f, 0.0f, 0.0f), Position, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, MaxLinearMotorForce, "lin max motor force", "Max linear motor force", Vec3f, btVector3(0.0f, 0.0f, 0.0f), Position, APropertyEditable)
    ANGULAR_PROPS(x, X)
    ANGULAR_PROPS(y, Y)
    ANGULAR_PROPS(z, Z)
    ACLASS_PROPERTY(Joint6DOF, LinearXSpringEnabled, "lin x spring enabled", "Linear x spring is enabled", Bool, false, Physics, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, LinearYSpringEnabled, "lin y spring enabled", "Linear y spring is enabled", Bool, false, Physics, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, LinearZSpringEnabled, "lin z spring enabled", "Linear z spring is enabled", Bool, false, Physics, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, AngularXSpringEnabled, "ang x spring enabled", "Angular x spring is enabled", Bool, false, Physics, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, AngularYSpringEnabled, "ang y spring enabled", "Angular y spring is enabled", Bool, false, Physics, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, AngularZSpringEnabled, "ang z spring enabled", "Angular z spring is enabled", Bool, false, Physics, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, LinearSpringStiffness, "lin spring stiffness", "Linear spring stiffness", Vec3f, btVector3(0.0f, 0.0f, 0.0f), Physics, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, AngularSpringStiffness, "ang spring stiffness", "Angular spring stiffness", Vec3f, btVector3(0.0f, 0.0f, 0.0f), Physics, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, LinearSpringDamping, "lin spring damping", "Linear spring damping", Vec3f, btVector3(1.0f, 1.0f, 1.0f), Physics, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, AngularSpringDamping, "ang spring damping", "Angular spring damping", Vec3f, btVector3(1.0f, 1.0f, 1.0f), Physics, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, LocalSpringEquilibriumPoint, "local spring eq pt", "Local spring equilibrium point", Transform, btTransform::getIdentity(), Position, APropertyEditable)
    ACLASS_PROPERTY(Joint6DOF, WorldSpringEquilibriumPoint, "world spring eq pt", "World spring equilibrium point", Transform, btTransform::getIdentity(), Position, APropertyEditable|APropertyTransient)
    ACLASS_DEFINE_END(Joint6DOF)

    Joint6DOF::Joint6DOF(const SceneObjectPtr& objectA, const SceneObjectPtr& objectB,
        bool collideConnected)
    : Joint(AClass_Joint6DOF, objectA, objectB, collideConnected)
    {
    }

    const AClass& Joint6DOF::staticKlass()
    {
        return AClass_Joint6DOF;
    }

    AObjectPtr Joint6DOF::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<Joint6DOF>(
            SceneObject::fromObjectAndPath(propVals.get(AProperty_ObjectA).toObject<SceneObject>(), propVals.get(AProperty_ObjectAPath).toString()),
            SceneObject::fromObjectAndPath(propVals.get(AProperty_ObjectB).toObject<SceneObject>(), propVals.get(AProperty_ObjectBPath).toString()),
            propVals.get(AProperty_CollideConnected).toBool());
        obj->afterCreate(propVals);
        return obj;
    }

    void Joint6DOF::render(bool drawA, PhysicsDebugDraw& dd, const btVector3& c, float tsz)
    {
        if (drawA) {
            if (lowerLinearLimit_.x() <= upperLinearLimit_.x() ||
                lowerLinearLimit_.y() <= upperLinearLimit_.y() ||
                lowerLinearLimit_.z() <= upperLinearLimit_.z()) {
                btTransform tr = worldFrameA();
                dd.drawBox(lowerLinearLimit_, upperLinearLimit_, tr, c);
            }
        } else {
            btTransform tr = worldFrameA();
            const btVector3& center = worldFrameB().getOrigin();
            btVector3 up = tr.getBasis().getColumn(2);
            btVector3 axis = tr.getBasis().getColumn(0);
            float minTh = -angularCfg_[AxisY].upperLimit;
            float maxTh = -angularCfg_[AxisY].lowerLimit;
            float minPs = -angularCfg_[AxisZ].upperLimit;
            float maxPs = -angularCfg_[AxisZ].lowerLimit;
            bool haveSpherePatch = false;
            if ((minTh <= maxTh) || (minPs <= maxPs)) {
                dd.drawSpherePatch(center, up, axis, tsz * 0.7f, minTh, maxTh, minPs, maxPs, c, 20.0f);
                haveSpherePatch = true;
            }

            btVector3 angleDiff;
            btMatrix3x3 relativeFrame = tr.getBasis().inverse() * worldFrameB().getBasis();
            matrixToEulerXYZ(relativeFrame, angleDiff);

            axis = tr.getBasis().getColumn(1);
            float ay = angleDiff.y();
            float az = angleDiff.z();
            float cy = btCos(ay);
            float sy = btSin(ay);
            float cz = btCos(az);
            float sz = btSin(az);
            btVector3 ref;
            ref[0] = cy * cz * axis[0] + cy * sz * axis[1] - sy * axis[2];
            ref[1] = -sz * axis[0] + cz * axis[1];
            ref[2] = cz * sy * axis[0] + sz * sy * axis[1] + cy * axis[2];
            tr = worldFrameB();
            btVector3 normal = -tr.getBasis().getColumn(0);
            float minFi = -angularCfg_[AxisX].upperLimit;
            float maxFi = -angularCfg_[AxisX].lowerLimit;
            if (minFi > maxFi) {
                if (haveSpherePatch) {
                    dd.drawArc(center, normal, ref, tsz, tsz, -SIMD_PI, SIMD_PI, c, false);
                }
            } else if (minFi < maxFi) {
                dd.drawArc(center, normal, ref, tsz, tsz, minFi, maxFi, c, true);
            }
        }
    }

    void Joint6DOF::setFrameA(const btTransform& value)
    {
        frameA_ = value;
        if (constraint_) {
            constraint_->setFrames(objectA()->localCenter().inverse() * frameA_, constraint_->getFrameOffsetB());
        }
        setDirty();
    }

    void Joint6DOF::setFrameB(const btTransform& value)
    {
        frameB_ = value;
        if (constraint_) {
            constraint_->setFrames(constraint_->getFrameOffsetA(), objectB()->localCenter().inverse() * frameB_);
        }
        setDirty();
    }

    void Joint6DOF::setLowerLinearLimit(const btVector3& value)
    {
        lowerLinearLimit_ = value;
        if (constraint_) {
            constraint_->setLinearLowerLimit(lowerLinearLimit_);
        }
        setDirty();
    }

    void Joint6DOF::setUpperLinearLimit(const btVector3& value)
    {
        upperLinearLimit_ = value;
        if (constraint_) {
            constraint_->setLinearUpperLimit(upperLinearLimit_);
        }
        setDirty();
    }

    void Joint6DOF::setLinearSoftness(float value)
    {
        linearSoftness_ = value;
        if (constraint_) {
            constraint_->getTranslationalLimitMotor()->m_limitSoftness = linearSoftness_;
        }
        setDirty();
    }

    void Joint6DOF::setLinearDamping(float value)
    {
        linearDamping_ = value;
        if (constraint_) {
            constraint_->getTranslationalLimitMotor()->m_damping = linearDamping_;
        }
        setDirty();
    }

    void Joint6DOF::setLinearRestitution(float value)
    {
        linearRestitution_ = value;
        if (constraint_) {
            constraint_->getTranslationalLimitMotor()->m_restitution = linearRestitution_;
        }
        setDirty();
    }

    void Joint6DOF::enableLinearMotor(Axis axis, bool value)
    {
        linearMotorEnabled_[axis] = value;
        if (constraint_) {
            constraint_->getTranslationalLimitMotor()->m_enableMotor[axis] = value;
        }
        setDirty();
    }

    void Joint6DOF::setLinearMotorVelocity(const btVector3& value)
    {
        linearMotorVelocity_ = value;
        if (constraint_) {
            constraint_->getTranslationalLimitMotor()->m_targetVelocity = -linearMotorVelocity_;
        }
        setDirty();
    }

    void Joint6DOF::setMaxLinearMotorForce(const btVector3& value)
    {
        maxLinearMotorForce_ = value;
        if (constraint_) {
            constraint_->getTranslationalLimitMotor()->m_maxMotorForce = maxLinearMotorForce_;
        }
        setDirty();
    }

    void Joint6DOF::setLowerAngularLimit(Axis axis, float value)
    {
        angularCfg_[axis].lowerLimit = value;
        if (constraint_) {
            constraint_->getRotationalLimitMotor(axis)->m_hiLimit = -angularCfg_[axis].lowerLimit;
        }
        setDirty();
    }

    void Joint6DOF::setUpperAngularLimit(Axis axis, float value)
    {
        angularCfg_[axis].upperLimit = value;
        if (constraint_) {
            constraint_->getRotationalLimitMotor(axis)->m_loLimit = -angularCfg_[axis].upperLimit;
        }
        setDirty();
    }

    void Joint6DOF::setAngularSoftness(Axis axis, float value)
    {
        angularCfg_[axis].softness = value;
        if (constraint_) {
            constraint_->getRotationalLimitMotor(axis)->m_limitSoftness = angularCfg_[axis].softness;
        }
        setDirty();
    }

    void Joint6DOF::setAngularDamping(Axis axis, float value)
    {
        angularCfg_[axis].damping = value;
        if (constraint_) {
            constraint_->getRotationalLimitMotor(axis)->m_damping = angularCfg_[axis].damping;
        }
        setDirty();
    }

    void Joint6DOF::setAngularRestitution(Axis axis, float value)
    {
        angularCfg_[axis].restitution = value;
        if (constraint_) {
            constraint_->getRotationalLimitMotor(axis)->m_bounce = angularCfg_[axis].restitution;
        }
        setDirty();
    }

    void Joint6DOF::enableAngularMotor(Axis axis, bool value)
    {
        angularCfg_[axis].motorEnabled = value;
        if (constraint_) {
            constraint_->getRotationalLimitMotor(axis)->m_enableMotor = angularCfg_[axis].motorEnabled;
        }
        setDirty();
    }

    void Joint6DOF::setAngularMotorVelocity(Axis axis, float value)
    {
        angularCfg_[axis].motorVelocity = value;
        if (constraint_) {
            constraint_->getRotationalLimitMotor(axis)->m_targetVelocity = -angularCfg_[axis].motorVelocity;
        }
        setDirty();
    }

    void Joint6DOF::setMaxAngularMotorForce(Axis axis, float value)
    {
        angularCfg_[axis].maxMotorForce = value;
        if (constraint_) {
            constraint_->getRotationalLimitMotor(axis)->m_maxMotorForce = angularCfg_[axis].maxMotorForce;
        }
        setDirty();
    }

    void Joint6DOF::setMaxAngularLimitForce(Axis axis, float value)
    {
        angularCfg_[axis].maxLimitForce = value;
        if (constraint_) {
            constraint_->getRotationalLimitMotor(axis)->m_maxLimitForce = angularCfg_[axis].maxLimitForce;
        }
        setDirty();
    }

    void Joint6DOF::enableLinearSpring(Axis axis, bool value)
    {
        linearSpringEnabled_[axis] = value;
        if (constraint_) {
            if (constraint_->isSpringEnabled(axis) ^ linearSpringEnabled_[axis]) {
                constraint_->enableSpring(axis, linearSpringEnabled_[axis]);
            }
        }
        setDirty();
    }

    void Joint6DOF::enableAngularSpring(Axis axis, bool value)
    {
        angularSpringEnabled_[axis] = value;
        if (constraint_) {
            if (constraint_->isSpringEnabled(axis + 3) ^ angularSpringEnabled_[axis]) {
                constraint_->enableSpring(axis + 3, angularSpringEnabled_[axis]);
            }
        }
        setDirty();
    }

    void Joint6DOF::setLinearSpringStiffness(const btVector3& value)
    {
        linearSpringStiffness_ = value;
        if (constraint_) {
            constraint_->setStiffness(AxisX, linearSpringStiffness_.x());
            constraint_->setStiffness(AxisY, linearSpringStiffness_.y());
            constraint_->setStiffness(AxisZ, linearSpringStiffness_.z());
        }
        setDirty();
    }

    void Joint6DOF::setAngularSpringStiffness(const btVector3& value)
    {
        angularSpringStiffness_ = value;
        if (constraint_) {
            constraint_->setStiffness(AxisX + 3, angularSpringStiffness_.x());
            constraint_->setStiffness(AxisY + 3, angularSpringStiffness_.y());
            constraint_->setStiffness(AxisZ + 3, angularSpringStiffness_.z());
        }
        setDirty();
    }

    void Joint6DOF::setLinearSpringDamping(const btVector3& value)
    {
        linearSpringDamping_ = value;
        if (constraint_) {
            constraint_->setDamping(AxisX, linearSpringDamping_.x());
            constraint_->setDamping(AxisY, linearSpringDamping_.y());
            constraint_->setDamping(AxisZ, linearSpringDamping_.z());
        }
        setDirty();
    }

    void Joint6DOF::setAngularSpringDamping(const btVector3& value)
    {
        angularSpringDamping_ = value;
        if (constraint_) {
            constraint_->setDamping(AxisX + 3, angularSpringDamping_.x());
            constraint_->setDamping(AxisY + 3, angularSpringDamping_.y());
            constraint_->setDamping(AxisZ + 3, angularSpringDamping_.z());
        }
        setDirty();
    }

    void Joint6DOF::setSpringEquilibriumPoint(const btTransform& value)
    {
        springEquilibriumPoint_ = value;
        if (constraint_) {
            constraint_->setEquilibriumPoint(AxisX, springEquilibriumPoint_.getOrigin().x());
            constraint_->setEquilibriumPoint(AxisY, springEquilibriumPoint_.getOrigin().y());
            constraint_->setEquilibriumPoint(AxisZ, springEquilibriumPoint_.getOrigin().z());
            btVector3 euler;
            matrixToEulerXYZ(springEquilibriumPoint_.getBasis(), euler);
            constraint_->setEquilibriumPoint(AxisX + 3, euler.x());
            constraint_->setEquilibriumPoint(AxisY + 3, euler.y());
            constraint_->setEquilibriumPoint(AxisZ + 3, euler.z());
        }
        setDirty();
    }

    btTransform Joint6DOF::worldSpringEquilibriumPoint() const
    {
        auto objA = objectA();
        return objA ? (objA->transform() * springEquilibriumPoint_) : (toTransform(pos()) * springEquilibriumPoint_);
    }

    void Joint6DOF::setWorldSpringEquilibriumPoint(const btTransform& value)
    {
        auto objA = objectA();
        if (objA) {
            setSpringEquilibriumPoint(objA->transform().inverse() * value);
        } else {
            setSpringEquilibriumPoint(toTransform(pos()).inverse() * value);
        }
    }

    btTransform Joint6DOF::worldFrameA() const
    {
        auto objA = objectA();
        return objA ? (objA->transform() * frameA()) : (toTransform(pos()) * frameA());
    }

    void Joint6DOF::setWorldFrameA(const btTransform& value)
    {
        auto objA = objectA();
        if (objA) {
            setFrameA(objA->transform().inverse() * value);
        } else {
            setPos(value.getOrigin() - frameA().getOrigin());
        }
    }

    btTransform Joint6DOF::worldFrameB() const
    {
        auto objB = objectB();
        return objB ? (objB->transform() * frameB()) : (toTransform(pos()) * frameB());
    }

    void Joint6DOF::setWorldFrameB(const btTransform& value)
    {
        auto objB = objectB();
        if (objB) {
            setFrameB(objB->transform().inverse() * value);
        } else {
            setPos(value.getOrigin() - frameB().getOrigin());
        }
    }

    void Joint6DOF::doRefresh(bool forceDelete)
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
                    constraint_ = new btGeneric6DofSpringConstraint(*objA->body(), *objB->body(),
                        objA->localCenter().inverse() * frameA_, objB->localCenter().inverse() * frameB_, true);
                }
            }
            if (constraint_) {
                constraint_->setLinearLowerLimit(lowerLinearLimit_);
                constraint_->setLinearUpperLimit(upperLinearLimit_);
                constraint_->getTranslationalLimitMotor()->m_limitSoftness = linearSoftness_;
                constraint_->getTranslationalLimitMotor()->m_damping = linearDamping_;
                constraint_->getTranslationalLimitMotor()->m_restitution = linearRestitution_;
                constraint_->getTranslationalLimitMotor()->m_targetVelocity = -linearMotorVelocity_;
                constraint_->getTranslationalLimitMotor()->m_maxMotorForce = maxLinearMotorForce_;
                for (Axis axis = AxisX; axis <= AxisZ; axis = static_cast<Axis>(static_cast<int>(axis) + 1)) {
                    constraint_->getTranslationalLimitMotor()->m_enableMotor[axis] = linearMotorEnabled_[axis];
                    auto rm = constraint_->getRotationalLimitMotor(axis);
                    rm->m_loLimit = -angularCfg_[axis].upperLimit;
                    rm->m_hiLimit = -angularCfg_[axis].lowerLimit;
                    rm->m_limitSoftness = angularCfg_[axis].softness;
                    rm->m_damping = angularCfg_[axis].damping;
                    rm->m_bounce = angularCfg_[axis].restitution;
                    rm->m_enableMotor = angularCfg_[axis].motorEnabled;
                    rm->m_targetVelocity = -angularCfg_[axis].motorVelocity;
                    rm->m_maxMotorForce = angularCfg_[axis].maxMotorForce;
                    rm->m_maxLimitForce = angularCfg_[axis].maxLimitForce;
                    if (constraint_->isSpringEnabled(axis) ^ linearSpringEnabled_[axis]) {
                        constraint_->enableSpring(axis, linearSpringEnabled_[axis]);
                    }
                    if (constraint_->isSpringEnabled(axis + 3) ^ angularSpringEnabled_[axis]) {
                        constraint_->enableSpring(axis + 3, angularSpringEnabled_[axis]);
                    }
                    constraint_->setStiffness(axis, linearSpringStiffness_[axis]);
                    constraint_->setStiffness(axis + 3, angularSpringStiffness_[axis]);
                    constraint_->setDamping(axis, linearSpringDamping_[axis]);
                    constraint_->setDamping(axis + 3, angularSpringDamping_[axis]);
                }
                constraint_->setEquilibriumPoint(AxisX, springEquilibriumPoint_.getOrigin().x());
                constraint_->setEquilibriumPoint(AxisY, springEquilibriumPoint_.getOrigin().y());
                constraint_->setEquilibriumPoint(AxisZ, springEquilibriumPoint_.getOrigin().z());
                btVector3 euler;
                matrixToEulerXYZ(springEquilibriumPoint_.getBasis(), euler);
                constraint_->setEquilibriumPoint(AxisX + 3, euler.x());
                constraint_->setEquilibriumPoint(AxisY + 3, euler.y());
                constraint_->setEquilibriumPoint(AxisZ + 3, euler.z());
            }
        }
    }

    void Joint6DOF::doAdopt(bool withEdit)
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

    void Joint6DOF::doAbandon()
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
