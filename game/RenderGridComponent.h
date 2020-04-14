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

#ifndef _RENDERGRIDCOMPONENT_H_
#define _RENDERGRIDCOMPONENT_H_

#include "RenderComponent.h"
#include "af3d/Plane.h"

namespace af3d
{
    class RenderGridComponent : public std::enable_shared_from_this<RenderGridComponent>,
        public RenderComponent
    {
    public:
        RenderGridComponent();
        ~RenderGridComponent() = default;

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        void update(float dt) override;

        void render(RenderList& rl, void* const* parts, size_t numParts) override;

        std::pair<AObjectPtr, float> testRay(const Frustum& frustum, const Ray& ray, void* part) override;

        inline float step() const { return step_; }
        inline void setStep(float value) { step_ = value; }

        inline const Color& color() const { return color_; }
        inline void setColor(const Color& value) { color_ = value; }

        inline const Color& xAxisColor() const { return xAxisColor_; }
        void setXAxisColor(const Color& value);

        inline const Color& yAxisColor() const { return yAxisColor_; }
        void setYAxisColor(const Color& value);

    private:
        void onRegister() override;

        void onUnregister() override;

        MaterialPtr material_;

        float step_ = 1.0f;
        Color color_ = Color(0.6f, 0.6f, 0.6f, 0.25f);
        Color xAxisColor_ = Color(1.0f, 0.0f, 0.0f, 1.0f);
        Color yAxisColor_ = Color(0.0f, 0.0f, 1.0f, 1.0f);

        btPlane plane_ = btPlaneMake(btVector3_zero, btVector3_forward);
    };

    using RenderGridComponentPtr = std::shared_ptr<RenderGridComponent>;

    ACLASS_DECLARE(RenderGridComponent)
}

#endif
