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

#ifndef _CAMERA_RENDERER_H_
#define _CAMERA_RENDERER_H_

#include "RenderTarget.h"
#include "RenderPass.h"
#include "RenderList.h"
#include "af3d/AABB2.h"

namespace af3d
{
    class CameraRenderer : boost::noncopyable
    {
    public:
        CameraRenderer();
        ~CameraRenderer() = default;

        inline int order() const { return order_; }
        inline void setOrder(int value) { order_ = value; }

        const AABB2i& viewport() const;
        void setViewport(const AABB2i& value);

        inline const AttachmentPoints& clearMask() const { return clearMask_; }
        inline void setClearMask(const AttachmentPoints& value) { clearMask_ = value; }

        inline const AttachmentColors& clearColors() const { return clearColors_; }
        inline const Color& clearColor(AttachmentPoint attachmentPoint = AttachmentPoint::Color0) const { return clearColors_[static_cast<int>(attachmentPoint)]; }
        inline void setClearColor(AttachmentPoint attachmentPoint, const Color& value) { clearColors_[static_cast<int>(attachmentPoint)] = value; }

        inline const RenderTarget& renderTarget(AttachmentPoint attachmentPoint = AttachmentPoint::Color0) const { return renderTarget_[static_cast<int>(attachmentPoint)]; }
        inline void setRenderTarget(AttachmentPoint attachmentPoint, const RenderTarget& value) { renderTarget_[static_cast<int>(attachmentPoint)] = value; }

        void addRenderPass(const RenderPassPtr& pass);

        void setAutoParams(const RenderList& rl, const RenderList::Geometry& geom, std::vector<HardwareTextureBinding>& textures,
            std::vector<StorageBufferBinding>& storageBuffers, MaterialParams& params) const;
        void setAutoParams(const RenderList& rl, const MaterialPtr& material, std::vector<HardwareTextureBinding>& textures,
            std::vector<StorageBufferBinding>& storageBuffers, MaterialParams& params,
            const Matrix4f& modelMat = Matrix4f::getIdentity(), const Matrix4f& prevModelMat = Matrix4f::getIdentity()) const;

        RenderNodePtr compile(const RenderList& rl) const;

    private:
        HardwareMRT getHardwareMRT() const;

        int order_ = 0;
        mutable AABB2i viewport_ = AABB2i(Vector2i(0, 0), Vector2i(0, 0));
        AttachmentPoints clearMask_ = AttachmentPoints(AttachmentPoint::Color0) | AttachmentPoint::Depth;
        AttachmentColors clearColors_;

        std::array<RenderTarget, static_cast<int>(AttachmentPoint::Max) + 1> renderTarget_;

        RenderPasses passes_;
    };
}

#endif
