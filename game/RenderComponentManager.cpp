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

#include "RenderComponentManager.h"
#include "RenderComponent.h"
#include "Settings.h"

namespace af3d
{
    RenderComponentManager::CollideCull::CollideCull(const Frustum& frustum, CullResultList& cullResults)
    : frustum_(frustum),
      cullResults_(cullResults)
    {
    }

    void RenderComponentManager::CollideCull::Process(const btDbvtNode* node)
    {
        auto nd = (NodeData*)node->data;
        cullResults_[nd->component].push_back(nd->data);
    }

    bool RenderComponentManager::CollideCull::Descent(const btDbvtNode* node)
    {
        return frustum_.isVisible(AABB(node->volume.Mins(), node->volume.Maxs()));
    }

    RenderComponentManager::CollideRayCast::CollideRayCast(const Frustum& frustum, const Ray& ray, const RayCastRenderFn& fn)
    : frustum_(frustum),
      ray_(ray),
      fn_(fn)
    {
    }

    void RenderComponentManager::CollideRayCast::Process(const btDbvtNode* node)
    {
        auto nd = (NodeData*)node->data;
        auto res = nd->component->testRay(frustum_, ray_, nd->data);
        if (!res.first || (res.second >= maxT_)) {
            return;
        }
        float newT = fn_(std::static_pointer_cast<RenderComponent>(nd->component->sharedThis()), res.first, ray_.getAt(res.second), res.second);
        if ((newT >= 0.0f) && (newT < maxT_)) {
            maxT_ = newT;
        }
    }

    bool RenderComponentManager::CollideRayCast::Descent(const btDbvtNode* node)
    {
        if (maxT_ <= 0.0f) {
            return false;
        }
        AABB aabb(node->volume.Mins(), node->volume.Maxs());
        if (aabb.contains(ray_.pos)) {
            return true;
        }
        auto res = ray_.testAABB(aabb);
        if (res.second >= maxT_) {
            return false;
        }
        return res.first;
    }

    RenderComponentManager::~RenderComponentManager()
    {
        assert(components_.empty());
        assert(frozenComponents_.empty());
    }

    void RenderComponentManager::cleanup()
    {
        assert(components_.empty());
        assert(frozenComponents_.empty());
    }

    void RenderComponentManager::addComponent(const ComponentPtr& component)
    {
        auto renderComponent = std::static_pointer_cast<RenderComponent>(component);

        btAssert(!component->manager());

        components_.insert(renderComponent);
        renderComponent->setManager(this);
    }

    void RenderComponentManager::removeComponent(const ComponentPtr& component)
    {
        auto renderComponent = std::static_pointer_cast<RenderComponent>(component);

        if (components_.erase(renderComponent) ||
            frozenComponents_.erase(renderComponent)) {
            renderComponent->setManager(nullptr);
        }
    }

    void RenderComponentManager::freezeComponent(const ComponentPtr& component)
    {
        auto renderComponent = std::static_pointer_cast<RenderComponent>(component);

        components_.erase(renderComponent);
        frozenComponents_.insert(renderComponent);
        component->onFreeze();
    }

    void RenderComponentManager::thawComponent(const ComponentPtr& component)
    {
        auto renderComponent = std::static_pointer_cast<RenderComponent>(component);

        frozenComponents_.erase(renderComponent);
        components_.insert(renderComponent);
        component->onThaw();
    }

    bool RenderComponentManager::update(float dt)
    {
        cullResults_.clear();

        for (const auto& c : components_) {
            c->update(dt);
            if (c->renderAlways()) {
                cullResults_[c.get()].push_back(nullptr);
            }
        }

        tree_.optimizeIncremental(1);

        return true;
    }

    void RenderComponentManager::debugDraw(RenderList& rl)
    {
        for (const auto& c : components_) {
            c->debugDraw(rl);
        }
    }

    RenderCookie* RenderComponentManager::addAABB(RenderComponent* component,
        const AABB& aabb,
        void* data)
    {
        auto it = nodeDataList_.insert(nodeDataList_.end(), NodeData());

        auto& nd = nodeDataList_.back();

        nd.it = it;
        nd.component = component;
        nd.data = data;

        return (RenderCookie*)tree_.insert(btDbvtVolume::FromMM(aabb.lowerBound, aabb.upperBound), &nd);
    }

    void RenderComponentManager::moveAABB(RenderCookie* cookie,
        const AABB& prevAABB,
        const AABB& aabb,
        const btVector3& displacement)
    {
        auto node = (btDbvtNode*)cookie;
        auto bv = btDbvtVolume::FromMM(aabb.lowerBound, aabb.upperBound);

        if (Intersect(node->volume, bv)) {
            auto prevBv = btDbvtVolume::FromMM(prevAABB.lowerBound, prevAABB.upperBound);

            btDbvtVolume combinedBv;
            Merge(prevBv, bv, combinedBv);

            auto d = displacement * 2.0f;

            tree_.update(node, combinedBv, d, 1.0f);
        } else {
            tree_.update(node, bv);
        }
    }

    void RenderComponentManager::removeAABB(RenderCookie* cookie)
    {
        auto node = (btDbvtNode*)cookie;
        auto nd = (NodeData*)node->data;

        nodeDataList_.erase(nd->it);

        tree_.remove(node);
    }

    void RenderComponentManager::render(RenderList& rl) const
    {
        CullResultList cr = cullResults_;
        CollideCull collide(rl.camera()->frustum(), cr);
        btDbvt::collideTU(tree_.m_root, collide);

        for (const auto& kv : cr) {
            if (kv.first->visible()) {
                kv.first->render(rl, &kv.second[0], kv.second.size());
            }
        }
    }

    void RenderComponentManager::rayCast(const Frustum& frustum, const Ray& ray, const RayCastRenderFn& fn) const
    {
        CollideRayCast collide(frustum, ray, fn);

        btDbvt::collideTU(tree_.m_root, collide);
    }
}
