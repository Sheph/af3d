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

namespace af3d
{
    RenderComponentManager::Collide::Collide(const Frustum& frustum, CullResultList& cullResults)
    : frustum_(frustum),
      cullResults_(cullResults)
    {
    }

    void RenderComponentManager::Collide::Process(const btDbvtNode* node)
    {
        auto nd = (NodeData*)node->data;
        cullResults_[nd->component].push_back(nd->data);
    }

    bool RenderComponentManager::Collide::Descent(const btDbvtNode* node)
    {
        return frustum_.isVisible(AABB(node->volume.Mins(), node->volume.Maxs()));
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

    void RenderComponentManager::update(float dt)
    {
        cullResults_.clear();
        cc_ = nullptr;

        for (const auto& c : components_) {
            c->update(dt);
            if (c->renderAlways()) {
                cullResults_[c.get()].push_back(nullptr);
            }
        }

        tree_.optimizeIncremental(1);
    }

    void RenderComponentManager::debugDraw()
    {
        for (const auto& c : components_) {
            c->debugDraw();
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

    void RenderComponentManager::cull(const CameraComponentPtr& cc)
    {
        Collide collide(cc->getFrustum(), cullResults_);

        btDbvt::collideTU(tree_.m_root, collide);
        cc_ = cc.get();
    }

    RenderNodePtr RenderComponentManager::render()
    {
        RenderList rl(cc_->shared_from_this());

        for (const auto& kv : cullResults_) {
            if (kv.first->visible()) {
                kv.first->render(rl, &kv.second[0], kv.second.size());
            }
        }

        return rl.compile();
    }
}
