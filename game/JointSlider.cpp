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

#include "JointSlider.h"
#include "RenderJointComponent.h"
#include "SceneObject.h"
#include "Scene.h"
#include "PhysicsDebugDraw.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(JointSlider, Joint)
    JOINT_PARAM(JointSlider, AProperty_ObjectA, "Object A", SceneObject, SceneObjectPtr())
    JOINT_PARAM(JointSlider, AProperty_ObjectAPath, "Object A path", String, "")
    JOINT_PARAM(JointSlider, AProperty_ObjectB, "Object B", SceneObject, SceneObjectPtr())
    JOINT_PARAM(JointSlider, AProperty_ObjectBPath, "Object B path", String, "")
    JOINT_PARAM(JointSlider, AProperty_CollideConnected, "Collide connected bodies", Bool, false)
    ACLASS_PROPERTY(JointSlider, LocalFrameA, AProperty_LocalFrameA, "Local frame A", Transform, btTransform::getIdentity(), Position, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, WorldFrameA, AProperty_WorldTransform, "World frame A", Transform, btTransform::getIdentity(), Position, APropertyEditable|APropertyTransient)
    ACLASS_PROPERTY(JointSlider, LocalFrameB, AProperty_LocalFrameB, "Local frame B", Transform, btTransform::getIdentity(), Position, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, WorldFrameB, AProperty_WorldFrameB, "World frame B", Transform, btTransform::getIdentity(), Position, APropertyEditable|APropertyTransient)
    ACLASS_PROPERTY(JointSlider, LowerLinearLimit, "lower lin limit", "Lower linear limit", Float, -1.0f, Position, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, UpperLinearLimit, "upper lin limit", "Upper linear limit", Float, 1.0f, Position, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, LowerAngularLimit, "lower ang limit", "Lower angular limit", FloatRadian, 0.0f, Position, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, UpperAngularLimit, "upper ang limit", "Upper angular limit", FloatRadian, 0.0f, Position, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, LinearSoftness, "lin softness", "Linear softness", UFloat, SLIDER_CONSTRAINT_DEF_SOFTNESS, Physics, APropertyEditable|APropertyTransient)
    ACLASS_PROPERTY(JointSlider, LinearRestitution, "lin restitution", "Linear restitution", UFloat, SLIDER_CONSTRAINT_DEF_RESTITUTION, Physics, APropertyEditable|APropertyTransient)
    ACLASS_PROPERTY(JointSlider, LinearDamping, "lin damping", "Linear damping", UFloat, SLIDER_CONSTRAINT_DEF_DAMPING, Physics, APropertyEditable|APropertyTransient)
    ACLASS_PROPERTY(JointSlider, AngularSoftness, "ang softness", "Angular softness", UFloat, SLIDER_CONSTRAINT_DEF_SOFTNESS, Physics, APropertyEditable|APropertyTransient)
    ACLASS_PROPERTY(JointSlider, AngularRestitution, "ang restitution", "Angular restitution", UFloat, SLIDER_CONSTRAINT_DEF_RESTITUTION, Physics, APropertyEditable|APropertyTransient)
    ACLASS_PROPERTY(JointSlider, AngularDamping, "ang damping", "Angular damping", UFloat, SLIDER_CONSTRAINT_DEF_DAMPING, Physics, APropertyEditable|APropertyTransient)
    ACLASS_PROPERTY(JointSlider, DirLinearSoftness, "dir lin softness", "Linear softness within limits", UFloat, SLIDER_CONSTRAINT_DEF_SOFTNESS, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, DirLinearRestitution, "dir lin restitution", "Linear restitution within limits", UFloat, SLIDER_CONSTRAINT_DEF_RESTITUTION, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, DirLinearDamping, "dir lin damping", "Linear damping within limits", UFloat, 0.0f, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, DirAngularSoftness, "dir ang softness", "Angular softness within limits", UFloat, SLIDER_CONSTRAINT_DEF_SOFTNESS, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, DirAngularRestitution, "dir ang restitution", "Angular restitution within limits", UFloat, SLIDER_CONSTRAINT_DEF_RESTITUTION, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, DirAngularDamping, "dir ang damping", "Angular damping within limits", UFloat, 0.0f, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, LimLinearSoftness, "lim lin softness", "Linear softness when hitting limit", UFloat, SLIDER_CONSTRAINT_DEF_SOFTNESS, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, LimLinearRestitution, "lim lin restitution", "Linear restitution when hitting limit", UFloat, SLIDER_CONSTRAINT_DEF_RESTITUTION, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, LimLinearDamping, "lim lin damping", "Linear damping when hitting limit", UFloat, SLIDER_CONSTRAINT_DEF_DAMPING, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, LimAngularSoftness, "lim ang softness", "Angular softness when hitting limit", UFloat, SLIDER_CONSTRAINT_DEF_SOFTNESS, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, LimAngularRestitution, "lim ang restitution", "Angular restitution when hitting limit", UFloat, SLIDER_CONSTRAINT_DEF_RESTITUTION, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, LimAngularDamping, "lim ang damping", "Angular damping when hitting limit", UFloat, SLIDER_CONSTRAINT_DEF_DAMPING, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, OrthoLinearSoftness, "ortho lin softness", "Linear softness against constraint axis", UFloat, SLIDER_CONSTRAINT_DEF_SOFTNESS, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, OrthoLinearRestitution, "ortho lin restitution", "Linear restitution against constraint axis", UFloat, SLIDER_CONSTRAINT_DEF_RESTITUTION, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, OrthoLinearDamping, "ortho lin damping", "Linear damping against constraint axis", UFloat, SLIDER_CONSTRAINT_DEF_DAMPING, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, OrthoAngularSoftness, "ortho ang softness", "Angular softness against constraint axis", UFloat, SLIDER_CONSTRAINT_DEF_SOFTNESS, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, OrthoAngularRestitution, "ortho ang restitution", "Angular restitution against constraint axis", UFloat, SLIDER_CONSTRAINT_DEF_RESTITUTION, Physics, APropertyEditable)
    ACLASS_PROPERTY(JointSlider, OrthoAngularDamping, "ortho ang damping", "Angular damping against constraint axis", UFloat, SLIDER_CONSTRAINT_DEF_DAMPING, Physics, APropertyEditable)
    ACLASS_DEFINE_END(JointSlider)

    JointSlider::JointSlider(const SceneObjectPtr& objectA, const SceneObjectPtr& objectB,
        bool collideConnected)
    : Joint(AClass_JointSlider, objectA, objectB, collideConnected)
    {
        dirConfig_.linearDamping = 0.0f;
        dirConfig_.angularDamping = 0.0f;
    }

    const AClass& JointSlider::staticKlass()
    {
        return AClass_JointSlider;
    }

    AObjectPtr JointSlider::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<JointSlider>(
            SceneObject::fromObjectAndPath(propVals.get(AProperty_ObjectA).toObject<SceneObject>(), propVals.get(AProperty_ObjectAPath).toString()),
            SceneObject::fromObjectAndPath(propVals.get(AProperty_ObjectB).toObject<SceneObject>(), propVals.get(AProperty_ObjectBPath).toString()),
            propVals.get(AProperty_CollideConnected).toBool());
        obj->afterCreate(propVals);
        return obj;
    }

    void JointSlider::render(bool drawA, PhysicsDebugDraw& dd, const btVector3& c, float sz)
    {
        if (drawA) {
            btTransform tr = worldFrameA() * fixup();

            btVector3 liMin = tr * btVector3(lowerLinearLimit_, 0.0f, 0.0f);
            btVector3 liMax = tr * btVector3(upperLinearLimit_, 0.0f, 0.0f);
            dd.drawLine(liMin, liMax, c);

            btVector3 normal = tr.getBasis().getColumn(0);
            btVector3 axis = tr.getBasis().getColumn(2);
            dd.drawArc(liMin, normal, axis, sz, sz, 0.0f, SIMD_2_PI, c, false);
            dd.drawArc(liMax, normal, axis, sz, sz, 0.0f, SIMD_2_PI, c, false);
        } else {
            btTransform tr = worldFrameB() * fixup();

            btVector3 normal = tr.getBasis().getColumn(0);
            btVector3 axis = tr.getBasis().getColumn(2);
            dd.drawArc(tr.getOrigin(), normal, axis, sz * 0.6f, sz * 0.6f, 0.0f, SIMD_2_PI, c, false);
        }
    }

    void JointSlider::setFrameA(const btTransform& value)
    {
        frameA_ = value;
        if (constraint_) {
            constraint_->setFrames(objectA()->localCenter().inverse() * frameAWithFixup(), constraint_->getFrameOffsetB());
        }
        setDirty();
    }

    void JointSlider::setFrameB(const btTransform& value)
    {
        frameB_ = value;
        if (constraint_) {
            constraint_->setFrames(constraint_->getFrameOffsetA(), objectB()->localCenter().inverse() * frameBWithFixup());
        }
        setDirty();
    }

    void JointSlider::setLowerLinearLimit(float value)
    {
        lowerLinearLimit_ = value;
        if (constraint_) {
            constraint_->setLowerLinLimit(value);
        }
        setDirty();
    }

    void JointSlider::setUpperLinearLimit(float value)
    {
        upperLinearLimit_ = value;
        if (constraint_) {
            constraint_->setUpperLinLimit(value);
        }
        setDirty();
    }

    void JointSlider::setLowerAngularLimit(float value)
    {
        lowerAngularLimit_ = value;
        if (constraint_) {
            constraint_->setUpperAngLimit(-lowerAngularLimit_);
        }
        setDirty();
    }

    void JointSlider::setUpperAngularLimit(float value)
    {
        upperAngularLimit_ = value;
        if (constraint_) {
            constraint_->setLowerAngLimit(-upperAngularLimit_);
        }
        setDirty();
    }

    void JointSlider::setDirConfig(const Config& value)
    {
        dirConfig_ = value;
        if (constraint_) {
            constraint_->setSoftnessDirLin(dirConfig_.linearSoftness);
            constraint_->setRestitutionDirLin(dirConfig_.linearRestitution);
            constraint_->setDampingDirLin(dirConfig_.linearDamping);

            constraint_->setSoftnessDirAng(dirConfig_.angularSoftness);
            constraint_->setRestitutionDirAng(dirConfig_.angularRestitution);
            constraint_->setDampingDirAng(dirConfig_.angularDamping);
        }
        setDirty();
    }

    void JointSlider::setLimConfig(const Config& value)
    {
        limConfig_ = value;
        if (constraint_) {
            constraint_->setSoftnessLimLin(limConfig_.linearSoftness);
            constraint_->setRestitutionLimLin(limConfig_.linearRestitution);
            constraint_->setDampingLimLin(limConfig_.linearDamping);

            constraint_->setSoftnessLimAng(limConfig_.angularSoftness);
            constraint_->setRestitutionLimAng(limConfig_.angularRestitution);
            constraint_->setDampingLimAng(limConfig_.angularDamping);
        }
        setDirty();
    }

    void JointSlider::setOrthoConfig(const Config& value)
    {
        orthoConfig_ = value;
        if (constraint_) {
            constraint_->setSoftnessOrthoLin(orthoConfig_.linearSoftness);
            constraint_->setRestitutionOrthoLin(orthoConfig_.linearRestitution);
            constraint_->setDampingOrthoLin(orthoConfig_.linearDamping);

            constraint_->setSoftnessOrthoAng(orthoConfig_.angularSoftness);
            constraint_->setRestitutionOrthoAng(orthoConfig_.angularRestitution);
            constraint_->setDampingOrthoAng(orthoConfig_.angularDamping);
        }
        setDirty();
    }

    btTransform JointSlider::worldFrameA() const
    {
        auto objA = objectA();
        return objA ? (objA->transform() * frameA()) : (toTransform(pos()) * frameA());
    }

    void JointSlider::setWorldFrameA(const btTransform& value)
    {
        auto objA = objectA();
        if (objA) {
            setFrameA(objA->transform().inverse() * value);
        } else {
            setPos(value.getOrigin() - frameA().getOrigin());
        }
    }

    btTransform JointSlider::worldFrameB() const
    {
        auto objB = objectB();
        return objB ? (objB->transform() * frameB()) : (toTransform(pos()) * frameB());
    }

    void JointSlider::setWorldFrameB(const btTransform& value)
    {
        auto objB = objectB();
        if (objB) {
            setFrameB(objB->transform().inverse() * value);
        } else {
            setPos(value.getOrigin() - frameB().getOrigin());
        }
    }

    void JointSlider::doRefresh(bool forceDelete)
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
                    constraint_ = new btSliderConstraint(*objA->body(), *objB->body(),
                        objA->localCenter().inverse() * frameAWithFixup(), objB->localCenter().inverse() * frameBWithFixup(), true);
                }
            }
            if (constraint_) {
                constraint_->setLowerLinLimit(lowerLinearLimit_);
                constraint_->setUpperLinLimit(upperLinearLimit_);
                constraint_->setLowerAngLimit(-upperAngularLimit_);
                constraint_->setUpperAngLimit(-lowerAngularLimit_);
                constraint_->setSoftnessDirLin(dirConfig_.linearSoftness);
                constraint_->setRestitutionDirLin(dirConfig_.linearRestitution);
                constraint_->setDampingDirLin(dirConfig_.linearDamping);
                constraint_->setSoftnessDirAng(dirConfig_.angularSoftness);
                constraint_->setRestitutionDirAng(dirConfig_.angularRestitution);
                constraint_->setDampingDirAng(dirConfig_.angularDamping);
                constraint_->setSoftnessLimLin(limConfig_.linearSoftness);
                constraint_->setRestitutionLimLin(limConfig_.linearRestitution);
                constraint_->setDampingLimLin(limConfig_.linearDamping);
                constraint_->setSoftnessLimAng(limConfig_.angularSoftness);
                constraint_->setRestitutionLimAng(limConfig_.angularRestitution);
                constraint_->setDampingLimAng(limConfig_.angularDamping);
                constraint_->setSoftnessOrthoLin(orthoConfig_.linearSoftness);
                constraint_->setRestitutionOrthoLin(orthoConfig_.linearRestitution);
                constraint_->setDampingOrthoLin(orthoConfig_.linearDamping);
                constraint_->setSoftnessOrthoAng(orthoConfig_.angularSoftness);
                constraint_->setRestitutionOrthoAng(orthoConfig_.angularRestitution);
                constraint_->setDampingOrthoAng(orthoConfig_.angularDamping);
            }
        }
    }

    void JointSlider::doAdopt(bool withEdit)
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

    void JointSlider::doAbandon()
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

    const btTransform& JointSlider::fixup()
    {
        static const btTransform xf(btQuaternion(btVector3_up, SIMD_HALF_PI) * btQuaternion(btVector3_right, SIMD_HALF_PI), btVector3_zero);
        return xf;
    }

    btTransform JointSlider::frameAWithFixup() const
    {
        return frameA_ * fixup();
    }

    btTransform JointSlider::frameBWithFixup() const
    {
        return frameB_ * fixup();
    }
}
