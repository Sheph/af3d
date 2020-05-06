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

#include "TVComponent.h"
#include "CameraComponent.h"
#include "Scene.h"
#include "SceneObject.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(TVComponent, PhasedComponent)
    ACLASS_DEFINE_END(TVComponent)

    TVComponent::TVComponent(const CameraUsageComponentPtr& cameraUsage, const AABB& tvAabb, const CameraPtr& filterCam)
    : PhasedComponent(AClass_TVComponent, phasePreRender),
      cameraUsage_(cameraUsage),
      tvAabb_(tvAabb),
      filterCam_(filterCam)
    {
    }

    const AClass& TVComponent::staticKlass()
    {
        return AClass_TVComponent;
    }

    AObjectPtr TVComponent::create(const APropertyValueMap& propVals)
    {
        return AObjectPtr();
    }

    void TVComponent::preRender(float dt)
    {
        checkUsage();
    }

    void TVComponent::onRegister()
    {
        checkUsage();
    }

    void TVComponent::onUnregister()
    {
        if (showing_) {
            cameraUsage_->decUseCount();
            scene()->removeCamera(filterCam_);
            showing_ = false;
        }
    }

    void TVComponent::checkUsage()
    {
        const auto& frustum = scene()->mainCamera()->findComponent<CameraComponent>()->camera()->frustum();
        bool show = frustum.isVisible(tvAabb_.getTransformed(parent()->smoothTransform())) &&
            (parent()->getSmoothForward().dot(parent()->smoothPos() - frustum.transform().getOrigin()) >= 0);
        if (!showing_ && show) {
            cameraUsage_->incUseCount();
            scene()->addCamera(filterCam_);
            showing_ = true;
        } else if (showing_ && !show) {
            cameraUsage_->decUseCount();
            scene()->removeCamera(filterCam_);
            showing_ = false;
        }
    }
}
