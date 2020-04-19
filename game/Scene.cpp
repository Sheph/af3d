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
#include "SceneAsset.h"
#include "SceneObject.h"
#include "SceneObjectFactory.h"
#include "Logger.h"
#include "InputManager.h"
#include "GameShell.h"
#include "Renderer.h"
#include "PhasedComponentManager.h"
#include "PhysicsComponentManager.h"
#include "RenderComponentManager.h"
#include "UIComponentManager.h"
#include "CollisionComponentManager.h"
#include "PhasedComponent.h"
#include "PhysicsComponent.h"
#include "RenderComponent.h"
#include "CollisionComponent.h"
#include "CameraComponent.h"
#include "UIComponent.h"
#include "MeshManager.h"
#include "PointLight.h"
#include "DirectionalLight.h"
#include "VertexArrayWriter.h"
#include "HardwareResourceManager.h"
#include "Const.h"
#include "PhysicsDebugDraw.h"
#include "editor/Playbar.h"
#include <Rocket/Core/ElementDocument.h>
#include <cmath>

namespace af3d
{
    ACLASS_DEFINE_BEGIN(Scene, SceneObjectManager)
    SCENE_PROPS(Scene)
    ACLASS_DEFINE_END(Scene)

    using TimerMap = std::map<std::int32_t, Scene::TimerFn>;

    class Scene::Impl
    {
    public:
        Impl()
        {
            int debugMode = 0;

            if (settings.physics.debugWireframe) {
                debugMode += btIDebugDraw::DBG_DrawWireframe;
            }

            if (settings.physics.debugAabb) {
                debugMode += btIDebugDraw::DBG_DrawAabb;
            }

            if (settings.physics.debugContactPoints) {
                debugMode += btIDebugDraw::DBG_DrawContactPoints;
            }

            if (settings.physics.debugNoDeactivation) {
                debugMode += btIDebugDraw::DBG_NoDeactivation;
            }

            if (settings.physics.debugJoints) {
                debugMode += btIDebugDraw::DBG_DrawConstraints;
            }

            if (settings.physics.debugJointLimits) {
                debugMode += btIDebugDraw::DBG_DrawConstraintLimits;
            }

            if (settings.physics.debugNormals) {
                debugMode += btIDebugDraw::DBG_DrawNormals;
            }

            if (settings.physics.debugFrames) {
                debugMode += btIDebugDraw::DBG_DrawFrames;
            }

            debugDraw_.setDebugMode(debugMode);
            debugDraw_.setAlpha(0.6f);

            phasedComponentManager_.reset(new PhasedComponentManager());
            collisionComponentManager_.reset(new CollisionComponentManager());
            physicsComponentManager_.reset(new PhysicsComponentManager(collisionComponentManager_.get(), &debugDraw_));
            renderComponentManager_.reset(new RenderComponentManager());
            uiComponentManager_.reset(new UIComponentManager());

            timerIt_ = timers_.end();
        }

        ~Impl()
        {
        }

        PhysicsDebugDraw debugDraw_;
        VertexArrayWriter defaultVa_;
        std::unique_ptr<PhasedComponentManager> phasedComponentManager_;
        std::unique_ptr<CollisionComponentManager> collisionComponentManager_;
        std::unique_ptr<PhysicsComponentManager> physicsComponentManager_;
        std::unique_ptr<RenderComponentManager> renderComponentManager_;
        std::unique_ptr<UIComponentManager> uiComponentManager_;
        TimerMap timers_;
        std::uint32_t nextTimerCookie_ = 1;
        TimerMap::const_iterator timerIt_;
        bool firstPhysicsStep_ = true;
    };

    Scene::Scene(const std::string& assetPath)
    : SceneObjectManager(AClass_Scene),
      impl_(new Impl()),
      inputMode_(InputMode::Game),
      playable_(false),
      paused_(false),
      cutscene_(false),
      quit_(false),
      assetPath_(assetPath),
      firstUpdate_(true),
      checkpoint_(0),
      timeScale_(1.0f),
      realDt_(0.0f)
    {
        aflagsSet(AObjectEditable);

        respawnPoint_.setIdentity();

        setScene(this);

        impl_->phasedComponentManager_->setScene(this);
        impl_->collisionComponentManager_->setScene(this);
        impl_->physicsComponentManager_->setScene(this);
        impl_->renderComponentManager_->setScene(this);
        impl_->uiComponentManager_->setScene(this);

        impl_->physicsComponentManager_->world().setInternalTickCallback(&Scene::worldTickCallback,
            impl_->physicsComponentManager_->world().getWorldUserInfo());

        if (settings.editor.enabled) {
            if (settings.editor.playing) {
                auto playbarObj = std::make_shared<SceneObject>();
                auto playbarC = std::make_shared<editor::Playbar>();
                playbarObj->addComponent(playbarC);
                addObject(playbarObj);
            } else {
                workspaceObj_ = std::make_shared<SceneObject>();
                workspace_ = std::make_shared<editor::Workspace>();
                workspaceObj_->addComponent(workspace_);
                addObject(workspaceObj_);
            }
        }

        camera_ = std::make_shared<SceneObject>();

        auto cc = std::make_shared<CameraComponent>();
        cc->setAspect(settings.viewAspect);
        camera_->addComponent(cc);

        addObject(camera_);

        dummy_ = std::make_shared<SceneObject>();

        imGuiC_ = std::make_shared<ImGuiComponent>(zOrderImGui);

        dummy_->addComponent(imGuiC_);

        addObject(dummy_);

        updateInputMode();
    }

    Scene::~Scene()
    {
        impl_.reset();
    }

    const AClass& Scene::staticKlass()
    {
        return AClass_Scene;
    }

    AObjectPtr Scene::create(const APropertyValueMap& propVals)
    {
        return AObjectPtr();
    }

    void Scene::prepare()
    {
    }

    void Scene::cleanup()
    {
        playable_ = true;
        cutscene_ = false;
        updateInputMode();

        removeAllObjects();

        camera_.reset();
        dummy_.reset();

        imGuiC_.reset();

        workspaceObj_.reset();
        workspace_.reset();

        impl_->timers_.clear();

        impl_->phasedComponentManager_->cleanup();
        impl_->physicsComponentManager_->cleanup();
        impl_->collisionComponentManager_->cleanup();
        impl_->renderComponentManager_->cleanup();
        impl_->uiComponentManager_->cleanup();
    }

    void Scene::registerComponent(const ComponentPtr& component)
    {
        if (aobjectCast<PhasedComponent>(component)) {
            impl_->phasedComponentManager_->addComponent(component);
        } else if (aobjectCast<RenderComponent>(component)) {
            impl_->renderComponentManager_->addComponent(component);
        } else if (aobjectCast<UIComponent>(component)) {
            impl_->uiComponentManager_->addComponent(component);
        } else if (aobjectCast<PhysicsComponent>(component)) {
            impl_->physicsComponentManager_->addComponent(component);
        } else if (aobjectCast<CollisionComponent>(component)) {
            impl_->collisionComponentManager_->addComponent(component);
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

        if (inputManager.slowmoPressed()) {
            dt /= settings.physics.slowmoFactor;
        }

        bool forceUpdateRender = false;

        if (!paused_ && (inputMode_ != InputMode::Menu) && inputManager.keyboard().triggered(KI_ESCAPE)) {
            LOG4CPLUS_INFO(logger(), "game paused");

            setPaused(true);

            forceUpdateRender = true;
        }

        bool inputProcessed = true;

        auto cc = camera_->findComponent<CameraComponent>();

        if (!paused_) {
            impl_->firstPhysicsStep_ = true;

            impl_->collisionComponentManager_->flushPending();

            bool physicsStepped = impl_->physicsComponentManager_->update(dt);

            if (physicsStepped) {
                inputManager.proceed();
            } else {
                inputManager.processed();
                inputProcessed = false;
            }

            cc->setViewport(AABB2i(Vector2i(settings.viewX, settings.viewY),
                Vector2i(settings.viewX + settings.viewWidth, settings.viewY + settings.viewHeight)));

            impl_->phasedComponentManager_->preRender(dt);

            if (physicsStepped) {
                //freezeThawObjects(cc->getTrueAABB());
            }

            impl_->uiComponentManager_->update(dt);

            /*
            * FIXME: Or not?
            * The reason we call render component manager update _after_ ui component manager
            * update is because ui component manager update can actually invoke
            * scripts (e.g. in-game dialog "ok" action) and, thus, modify
            * scene, i.e. remove/add render components. However, render component manager assumes
            * that scene is not modified in between its "update" and "render" calls.
            * Having render component manager update after ui component manager update should be
            * ok though since render component manager update is a part of render itself, i.e. it
            * just culls the scene. However, ui component manager update is more than render preparations,
            * it can run custom logic...
            */
            impl_->renderComponentManager_->update(dt);
            impl_->renderComponentManager_->cull(cc);
        } else {
            cc->setViewport(AABB2i(Vector2i(settings.viewX, settings.viewY),
                Vector2i(settings.viewX + settings.viewWidth, settings.viewY + settings.viewHeight)));

            impl_->uiComponentManager_->update(dt);
            if (forceUpdateRender || !paused_) {
                impl_->renderComponentManager_->update(dt);
                impl_->renderComponentManager_->cull(cc);
            }
        }

        RenderNodeList rnList;
        rnList.reserve(2);

        {
            RenderList rl(cc->viewport(), cc->getFrustum(), cc->renderSettings(), impl_->defaultVa_);

            impl_->renderComponentManager_->render(rl);

            if (inputManager.physicsDebugPressed()) {
                impl_->debugDraw_.setRenderList(&rl);
                impl_->physicsComponentManager_->world().debugDrawWorld();
                impl_->debugDraw_.setRenderList(nullptr);
            }

            if (inputManager.gameDebugPressed()) {
                impl_->physicsComponentManager_->debugDraw(rl);
                impl_->phasedComponentManager_->debugDraw(rl);
                impl_->renderComponentManager_->debugDraw(rl);
            }

            rnList.push_back(rl.compile());
        }

        rnList.push_back(impl_->uiComponentManager_->render(impl_->defaultVa_));

        impl_->defaultVa_.upload();

        inputManager.update();

        if (inputProcessed) {
            inputManager.processed();
        } else {
            inputManager.proceed();
        }

        renderer.swap(std::move(rnList));

        firstUpdate_ = false;
    }

    void Scene::updateStep(float dt)
    {
        impl_->collisionComponentManager_->update(dt);

        if (!impl_->timers_.empty()) {
            auto lastCookie = impl_->timers_.rbegin()->first;
            for (impl_->timerIt_ = impl_->timers_.begin();
                impl_->timerIt_ != impl_->timers_.end();) {
                if (impl_->timerIt_->first > lastCookie) {
                    break;
                }
                const Scene::TimerFn& fn = impl_->timerIt_->second;
                ++impl_->timerIt_;
                fn(dt);
            }
            impl_->timerIt_ = impl_->timers_.end();
        }

        impl_->phasedComponentManager_->update(dt);

        if (impl_->firstPhysicsStep_) {
            impl_->firstPhysicsStep_ = false;
            inputManager.processed();
        }

        impl_->collisionComponentManager_->flushPending();
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
        impl_->physicsComponentManager_->world().setGravity(value);
    }

    btVector3 Scene::gravity() const
    {
        return impl_->physicsComponentManager_->world().getGravity();
    }

    void Scene::rayCastRender(const Frustum& frustum, const Ray& ray, const RayCastRenderFn& fn) const
    {
        impl_->renderComponentManager_->rayCast(frustum, ray, fn);
    }

    void Scene::rayCast(const btVector3& p1, const btVector3& p2, const RayCastFn& fn) const
    {
        impl_->physicsComponentManager_->rayCast(p1, p2, fn);
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

    void Scene::setNextLevel(const std::string& assetPath)
    {
        nextAssetPath_ = assetPath;
    }

    void Scene::restartLevel()
    {
    }

    bool Scene::getNextLevel(std::string& assetPath)
    {
        if (nextAssetPath_.empty()) {
            return false;
        }

        assetPath = nextAssetPath_;
        nextAssetPath_.clear();

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

    APropertyValue Scene::propertyClearColorGet(const std::string&) const
    {
        return camera_->findComponent<CameraComponent>()->renderSettings().clearColor();
    }

    void Scene::propertyClearColorSet(const std::string&, const APropertyValue& value)
    {
        camera_->findComponent<CameraComponent>()->renderSettings().setClearColor(value.toColor());
    }

    APropertyValue Scene::propertyAmbientColorGet(const std::string&) const
    {
        return camera_->findComponent<CameraComponent>()->renderSettings().ambientColor();
    }

    void Scene::propertyAmbientColorSet(const std::string&, const APropertyValue& value)
    {
        camera_->findComponent<CameraComponent>()->renderSettings().setAmbientColor(value.toColor());
    }

    APropertyValue Scene::propertyCameraTransformGet(const std::string&) const
    {
        return camera_->transform();
    }

    void Scene::worldTickCallback(btDynamicsWorld* world, btScalar timeStep)
    {
        PhysicsComponentManager::fromWorld(world)->scene()->updateStep(timeStep);
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

    std::vector<AObjectPtr> Scene::getChildren() const
    {
        std::vector<AObjectPtr> res;
        res.reserve(objects().size());
        for (const auto& obj : objects()) {
            if ((obj->aflags() & AObjectEditable) != 0) {
                res.push_back(obj);
            }
        }
        return res;
    }

    void Scene::setChildren(const std::vector<AObjectPtr>& value)
    {
        auto objs = objects();
        for (const auto& obj : objs) {
            if ((obj->aflags() & AObjectEditable) != 0) {
                obj->removeFromParent();
            }
        }
        for (const auto& obj : value) {
            auto sObj = aobjectCast<SceneObject>(obj);
            if (sObj) {
                addObject(sObj);
            } else {
                LOG4CPLUS_ERROR(logger(), "Bad child object \"" << obj->name() << "\", class - \"" << obj->klass().name() << "\"");
            }
        }
    }
}
