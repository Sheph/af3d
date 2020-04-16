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

#include "CollisionShapeSphere.h"
#include "PhysicsDebugDraw.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(CollisionShapeSphere, CollisionShape)
    COLLISIONSHAPE_PARAM(CollisionShapeSphere, "radius", "Sphere radius", Float, 0.5f)
    ACLASS_DEFINE_END(CollisionShapeSphere)

    CollisionShapeSphere::CollisionShapeSphere(float radius)
    : CollisionShape(AClass_CollisionShapeSphere),
      shape_(radius)
    {
    }

    const AClass& CollisionShapeSphere::staticKlass()
    {
        return AClass_CollisionShapeSphere;
    }

    AObjectPtr CollisionShapeSphere::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<CollisionShapeSphere>(propVals.get("radius").toFloat());
        obj->afterCreate(propVals);
        return obj;
    }

    void CollisionShapeSphere::render(PhysicsDebugDraw& dd, const btVector3& c)
    {
        float radius = shape_.getMargin();
        dd.drawSphere(radius, worldTransform(), c);
    }
}
