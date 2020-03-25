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
#include "LightComponent.h"
#include "ImGuiComponent.h"
#include "af3d/AABB.h"
#include <functional>

namespace af3d
{
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

        explicit Scene(const std::string& scriptPath);
        ~Scene();

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        std::vector<AObjectPtr> getChildren() const override;

        void prepare();

        void cleanup();

        void registerComponent(const ComponentPtr& component);

        void unregisterComponent(const ComponentPtr& component);

        void freezeComponent(const ComponentPtr& component);

        void thawComponent(const ComponentPtr& component);

        void update(float dt);

        std::uint32_t addTimer(const TimerFn& fn);

        void removeTimer(std::uint32_t cookie);

        void setGravity(const btVector3& value);
        btVector3 gravity() const;

        inline const SceneObjectPtr& camera() const { return camera_; }

        inline const LightComponentPtr& lighting() const { return lightC_; }

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

        void setNextLevel(const std::string& scriptPath);
        void restartLevel();
        bool getNextLevel(std::string& scriptPath);

        inline void setTimeScale(float value) { timeScale_ = value; }
        inline float timeScale() const { return timeScale_; }

        inline float realDt() const { return realDt_; }

    private:
        void freezeThawObjects(const AABB& aabb);

        void updateInputMode();

        class Impl;
        std::unique_ptr<Impl> impl_;

        InputMode inputMode_;
        SceneObjectPtr camera_;
        SceneObjectPtr dummy_;
        ImGuiComponentPtr imGuiC_;
        LightComponentPtr lightC_;

        bool playable_;
        btTransform respawnPoint_;

        bool paused_;
        bool cutscene_;
        bool quit_;

        std::string nextScriptPath_;

        bool firstUpdate_;
        int checkpoint_;
        float timeScale_;
        float realDt_;
    };

    ACLASS_DECLARE(Scene)
}

#endif
