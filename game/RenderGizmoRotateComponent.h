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

#ifndef _RENDERGIZMOROTATECOMPONENT_H_
#define _RENDERGIZMOROTATECOMPONENT_H_

#include "RenderComponent.h"

namespace af3d
{
    enum class RotateType
    {
        None = 0,
        PlaneX,
        PlaneY,
        PlaneZ,
        PlaneCurrent,
        Trackball
    };

    class RenderGizmoRotateComponent : public std::enable_shared_from_this<RenderGizmoRotateComponent>,
        public RenderComponent
    {
    public:
        RenderGizmoRotateComponent();
        ~RenderGizmoRotateComponent() = default;

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        void update(float dt) override;

        void render(RenderList& rl, void* const* parts, size_t numParts) override;

        std::pair<AObjectPtr, float> testRay(const Frustum& frustum, const Ray& ray, void* part) override;

        void debugDraw() override;

        inline const AObjectPtr& target() const { return target_; }
        inline void setTarget(const AObjectPtr& value) { target_ = value; }

        inline RotateType rotateType() const { return rotateType_; }
        inline void setRotateType(RotateType value) { rotateType_ = value; }

        inline float alphaInactive() const { return alpha_[0]; }
        inline void setAlphaInactive(float value) { alpha_[0] = value; }

        inline float alphaActive() const { return alpha_[1]; }
        inline void setAlphaActive(float value) { alpha_[1] = value; }

        RotateType testRay(const Frustum& frustum, const Ray& ray) const;

    private:
        struct Sizes
        {
            float radius1;
            float radius2;
            float width;
        };

        void onRegister() override;

        void onUnregister() override;

        AABB calcAABB();

        btTransform getTargetXf() const;

        Sizes getSizes(const Frustum& frustum) const;

        inline float alpha(RotateType rt) const { return (rt == rotateType_) ? alpha_[1] : alpha_[0]; }

        MaterialPtr material_;

        AObjectPtr target_;
        RotateType rotateType_ = RotateType::None;
        float alpha_[2] = {1.0f, 1.0f};
        float radius_ = 5.0f;
        float viewportRadius_ = 0.08f;
        float viewportWidth_ = 0.0015f;

        bool dirty_ = false;

        btTransform targetXf_;
        AABB prevAABB_;
        RenderCookie* cookie_ = nullptr;
    };

    using RenderGizmoRotateComponentPtr = std::shared_ptr<RenderGizmoRotateComponent>;

    ACLASS_DECLARE(RenderGizmoRotateComponent)
}

#endif
