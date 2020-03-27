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
#include "af3d/Utils.h"
#include "bullet/LinearMath/btAabbUtil2.h"

namespace af3d
{
    class AABB
    {
    public:
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

        AABB scaled(const btVector3& s) const
        {
            return AABB(lowerBound * s, upperBound * s);
        }

        void scale(const btVector3& s)
        {
            lowerBound *= s;
            upperBound *= s;
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
