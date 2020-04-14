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

#ifndef _RENDERQUADCOMPONENT_H_
#define _RENDERQUADCOMPONENT_H_

#include "RenderComponent.h"
#include "Drawable.h"

namespace af3d
{
    class RenderQuadComponent : public std::enable_shared_from_this<RenderQuadComponent>,
        public RenderComponent
    {
    public:
        RenderQuadComponent();
        ~RenderQuadComponent() = default;

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        void update(float dt) override;

        void render(RenderList& rl, void* const* parts, size_t numParts) override;

        std::pair<AObjectPtr, float> testRay(const Frustum& frustum, const Ray& ray, void* part) override;

        inline const DrawablePtr& drawable() const { return drawable_; }
        void setDrawable(const DrawablePtr& value);

        inline const btVector3& pos() const { return pos_; }
        void setPos(const btVector3& value);

        inline float angle() const { return angle_; }
        void setAngle(float value);

        inline const Vector2f& size() const { return size_; }
        void setSize(const Vector2f& value);

        void setWidth(float value, bool keepAspect = true);
        void setHeight(float value, bool keepAspect = true);

        inline bool fixedPos() const { return fixedPos_; }
        void setFixedPos(bool value);

        inline bool flip() const { return flip_; }
        void setFlip(bool value);

        inline const Color& color() const { return color_; }
        inline void setColor(const Color& value) { color_ = value; }

        inline bool depthTest() const { return depthTest_; }
        void setDepthTest(bool value);

        inline float viewportHeight() const { return viewportHeight_; }
        void setViewportHeight(float value);

        APropertyValue propertyPositionGet(const std::string&) const { return pos_; }
        void propertyPositionSet(const std::string&, const APropertyValue& value) { setPos(value.toVec3()); }

        APropertyValue propertyAngleGet(const std::string&) const { return angle_; }
        void propertyAngleSet(const std::string&, const APropertyValue& value) { setAngle(value.toFloat()); }

        APropertyValue propertySizeGet(const std::string&) const { return size_; }
        void propertySizeSet(const std::string&, const APropertyValue& value) { setSize(value.toVec2f()); }

        APropertyValue propertyWidthGet(const std::string&) const { return size_.x(); }
        void propertyWidthSet(const std::string&, const APropertyValue& value) { setWidth(value.toFloat()); }

        APropertyValue propertyHeightGet(const std::string&) const { return size_.y(); }
        void propertyHeightSet(const std::string&, const APropertyValue& value) { setHeight(value.toFloat()); }

    private:
        void onRegister() override;

        void onUnregister() override;

        void updatePoints(const Vector2f& sz);

        AABB updateAABB();

        DrawablePtr drawable_;
        btVector3 pos_ = btVector3_zero;
        float angle_ = 0.0f;
        Vector2f size_ = Vector2f_zero;
        bool fixedPos_ = false;
        bool flip_ = false;
        Color color_ = Color_one;
        bool depthTest_ = true;
        float viewportHeight_ = 0.0f;
        MaterialPtr material_;

        bool dirty_ = false;

        std::array<btVector3, 4> points_;
        btVector3 worldCenter_;

        btTransform prevParentXf_;
        AABB prevAABB_;
        RenderCookie* cookie_ = nullptr;
    };

    using RenderQuadComponentPtr = std::shared_ptr<RenderQuadComponent>;

    ACLASS_DECLARE(RenderQuadComponent)
}

#endif
