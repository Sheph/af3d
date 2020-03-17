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

#include "Scene.h"
#include "Settings.h"
#include "SceneObject.h"
#include "SceneObjectFactory.h"
#include "Logger.h"
#include "InputManager.h"
#include "GameShell.h"
#include "Renderer.h"
#include "PhasedComponentManager.h"
#include "RenderComponentManager.h"
#include "PhasedComponent.h"
#include "RenderComponent.h"
#include "CameraComponent.h"
#include "PointLight.h"
#include "DirectionalLight.h"
#include <Rocket/Core/ElementDocument.h>
#include <cmath>

namespace af3d
{
    using TimerMap = std::map<std::int32_t, Scene::TimerFn>;

    class Scene::Impl
    {
    public:
        explicit Impl(Scene* scene)
        {
            phasedComponentManager_.reset(new PhasedComponentManager());
            renderComponentManager_.reset(new RenderComponentManager());

            timerIt_ = timers_.end();
        }

        ~Impl()
        {
        }

        std::unique_ptr<PhasedComponentManager> phasedComponentManager_;
        std::unique_ptr<RenderComponentManager> renderComponentManager_;
        TimerMap timers_;
        std::uint32_t nextTimerCookie_ = 1;
        TimerMap::const_iterator timerIt_;
    };

    Scene::Scene(const std::string& scriptPath)
    : impl_(new Impl(this)),
      inputMode_(InputMode::Game),
      playable_(false),
      paused_(false),
      cutscene_(false),
      quit_(false),
      firstUpdate_(true),
      checkpoint_(0),
      timeScale_(1.0f),
      realDt_(0.0f)
    {
        respawnPoint_.setIdentity();

        setScene(this);

        impl_->phasedComponentManager_->setScene(this);
        impl_->renderComponentManager_->setScene(this);

        camera_ = std::make_shared<SceneObject>();

        auto cc = std::make_shared<CameraComponent>();
        cc->setAspect(settings.viewAspect);
        camera_->addComponent(cc);

        addObject(camera_);

        dummy_ = std::make_shared<SceneObject>();

        lightC_ = std::make_shared<LightComponent>();

        dummy_->addComponent(lightC_);

        addObject(dummy_);

        updateInputMode();
    }

    Scene::~Scene()
    {
        impl_.reset();
    }

    void Scene::prepare()
    {
        auto obj = sceneObjectFactory.createLitBox(btVector3(300.0f, 1.0f, 300.0f), "glass1.png", false);
        obj->setPos(btVector3(50.0f, -3.0f, -50.0f));
        addObject(obj);

        int i = 0;
        for (float z = -20.0f; z >= -100.0f; z -= 7.0f) {
            for (float x = 2.0f; x < 50.0f; x += 5.0f) {
                auto obj = sceneObjectFactory.createLitBox(btVector3(1.0f, 2.0f, 3.0f), (i % 4 == 0) ? "" : ((i % 3 == 0) ? "glass1.png" : "bb.png"));
                ++i;
                obj->setPos(btVector3(x, 6.0f, z));
                obj->setRotation(btQuaternion(btRadians(x), btRadians(x), btRadians(x)));
                addObject(obj);
            }
        }

        obj = sceneObjectFactory.createColoredBox(btVector3(1.0f, 1.0f, 1.0f));
        obj->setPos(btVector3(0.0f, 0.0f, -5.0f));
        obj->setRotation(btQuaternion(0.0f, btRadians(10.0f), 0.0f));
        addObject(obj);

        auto l = std::make_shared<DirectionalLight>("light0");
        l->setTransform(makeLookDir(btVector3_zero, btVector3(-1.0f, -0.2f, -0.6f), btVector3_up));
        l->setColor(Color(1.0f, 1.0f, 1.0f, 0.5f));
        lighting()->addLight(l);

        auto l2 = std::make_shared<PointLight>("light1");
        l2->setTransform(makeLookDir(btVector3(20.0f, 20.0f, -30.0f), btVector3_forward, btVector3_up));
        l2->setRadius(70.0f);
        l2->setColor(Color(1.0f, 0.0f, 0.0f, 1.0f));
        lighting()->addLight(l2);

        l2 = std::make_shared<PointLight>("light2");
        l2->setTransform(makeLookDir(btVector3(25.0f, 15.0f, -75.0f), btVector3_forward, btVector3_up));
        l2->setRadius(45.0f);
        l2->setColor(Color(0.0f, 1.0f, 0.0f, 1.0f));
        lighting()->addLight(l2);
    }

    void Scene::cleanup()
    {
        playable_ = true;
        cutscene_ = false;
        updateInputMode();

        removeAllObjects();

        camera_.reset();
        dummy_.reset();

        lightC_.reset();

        impl_->timers_.clear();

        impl_->phasedComponentManager_->cleanup();
        impl_->renderComponentManager_->cleanup();
    }

    void Scene::registerComponent(const ComponentPtr& component)
    {
        if (std::dynamic_pointer_cast<PhasedComponent>(component)) {
            impl_->phasedComponentManager_->addComponent(component);
        } else if (std::dynamic_pointer_cast<RenderComponent>(component)) {
            impl_->renderComponentManager_->addComponent(component);
        } else {
            runtime_assert(false);
        }
    }

    void Scene::unregisterComponent(const ComponentPtr& component)
    {
        if (component->manager()) {
            component->manager()->removeComponent(component);
        }
    }

    void Scene::freezeComponent(const ComponentPtr& component)
    {
        if (component->manager()) {
            component->manager()->freezeComponent(component);
        }
    }

    void Scene::thawComponent(const ComponentPtr& component)
    {
        if (component->manager()) {
            component->manager()->thawComponent(component);
        }
    }

    void Scene::update(float dt)
    {
        realDt_ = dt;

        if (!paused_) {
            dt *= timeScale_;
        }

        bool forceUpdateRender = false;

        if (!paused_ && (inputMode_ != InputMode::Menu) && inputManager.keyboard().triggered(KI_ESCAPE)) {
            LOG4CPLUS_INFO(logger(), "game paused");

            setPaused(true);

            forceUpdateRender = true;
        }

        bool inputProcessed = true;

        if (!paused_) {
            for (std::uint32_t i = 0; i < 1; ++i) {
                if (!impl_->timers_.empty()) {
                    auto lastCookie = impl_->timers_.rbegin()->first;
                    for (impl_->timerIt_ = impl_->timers_.begin();
                        impl_->timerIt_ != impl_->timers_.end();) {
                        if (impl_->timerIt_->first > lastCookie) {
                            break;
                        }
                        const Scene::TimerFn& fn = impl_->timerIt_->second;
                        ++impl_->timerIt_;
                        fn(0);
                    }
                    impl_->timerIt_ = impl_->timers_.end();
                }

                impl_->phasedComponentManager_->update(0);

                if (i == 0) {
                    inputManager.processed();
                }
            }

            if (true) {
                inputManager.proceed();
            } else {
                inputManager.processed();
                inputProcessed = false;
            }

            impl_->phasedComponentManager_->preRender(dt);

            if (true) {
                //freezeThawObjects(cc->getTrueAABB());
            }

            impl_->renderComponentManager_->update(dt);
            auto cc = camera_->findComponent<CameraComponent>();
            cc->setViewport(AABB2i(Vector2i(settings.viewX, settings.viewY),
                Vector2i(settings.viewX + settings.viewWidth, settings.viewY + settings.viewHeight)));
            impl_->renderComponentManager_->cull(cc);
        } else {
            if (forceUpdateRender || !paused_) {
                impl_->renderComponentManager_->update(dt);
                auto cc = camera_->findComponent<CameraComponent>();
                cc->setViewport(AABB2i(Vector2i(settings.viewX, settings.viewY),
                    Vector2i(settings.viewX + settings.viewWidth, settings.viewY + settings.viewHeight)));
                impl_->renderComponentManager_->cull(cc);
            }
        }

        auto rn = impl_->renderComponentManager_->render();

        inputManager.update();

        if (inputProcessed) {
            inputManager.processed();
        } else {
            inputManager.proceed();
        }

        renderer.swap(rn);

        firstUpdate_ = false;
    }

    std::uint32_t Scene::addTimer(const TimerFn& fn)
    {
        impl_->timers_[impl_->nextTimerCookie_] = fn;

        return impl_->nextTimerCookie_++;
    }

    void Scene::removeTimer(std::uint32_t cookie)
    {
        TimerMap::iterator it = impl_->timers_.find(cookie);

        if (it == impl_->timers_.end()) {
            LOG4CPLUS_WARN(logger(), "removeTimer(" << cookie << "), " << cookie << " doesn't exist");
            return;
        }

        if (impl_->timerIt_ == it) {
            ++impl_->timerIt_;
        }

        impl_->timers_.erase(it);
    }

    void Scene::setGravity(const btVector3& value)
    {
    }

    btVector3 Scene::gravity() const
    {
        return btVector3_zero;
    }

    void Scene::setRespawnPoint(const btTransform& value)
    {
        playable_ = true;
        respawnPoint_ = value;
        updateInputMode();
    }

    void Scene::setCutscene(bool value)
    {
        cutscene_ = value;
        updateInputMode();
    }

    void Scene::setNextLevel(const std::string& scriptPath)
    {
        nextScriptPath_ = scriptPath;
    }

    void Scene::restartLevel()
    {
    }

    bool Scene::getNextLevel(std::string& scriptPath)
    {
        if (nextScriptPath_.empty()) {
            return false;
        }

        scriptPath = nextScriptPath_;

        return true;
    }

    void Scene::updateInputMode()
    {
        InputMode newInputMode;

        if (!paused_) {
            if (playable_) {
                newInputMode = cutscene_ ? InputMode::Cutscene : InputMode::Game;
            } else {
                newInputMode = InputMode::Menu;
            }
        } else {
            newInputMode = InputMode::Menu;
        }

        if (inputMode_ != newInputMode) {
            if (newInputMode == InputMode::Menu) {

            } else if (inputMode_ == InputMode::Menu) {

            }
        }

        inputMode_ = newInputMode;
    }

    void Scene::freezeThawObjects(const AABB& aabb)
    {
        static std::vector<SceneObjectPtr> tmp;

        tmp.reserve(objects().size());

        for (const auto& obj : objects()) {
            tmp.push_back(obj);
        }

        for (const auto& obj : tmp) {
            if (obj->frozen()) {
                if (obj->freezable()) {
                    /*
                     * Check if this object should be thawed.
                     */
                    btVector3 v(obj->freezeRadius(), obj->freezeRadius(), obj->freezeRadius());
                    AABB freezeAABB;
                    freezeAABB.lowerBound = obj->pos() - v;
                    freezeAABB.upperBound = obj->pos() + v;

                    if (aabb.overlaps(freezeAABB)) {
                        obj->thaw();
                    }
                } else {
                    /*
                     * This object doesn't want to be freezable anymore, thaw it.
                     */
                    obj->thaw();
                }
            } else if (obj->freezable()) {
                /*
                 * Check if this object should be frozen.
                 */
                btVector3 v(obj->freezeRadius(), obj->freezeRadius(), obj->freezeRadius());
                AABB freezeAABB;
                freezeAABB.lowerBound = obj->pos() - v;
                freezeAABB.upperBound = obj->pos() + v;

                if (aabb.overlaps(freezeAABB)) {
                    obj->freeze();
                }
            }
        }

        tmp.clear();
    }
}
