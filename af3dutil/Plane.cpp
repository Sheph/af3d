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

btVector3 btPlaneLineIntersection(const btPlane& plane, const btVector3& p0, const btVector3& p1)
{
    btVector3 dif = p1 - p0;
    float dn = plane.normal.dot(dif);
    float t = -(plane.dist + plane.normal.dot(p0)) / dn;
    return p0 + (dif * t);
}

btVector3 btPlaneThreeIntersection(const btPlane& p0, const btPlane& p1, const btPlane& p2)
{
    btVector3 N1 = p0.normal;
    btVector3 N2 = p1.normal;
    btVector3 N3 = p2.normal;

    btVector3 n2n3;
    n2n3 = N2.cross(N3);
    btVector3 n3n1;
    n3n1 = N3.cross(N1);
    btVector3 n1n2;
    n1n2 = N1.cross(N2);

    float quotient = (N1.dot(n2n3));

    btAssert(btFabs(quotient) > 0.000001f);

    quotient = -1.0f / quotient;
    n2n3 *= p0.dist;
    n3n1 *= p1.dist;
    n1n2 *= p2.dist;
    btVector3 potentialVertex = n2n3;
    potentialVertex += n3n1;
    potentialVertex += n1n2;
    potentialVertex *= quotient;

    btVector3 result(potentialVertex.getX(), potentialVertex.getY(), potentialVertex.getZ());
    return result;
}

af3d::PlaneSide btPlanePointTest(const btPlane& plane, const btVector3& p)
{
    float dist = p.dot(plane.normal) + plane.dist;
    if (dist < 0.0f) {
        return af3d::PlaneSide::Under;
    } else if (dist > 0.0f) {
        return af3d::PlaneSide::Over;
    } else {
        return af3d::PlaneSide::None;
    }
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
