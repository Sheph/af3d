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

#ifndef _AF3D_AABB_H_
#define _AF3D_AABB_H_

#include "af3d/Types.h"
#include "af3d/Vector3.h"
#include "af3d/Utils.h"
#include "bullet/LinearMath/btAabbUtil2.h"

namespace af3d
{
    class AABB
    {
    public:
        enum Corner
        {
            FarLeftBottom = 0,
            FarLeftTop = 1,
            FarRightTop = 2,
            FarRightBottom = 3,
            NearRightBottom = 7,
            NearLeftBottom = 6,
            NearLeftTop = 5,
            NearRightTop = 4
        };

        btVector3 lowerBound;
        btVector3 upperBound;

        AABB() = default;
        AABB(const btVector3& lowerBound, const btVector3& upperBound)
        : lowerBound(lowerBound),
          upperBound(upperBound)
        {
        }

        bool empty() const
        {
            btVector3 d = upperBound - lowerBound;
            return d.x() <= 0.0f || d.y() <= 0.0f || d.z() <= 0.0f;
        }

        btVector3 getSize() const
        {
            return upperBound - lowerBound;
        }

        float getArea() const
        {
            auto sz = getSize();
            return sz.x() * sz.y() * sz.z();
        }

        float getLargestSize() const
        {
            auto v = getSize();
            return btMax(btMax(v.x(), v.y()), v.z());
        };

        // Get the center of the AABB.
        btVector3 getCenter() const
        {
            return 0.5f * (lowerBound + upperBound);
        }

        // Get the extents of the AABB (half-widths).
        btVector3 getExtents() const
        {
            return 0.5f * (upperBound - lowerBound);
        }

        // Combine an AABB into this one.
        void combine(const AABB& aabb)
        {
            lowerBound.setMin(aabb.lowerBound);
            upperBound.setMax(aabb.upperBound);
        }

        void combine(const btVector3& other)
        {
            lowerBound.setMin(other);
            upperBound.setMax(other);
        }

        bool contains(const AABB& aabb) const
        {
            bool result = true;
            result = result && lowerBound.x() <= aabb.lowerBound.x();
            result = result && lowerBound.y() <= aabb.lowerBound.y();
            result = result && lowerBound.z() <= aabb.lowerBound.z();
            result = result && aabb.upperBound.x() <= upperBound.x();
            result = result && aabb.upperBound.y() <= upperBound.y();
            result = result && aabb.upperBound.z() <= upperBound.z();
            return result;
        }

        bool contains(const btVector3& pt) const
        {
            return TestPointAgainstAabb2(lowerBound, upperBound, pt);
        }

        bool overlaps(const AABB& aabb) const
        {
            return TestAabbAgainstAabb2(lowerBound, upperBound, aabb.lowerBound, aabb.upperBound);
        }

        AABB getTransformed(const btTransform& xf) const
        {
            AABB res;
            btTransformAabb(lowerBound, upperBound, 0.0f, xf, res.lowerBound, res.upperBound);
            return res;
        }

        AABB scaledAt0(const btVector3& s) const
        {
            auto sa = s.absolute();
            return AABB(lowerBound * sa, upperBound * sa);
        }

        void scaleAt0(const btVector3& s)
        {
            auto sa = s.absolute();
            lowerBound *= sa;
            upperBound *= sa;
        }

        AABB scaledAtCenter(const btVector3& s) const
        {
            auto c = getCenter();
            auto hsz = getExtents() * s.absolute();
            return AABB(c - hsz, c + hsz);
        }

        void scaleAtCenter(const btVector3& s)
        {
            auto c = getCenter();
            auto hsz = getExtents() * s.absolute();
            lowerBound = c - hsz;
            upperBound = c + hsz;
        }

        btVector3 getCorner(Corner corner) const
        {
            switch(corner) {
            case FarLeftBottom:
                return lowerBound;
            case FarLeftTop:
                return btVector3(lowerBound.x(), upperBound.y(), lowerBound.z());
            case FarRightTop:
                return btVector3(upperBound.x(), upperBound.y(), lowerBound.z());
            case FarRightBottom:
                return btVector3(upperBound.x(), lowerBound.y(), lowerBound.z());
            case NearRightBottom:
                return btVector3(upperBound.x(), lowerBound.y(), upperBound.z());
            case NearLeftBottom:
                return btVector3(lowerBound.x(), lowerBound.y(), upperBound.z());
            case NearLeftTop:
                return btVector3(lowerBound.x(), upperBound.y(), upperBound.z());
            case NearRightTop:
                return upperBound;
            default:
                btAssert(false);
                return btVector3_zero;
            }
        }

        inline bool operator==(const AABB& other) const
        {
            return (lowerBound == other.lowerBound) &&
                (upperBound == other.upperBound);
        }

        inline bool operator!=(const AABB& other) const
        {
            return !(*this == other);
        }
    };

    inline bool btIsValid(const AABB& aabb)
    {
        btVector3 d = aabb.upperBound - aabb.lowerBound;
        bool valid = d.x() >= 0.0f && d.y() >= 0.0f && d.z() >= 0.0f;
        valid = valid && btIsValid(aabb.lowerBound) && btIsValid(aabb.upperBound);
        return valid;
    }

    extern const AABB AABB_empty;
}

#endif
