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

#ifndef _AF3D_AABB2_H_
#define _AF3D_AABB2_H_

#include "af3d/Types.h"
#include "af3d/Vector2.h"
#include "af3d/Utils.h"

namespace af3d
{
    template <class T>
    class AABB2
    {
    public:
        Vector2<T> lowerBound;
        Vector2<T> upperBound;

        AABB2() {}
        AABB2(const Vector2<T>& lowerBound, const Vector2<T>& upperBound)
        : lowerBound(lowerBound),
          upperBound(upperBound)
        {
        }

        // Get the center of the AABB.
        Vector2<T> getCenter() const
        {
            return (lowerBound + upperBound) / 2;
        }

        // Get the extents of the AABB (half-widths).
        Vector2<T> getExtents() const
        {
            return (upperBound - lowerBound) / 2;
        }

        // Combine an AABB into this one.
        void combine(const AABB2<T>& aabb)
        {
            lowerBound.setMin(aabb.lowerBound);
            upperBound.setMax(aabb.upperBound);
        }

        bool contains(const AABB2<T>& aabb) const
        {
            bool result = true;
            result = result && lowerBound.x() <= aabb.lowerBound.x();
            result = result && lowerBound.y() <= aabb.lowerBound.y();
            result = result && aabb.upperBound.x() <= upperBound.x();
            result = result && aabb.upperBound.y() <= upperBound.y();
            return result;
        }
    };

    using AABB2f = AABB2<float>;
    using AABB2i = AABB2<int>;

    inline bool btIsValid(const AABB2f& aabb)
    {
        auto d = aabb.upperBound - aabb.lowerBound;
        bool valid = d.x() >= 0.0f && d.y() >= 0.0f;
        valid = valid && btIsValid(aabb.lowerBound) && btIsValid(aabb.upperBound);
        return valid;
    }
}

#endif
