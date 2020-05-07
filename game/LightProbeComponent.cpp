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

#include "LightProbeComponent.h"
#include "CameraComponent.h"
#include "TextureManager.h"
#include "MaterialManager.h"
#include "MeshManager.h"
#include "SceneObject.h"
#include "Scene.h"
#include "Logger.h"
#include "Settings.h"
#include "Const.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(LightProbeComponent, PhasedComponent)
    ACLASS_DEFINE_END(LightProbeComponent)

    LightProbeComponent::LightProbeComponent(float resolution)
    : PhasedComponent(AClass_LightProbeComponent, phasePreRender)
    {
        irradianceTexture_ = textureManager.createRenderTexture(TextureTypeCubeMap, resolution, resolution, GL_RGB16F, GL_RGB, GL_FLOAT);
    }

    const AClass& LightProbeComponent::staticKlass()
    {
        return AClass_LightProbeComponent;
    }

    AObjectPtr LightProbeComponent::create(const APropertyValueMap& propVals)
    {
        return AObjectPtr();
    }

    void LightProbeComponent::preRender(float dt)
    {
        if (irradianceGenFilters_[0] && (irradianceGenFilters_[0]->numFramesRendered() > 0)) {
            stopIrradianceGen();
            LOG4CPLUS_INFO(logger(), "LightProbe(" << parent()->name() << "): done");
        }
    }

    void LightProbeComponent::recreate()
    {
        btAssert(scene());

        if (irradianceGenFilters_[0]) {
            LOG4CPLUS_WARN(logger(), "LightProbe(" << parent()->name() << "): recreation still in progress...");
            return;
        }

        LOG4CPLUS_INFO(logger(), "LightProbe(" << parent()->name() << "): recreating...");

        auto sceneCaptureTexture = textureManager.createRenderTexture(TextureTypeCubeMap, 512, 512, GL_RGB16F, GL_RGB, GL_FLOAT);

        auto mainCamera = scene()->mainCamera()->findComponent<CameraComponent>()->camera();

        auto filterMaterial = materialManager.createMaterial(MaterialTypeFilterIrradianceConv);
        filterMaterial->setDepthTest(false);
        filterMaterial->setDepthWrite(false);
        filterMaterial->setCullFaceMode(0);
        filterMaterial->setTextureBinding(SamplerName::Main,
            TextureBinding(sceneCaptureTexture,
                SamplerParams(GL_LINEAR, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR)));
        auto mesh = meshManager.createBoxMesh(btVector3(2.0f, 2.0f, 2.0f), filterMaterial);

        for (size_t i = 0; i < 6; ++i) {
            auto face = static_cast<TextureCubeFace>(i);
            auto cam = std::make_shared<Camera>();
            cam->setOrder(camOrderLightProbe);
            cam->setFov(btRadians(90.0f));
            cam->setAspect(1.0f);
            cam->setClearColor(mainCamera->clearColor());
            cam->setAmbientColor(mainCamera->ambientColor());
            cam->setTransform(btTransform(textureCubeFaceBasis(face), parent()->pos()));
            cam->setRenderTarget(RenderTarget(sceneCaptureTexture, 0, face));
            scene()->addCamera(cam);
            sceneCaptureCameras_[i] = cam;

            irradianceGenFilters_[i] = std::make_shared<RenderFilterComponent>(mesh);
            irradianceGenFilters_[i]->camera()->setOrder(camOrderLightProbe + 1);
            irradianceGenFilters_[i]->camera()->setFov(btRadians(90.0f));
            irradianceGenFilters_[i]->camera()->setAspect(1.0f);
            irradianceGenFilters_[i]->camera()->setTransform(btTransform(textureCubeFaceBasis(face)));
            irradianceGenFilters_[i]->camera()->setRenderTarget(RenderTarget(irradianceTexture_, 0, face));
            parent()->addComponent(irradianceGenFilters_[i]);
        }
    }

    void LightProbeComponent::onRegister()
    {
        scene()->addLightProbe(this);
    }

    void LightProbeComponent::onUnregister()
    {
        stopIrradianceGen();
        scene()->removeLightProbe(this);
    }

    void LightProbeComponent::stopIrradianceGen()
    {
        if (irradianceGenFilters_[0]) {
            for (size_t i = 0; i < irradianceGenFilters_.size(); ++i) {
                irradianceGenFilters_[i]->removeFromParent();
                irradianceGenFilters_[i].reset();
            }
            for (size_t i = 0; i < sceneCaptureCameras_.size(); ++i) {
                scene()->removeCamera(sceneCaptureCameras_[i]);
                sceneCaptureCameras_[i].reset();
            }
        }
    }
}
