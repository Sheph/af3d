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
    void Frustum::setProjectionType(ProjectionType value)
    {
        if (projectionType_ != value) {
            projectionType_ = value;
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

    void Frustum::setFlipY(bool value)
    {
        if (flipY_ != value) {
            flipY_ = value;
            projUpdated();
        }
    }

    void Frustum::setTransform(const btTransform& value)
    {
        if (xf_ != value) {
            xf_ = value;
            recalcViewProjMat_ = true;
            recalcPlanes_ = true;
        }
    }

    void Frustum::setJitter(const Vector2f& value)
    {
        if (jitter_ != value) {
            jitter_ = value;
            recalcViewProjMat_ = true;
        }
    }

    const Matrix4f& Frustum::projMat() const
    {
        updateViewProjMat();
        return cachedProjMat_;
    }

    const Matrix4f& Frustum::viewProjMat() const
    {
        updateViewProjMat();
        return cachedViewProjMat_;
    }

    const Matrix4f& Frustum::jitteredViewProjMat() const
    {
        updateViewProjMat();
        return cachedJitteredViewProjMat_;
    }

    const Matrix4f& Frustum::viewMat() const
    {
        updateViewProjMat();
        return cachedViewMat_;
    }

    const Frustum::Planes& Frustum::planes() const
    {
        updateViewProjMat();
        if (recalcPlanes_) {
            recalcPlanes_ = false;

            const Matrix4f& m = cachedViewProjMat_;

            cachedPlane(Plane::Left).normal.setX(m[3][0] + m[0][0]);
            cachedPlane(Plane::Left).normal.setY(m[3][1] + m[0][1]);
            cachedPlane(Plane::Left).normal.setZ(m[3][2] + m[0][2]);
            cachedPlane(Plane::Left).dist = m[3][3] + m[0][3];

            cachedPlane(Plane::Right).normal.setX(m[3][0] - m[0][0]);
            cachedPlane(Plane::Right).normal.setY(m[3][1] - m[0][1]);
            cachedPlane(Plane::Right).normal.setZ(m[3][2] - m[0][2]);
            cachedPlane(Plane::Right).dist = m[3][3] - m[0][3];

            cachedPlane(Plane::Top).normal.setX(m[3][0] - m[1][0]);
            cachedPlane(Plane::Top).normal.setY(m[3][1] - m[1][1]);
            cachedPlane(Plane::Top).normal.setZ(m[3][2] - m[1][2]);
            cachedPlane(Plane::Top).dist = m[3][3] - m[1][3];

            cachedPlane(Plane::Bottom).normal.setX(m[3][0] + m[1][0]);
            cachedPlane(Plane::Bottom).normal.setY(m[3][1] + m[1][1]);
            cachedPlane(Plane::Bottom).normal.setZ(m[3][2] + m[1][2]);
            cachedPlane(Plane::Bottom).dist = m[3][3] + m[1][3];

            cachedPlane(Plane::Near).normal.setX(m[3][0] + m[2][0]);
            cachedPlane(Plane::Near).normal.setY(m[3][1] + m[2][1]);
            cachedPlane(Plane::Near).normal.setZ(m[3][2] + m[2][2]);
            cachedPlane(Plane::Near).dist = m[3][3] + m[2][3];

            cachedPlane(Plane::Far).normal.setX(m[3][0] - m[2][0]);
            cachedPlane(Plane::Far).normal.setY(m[3][1] - m[2][1]);
            cachedPlane(Plane::Far).normal.setZ(m[3][2] - m[2][2]);
            cachedPlane(Plane::Far).dist = m[3][3] - m[2][3];

            for (int i = 0; i < 6; ++i) {
                float length = btZeroNormalize(cachedPlanes_[i].normal);
                cachedPlanes_[i].dist /= length;
            }
        }
        return cachedPlanes_;
    }

    void Frustum::projUpdated()
    {
        recalcProjMat_ = true;
        recalcViewProjMat_ = true;
        recalcPlanes_ = true;
    }

    void Frustum::updateViewProjMat() const
    {
        if (!recalcViewProjMat_) {
            return;
        }
        recalcViewProjMat_ = false;

        if (recalcProjMat_) {
            recalcProjMat_ = false;
            if (projectionType_ == ProjectionType::Perspective) {
                float top = btTan(fov_ * 0.5f) * nearDist_;
                float bottom = -top;
                float right = aspect_ * top;
                float left = -right;
                if (flipY_) {
                    top = -top;
                    bottom = -bottom;
                }
                cachedProjMat_.setPerspective(left, right, bottom, top, nearDist_, farDist_);
            } else {
                float orthoWidth = orthoHeight_ * aspect_;
                float left = -orthoWidth / 2.0f;
                float right = orthoWidth / 2.0f;
                float bottom = -orthoHeight_ / 2.0f;
                float top = orthoHeight_ / 2.0f;
                if (flipY_) {
                    top = -top;
                    bottom = -bottom;
                }
                cachedProjMat_.setOrtho(left, right, bottom, top, nearDist_, farDist_);
            }
        }

        Matrix4f xfInv(xf_.inverse());

        cachedViewProjMat_ = cachedProjMat_ * xfInv;
        auto t1 = cachedProjMat_[0][2];
        auto t2 = cachedProjMat_[1][2];
        cachedProjMat_[0][2] += jitter_.x();
        cachedProjMat_[1][2] += jitter_.y();
        cachedJitteredViewProjMat_ = cachedProjMat_ * xfInv;
        cachedProjMat_[0][2] = t1;
        cachedProjMat_[1][2] = t2;
        cachedViewMat_ = Matrix4f(xf_);
    }

    bool Frustum::isVisible(const AABB& aabb) const
    {
        btVector3 center = aabb.getCenter();
        btVector3 extents = aabb.getExtents();

        const Planes& p = planes();

        for (int i = 0; i < 6; ++i) {
            PlaneSide side = btPlaneAABBTest(p[i], center, extents);
            if (side == PlaneSide::Under) {
                return false;
            }
        }

        return true;
    }

    Vector2f Frustum::getExtents(const btVector3& worldPos) const
    {
        updateViewProjMat();
        auto w = cachedViewProjMat_[3].dot(Vector4f(worldPos, 1.0f));
        return Vector2f(2.0f * w / cachedProjMat_[0][0],
            2.0f * w / cachedProjMat_[1][1]);
    }
}
