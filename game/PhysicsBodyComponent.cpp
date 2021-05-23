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

#include "PhysicsBodyComponent.h"
#include "SceneObject.h"
#include "Scene.h"
#include "Settings.h"
#include "MotionState.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(PhysicsBodyComponent, PhysicsComponent)
    ACLASS_PROPERTY(PhysicsBodyComponent, Children, AProperty_Children, "Children", ArrayAObject, std::vector<APropertyValue>{}, Hierarchy, 0)
    ACLASS_DEFINE_END(PhysicsBodyComponent)

    PhysicsBodyComponent::PhysicsBodyComponent()
    : PhysicsComponent(AClass_PhysicsBodyComponent),
      compound_(std::make_shared<CollisionShapeCompound>())
    {
        compound_->adopt(this);
    }

    PhysicsBodyComponent::~PhysicsBodyComponent()
    {
        for (int i = 0; i < numShapes(); ++i) {
            shape(i)->abandon();
        }
        compound_->abandon();
    }

    const AClass& PhysicsBodyComponent::staticKlass()
    {
        return AClass_PhysicsBodyComponent;
    }

    AObjectPtr PhysicsBodyComponent::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<PhysicsBodyComponent>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void PhysicsBodyComponent::addShape(const CollisionShapePtr& cs)
    {
        runtime_assert(!cs->parent());

        cs->adopt(this);

        compound_->shape()->addChildShape(cs->transform(), cs->shape());
        cs->assignUserPointer();

        if (parent()) {
            cs->setHoldingObject(parent());
        }

        updateBodyCollision(true);

        notifyShape(cs.get(), true);
    }

    void PhysicsBodyComponent::removeShape(const CollisionShapePtr& cs)
    {
        runtime_assert(cs->parent() == this);

        notifyShape(cs.get(), false);

        cs->abandon();

        compound_->shape()->removeChildShape(cs->shape());
        cs->resetUserPointer();

        updateBodyCollision(true);
    }

    int PhysicsBodyComponent::numShapes() const
    {
        return compound_->shape()->getNumChildShapes();
    }

    CollisionShape* PhysicsBodyComponent::shape(int index)
    {
        return CollisionShape::fromShape(compound_->shape()->getChildShape(index));
    }

    const CollisionShape* PhysicsBodyComponent::shape(int index) const
    {
        return CollisionShape::fromShape(compound_->shape()->getChildShape(index));
    }

    CollisionShapes PhysicsBodyComponent::getShapes()
    {
        CollisionShapes res;
        res.reserve(numShapes());
        for (int i = 0; i < numShapes(); ++i) {
            res.push_back(std::static_pointer_cast<CollisionShape>(shape(i)->sharedThis()));
        }
        return res;
    }

    CollisionShapes PhysicsBodyComponent::getShapes(const std::string& name)
    {
        CollisionShapes res;
        res.reserve(numShapes());
        for (int i = 0; i < numShapes(); ++i) {
            if (shape(i)->name() == name) {
                res.push_back(std::static_pointer_cast<CollisionShape>(shape(i)->sharedThis()));
            }
        }
        return res;
    }

    void PhysicsBodyComponent::onFreeze()
    {
    }

    void PhysicsBodyComponent::onThaw()
    {
    }

    void PhysicsBodyComponent::onRegister()
    {
        compound_->setHoldingObject(parent());

        btRigidBody* body;

        if (parent()->body()) {
            body = parent()->body();
            CollisionShape::fromShape(parent()->body()->getCollisionShape())->resetUserPointer();
            compound_->assignUserPointer(); // Inc ref count.
            parent()->body()->setCollisionShape(compound_->shape());

            for (int i = 0; i < numShapes(); ++i) {
                shape(i)->setHoldingObject(parent());
            }

            updateBodyCollision(true);
        } else {
            std::vector<float> masses(numShapes());
            float totalMass = 0.0f;
            for (int i = 0; i < numShapes(); ++i) {
                shape(i)->setHoldingObject(parent());
                masses[i] = shape(i)->mass();
                totalMass += masses[i];
            }

            btTransform principalXf = btTransform::getIdentity();
            btVector3 inertia = btVector3_zero;

            if (!masses.empty()) {
                calculatePrincipalAxisTransform(&masses[0], principalXf, inertia);
            }
            for (int i = 0; i < numShapes(); ++i) {
                compound_->shape()->updateChildTransform(i, principalXf.inverse() * shape(i)->transform(), i == (numShapes() - 1));
            }

            MotionState* motionState = new MotionState(principalXf, parent()->transform());

            if (parent()->bodyType() == BodyType::Dynamic) {
                if (totalMass <= 0.0f) {
                    totalMass = 1.0f;
                }
            } else {
                totalMass = 0.0f;
            }

            btRigidBody::btRigidBodyConstructionInfo bci(totalMass, motionState, compound_->shape(), inertia);
            bci.m_linearSleepingThreshold = parent()->linearSleepingThreshold();
            bci.m_angularSleepingThreshold = parent()->angularSleepingThreshold();
            bci.m_linearDamping = parent()->linearDamping();
            bci.m_angularDamping = parent()->angularDamping();
            bci.m_friction = parent()->friction();
            bci.m_restitution = parent()->restitution();

            compound_->assignUserPointer(); // Inc ref count.
            body = new btRigidBody(bci);
            if (parent()->bodyType() == BodyType::Static) {
                body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
            } else if (parent()->bodyType() == BodyType::Kinematic) {
                body->setCollisionFlags((body->getCollisionFlags() & ~btCollisionObject::CF_STATIC_OBJECT) | btCollisionObject::CF_KINEMATIC_OBJECT);
            }

            if (parent()->isSensor()) {
                body->setCollisionFlags(body->getCollisionFlags() | btCollisionObject::CF_NO_CONTACT_RESPONSE);
            }

            if (!parent()->gravityEnabled()) {
                body->setFlags(body->getFlags() | BT_DISABLE_WORLD_GRAVITY);
            }

            if (scene()->workspace() && settings.editor.disableSimulation) {
                body->setActivationState(DISABLE_SIMULATION);
            }

            if (parent()->bodyType() == BodyType::Dynamic) {
                body->setLinearVelocity(parent()->linearVelocity());
                body->setAngularVelocity(parent()->angularVelocity());
            }
        }

        if (!parent()->body()) {
            parent()->setBody(body);
        }

        if (parent()->physicsActive()) {
            manager()->world().addRigidBody(body);
        }

        notifyShapes(true);
    }

    void PhysicsBodyComponent::onUnregister()
    {
        setActive(false);
    }

    void PhysicsBodyComponent::updateBodyCollision(bool addRemove)
    {
        if (!parent()) {
            return;
        }

        std::vector<float> masses(numShapes());
        float totalMass = 0.0f;
        for (int i = 0; i < numShapes(); ++i) {
            masses[i] = shape(i)->mass();
            totalMass += masses[i];
        }

        btTransform principalXf = btTransform::getIdentity();
        btVector3 inertia = btVector3_zero;

        for (int i = 0; i < numShapes(); ++i) {
            compound_->shape()->updateChildTransform(i, shape(i)->transform(), false);
        }
        calculatePrincipalAxisTransform(&masses[0], principalXf, inertia);
        for (int i = 0; i < numShapes(); ++i) {
            compound_->shape()->updateChildTransform(i, principalXf.inverse() * shape(i)->transform(), i == (numShapes() - 1));
        }

        parent()->setLocalCenter(principalXf);

        if (parent()->bodyType() == BodyType::Dynamic) {
            if (totalMass <= 0.0f) {
                totalMass = 1.0f;
            }
        } else {
            totalMass = 0.0f;
        }

        parent()->body()->setMassProps(totalMass, inertia);
        if (parent()->bodyType() != BodyType::Static) {
            parent()->body()->setCollisionFlags(parent()->body()->getCollisionFlags() & ~btCollisionObject::CF_STATIC_OBJECT);
        }

        if (addRemove) {
            parent()->body()->setCollisionShape(compound_->shape());
            parent()->body()->activate(true);

            if (parent()->body()->isInWorld()) {
                manager()->world().removeRigidBody(parent()->body());
                manager()->world().addRigidBody(parent()->body());
            }
        }
    }

    void PhysicsBodyComponent::setActive(bool value)
    {
        if (!parent()) {
            return;
        }

        if (!value && parent()->body()->isInWorld()) {
            notifyShapes(false);
            manager()->world().removeRigidBody(parent()->body());
        } else if (value && !parent()->body()->isInWorld()) {
            manager()->world().addRigidBody(parent()->body());
            notifyShapes(true);
        }
    }

    void PhysicsBodyComponent::setWorldGravity()
    {
        if (!parent()) {
            return;
        }

        parent()->body()->setGravity(manager()->world().getGravity());
    }

    void PhysicsBodyComponent::calculatePrincipalAxisTransform(const btScalar* masses, btTransform& principal, btVector3& inertia)
    {
        if (numShapes() <= 0) {
            return;
        }

        for (int i = 0; i < numShapes(); ++i) {
            if (shape(i)->shape()->isConcave()) {
                // Concave shapes cannot be moved.
                return;
            }
        }

        compound_->shape()->calculatePrincipalAxisTransform(masses, principal, inertia);
    }

    void PhysicsBodyComponent::notifyShape(CollisionShape* shape, bool added)
    {
        if (!parent() || !parent()->body()->isInWorld()) {
            return;
        }

        if (added) {
            shape->onActivate();
        } else {
            shape->onDeactivate();
        }
    }

    void PhysicsBodyComponent::notifyShapes(bool active)
    {
        if (!parent() || !parent()->body()->isInWorld()) {
            return;
        }

        for (int i = 0; i < numShapes(); ++i) {
            if (active) {
                shape(i)->onActivate();
            } else {
                shape(i)->onDeactivate();
            }
        }

        if (((aflags() & AObjectEditable) != 0) && scene()->workspace()) {
            if (active) {
                btAssert(!markerRc_);
                markerRc_ = std::make_shared<RenderCollisionShapeComponent>();
                markerRc_->setShape(compound_);
                parent()->addComponent(markerRc_);
            } else {
                btAssert(markerRc_);
                markerRc_->removeFromParent();
                markerRc_.reset();
            }
        }
    }
}
