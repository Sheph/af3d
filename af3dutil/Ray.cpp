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

#include "af3d/Ray.h"
#include "af3d/Vector3.h"

namespace af3d
{
    Ray::Ray(const btVector3& pos, const btVector3& dir)
    : pos(pos),
      dir(dir)
    {
    }

    bool Ray::empty() const
    {
        return dir == btVector3_zero;
    }

    btVector3 Ray::getAt(float t) const
    {
        return pos + (dir * t);
    }

    Ray Ray::getTransformed(const btTransform& xf) const
    {
        return Ray(xf * pos, xf.getBasis() * dir);
    }

    // See: http://psgraphics.blogspot.com/2016/02/new-simple-ray-box-test-from-andrew.html
    RayTestResult Ray::testAABB(const AABB& aabb) const
    {
        float tmin = -std::numeric_limits<float>::max();
        float tmax = std::numeric_limits<float>::max();

        for (int i = 0; i < 3; ++i) {
            float invD = 1.0f / dir[i];
            float t0 = (aabb.lowerBound[i] - pos[i]) * invD;
            float t1 = (aabb.upperBound[i] - pos[i]) * invD;
            if (invD < 0.0f) {
                std::swap(t0, t1);
            }
            tmin = t0 > tmin ? t0 : tmin;
            tmax = t1 < tmax ? t1 : tmax;
            if (tmax <= tmin) {
                return RayTestResult(false, 0.0f);
            }
        }

        if (tmin < 0.0f) {
            // We're inside the box.
            return RayTestResult(false, 0.0f);
        } else {
            return RayTestResult(true, tmin);
        }
    }

    RayTestResult Ray::testSphere(const Sphere& s) const
    {
        auto rayOrig = pos - s.pos;

        if (rayOrig.length2() <= s.radius * s.radius) {
            return RayTestResult(false, 0.0f);
        }

        // Quadratics
        // Build coeffs which can be used with std quadratic solver
        // ie t = (-b +/- sqrt(b*b + 4ac)) / 2a
        float a = dir.dot(dir);
        float b = 2.0f * rayOrig.dot(dir);
        float c = rayOrig.dot(rayOrig) - s.radius * s.radius;

        // Calc determinant
        float d = (b * b) - (4 * a * c);
        if (d < 0) {
            // No intersection
            return RayTestResult(false, 0.0f);
        } else {
            // BTW, if d=0 there is one intersection, if d > 0 there are 2
            // But we only want the closest one, so that's ok, just use the
            // '-' version of the solver
            float t = (-b - btSqrt(d)) / (2 * a);
            if (t < 0) {
                t = (-b + btSqrt(d)) / (2 * a);
            }
            return RayTestResult(true, t);
        }
    }
}
