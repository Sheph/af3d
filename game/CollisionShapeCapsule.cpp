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

#include "CollisionShapeCapsule.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(CollisionShapeCapsule, CollisionShape)
    COLLISIONSHAPE_PARAM(CollisionShapeCapsule, "radius", "Capsule radius", Float, 1.0f)
    COLLISIONSHAPE_PARAM(CollisionShapeCapsule, "height", "Capsule height", Float, 1.0f)
    ACLASS_DEFINE_END(CollisionShapeCapsule)

    CollisionShapeCapsule::CollisionShapeCapsule(float radius, float height)
    : CollisionShape(AClass_CollisionShapeCapsule),
      shape_(radius, height)
    {
    }

    const AClass& CollisionShapeCapsule::staticKlass()
    {
        return AClass_CollisionShapeCapsule;
    }

    AObjectPtr CollisionShapeCapsule::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<CollisionShapeCapsule>(propVals.get("radius").toFloat(),
            propVals.get("height").toFloat());
        obj->afterCreate(propVals);
        return obj;
    }

    void CollisionShapeCapsule::render(PhysicsDebugDraw& dd, const btVector3& c)
    {
    }
}
