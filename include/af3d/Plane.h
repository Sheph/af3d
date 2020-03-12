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

#ifndef _AF3D_PLANE_H_
#define _AF3D_PLANE_H_

#include "af3d/AABB.h"
#include "bullet/LinearMath/btConvexHull.h"

namespace af3d
{
    enum class PlaneSide
    {
        None = 0,
        Under,
        Over,
        Both
    };
}

inline bool operator==(const btPlane& a, const btPlane& b)
{
    return (a.normal == b.normal && a.dist == b.dist);
}

inline bool operator!=(const btPlane& a, const btPlane& b)
{
    return !(a == b);
}

inline btPlane btPlaneFlip(const btPlane& plane)
{
    return btPlane(-plane.normal, -plane.dist);
}

inline bool btPlaneIsCoplanar(const btPlane& a, const btPlane& b)
{
    return (a == b || a == btPlaneFlip(b));
}

btVector3 btPlaneProject(const btPlane& plane, const btVector3& point);
btVector3 btPlaneLineIntersection(const btPlane& plane, const btVector3& p0, const btVector3& p1);
btVector3 btPlaneThreeIntersection(const btPlane& p0, const btPlane& p1, const btPlane& p2);
af3d::PlaneSide btPlanePointTest(const btPlane& plane, const btVector3& p);
af3d::PlaneSide btPlaneAABBTest(const btPlane& plane, const btVector3& center, const btVector3& extents);
af3d::PlaneSide btPlaneAABBTest(const btPlane& plane, const af3d::AABB& aabb);

#endif
