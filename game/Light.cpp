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

#include "Light.h"
#include "SceneObject.h"
#include "CameraComponent.h"
#include "Scene.h"
#include "MeshManager.h"
#include "MaterialManager.h"
#include "Settings.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN_ABSTRACT(Light, RenderComponent)
    ACLASS_PROPERTY(Light, LocalTransform, AProperty_LocalTransform, "Local transform", Transform, btTransform::getIdentity(), Position, APropertyEditable)
    ACLASS_PROPERTY(Light, WorldTransform, AProperty_WorldTransform, "World transform", Transform, btTransform::getIdentity(), Position, APropertyEditable|APropertyTransient)
    ACLASS_PROPERTY(Light, Color, "color", "Color", ColorRGB, Color(1.0f, 1.0f, 1.0f, 1.0f), General, APropertyEditable)
    ACLASS_PROPERTY(Light, Intensity, "intensity", "Intensity", Float, 1.0f, General, APropertyEditable)
    ACLASS_DEFINE_END(Light)

    Light::Light(const AClass& klass, int typeId, bool usesDirection)
    : RenderComponent(klass),
      typeId_(typeId),
      usesDirection_(usesDirection)
    {
        btAssert(typeId > 0);
    }

    const AClass& Light::staticKlass()
    {
        return AClass_Light;
    }

    void Light::update(float dt)
    {
        if (markerRc_) {
            auto camera = scene()->mainCamera()->findComponent<CameraComponent>()->camera();
            auto viewExt = camera->frustum().getExtents((parent()->transform() * xf_).getOrigin());
            if (viewExt.y() * viewExt.y() >= 0.0000001f) {
                float markerSz = markerRc_->mesh()->aabb().getLargestSize();
                if ((markerSz * settings.viewHeight / viewExt.y()) > settings.editor.lightMarkerSizePixels) {
                    markerRc_->setScale(btVector3_one * viewExt.y() *
                        (static_cast<float>(settings.editor.lightMarkerSizePixels) / settings.viewHeight) / markerSz);
                } else {
                    markerRc_->setScale(btVector3_one);
                }
                if (!usesDirection_) {
                    markerRc_->setTransform(
                        btTransform(parent()->basis().inverse() * scene()->mainCamera()->transform().getBasis(),
                            markerRc_->transform().getOrigin()));
                }

                auto w = scene()->workspace();
                auto em = w->emLight();

                bool showMarker = em->active() || w->emObject()->active();

                markerRc_->setVisible(showMarker);

                if (showMarker) {
                    if (em->active()) {
                        if (em->isSelected(sharedThis())) {
                            setMarkerParams(settings.editor.lightMarkerAlphaSelected, false);
                        } else if (em->isHovered(sharedThis())) {
                            setMarkerParams(settings.editor.lightMarkerAlphaHovered, false);
                        } else {
                            setMarkerParams(settings.editor.lightMarkerAlphaInactive, false);
                        }
                    } else {
                        setMarkerParams(settings.editor.lightMarkerAlphaOff, true);
                    }
                }
            }
        }

        if ((parent()->smoothTransform() == prevParentXf_) && !dirty_) {
            return;
        }

        worldXf_ = parent()->smoothTransform() * xf_;
        dirty_ = false;

        AABB aabb = getWorldAABB();

        btVector3 displacement = parent()->smoothTransform().getOrigin() - prevParentXf_.getOrigin();

        manager()->moveAABB(cookie_, prevAABB_, aabb, displacement);

        prevParentXf_ = parent()->smoothTransform();
        prevAABB_ = aabb;
    }

    void Light::render(RenderList& rl, void* const* parts, size_t numParts)
    {
        rl.addLight(std::static_pointer_cast<Light>(sharedThis()));
    }

    std::pair<AObjectPtr, float> Light::testRay(const Frustum& frustum, const Ray& ray, void* part)
    {
        auto res = ray.testAABB(prevAABB_);
        if (res.first) {
            return std::make_pair(sharedThis(), res.second);
        } else {
            return std::make_pair(AObjectPtr(), 0.0f);
        }
    }

    void Light::setTransform(const btTransform& value)
    {
        xf_ = value;
        worldXf_ = prevParentXf_ * xf_;
        dirty_ = true;
        if (markerRc_) {
            markerRc_->setTransform(xf_);
        }
    }

    AABB Light::getWorldAABB() const
    {
        return localAABB_.getTransformed(worldTransform());
    }

    void Light::setupMaterial(const btVector3& eyePos, MaterialParams& params) const
    {
        const auto& xf = worldTransform();
        params.setUniform(UniformName::LightPos, Vector4f(xf.getOrigin(), typeId_));
        params.setUniform(UniformName::LightColor, Vector3f(color_.x(), color_.y(), color_.z()) * color_.w());
        doSetupMaterial(eyePos, params);
    }

    void Light::setLocalAABBImpl(const AABB& value)
    {
        localAABB_ = value;
        dirty_ = true;
    }

    void Light::onRegister()
    {
        prevParentXf_ = parent()->smoothTransform();
        worldXf_ = prevParentXf_ * xf_;
        prevAABB_ = getWorldAABB();
        cookie_ = manager()->addAABB(this, prevAABB_, nullptr);

        if (((aflags() & AObjectEditable) != 0) && scene()->workspace()) {
            origMarkerMesh_ = meshManager.loadConvertedMesh("light_marker.fbx", MaterialTypeUnlit);
            auto mesh = origMarkerMesh_->clone();
            for (const auto& sm : mesh->subMeshes()) {
                sm->material()->setBlendingParams(BlendingParams(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
            }
            markerRc_ = std::make_shared<RenderMeshComponent>();
            markerRc_->setMesh(mesh);
            markerRc_->setTransform(xf_);
            markerRc_->aflagsSet(AObjectMarkerLight);
            markerRc_->cameraFilter().layers() = CameraLayer::Main;
            setMarkerParams(settings.editor.lightMarkerAlphaOff, true);
            parent()->addComponent(markerRc_);
        }
    }

    void Light::onUnregister()
    {
        manager()->removeAABB(cookie_);
        if (markerRc_) {
            markerRc_->removeFromParent();
            markerRc_.reset();
            origMarkerMesh_.reset();
        }
    }

    void Light::setMarkerParams(float alpha, bool depthTest)
    {
        if (!markerRc_) {
            return;
        }
        for (size_t i = 0; i < origMarkerMesh_->subMeshes().size(); ++i) {
            markerRc_->mesh()->subMeshes()[i]->material()->setDepthTest(depthTest);
            Color c;
            if (origMarkerMesh_->subMeshes()[i]->material()->params().getUniform(UniformName::MainColor, c, true)) {
                c *= color_;
                c.setW(alpha);
                markerRc_->mesh()->subMeshes()[i]->material()->params().setUniform(UniformName::MainColor, c);
            }
        }
    }
}
