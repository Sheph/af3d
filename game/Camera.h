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

#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "Texture.h"
#include "OGL.h"
#include "af3d/Frustum.h"
#include "af3d/AABB2.h"

namespace af3d
{
    class Camera : public std::enable_shared_from_this<Camera>,
        public AObject
    {
    public:
        Camera();
        ~Camera() = default;

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        inline int order() const { return order_; }
        inline void setOrder(int value) { order_ = value; }

        inline bool isMain() const { return isMain_; }
        inline void setIsMain(bool value) { isMain_ = value; }

        inline const btTransform& transform() const { return frustum_.transform(); }
        inline void setTransform(const btTransform& value) { frustum_.setTransform(value); }

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

        inline bool flipY() const { return frustum_.flipY(); }
        inline void setFlipY(bool value) { frustum_.setFlipY(value); }

        inline const AABB2i& viewport() const { return viewport_; }
        inline void setViewport(const AABB2i& value) { viewport_ = value; }

        inline const Frustum& frustum() const { return frustum_; }

        inline GLenum clearMask() const { return clearMask_; }
        inline void setClearMask(GLenum value) { clearMask_ = value; }

        inline const Color& clearColor() const { return clearColor_; }
        inline void setClearColor(const Color& value) { clearColor_ = value; }

        inline const Color& ambientColor() const { return ambientColor_; }
        inline void setAmbientColor(const Color& value) { ambientColor_ = value; }

        inline const TexturePtr& targetTexture() const { return targetTexture_; }
        inline void setTargetTexture(const TexturePtr& value) { targetTexture_ = value; }

    private:
        int order_ = 0;
        bool isMain_ = false;
        Frustum frustum_;
        AABB2i viewport_ = AABB2i(Vector2i(0, 0), Vector2i(0, 0));
        GLenum clearMask_ = GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT;
        Color clearColor_ = Color(0.23f, 0.23f, 0.23f, 1.0f);
        Color ambientColor_ = Color(0.2f, 0.2f, 0.2f, 1.0f);

        TexturePtr targetTexture_;
    };

    using CameraPtr = std::shared_ptr<Camera>;

    ACLASS_DECLARE(Camera)
}

#endif
