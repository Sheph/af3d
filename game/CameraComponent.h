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

#ifndef _CAMERACOMPONENT_H_
#define _CAMERACOMPONENT_H_

#include "PhasedComponent.h"
#include "af3d/Frustum.h"
#include "af3d/AABB2.h"

namespace af3d
{
    class CameraComponent : public std::enable_shared_from_this<CameraComponent>,
        public PhasedComponent
    {
    public:
        explicit CameraComponent(const btTransform& xf = btTransform::getIdentity());
        ~CameraComponent() = default;

        ComponentPtr sharedThis() override { return shared_from_this(); }

        void preRender(float dt) override;

        inline ProjectionType projectionType() const { return frustum_.projectionType(); }
        inline void setProjectionType(ProjectionType value) { frustum_.setProjectionType(value); }

        inline float fov() const { return frustum_.fov(); }
        inline void setFov(float value) { frustum_.setFov(value); }

        inline float orthoHeight() const { return frustum_.orthoHeight(); }
        inline void setOrthoHeight(float value) { frustum_.setOrthoHeight(value); }

        inline float aspect() const { return frustum_.aspect(); }
        inline void setAspect(float value) { frustum_.setAspect(value); }

        inline float nearDist() const { return frustum_.nearDist(); }
        inline void setNearDist(float value) { frustum_.setNearDist(value); }

        inline float farDist() const { return frustum_.farDist(); }
        inline void setFarDist(float value) { frustum_.setFarDist(value); }

        inline const Color& clearColor() const { return clearColor_; }
        inline void setClearColor(const Color& value) { clearColor_ = value; }

        inline const AABB2i& viewport() const { return viewport_; }
        inline void setViewport(const AABB2i& value) { viewport_ = value; }

        const Frustum& getFrustum() const;

    private:
        void onRegister() override;

        void onUnregister() override;

        btTransform xf_;

        Color clearColor_ = Color(0.0f, 0.0f, 0.4f, 1.0f);
        AABB2i viewport_{Vector2i(0, 0), Vector2i(0, 0)};

        mutable Frustum frustum_;
    };

    using CameraComponentPtr = std::shared_ptr<CameraComponent>;
}

#endif
