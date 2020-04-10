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

#include "PhysicsBodyComponent.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(PhysicsBodyComponent, PhysicsComponent)
    ACLASS_DEFINE_END(PhysicsBodyComponent)

    PhysicsBodyComponent::PhysicsBodyComponent()
    : PhysicsComponent(AClass_PhysicsBodyComponent)
    {
    }

    const AClass& PhysicsBodyComponent::staticKlass()
    {
        return AClass_PhysicsBodyComponent;
    }

    AObjectPtr PhysicsBodyComponent::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<PhysicsBodyComponent>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void PhysicsBodyComponent::onFreeze()
    {
    }

    void PhysicsBodyComponent::onThaw()
    {
    }

    void PhysicsBodyComponent::onRegister()
    {
    }

    void PhysicsBodyComponent::onUnregister()
    {
    }
}
