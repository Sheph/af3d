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

#ifndef _RENDERCOMPONENTMANAGER_H_
#define _RENDERCOMPONENTMANAGER_H_

#include "ComponentManager.h"
#include "CameraComponent.h"
#include "RenderNode.h"
#include "VertexArrayWriter.h"
#include "af3d/Ray.h"
#include "bullet/BulletCollision/BroadphaseCollision/btDbvt.h"
#include <unordered_set>
#include <list>

namespace af3d
{
    struct RenderCookie {};

    class RenderComponent;
    using RenderComponentPtr = std::shared_ptr<RenderComponent>;

    using RayCastRenderFn = std::function<float(const RenderComponentPtr&, const AObjectPtr&, const btVector3&, float)>;

    class RenderComponentManager : public ComponentManager
    {
    public:
        RenderComponentManager() = default;
        ~RenderComponentManager();

        virtual void cleanup() override;

        virtual void addComponent(const ComponentPtr& component) override;

        virtual void removeComponent(const ComponentPtr& component) override;

        virtual void freezeComponent(const ComponentPtr& component) override;

        virtual void thawComponent(const ComponentPtr& component) override;

        virtual void update(float dt) override;

        virtual void debugDraw() override;

        RenderCookie* addAABB(RenderComponent* component,
            const AABB& aabb,
            void* data);

        void moveAABB(RenderCookie* cookie,
            const AABB& prevAABB,
            const AABB& aabb,
            const btVector3& displacement);

        void removeAABB(RenderCookie* cookie);

        void cull(const CameraComponentPtr& cc);

        RenderNodePtr render(VertexArrayWriter& defaultVa);

        void rayCast(const Frustum& frustum, const Ray& ray, const RayCastRenderFn& fn) const;

    private:
        struct NodeData
        {
            std::list<NodeData>::iterator it;
            RenderComponent* component;
            void* data;
        };

        using NodeDataList = std::list<NodeData>;
        using CullResultList = std::unordered_map<RenderComponent*, std::vector<void*>>;

        class CollideCull : public btDbvt::ICollide
        {
        public:
            CollideCull(const Frustum& frustum, CullResultList& cullResults);
            ~CollideCull() = default;

            void Process(const btDbvtNode* node) override;
            bool Descent(const btDbvtNode* node) override;

        private:
            const Frustum& frustum_;
            CullResultList& cullResults_;
        };

        class CollideRayCast : public btDbvt::ICollide
        {
        public:
            CollideRayCast(const Frustum& frustum, const Ray& ray, const RayCastRenderFn& fn);
            ~CollideRayCast() = default;

            void Process(const btDbvtNode* node) override;
            bool Descent(const btDbvtNode* node) override;

        private:
            const Frustum& frustum_;
            const Ray& ray_;
            const RayCastRenderFn& fn_;
            float maxT_ = std::numeric_limits<float>::max();
        };

        std::unordered_set<RenderComponentPtr> components_;
        std::unordered_set<RenderComponentPtr> frozenComponents_;

        NodeDataList nodeDataList_;

        btDbvt tree_;

        CameraComponent* cc_ = nullptr;
        CullResultList cullResults_;
    };
}

#endif
