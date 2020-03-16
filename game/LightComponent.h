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

#ifndef _LIGHTCOMPONENT_H_
#define _LIGHTCOMPONENT_H_

#include "RenderComponent.h"
#include "Light.h"

namespace af3d
{
    class LightComponent : public std::enable_shared_from_this<LightComponent>,
        public RenderComponent
    {
    public:
        using Lights = std::vector<LightPtr>;

        LightComponent();
        ~LightComponent();

        ComponentPtr sharedThis() override { return shared_from_this(); }

        void update(float dt) override;

        void render(RenderList& rl, void* const* parts, size_t numParts) override;

        void debugDraw() override;

        void addLight(const LightPtr& light);

        void removeLight(const LightPtr& light);

        inline const Lights& lights() const { return lights_; }

        Lights getLights(const std::string& name) const;

        template <class T>
        std::shared_ptr<T> getLight(const std::string& name) const
        {
            for (const auto& tmp : lights_) {
                const auto& light = std::dynamic_pointer_cast<T>(tmp);
                if (light && (light->name() == name)) {
                    return light;
                }
            }
            return std::shared_ptr<T>();
        }

    private:
        void onRegister() override;

        void onUnregister() override;

        Lights lights_;

        btTransform prevParentXf_ = btTransform::getIdentity();
    };

    using LightComponentPtr = std::shared_ptr<LightComponent>;
}

#endif
