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

#include "PhysicsComponentManager.h"
#include "PhysicsComponent.h"
#include "Settings.h"
#include "SceneObject.h"
#include "MotionState.h"
#include "bullet/BulletCollision/NarrowPhaseCollision/btRaycastCallback.h"

namespace af3d
{
    namespace
    {
        struct RayResultCallback : public btCollisionWorld::RayResultCallback
        {
        public:
            RayResultCallback(const btVector3& p1, const btVector3& p2, const RayCastFn& fn)
            : p1_(p1),
              p2_(p2),
              fn_(fn) {}

            float addSingleResult(btCollisionWorld::LocalRayResult& rayResult, bool normalInWorldSpace) override
            {
                btVector3 normalWorld;
                if (normalInWorldSpace) {
                    normalWorld = rayResult.m_hitNormalLocal;
                } else {
                    normalWorld = rayResult.m_collisionObject->getWorldTransform().getBasis() * rayResult.m_hitNormalLocal;
                }
                btVector3 pointWorld;
                pointWorld.setInterpolate3(p1_, p2_, rayResult.m_hitFraction);

                auto shape = rayResult.m_collisionObject->getCollisionShape();
                if (shape->isCompound()) {
                    auto cShape = static_cast<const btCompoundShape*>(shape);
                    btAssert(m_childIdx >= 0);
                    btAssert(m_childIdx < cShape->getNumChildShapes());
                    shape = cShape->getChildShape(m_childIdx);
                }

                float f = fn_(const_cast<btCollisionShape*>(shape), pointWorld, normalWorld, rayResult.m_hitFraction);
                if ((f >= 0.0f) && (f < m_closestHitFraction)) {
                    m_closestHitFraction = f;
                }
                return m_closestHitFraction;
            }

        private:
            const btVector3& p1_;
            const btVector3& p2_;
            const RayCastFn& fn_;
        };
    };

    PhysicsComponentManager::PhysicsComponentManager(CollisionComponentManager* collisionMgr, btIDebugDraw* debugDraw)
    : collisionMgr_(collisionMgr),
      collisionDispatcher_(&collisionCfg_),
      world_(&collisionDispatcher_, &broadphase_, &solver_, &collisionCfg_)
    {
        world_.setWorldUserInfo(this);

        world_.setDebugDrawer(debugDraw);
    }

    PhysicsComponentManager::~PhysicsComponentManager()
    {
        btAssert(components_.empty());
        btAssert(frozenComponents_.empty());
    }

    PhysicsComponentManager* PhysicsComponentManager::fromWorld(btDynamicsWorld* world)
    {
        return static_cast<PhysicsComponentManager*>(world->getWorldUserInfo());
    }

    void PhysicsComponentManager::cleanup()
    {
        btAssert(components_.empty());
        btAssert(frozenComponents_.empty());
    }

    void PhysicsComponentManager::addComponent(const ComponentPtr& component)
    {
        PhysicsComponentPtr physicsComponent = std::static_pointer_cast<PhysicsComponent>(component);

        btAssert(!component->manager());

        components_.insert(physicsComponent);
        physicsComponent->setManager(this);
    }

    void PhysicsComponentManager::removeComponent(const ComponentPtr& component)
    {
        PhysicsComponentPtr physicsComponent = std::static_pointer_cast<PhysicsComponent>(component);

        if (components_.erase(physicsComponent) ||
            frozenComponents_.erase(physicsComponent)) {
            physicsComponent->setManager(nullptr);
        }
    }

    void PhysicsComponentManager::freezeComponent(const ComponentPtr& component)
    {
        PhysicsComponentPtr physicsComponent = std::static_pointer_cast<PhysicsComponent>(component);

        components_.erase(physicsComponent);
        frozenComponents_.insert(physicsComponent);
        component->onFreeze();
    }

    void PhysicsComponentManager::thawComponent(const ComponentPtr& component)
    {
        PhysicsComponentPtr physicsComponent = std::static_pointer_cast<PhysicsComponent>(component);

        frozenComponents_.erase(physicsComponent);
        components_.insert(physicsComponent);
        component->onThaw();
    }

    bool PhysicsComponentManager::update(float dt)
    {
        if (dt > 0.0f) {
            for (int i = 0; i < world_.getNumCollisionObjects(); ++i) {
                btCollisionObject* c = world_.getCollisionObjectArray()[i];
                btRigidBody* body = btRigidBody::upcast(c);
                if (body && (body->getActivationState() == DISABLE_DEACTIVATION) &&
                    body->isKinematicObject()) {
                    auto obj = SceneObject::fromBody(body);
                    auto ms = static_cast<MotionState*>(body->getMotionState());
                    btTransform xf;
                    btTransformUtil::integrateTransform(ms->smoothXf, obj->linearVelocity(), obj->angularVelocity(), dt, xf);
                    ms->smoothXf = xf;
                }
            }
        }

        return world_.stepSimulation(dt, settings.physics.maxSteps, settings.physics.fixedTimestep) > 0;
    }

    void PhysicsComponentManager::debugDraw(RenderList& rl)
    {
        for (const auto& c : components_) {
            c->debugDraw(rl);
        }
    }

    void PhysicsComponentManager::rayCast(const btVector3& p1, const btVector3& p2, const RayCastFn& fn) const
    {
        RayResultCallback cb(p1, p2, fn);
        cb.m_flags |= btTriangleRaycastCallback::kF_UseGjkConvexCastRaytest;
        world_.rayTest(p1, p2, cb);
    }
}
