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

#include "af3d/Frustum.h"

namespace af3d
{
    void Frustum::setType(Type value)
    {
        if (type_ != value) {
            type_ = value;
            projUpdated();
        }
    }

    void Frustum::setFov(float value)
    {
        if (fov_ != value) {
            fov_ = value;
            projUpdated();
        }
    }

    void Frustum::setOrthoHeight(float value)
    {
        if (orthoHeight_ != value) {
            orthoHeight_ = value;
            projUpdated();
        }
    }

    void Frustum::setAspect(float value)
    {
        if (aspect_ != value) {
            aspect_ = value;
            projUpdated();
        }
    }

    void Frustum::setNearDist(float value)
    {
        if (nearDist_ != value) {
            nearDist_ = value;
            projUpdated();
        }
    }

    void Frustum::setFarDist(float value)
    {
        if (farDist_ != value) {
            farDist_ = value;
            projUpdated();
        }
    }

    void Frustum::setTransform(const btTransform& value)
    {
        if (xf_ != value) {
            xf_ = value;
            recalcViewProjMat_ = true;
            recalcPlanes_ = true;
            recalcAABB_ = true;
        }
    }

    const Matrix4f& Frustum::viewProjMat() const
    {
        updateViewProjMat();
        return cachedViewProjMat_;
    }

    const Frustum::Planes& Frustum::planes() const
    {
        updateViewProjMat();
        if (recalcPlanes_) {
            recalcPlanes_ = false;
        }
        return cachedPlanes_;
    }

    const AABB& Frustum::aabb() const
    {
        updateViewProjMat();
        if (recalcAABB_) {
            recalcAABB_ = false;
        }
        return cachedAABB_;
    }

    void Frustum::projUpdated()
    {
        recalcProjMat_ = true;
        recalcViewProjMat_ = true;
        recalcPlanes_ = true;
        recalcAABB_ = true;
    }

    void Frustum::updateViewProjMat() const
    {
        if (!recalcViewProjMat_) {
            return;
        }
        recalcViewProjMat_ = false;

        if (recalcProjMat_) {
            recalcProjMat_ = false;
            if (type_ == Perspective) {
                float top = btTan(fov_ * 0.5f) * nearDist_;
                float bottom = -top;
                float right = aspect_ * top;
                float left = aspect_ * bottom;
                cachedProjMat_.setPerspective(left, right, bottom, top, nearDist_, farDist_);
            } else {
                float orthoWidth = orthoHeight_ * aspect_;
                float left = -orthoWidth / 2.0f;
                float right = orthoWidth / 2.0f;
                float bottom = -orthoHeight_ / 2.0f;
                float top = orthoHeight_ / 2.0f;
                cachedProjMat_.setOrtho(left, right, bottom, top, nearDist_, farDist_);
            }
        }

        cachedViewProjMat_ = cachedProjMat_ * Matrix4f(xf_);
    }
}
