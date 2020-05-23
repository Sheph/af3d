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

#ifndef _SCENE_H_
#define _SCENE_H_

#include "SceneObjectManager.h"
#include "PhysicsComponent.h"
#include "RenderFilterComponent.h"
#include "ImGuiComponent.h"
#include "CollisionMatrix.h"
#include "Joint.h"
#include "Camera.h"
#include "editor/Workspace.h"
#include "af3d/AABB.h"
#include <functional>

namespace af3d
{
    class LightProbeComponent;

    class Scene : public SceneObjectManager
    {
    public:
        enum class InputMode
        {
            Menu = 0,
            Game,
            Cutscene
        };

        using TimerFn = std::function<void(float)>;

        explicit Scene(const std::string& assetPath);
        ~Scene();

        inline const std::string& assetPath() const { return assetPath_; }

        inline const std::string& scriptPath() const { return scriptPath_; }
        inline void setScriptPath(const std::string& value) { scriptPath_ = value; }

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        void setName(const std::string& value) override;

        AObjectPtr sharedThis() override { return makeSharedNullDeleter(this); }

        void prepare();

        void cleanup();

        void registerComponent(const ComponentPtr& component);

        void unregisterComponent(const ComponentPtr& component);

        void freezeComponent(const ComponentPtr& component);

        void thawComponent(const ComponentPtr& component);

        void update(float dt);

        void addJoint(const JointPtr& joint);

        void removeJoint(const JointPtr& joint);

        std::vector<JointPtr> getJoints(const std::string& name) const;

        const std::unordered_set<JointPtr>& joints() const;

        Joint* getJoint(btTypedConstraint* constraint) const;

        std::uint32_t addTimer(const TimerFn& fn);

        void removeTimer(std::uint32_t cookie);

        void setGravity(const btVector3& value);
        btVector3 gravity() const;

        void rayCastRender(const Frustum& frustum, const Ray& ray, const RayCastRenderFn& fn) const;

        void rayCast(const btVector3& p1, const btVector3& p2, const RayCastFn& fn) const;

        bool collidesWith(btCollisionObject* thisObj, btCollisionObject* other) const;

        inline const editor::WorkspacePtr& workspace() const { return workspace_; }

        inline const std::unordered_set<CameraPtr>& cameras() const { return cameras_; }

        inline const SceneObjectPtr& mainCamera() const { return mainCamera_; }

        void addCamera(const CameraPtr& c);

        void removeCamera(const CameraPtr& c);

        void addLightProbe(LightProbeComponent* probe);

        void removeLightProbe(LightProbeComponent* probe);

        void recreateGlobalLightProbe(std::uint32_t irradianceRes,
            std::uint32_t specularRes,
            std::uint32_t specularMipLevels);

        inline bool playable() const { return playable_; }

        inline const btTransform& respawnPoint() const { return respawnPoint_; }
        void setRespawnPoint(const btTransform& value);

        inline void setPaused(bool value) { paused_ = value; updateInputMode(); }
        inline bool paused() const { return paused_; }

        inline bool cutscene() const { return cutscene_; }
        void setCutscene(bool value);

        inline InputMode inputMode() const { return inputMode_; }

        inline bool quit() const { return quit_; }
        inline void setQuit(bool value) { quit_ = value; }

        inline int checkpoint() const { return checkpoint_; }
        inline void setCheckpoint(int value) { checkpoint_ = value; }

        void setNextLevel(const std::string& assetPath);
        void restartLevel();
        bool getNextLevel(std::string& assetPath);

        inline void setTimeScale(float value) { timeScale_ = value; }
        inline float timeScale() const { return timeScale_; }

        inline void setRoot(const SceneObjectPtr& value) { root_ = value; }
        inline const SceneObjectPtr& root() const { return root_; }

        inline void setCollisionMatrix(const CollisionMatrixPtr& value) { btAssert(value); collisionMatrix_ = value; }
        inline const CollisionMatrixPtr& collisionMatrix() const { return collisionMatrix_; }

        APropertyValue propertyGravityGet(const std::string&) const { return gravity(); }
        void propertyGravitySet(const std::string&, const APropertyValue& value) { setGravity(value.toVec3()); }

        APropertyValue propertyClearColorGet(const std::string&) const;
        void propertyClearColorSet(const std::string&, const APropertyValue& value);

        APropertyValue propertyAmbientColorGet(const std::string&) const;
        void propertyAmbientColorSet(const std::string&, const APropertyValue& value);

        APropertyValue propertyScriptGet(const std::string&) const { return scriptPath(); }
        void propertyScriptSet(const std::string&, const APropertyValue& value) { setScriptPath(value.toString()); }

        APropertyValue propertyRootGet(const std::string&) const { return APropertyValue(root()); }
        void propertyRootSet(const std::string&, const APropertyValue& value) { setRoot(value.toObject<SceneObject>()); }

        APropertyValue propertyCollisionMatrixGet(const std::string&) const { return APropertyValue(collisionMatrix()); }
        void propertyCollisionMatrixSet(const std::string&, const APropertyValue& value) { setCollisionMatrix(value.toObject<CollisionMatrix>()); }

        APropertyValue propertyCameraTransformGet(const std::string&) const;

        APropertyValue propertyGlobalIrradianceResGet(const std::string&) const;
        void propertyGlobalIrradianceResSet(const std::string&, const APropertyValue& value);

        APropertyValue propertyGlobalSpecularResGet(const std::string&) const;
        void propertyGlobalSpecularResSet(const std::string&, const APropertyValue& value);

        APropertyValue propertyGlobalSpecularMipLevelsGet(const std::string&) const;
        void propertyGlobalSpecularMipLevelsSet(const std::string&, const APropertyValue& value);

        APropertyValue propertyUpdateLightProbesGet(const std::string&) const { return false; }
        void propertyUpdateLightProbesSet(const std::string&, const APropertyValue& value);

        // Internal, do not call.
        void onLeave(SceneObject* obj);

    private:
        static void worldPreTickCallback(btDynamicsWorld* world, btScalar timeStep);

        static void worldTickCallback(btDynamicsWorld* world, btScalar timeStep);

        void freezeThawObjects(const AABB& aabb);

        void updateInputMode();

        std::vector<AObjectPtr> getChildren() const override;

        void setChildren(const std::vector<AObjectPtr>& value) override;

        void updatePreStep(float dt);

        void updateStep(float dt);

        void reapJoints();

        void postProcessTAA(int order, const CameraPtr& inputCamera,
            const std::vector<MaterialPtr>& destMaterials);
        TexturePtr postProcessBloom(int order, const TexturePtr& inputTex,
            float brightnessThreshold, int blurKSize, float blurSigma, float compositeStrength, std::vector<MaterialPtr>& mats);
        RenderFilterComponentPtr postProcessToneMapping(int order, const TexturePtr& inputTex);
        RenderFilterComponentPtr postProcessFXAA(int order, const TexturePtr& inputTex);

        class Impl;
        std::unique_ptr<Impl> impl_;

        std::unordered_set<CameraPtr> cameras_;
        SceneObjectPtr mainCamera_;
        CameraPtr ppCamera_;

        InputMode inputMode_;
        SceneObjectPtr workspaceObj_;
        SceneObjectPtr dummy_;
        SceneObjectPtr root_;
        editor::WorkspacePtr workspace_;
        ImGuiComponentPtr imGuiC_;

        bool playable_;
        btTransform respawnPoint_;

        bool paused_;
        bool cutscene_;
        bool quit_;

        std::string assetPath_;
        std::string nextAssetPath_;

        std::string scriptPath_;

        CollisionMatrixPtr collisionMatrix_;

        bool firstUpdate_;
        int checkpoint_;
        float timeScale_;
    };

    ACLASS_DECLARE(Scene)
}

#endif
