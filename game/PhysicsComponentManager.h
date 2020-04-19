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

#ifndef _PHYSICSCOMPONENTMANAGER_H_
#define _PHYSICSCOMPONENTMANAGER_H_

#include "ComponentManager.h"
#include <unordered_set>
#include "bullet/btBulletDynamicsCommon.h"

namespace af3d
{
    class CollisionComponentManager;
    class PhysicsComponent;
    using PhysicsComponentPtr = std::shared_ptr<PhysicsComponent>;

    using RayCastFn = std::function<float(btCollisionShape*, const btVector3&, const btVector3&, float)>;

    class PhysicsComponentManager : public ComponentManager
    {
    public:
        PhysicsComponentManager(CollisionComponentManager* collisionMgr, btIDebugDraw* debugDraw);
        ~PhysicsComponentManager();

        static PhysicsComponentManager* fromWorld(btDynamicsWorld* world);

        void cleanup() override;

        void addComponent(const ComponentPtr& component) override;

        void removeComponent(const ComponentPtr& component) override;

        void freezeComponent(const ComponentPtr& component) override;

        void thawComponent(const ComponentPtr& component) override;

        bool update(float dt) override;

        void debugDraw(RenderList& rl) override;

        void rayCast(const btVector3& p1, const btVector3& p2, const RayCastFn& fn) const;

        inline btDiscreteDynamicsWorld& world() { return world_; }

    private:
        class CollisionDispatcher : public btCollisionDispatcher
        {
        public:
            CollisionDispatcher(CollisionComponentManager* collisionMgr,
                btCollisionConfiguration* collisionConfiguration);

            void releaseManifold(btPersistentManifold* manifold) override;

        private:
            CollisionComponentManager* collisionMgr_ = nullptr;
        };

        CollisionComponentManager* collisionMgr_ = nullptr;

        btDefaultCollisionConfiguration collisionCfg_;
        CollisionDispatcher collisionDispatcher_;
        btDbvtBroadphase broadphase_;
        btSequentialImpulseConstraintSolver solver_;
        btDiscreteDynamicsWorld world_;

        std::unordered_set<PhysicsComponentPtr> components_;
        std::unordered_set<PhysicsComponentPtr> frozenComponents_;
    };
}

#endif
