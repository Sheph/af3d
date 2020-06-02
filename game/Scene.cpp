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
#include "SceneEnvironment.h"
#include "Logger.h"
#include "InputManager.h"
#include "GameShell.h"
#include "Renderer.h"
#include "PhasedComponentManager.h"
#include "PhysicsComponentManager.h"
#include "RenderComponentManager.h"
#include "RenderFilterComponent.h"
#include "UIComponentManager.h"
#include "CollisionComponentManager.h"
#include "PhasedComponent.h"
#include "PhysicsComponent.h"
#include "RenderComponent.h"
#include "CollisionComponent.h"
#include "FPComponent.h"
#include "CameraComponent.h"
#include "UIComponent.h"
#include "MeshManager.h"
#include "PointLight.h"
#include "DirectionalLight.h"
#include "HardwareResourceManager.h"
#include "Const.h"
#include "PhysicsDebugDraw.h"
#include "AssetManager.h"
#include "TextureManager.h"
#include "TAAComponent.h"
#include "LightProbeComponent.h"
#include "editor/Playbar.h"
#include <Rocket/Core/ElementDocument.h>
#include <cmath>

namespace af3d
{
    ACLASS_DEFINE_BEGIN(Scene, SceneObjectManager)
    SCENE_PROPS(Scene)
    ACLASS_DEFINE_END(Scene)

    using TimerMap = std::map<std::int32_t, Scene::TimerFn>;
    using JointSet = std::unordered_set<JointPtr>;
    using ConstraintJointMap = std::unordered_map<btTypedConstraint*, Joint*>;

    namespace
    {
        class OverlapFilterCallback : public btOverlapFilterCallback
        {
        public:
            explicit OverlapFilterCallback(Scene* scene)
            : scene_(scene) {}

            bool needBroadphaseCollision(btBroadphaseProxy* proxy0, btBroadphaseProxy* proxy1) const override
            {
                btRigidBody* bodyA = (btRigidBody*)proxy0->m_clientObject;
                btRigidBody* bodyB = (btRigidBody*)proxy1->m_clientObject;

                SceneObject* objectA = SceneObject::fromBody(bodyA);
                SceneObject* objectB = SceneObject::fromBody(bodyB);

                const Layers& layersA = scene_->collisionMatrix()->row(objectA->layer());
                const Layers& layersB = scene_->collisionMatrix()->row(objectB->layer());

                return ((proxy0->m_collisionFilterGroup & proxy1->m_collisionFilterMask) != 0) &&
                    ((proxy1->m_collisionFilterGroup & proxy0->m_collisionFilterMask) != 0) &&
                    layersB[objectA->layer()] && layersA[objectB->layer()] &&
                    (!objectA->collisionFilter() || objectA->collisionFilter()->shouldCollideWith(bodyA, bodyB)) &&
                    (!objectB->collisionFilter() || objectB->collisionFilter()->shouldCollideWith(bodyB, bodyA));
            }

        private:
            Scene* scene_;
        };
    }

    class Scene::Impl
    {
    public:
        explicit Impl(Scene* scene)
        : filterCallback_(scene),
          env_(std::make_shared<SceneEnvironment>())
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
            physicsComponentManager_.reset(new PhysicsComponentManager(collisionComponentManager_.get(), &debugDraw_, &filterCallback_,
                std::bind(&Impl::onBodyAdd, this, std::placeholders::_1),
                std::bind(&Impl::onBodyRemove, this, std::placeholders::_1)));
            renderComponentManager_.reset(new RenderComponentManager());
            uiComponentManager_.reset(new UIComponentManager());

            timerIt_ = timers_.end();
        }

        ~Impl()
        {
        }

        void onBodyAdd(btRigidBody* body)
        {
            // Body added to world
            btAssert(body->isInWorld());
            refreshJoints(body, false);
        }

        void onBodyRemove(btRigidBody* body)
        {
            // Body removed from world
            btAssert(!body->isInWorld());
            refreshJoints(body, false);
        }

        void onBodyLeave(btRigidBody* body)
        {
            // Body's leaving for good
            btAssert(!body->isInWorld());
            refreshJoints(body, true);
        }

        void refreshJoints(btRigidBody* body, bool forceDelete)
        {
            btAssert(SceneObject::fromBody(body));

            for (const auto& j : joints_) {
                auto c = j->constraint();
                if (c) {
                    if ((&c->getRigidBodyA() == body) ||
                        (&c->getRigidBodyB() == body)) {
                        if (forceDelete) {
                            physicsComponentManager_->world().removeConstraint(c);
                            runtime_assert(constraintToJoint_.erase(c) > 0);
                        }
                        j->refresh(forceDelete);
                    }
                } else {
                    j->refresh(forceDelete);
                    c = j->constraint();
                    if (c) {
                        runtime_assert(constraintToJoint_.emplace(c, j.get()).second);
                        physicsComponentManager_->world().addConstraint(c, !j->collideConnected());
                    }
                }
            }
        }

        OverlapFilterCallback filterCallback_;
        JointSet joints_;
        ConstraintJointMap constraintToJoint_;
        PhysicsDebugDraw debugDraw_;
        SceneEnvironmentPtr env_;
        std::unique_ptr<PhasedComponentManager> phasedComponentManager_;
        std::unique_ptr<CollisionComponentManager> collisionComponentManager_;
        std::unique_ptr<PhysicsComponentManager> physicsComponentManager_;
        std::unique_ptr<RenderComponentManager> renderComponentManager_;
        std::unique_ptr<UIComponentManager> uiComponentManager_;
        TimerMap timers_;
        std::uint32_t nextTimerCookie_ = 1;
        TimerMap::const_iterator timerIt_;
        bool firstPhysicsStep_ = true;
        int tick_ = 0;
    };

    Scene::Scene(const std::string& assetPath)
    : SceneObjectManager(AClass_Scene),
      impl_(new Impl(this)),
      inputMode_(InputMode::Game),
      playable_(false),
      paused_(false),
      cutscene_(false),
      quit_(false),
      assetPath_(assetPath),
      collisionMatrix_(assetManager.getCollisionMatrix("default.cm")),
      firstUpdate_(true),
      checkpoint_(0),
      timeScale_(1.0f)
    {
        if (!collisionMatrix_) {
            collisionMatrix_ = std::make_shared<CollisionMatrix>();
            collisionMatrix_->setName("default.cm");
        }

        aflagsSet(AObjectEditable);

        respawnPoint_.setIdentity();

        setScene(this);

        impl_->phasedComponentManager_->setScene(this);
        impl_->collisionComponentManager_->setScene(this);
        impl_->physicsComponentManager_->setScene(this);
        impl_->renderComponentManager_->setScene(this);
        impl_->uiComponentManager_->setScene(this);

        impl_->physicsComponentManager_->world().setInternalTickCallback(&Scene::worldPreTickCallback,
            impl_->physicsComponentManager_->world().getWorldUserInfo(), true);
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

        dummy_ = std::make_shared<SceneObject>();

        auto screenTex = textureManager.createRenderTextureScaled(TextureType2D,
            1.0f, GL_RGB16F, GL_RGB, GL_FLOAT);
        auto velocityTex = textureManager.createRenderTextureScaled(TextureType2D,
            1.0f, GL_RG16F, GL_RG, GL_FLOAT);
        auto depthTex = textureManager.createRenderTextureScaled(TextureType2D,
            1.0f, GL_DEPTH_COMPONENT24, GL_DEPTH_COMPONENT, GL_FLOAT);

        auto mc = std::make_shared<Camera>();
        mc->setLayer(CameraLayer::Main);
        mc->setAspect(settings.viewAspect);
        mc->setRenderTarget(AttachmentPoint::Color0, RenderTarget(screenTex));
        mc->setRenderTarget(AttachmentPoint::Color1, RenderTarget(velocityTex));
        mc->setRenderTarget(AttachmentPoint::Depth, RenderTarget(depthTex));
        mc->setClearMask(mc->clearMask() | AttachmentPoint::Color1);
        mc->setClearColor(AttachmentPoint::Color1, linearToGamma(Color(65535.0f, 65535.0f, 65535.0f, 65535.0f)));
        addCamera(mc);

        if (settings.bloom) {
            std::vector<MaterialPtr> mats;
            auto tex = postProcessBloom(camOrderPostProcess + 1, screenTex, 1.0f, 11, 2.0f, 0.5f, mats);
            if (settings.aaMode == Settings::AAMode::TAA) {
                postProcessTAA(camOrderPostProcess, mc, mats);
            }
            auto filter = postProcessToneMapping(camOrderPostProcess + 100, tex);
            if (settings.aaMode == Settings::AAMode::FXAA) {
                auto toneMappedTex = textureManager.createRenderTextureScaled(TextureType2D,
                    1.0f, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
                filter->camera()->setRenderTarget(AttachmentPoint::Color0, RenderTarget(toneMappedTex));
                filter = postProcessFXAA(camOrderPostProcess + 200, filter->camera()->renderTarget().texture());
            }
            ppCamera_ = filter->camera();
        } else {
            auto filter = postProcessToneMapping(camOrderPostProcess + 100, screenTex);
            if (settings.aaMode == Settings::AAMode::TAA) {
                postProcessTAA(camOrderPostProcess, mc, {filter->material()});
            } else if (settings.aaMode == Settings::AAMode::FXAA) {
                auto toneMappedTex = textureManager.createRenderTextureScaled(TextureType2D,
                    1.0f, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE);
                filter->camera()->setRenderTarget(AttachmentPoint::Color0, RenderTarget(toneMappedTex));
                filter = postProcessFXAA(camOrderPostProcess + 200, filter->camera()->renderTarget().texture());
            }
            ppCamera_ = filter->camera();
        }

        ppCamera_->setViewport(AABB2i(Vector2i(settings.viewX, settings.viewY),
            Vector2i(settings.viewX + settings.viewWidth, settings.viewY + settings.viewHeight)));

        mainCamera_ = std::make_shared<SceneObject>();
        mainCamera_->addComponent(std::make_shared<CameraComponent>(mc));
        mainCamera_->addComponent(std::make_shared<FPComponent>());
        addObject(mainCamera_);

        imGuiC_ = std::make_shared<ImGuiComponent>(zOrderImGui);

        dummy_->addComponent(imGuiC_);

        addObject(dummy_);

        updateInputMode();
    }

    Scene::~Scene()
    {
        btAssert(impl_->joints_.empty());
        btAssert(impl_->constraintToJoint_.empty());

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

    void Scene::setName(const std::string& value)
    {
        SceneObjectManager::setName(value);
        auto probe = impl_->env_->globalLightProbe();
        if (probe) {
            recreateGlobalLightProbe(probe->irradianceResolution(),
                probe->specularResolution(), probe->specularMipLevels());
        }
    }

    void Scene::prepare()
    {
        if (!impl_->env_->globalLightProbe()) {
            recreateGlobalLightProbe(64, 128, 5);
        }
    }

    void Scene::cleanup()
    {
        playable_ = true;
        cutscene_ = false;
        updateInputMode();

        removeAllObjects();

        cameras_.clear();
        mainCamera_.reset();
        ppCamera_.reset();

        dummy_.reset();
        root_.reset();

        imGuiC_.reset();

        workspaceObj_.reset();
        workspace_.reset();

        while (!impl_->joints_.empty()) {
            removeJoint(*impl_->joints_.begin());
        }
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
        float realDt = dt;

        if (!paused_) {
            dt *= timeScale_;
        }

        if (inputManager.slowmoPressed()) {
            dt /= settings.physics.slowmoFactor;
        }

        impl_->env_->update(realDt, dt);

        bool forceUpdateRender = false;

        if (!paused_ && (inputMode_ != InputMode::Menu) && inputManager.keyboard().triggered(KI_ESCAPE)) {
            LOG4CPLUS_INFO(logger(), "game paused");

            setPaused(true);

            forceUpdateRender = true;
        }

        bool inputProcessed = true;

        auto cc = mainCamera()->findComponent<CameraComponent>();

        if (!paused_ || forceUpdateRender) {
            for (auto& c : cameras_) {
                c->updatePrevViewProjMat();
            }
        }

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

            ppCamera_->setViewport(AABB2i(Vector2i(settings.viewX, settings.viewY),
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
        } else {
            ppCamera_->setViewport(AABB2i(Vector2i(settings.viewX, settings.viewY),
                Vector2i(settings.viewX + settings.viewWidth, settings.viewY + settings.viewHeight)));

            impl_->uiComponentManager_->update(dt);
            if (forceUpdateRender) {
                impl_->renderComponentManager_->update(dt);
            }
        }

        std::vector<CameraPtr> cams(cameras_.begin(), cameras_.end());
        std::sort(cams.begin(), cams.end(), [](const CameraPtr& a, const CameraPtr& b) {
            return a->order() < b->order();
        });

        RenderNodeList rnList;
        rnList.reserve(cams.size() + 1);

        for (const auto& c : cams) {
            RenderList rl(c, impl_->env_);

            impl_->renderComponentManager_->render(rl);

            if (c == cc->camera()) {
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
            }

            rnList.push_back(rl.compile());
        }

        rnList.push_back(impl_->uiComponentManager_->render(impl_->env_));

        impl_->env_->preSwap();

        inputManager.update();

        if (inputProcessed) {
            inputManager.processed();
        } else {
            inputManager.proceed();
        }

        renderer.swap(std::move(rnList));

        firstUpdate_ = false;
    }

    void Scene::updatePreStep(float dt)
    {
        if (dt != 0.0f) {
            for (int i = 0; i < impl_->physicsComponentManager_->world().getNumCollisionObjects(); ++i) {
                btCollisionObject* c = impl_->physicsComponentManager_->world().getCollisionObjectArray()[i];
                btRigidBody* body = btRigidBody::upcast(c);
                if (body && body->isKinematicObject()) {
                    if (body->getActivationState() != DISABLE_DEACTIVATION) {
                        body->m_linearVelocity.setZero();
                        body->m_angularVelocity.setZero();
                    } else {
                        auto obj = SceneObject::fromBody(body);
                        body->m_linearVelocity = obj->linearVelocity();
                        body->m_angularVelocity = obj->angularVelocity();
                        btTransformUtil::integrateTransform(body->m_interpolationWorldTransform,
                            body->m_linearVelocity, body->m_angularVelocity, dt, body->m_worldTransform);
                        body->m_interpolationWorldTransform = body->m_worldTransform;
                    }
                    body->m_interpolationLinearVelocity = body->m_linearVelocity;
                    body->m_interpolationAngularVelocity = body->m_angularVelocity;
                }
            }
        }
    }

    void Scene::updateStep(float dt)
    {
        ++impl_->tick_;

        reapJoints();

        impl_->collisionComponentManager_->step(&impl_->physicsComponentManager_->world());
        impl_->collisionComponentManager_->flushPending();
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

    void Scene::reapJoints()
    {
        // If we're not in editor and joint doesn't have
        // some bodies alive and constraint is no longer there
        // then there's no way this joint can become active again, so
        // check it and remove such joints.

        if (workspace()) {
            return;
        }

        if (impl_->tick_ % 100 != 0) {
            return;
        }

        for (auto it = impl_->joints_.begin(); it != impl_->joints_.end();) {
            const auto& j = *it;
            ++it;
            if (!j->constraint() && (!j->objectA() || !j->objectB())) {
                removeJoint(j);
            }
        }
    }

    void Scene::postProcessTAA(int order, const CameraPtr& inputCamera,
        const std::vector<MaterialPtr>& destMaterials)
    {
        auto taa = std::make_shared<TAAComponent>(inputCamera, destMaterials, order);
        dummy_->addComponent(taa);
    }

    TexturePtr Scene::postProcessBloom(int order, const TexturePtr& inputTex,
        float brightnessThreshold, int blurKSize, float blurSigma, float compositeStrength,
        std::vector<MaterialPtr>& mats)
    {
        auto tex1 = textureManager.createRenderTextureScaled(TextureType2D,
            2.0f, GL_RGB16F, GL_RGB, GL_FLOAT, true);
        auto tex2 = textureManager.createRenderTextureScaled(TextureType2D,
            2.0f, GL_RGB16F, GL_RGB, GL_FLOAT, true);
        auto outTex = textureManager.createRenderTextureScaled(TextureType2D,
            1.0f, GL_RGB16F, GL_RGB, GL_FLOAT);

        auto ppFilter = std::make_shared<RenderFilterComponent>(MaterialTypeFilterBloomPass1);
        ppFilter->material()->setTextureBinding(SamplerName::Main,
            TextureBinding(inputTex,
                SamplerParams(GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR)));
        ppFilter->camera()->setOrder(order++);
        ppFilter->camera()->setRenderTarget(AttachmentPoint::Color0, RenderTarget(outTex));
        ppFilter->material()->params().setUniform(UniformName::Threshold, brightnessThreshold);
        dummy_->addComponent(ppFilter);
        mats.push_back(ppFilter->material());

        const int numPasses = 5;

        for (int i = 0; i < numPasses; ++i) {
            auto ppFilter = std::make_shared<RenderFilterComponent>(MaterialTypeFilterDownscale);
            ppFilter->material()->setTextureBinding(SamplerName::Main,
                TextureBinding(i == 0 ? outTex : tex1,
                    SamplerParams(i == 0 ? GL_LINEAR : GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR)));
            ppFilter->camera()->setOrder(order++);
            ppFilter->camera()->setRenderTarget(AttachmentPoint::Color0, RenderTarget(tex1, i));
            ppFilter->material()->params().setUniform(UniformName::MipLevel, static_cast<float>(i == 0 ? 0 : i - 1));
            dummy_->addComponent(ppFilter);
        }

        for (int i = 0; i < numPasses; ++i) {
            auto ppFilter = std::make_shared<RenderFilterComponent>(MaterialTypeFilterGaussianBlur);
            ppFilter->material()->setTextureBinding(SamplerName::Main,
                TextureBinding(tex1,
                    SamplerParams(GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR)));
            ppFilter->camera()->setOrder(order++);
            ppFilter->camera()->setRenderTarget(AttachmentPoint::Color0, RenderTarget(tex2, i));
            setGaussianBlurParams(ppFilter->material()->params(), blurKSize, blurSigma, false);
            ppFilter->material()->params().setUniform(UniformName::MipLevel, static_cast<float>(i));
            dummy_->addComponent(ppFilter);

            ppFilter = std::make_shared<RenderFilterComponent>(MaterialTypeFilterGaussianBlur);
            ppFilter->material()->setTextureBinding(SamplerName::Main,
                TextureBinding(tex2,
                    SamplerParams(GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR)));
            ppFilter->camera()->setOrder(order++);
            ppFilter->camera()->setRenderTarget(AttachmentPoint::Color0, RenderTarget(tex1, i));
            setGaussianBlurParams(ppFilter->material()->params(), blurKSize, blurSigma, true);
            ppFilter->material()->params().setUniform(UniformName::MipLevel, static_cast<float>(i));
            dummy_->addComponent(ppFilter);
        }

        ppFilter = std::make_shared<RenderFilterComponent>(MaterialTypeFilterBloomPass2);
        ppFilter->material()->setTextureBinding(SamplerName::Main,
            TextureBinding(inputTex,
                SamplerParams(GL_LINEAR, GL_LINEAR)));
        ppFilter->material()->setTextureBinding(SamplerName::Specular,
            TextureBinding(tex1,
                SamplerParams(GL_LINEAR_MIPMAP_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR)));
        ppFilter->camera()->setOrder(order++);
        ppFilter->camera()->setRenderTarget(AttachmentPoint::Color0, RenderTarget(outTex));
        ppFilter->material()->params().setUniform(UniformName::Strength, compositeStrength);
        ppFilter->material()->params().setUniform(UniformName::MipLevel, static_cast<float>(numPasses));
        dummy_->addComponent(ppFilter);
        mats.push_back(ppFilter->material());

        return outTex;
    }

    RenderFilterComponentPtr Scene::postProcessToneMapping(int order, const TexturePtr& inputTex)
    {
        auto ppFilter = std::make_shared<RenderFilterComponent>(MaterialTypeFilterToneMapping);
        ppFilter->material()->setTextureBinding(SamplerName::Main,
            TextureBinding(inputTex,
                SamplerParams(GL_NEAREST, GL_NEAREST)));
        ppFilter->camera()->setOrder(order);
        dummy_->addComponent(ppFilter);
        return ppFilter;
    }

    RenderFilterComponentPtr Scene::postProcessFXAA(int order, const TexturePtr& inputTex)
    {
        auto ppFilter = std::make_shared<RenderFilterComponent>(MaterialTypeFilterFXAA);
        ppFilter->material()->setTextureBinding(SamplerName::Main,
            TextureBinding(inputTex,
                SamplerParams(GL_LINEAR, GL_LINEAR)));
        ppFilter->camera()->setOrder(order);
        dummy_->addComponent(ppFilter);
        return ppFilter;
    }

    void Scene::addJoint(const JointPtr& joint)
    {
        runtime_assert(impl_->joints_.insert(joint).second);

        joint->adopt(this);

        auto c = joint->constraint();
        if (c) {
            runtime_assert(impl_->constraintToJoint_.emplace(c, joint.get()).second);
            impl_->physicsComponentManager_->world().addConstraint(c, !joint->collideConnected());
        }
    }

    void Scene::removeJoint(const JointPtr& joint)
    {
        // Hold on to it while removing.
        JointPtr tmp = joint;

        if (impl_->joints_.erase(joint) == 0) {
            return;
        }

        auto c = joint->constraint();
        if (c) {
            impl_->physicsComponentManager_->world().removeConstraint(c);
            runtime_assert(impl_->constraintToJoint_.erase(c) > 0);
        }

        joint->abandon();
    }

    std::vector<JointPtr> Scene::getJoints(const std::string& name) const
    {
        std::vector<JointPtr> res;

        for (const auto& j : impl_->joints_) {
            if (j->name() == name) {
                res.push_back(j);
            }
        }

        return res;
    }

    const std::unordered_set<JointPtr>& Scene::joints() const
    {
        return impl_->joints_;
    }

    Joint* Scene::getJoint(btTypedConstraint* constraint) const
    {
        auto it = impl_->constraintToJoint_.find(constraint);
        return (it == impl_->constraintToJoint_.end()) ? nullptr : it->second;
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

    bool Scene::collidesWith(btCollisionObject* thisObj, btCollisionObject* other) const
    {
        return impl_->filterCallback_.needBroadphaseCollision(thisObj->getBroadphaseHandle(), other->getBroadphaseHandle());
    }

    void Scene::addCamera(const CameraPtr& c)
    {
        cameras_.insert(c);
    }

    void Scene::removeCamera(const CameraPtr& c)
    {
        if (c->layer() != CameraLayer::Main) {
            cameras_.erase(c);
        }
    }

    int Scene::addLight(Light* light)
    {
        return impl_->env_->addLight(light);
    }

    void Scene::removeLight(Light* light)
    {
        impl_->env_->removeLight(light);
    }

    void Scene::addLightProbe(LightProbeComponent* probe)
    {
        impl_->env_->addLightProbe(probe);
    }

    void Scene::removeLightProbe(LightProbeComponent* probe)
    {
        impl_->env_->removeLightProbe(probe);
    }

    void Scene::recreateGlobalLightProbe(std::uint32_t irradianceRes,
        std::uint32_t specularRes,
        std::uint32_t specularMipLevels)
    {
        auto oldProbe = impl_->env_->globalLightProbe();
        if (oldProbe) {
            oldProbe->removeFromParent();
        }
        dummy_->addComponent(
            std::make_shared<LightProbeComponent>(irradianceRes, specularRes, specularMipLevels));
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
        return mainCamera()->findComponent<CameraComponent>()->camera()->clearColor();
    }

    void Scene::propertyClearColorSet(const std::string&, const APropertyValue& value)
    {
        mainCamera()->findComponent<CameraComponent>()->camera()->setClearColor(AttachmentPoint::Color0, value.toColor());
    }

    APropertyValue Scene::propertyAmbientColorGet(const std::string&) const
    {
        return mainCamera()->findComponent<CameraComponent>()->camera()->ambientColor();
    }

    void Scene::propertyAmbientColorSet(const std::string&, const APropertyValue& value)
    {
        mainCamera()->findComponent<CameraComponent>()->camera()->setAmbientColor(value.toColor());
    }

    APropertyValue Scene::propertyCameraTransformGet(const std::string&) const
    {
        return mainCamera()->transform();
    }

    APropertyValue Scene::propertyGlobalIrradianceResGet(const std::string&) const
    {
        auto probe = impl_->env_->globalLightProbe();
        return probe ? static_cast<int>(probe->irradianceResolution()) : 0;
    }

    void Scene::propertyGlobalIrradianceResSet(const std::string&, const APropertyValue& value)
    {
        auto probe = impl_->env_->globalLightProbe();
        if (probe) {
            recreateGlobalLightProbe(value.toInt(),
                probe->specularResolution(), probe->specularMipLevels());
        }
    }

    APropertyValue Scene::propertyGlobalSpecularResGet(const std::string&) const
    {
        auto probe = impl_->env_->globalLightProbe();
        return probe ? static_cast<int>(probe->specularResolution()) : 0;
    }

    void Scene::propertyGlobalSpecularResSet(const std::string&, const APropertyValue& value)
    {
        auto probe = impl_->env_->globalLightProbe();
        if (probe) {
            recreateGlobalLightProbe(probe->irradianceResolution(),
                value.toInt(), probe->specularMipLevels());
        }
    }

    APropertyValue Scene::propertyGlobalSpecularMipLevelsGet(const std::string&) const
    {
        auto probe = impl_->env_->globalLightProbe();
        return probe ? static_cast<int>(probe->specularMipLevels()) : 0;
    }

    void Scene::propertyGlobalSpecularMipLevelsSet(const std::string&, const APropertyValue& value)
    {
        auto probe = impl_->env_->globalLightProbe();
        if (probe) {
            recreateGlobalLightProbe(probe->irradianceResolution(),
                probe->specularResolution(), value.toInt());
        }
    }

    void Scene::propertyUpdateLightProbesSet(const std::string&, const APropertyValue& value)
    {
        if (!value.toBool()) {
            return;
        }

        impl_->env_->updateLightProbes();
    }

    void Scene::onLeave(SceneObject* obj)
    {
        if (obj->body()) {
            impl_->onBodyLeave(obj->body());
        }
    }

    void Scene::worldPreTickCallback(btDynamicsWorld* world, btScalar timeStep)
    {
        PhysicsComponentManager::fromWorld(world)->scene()->updatePreStep(timeStep);
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
        res.reserve(objects().size() + joints().size());

        for (const auto& obj : objects()) {
            if ((obj->aflags() & AObjectEditable) != 0) {
                res.push_back(obj);
            }
        }

        for (const auto& j : joints()) {
            if ((j->aflags() & AObjectEditable) != 0) {
                res.push_back(j);
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

        auto js = joints();
        for (const auto& j : js) {
            if ((j->aflags() & AObjectEditable) != 0) {
                j->removeFromParent();
            }
        }

        for (const auto& obj : value) {
            if (auto sObj = aobjectCast<SceneObject>(obj)) {
                addObject(sObj);
            } else if (auto j = aobjectCast<Joint>(obj)) {
                addJoint(j);
            } else {
                LOG4CPLUS_ERROR(logger(), "Bad child object \"" << obj->name() << "\", class - \"" << obj->klass().name() << "\"");
            }
        }
    }
}
