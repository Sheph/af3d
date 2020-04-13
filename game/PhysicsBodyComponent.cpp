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

        compound_->actualShape().addChildShape(cs->transform(), cs->shape());
        cs->assignUserPointer();
    }

    void PhysicsBodyComponent::removeShape(const CollisionShapePtr& cs)
    {
        runtime_assert(cs->parent() == this);

        cs->abandon();

        compound_->actualShape().removeChildShape(cs->shape());
        cs->resetUserPointer();
    }

    int PhysicsBodyComponent::numShapes() const
    {
        return compound_->actualShape().getNumChildShapes();
    }

    CollisionShape* PhysicsBodyComponent::shape(int index)
    {
        return CollisionShape::fromShape(compound_->actualShape().getChildShape(index));
    }

    const CollisionShape* PhysicsBodyComponent::shape(int index) const
    {
        return CollisionShape::fromShape(compound_->actualShape().getChildShape(index));
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
        std::vector<float> masses(numShapes());
        float totalMass = 0.0f;
        for (int i = 0; i < numShapes(); ++i) {
            masses[i] = shape(i)->mass();
            totalMass += masses[i];
        }

        btTransform principalXf = btTransform::getIdentity();
        btVector3 inertia = btVector3_zero;

        if (numShapes() > 0) {
            compound_->actualShape().calculatePrincipalAxisTransform(&masses[0], principalXf, inertia);
            for (int i = 0; i < numShapes(); ++i) {
                compound_->actualShape().updateChildTransform(i, shape(i)->transform() * principalXf.inverse(), i == (numShapes() - 1));
            }
        }

        btRigidBody* body;

        if (parent()->body()) {
            body = parent()->body();
            CollisionShape::fromShape(parent()->body()->getCollisionShape())->resetUserPointer();
            compound_->assignUserPointer(); // Inc ref count.
            parent()->body()->setCollisionShape(compound_->shape());
            static_cast<MotionState*>(parent()->body()->getMotionState())->centerOfMassXf = principalXf;
            parent()->body()->setMassProps(totalMass, inertia);
        } else {
            MotionState* motionState = new MotionState(principalXf, parent()->transform());

            btRigidBody::btRigidBodyConstructionInfo bci(totalMass, motionState, compound_->shape(), inertia);
            bci.m_linearDamping = parent()->linearDamping();
            bci.m_angularDamping = parent()->angularDamping();
            bci.m_friction = parent()->friction();
            bci.m_restitution = parent()->restitution();

            compound_->assignUserPointer(); // Inc ref count.
            body = new btRigidBody(bci);

            body->setLinearVelocity(parent()->linearVelocity());
            body->setAngularVelocity(parent()->angularVelocity());
        }

        if (parent()->physicsActive()) {
            manager()->world().addRigidBody(body);
        }

        if (!parent()->body()) {
            parent()->setBody(body);
        }
    }

    void PhysicsBodyComponent::onUnregister()
    {
        if (parent()->body()->isInWorld()) {
            manager()->world().removeRigidBody(parent()->body());
        }
    }
}
