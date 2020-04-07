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

#ifndef _LIGHT_H_
#define _LIGHT_H_

#include "RenderComponent.h"
#include "Material.h"
#include "AObject.h"
#include "af3d/Types.h"
#include "af3d/AABB.h"
#include "af3d/Vector4.h"

namespace af3d
{
    class Light : public RenderComponent
    {
    public:
        Light(const AClass& klass, int typeId);
        ~Light() = default;

        static const AClass& staticKlass();

        void update(float dt) override;

        void render(RenderList& rl, void* const* parts, size_t numParts) override;

        std::pair<AObjectPtr, float> testRay(const Frustum& frustum, const Ray& ray, void* part) override;

        void debugDraw() override;

        inline const btTransform& transform() const { return xf_; }
        void setTransform(const btTransform& value);

        inline const btTransform& worldTransform() const { return worldXf_; }

        inline const Color& color() const { return color_; }
        inline void setColor(const Color& value) { color_ = value; }

        inline float intensity() const { return color_.w(); }
        inline void setIntensity(float value) { color_.setW(value); }

        inline const AABB& localAABB() const { return localAABB_; }
        AABB getWorldAABB() const;

        void setupMaterial(const btVector3& eyePos, MaterialParams& params) const;

        APropertyValue propertyLocalTransformGet(const std::string&) const { return transform(); }
        void propertyLocalTransformSet(const std::string&, const APropertyValue& value) { setTransform(value.toTransform()); }

        APropertyValue propertyWorldTransformGet(const std::string&) const { return worldTransform(); }
        void propertyWorldTransformSet(const std::string&, const APropertyValue& value) { setTransform(prevParentXf_.inverse() * value.toTransform()); }

        APropertyValue propertyColorGet(const std::string&) const { return Color(color().x(), color().y(), color().z(), 1.0f); }
        void propertyColorSet(const std::string&, const APropertyValue& value)
        {
            auto v = value.toVec3f();
            setColor(Color(v.x(), v.y(), v.z(), color().w()));
        }

        APropertyValue propertyIntensityGet(const std::string&) const { return intensity(); }
        void propertyIntensitySet(const std::string&, const APropertyValue& value) { setIntensity(value.toFloat()); }

    protected:
        void setLocalAABBImpl(const AABB& value);

    private:
        void onRegister() override;

        void onUnregister() override;

        virtual void doSetupMaterial(const btVector3& eyePos, MaterialParams& params) const = 0;

        int typeId_ = 0; // 0 - ambient light.
        btTransform xf_ = btTransform::getIdentity();
        Color color_ = Color_one; // Color in rgb, alpha = intensity.
        AABB localAABB_ = AABB_empty;
        bool dirty_ = false;

        btTransform worldXf_ = btTransform::getIdentity();

        btTransform prevParentXf_ = btTransform::getIdentity();
        AABB prevAABB_ = AABB_empty;
        RenderCookie* cookie_ = nullptr;
    };

    ACLASS_DECLARE(Light)
}

#endif
