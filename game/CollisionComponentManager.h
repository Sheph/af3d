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

#ifndef _COLLISIONCOMPONENTMANAGER_H_
#define _COLLISIONCOMPONENTMANAGER_H_

#include "ComponentManager.h"
#include "SceneObjectManager.h"
#include "CollisionShape.h"
#include <unordered_set>
#include <unordered_map>

namespace af3d
{
    struct Contact
    {
        Contact() = default;
        explicit Contact(std::uint64_t cookie);
        ~Contact() = default;

        SceneObject* getOther(SceneObject* obj) const;

        const CollisionShapePtr& getOtherFixture(SceneObject* obj) const;

        const CollisionShapePtr& getThisFixture(SceneObject* obj) const;

        std::uint64_t cookie = 0;

        CollisionShapePtr shapeA;
        CollisionShapePtr shapeB;

        // pointCount is always 0 on 'endContact' and non-0 otherwise.
        int pointCount = 0;
        std::array<btManifoldPoint, MANIFOLD_CACHE_SIZE> points;
    };

    class CollisionComponent;
    using CollisionComponentPtr = std::shared_ptr<CollisionComponent>;

    class CollisionComponentManager : public ComponentManager
    {
    public:
        CollisionComponentManager() = default;
        ~CollisionComponentManager();

        void cleanup() override;

        void addComponent(const ComponentPtr& component) override;

        void removeComponent(const ComponentPtr& component) override;

        void freezeComponent(const ComponentPtr& component) override;

        void thawComponent(const ComponentPtr& component) override;

        bool update(float dt) override;

        void debugDraw(RenderList& rl) override;

        void step(btCollisionWorld* world);

        void flushPending();

        void endContact(btPersistentManifold* manifold);

    private:
        struct ContactEvent
        {
            ContactEvent() = default;

            Contact contact;
            bool isNew = false;

            // Keep objects alive for the time
            // of processing.
            SceneObjectPtr objA;
            SceneObjectPtr objB;

            // Keep components alive for the
            // time of processing.
            CollisionComponentPtr cA;
            CollisionComponentPtr cB;
        };

        using ContactEvents = std::vector<ContactEvent>;

        struct ContactInfo
        {
            std::uint64_t cookie = 0;

            CollisionShapePtr shapeA;
            CollisionShapePtr shapeB;

            int numPoints = 0;
            std::array<const btManifoldPoint*, MANIFOLD_CACHE_SIZE> points;
        };

        struct Manifold
        {
            Manifold() = default;
            explicit Manifold(btPersistentManifold* manifold);

            btPersistentManifold* manifold = nullptr;

            SceneObjectPtr objA;
            SceneObjectPtr objB;

            // Each of those MANIFOLD_CACHE_SIZE points can be for
            // different shape, thus, max MANIFOLD_CACHE_SIZE contacts per manifold.
            std::array<ContactInfo, MANIFOLD_CACHE_SIZE> contacts;
        };

        using Manifolds = std::unordered_map<btPersistentManifold*, Manifold>;

        void addEvents(Manifold& m);

        std::unordered_set<CollisionComponentPtr> components_;
        std::unordered_set<CollisionComponentPtr> frozenComponents_;

        // Only manifolds with at least some points are in this map.
        Manifolds manifolds_;
        ContactEvents events_;

        std::uint64_t nextCookie_ = 1;
    };
}

#endif
