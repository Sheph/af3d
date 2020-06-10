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

#include "Camera.h"
#include "CameraRenderer.h"
#include "Settings.h"
#include "RenderPassCluster.h"
#include "RenderPassGeometry.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(Camera, AObject)
    ACLASS_DEFINE_END(Camera)

    Camera::Camera(bool withDefaultRenderer)
    : AObject(AClass_Camera)
    {
        if (withDefaultRenderer) {
            auto r = std::make_shared<CameraRenderer>();
            r->addRenderPass(std::make_shared<RenderPassCluster>());
            r->addRenderPass(std::make_shared<RenderPassGeometry>(AttachmentPoint::Color0, true, true, false));
            addRenderer(r);
        }
    }

    const AClass& Camera::staticKlass()
    {
        return AClass_Camera;
    }

    AObjectPtr Camera::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<Camera>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void Camera::addRenderer(const CameraRendererPtr& cr)
    {
        renderers_.push_back(cr);
    }

    int Camera::order() const
    {
        return renderers_[0]->order();
    }

    void Camera::setOrder(int value)
    {
        renderers_[0]->setOrder(value);
    }

    const AABB2i& Camera::viewport() const
    {
        return renderers_[0]->viewport();
    }

    void Camera::setViewport(const AABB2i& value)
    {
        renderers_[0]->setViewport(value);
    }

    const AttachmentPoints& Camera::clearMask() const
    {
        return renderers_[0]->clearMask();
    }

    void Camera::setClearMask(const AttachmentPoints& value)
    {
        renderers_[0]->setClearMask(value);
    }

    const AttachmentColors& Camera::clearColors() const
    {
        return renderers_[0]->clearColors();
    }

    const Color& Camera::clearColor(AttachmentPoint attachmentPoint) const
    {
        return renderers_[0]->clearColor(attachmentPoint);
    }

    void Camera::setClearColor(AttachmentPoint attachmentPoint, const Color& value)
    {
        renderers_[0]->setClearColor(attachmentPoint, value);
    }

    const RenderTarget& Camera::renderTarget(AttachmentPoint attachmentPoint) const
    {
        return renderers_[0]->renderTarget(attachmentPoint);
    }

    void Camera::setRenderTarget(AttachmentPoint attachmentPoint, const RenderTarget& value)
    {
        renderers_[0]->setRenderTarget(attachmentPoint, value);
    }
}
