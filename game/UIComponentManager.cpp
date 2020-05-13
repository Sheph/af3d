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

#include "UIComponentManager.h"
#include "UIComponent.h"
#include "Scene.h"
#include "Settings.h"

namespace af3d
{
    bool UIComponentComparer::operator()(const UIComponentPtr& l, const UIComponentPtr& r) const
    {
        if (l->zOrder() == r->zOrder()) {
            return l.get() < r.get();
        } else {
            return l->zOrder() < r->zOrder();
        }
    }

    UIComponentManager::UIComponentManager()
    : uiCamera_(std::make_shared<Camera>())
    {
        uiCamera_->setProjectionType(ProjectionType::Orthographic);
        uiCamera_->setAspect(settings.viewAspect);
        uiCamera_->setNearDist(-1.0f);
        uiCamera_->setFarDist(1.0f);
        uiCamera_->setFlipY(true);
        uiCamera_->setClearMask(AttachmentPoints());
    }

    UIComponentManager::~UIComponentManager()
    {
        btAssert(components_.empty());
    }

    void UIComponentManager::cleanup()
    {
        btAssert(components_.empty());
    }

    void UIComponentManager::addComponent(const ComponentPtr& component)
    {
        auto uiComponent = std::static_pointer_cast<UIComponent>(component);

        btAssert(!component->manager());

        components_.insert(uiComponent);
        uiComponent->setManager(this);
    }

    void UIComponentManager::removeComponent(const ComponentPtr& component)
    {
        auto uiComponent = std::static_pointer_cast<UIComponent>(component);

        if (components_.erase(uiComponent)) {
            uiComponent->setManager(nullptr);
        }
    }

    void UIComponentManager::freezeComponent(const ComponentPtr& component)
    {
    }

    void UIComponentManager::thawComponent(const ComponentPtr& component)
    {
    }

    bool UIComponentManager::update(float dt)
    {
        static std::vector<UIComponentPtr> tmp;

        tmp.reserve(components_.size());

        for (const auto& c : components_) {
            tmp.push_back(c);
        }

        for (const auto& c : tmp) {
            if (c->manager() &&
                (!scene()->paused() || (c->zOrder() > 0))) {
                c->update(dt);
            }
        }

        tmp.resize(0);

        return true;
    }

    void UIComponentManager::debugDraw(RenderList& rl)
    {
        for (const auto& c : components_) {
            c->debugDraw(rl);
        }
    }

    RenderNodePtr UIComponentManager::render(const SceneEnvironmentPtr& env)
    {
        AABB2i viewport(Vector2i(settings.viewX, settings.viewY),
                Vector2i(settings.viewX + settings.viewWidth, settings.viewY + settings.viewHeight));

        uiCamera_->setOrthoHeight(settings.viewHeight);
        btTransform xf = btTransform::getIdentity();
        xf.setOrigin(btVector3(0.5f * settings.viewWidth, 0.5f * settings.viewHeight, 0.0f));
        uiCamera_->setTransform(xf);
        uiCamera_->setViewport(viewport);

        RenderList rl(uiCamera_, env);

        for (const auto& c : components_) {
            if (c->visible()) {
                c->render(rl);
            }
        }

        if (inputManager.gameDebugPressed()) {
            debugDraw(rl);
        }

        return rl.compile();
    }
}
