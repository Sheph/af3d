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

#include "ShadowMapCSM.h"
#include "ShadowManager.h"
#include "RenderPassCSM.h"
#include "Const.h"
#include "CameraRenderer.h"
#include "Scene.h"
#include "Logger.h"

namespace af3d
{
    ShadowMapCSM::ShadowMapCSM(Scene* scene)
    : ShadowMap(scene)
    {
        biasMat_[0].setValue(0.5f, 0.0f, 0.0f, 0.5f);
        biasMat_[1].setValue(0.0f, 0.5f, 0.0f, 0.5f);
        biasMat_[2].setValue(0.0f, 0.0f, 0.5f, 0.5f);
        biasMat_[3].setValue(0.0f, 0.0f, 0.0f, 1.0f);
    }

    ShadowMapCSM::~ShadowMapCSM()
    {
        btAssert(mgr_ == nullptr);
    }

    void ShadowMapCSM::remove()
    {
        if (mgr_) {
            mgr_->removeShadowMap(this);
        }
    }

    void ShadowMapCSM::update(const Frustum& viewFrustum, const btTransform& lightXf)
    {
        btAssert(!splits_.empty());

        if ((prevViewFov_ != viewFrustum.fov()) ||
            (prevViewAspect_ != viewFrustum.aspect()) ||
            (prevViewNearDist_ != viewFrustum.nearDist()) ||
            (prevViewFarDist_ != viewFrustum.farDist())) {
            LOG4CPLUS_TRACE(logger(), "Updating CSM " << this << " proj params");

            prevViewFov_ = viewFrustum.fov();
            prevViewAspect_ = viewFrustum.aspect();
            prevViewNearDist_ = viewFrustum.nearDist();
            prevViewFarDist_ = viewFrustum.farDist();

            float nd = viewFrustum.nearDist();
            float fd = viewFrustum.farDist();

            float ratio = fd / nd;
            splits_[0].viewFrustum.setNearDist(nd);

            for (size_t i = 0; i < splits_.size(); ++i) {
                splits_[i].viewFrustum.setFov(viewFrustum.fov() + btRadians(11.5f));
                splits_[i].viewFrustum.setAspect(viewFrustum.aspect());
                if (i > 0) {
                    float si = i / (float)splits_.size();
                    float curNear = splitWeight_ * (nd * std::pow(ratio, si)) + (1.0f - splitWeight_) * (nd + (fd - nd) * si);
                    float curFar = curNear * 1.005f;
                    splits_[i].viewFrustum.setNearDist(curNear);
                    splits_[i - 1].viewFrustum.setFarDist(curFar);
                }
            }

            splits_.back().viewFrustum.setFarDist(fd);
        }

        auto lightViewXf = lightXf.inverse();

        for (auto& split : splits_) {
            btVector3 tMax = btVector3_one * -(std::numeric_limits<float>::max)();
            btVector3 tMin = btVector3_one * (std::numeric_limits<float>::max)();

            split.viewFrustum.setTransform(viewFrustum.transform());

            const auto& corners = split.viewFrustum.corners();

            for (size_t i = 0; i < corners.size(); ++i) {
                auto viewPt = lightViewXf * corners[i];
                tMin.setZ(btMin(tMin.z(), viewPt.z()));
                tMax.setZ(btMax(tMax.z(), viewPt.z()));
            }

            tMax.setZ(tMax.z() + 50.0f);

            split.cam->setTransform(lightXf);
            split.cam->setAspect(1.0f);
            split.cam->setOrthoHeight(2.0f);
            split.cam->setNearDist(-tMax.z());
            split.cam->setFarDist(-tMin.z());

            const auto& vp = split.cam->frustum().viewProjMat();

            for (const auto& corner : corners) {
                auto cornerNDC = vp * Vector4f(corner, 1.0f);
                btAssert(cornerNDC.w() == 1.0f);

                tMin.setX(btMin(tMin.x(), cornerNDC.x()));
                tMin.setY(btMin(tMin.y(), cornerNDC.y()));
                tMax.setX(btMax(tMax.x(), cornerNDC.x()));
                tMax.setY(btMax(tMax.y(), cornerNDC.y()));
            }

            btVector3 offset((tMin.x() + tMax.x()) * 0.5f, (tMin.y() + tMax.y()) * 0.5f, 0.0f);

            split.cam->setOrthoHeight(tMax.y() - tMin.y());
            split.cam->setAspect((tMax.x() - tMin.x()) / split.cam->orthoHeight());
            split.cam->setTransform(lightXf * toTransform(offset));

            split.mat = biasMat_ * split.cam->frustum().viewProjMat();

            split.farBound = 0.5f * (-split.viewFrustum.farDist() * viewFrustum.projMat()[2][2] + viewFrustum.projMat()[2][3]) /
                split.viewFrustum.farDist() + 0.5f;
        }
    }

    void ShadowMapCSM::setupSSBO(ShaderCSM& sCSM) const
    {
        btAssert(splits_.size() <= sizeof(sCSM.farBounds) / sizeof(sCSM.farBounds[0]));
        for (size_t i = 0; i < splits_.size(); ++i) {
            sCSM.farBounds[i] = splits_[i].farBound;
            sCSM.mat[i] = splits_[i].mat;
            sCSM.texIdx[i] = splits_[i].cam->renderTarget(AttachmentPoint::Depth).layer();
        }
    }

    void ShadowMapCSM::adopt(ShadowManager* mgr, int index, const CSMRenderTarget& rt)
    {
        btAssert(!mgr_);
        mgr_ = mgr;
        index_ = index;
        for (int i = rt.layers.first; i <= rt.layers.second; ++i) {
            auto cam = std::make_shared<Camera>(false);
            cam->setProjectionType(ProjectionType::Orthographic);
            cam->setAspect(1.0f);
            cam->setOrthoHeight(2.0f);
            cam->setCanSeeShadows(false);

            auto r = std::make_shared<CameraRenderer>();
            r->setOrder(camOrderShadow);
            r->setRenderTarget(AttachmentPoint::Depth, RenderTarget(rt.tex, 0, TextureCubeXP, i));
            r->setClearMask(AttachmentPoint::Depth);
            r->addRenderPass(std::make_shared<RenderPassCSM>());
            cam->addRenderer(r);
            scene()->addCamera(cam);

            splits_.emplace_back(cam);
        }
    }

    void ShadowMapCSM::abandon()
    {
        btAssert(mgr_);
        mgr_ = nullptr;
        index_ = -1;
        for (auto& split : splits_) {
            scene()->removeCamera(split.cam);
        }
        splits_.clear();
    }
}
