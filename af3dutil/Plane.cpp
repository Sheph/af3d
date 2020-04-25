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

#include "af3d/Plane.h"

btVector3 btPlaneProject(const btPlane& plane, const btVector3& point)
{
    return point - plane.normal * (point.dot(plane.normal) + plane.dist);
}

af3d::PlaneSide btPlanePointTest(const btPlane& plane, const btVector3& p)
{
    float dist = btPlanePointDistance(plane, p);
    if (dist < 0.0f) {
        return af3d::PlaneSide::Under;
    } else if (dist > 0.0f) {
        return af3d::PlaneSide::Over;
    } else {
        return af3d::PlaneSide::None;
    }
}

float btPlanePointDistance(const btPlane& plane, const btVector3& p)
{
    return p.dot(plane.normal) + plane.dist;
}

af3d::PlaneSide btPlaneAABBTest(const btPlane& plane, const btVector3& center, const btVector3& extents)
{
    float dist = center.dot(plane.normal) + plane.dist;

    float maxAbsDist = plane.normal.absolute().dot(extents.absolute());

    if (dist < -maxAbsDist) {
        return af3d::PlaneSide::Under;
    } else if (dist > maxAbsDist) {
        return af3d::PlaneSide::Over;
    } else {
        return af3d::PlaneSide::Both;
    }
}

af3d::PlaneSide btPlaneAABBTest(const btPlane& plane, const af3d::AABB& aabb)
{
    return btPlaneAABBTest(plane, aabb.getCenter(), aabb.getExtents());
}
