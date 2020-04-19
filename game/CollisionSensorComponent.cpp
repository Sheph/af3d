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

#include "CollisionSensorComponent.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(CollisionSensorComponent, CollisionComponent)
    ACLASS_DEFINE_END(CollisionSensorComponent)

    CollisionSensorComponent::CollisionSensorComponent()
    : CollisionComponent(AClass_CollisionSensorComponent),
      allowSensor_(false)
    {
    }

    const AClass& CollisionSensorComponent::staticKlass()
    {
        return AClass_CollisionSensorComponent;
    }

    AObjectPtr CollisionSensorComponent::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<CollisionSensorComponent>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void CollisionSensorComponent::updateContact(const Contact& contact, bool isNew)
    {
        auto it = contacts_.find(contact.cookie);
        if (it != contacts_.end()) {
            return;
        }

        SceneObjectPtr obj = contact.getOther(parent())->shared_from_this();

        contacts_[contact.cookie] = obj;

        auto cnt = ++counts_[obj];

        if ((cnt == 1) && listener_) {
            listener_->sensorEnter(obj);
        }
    }

    void CollisionSensorComponent::endContact(const Contact& contact)
    {
        auto it = contacts_.find(contact.cookie);
        if (it == contacts_.end()) {
            return;
        }

        auto cnt = --counts_[it->second];
        btAssert(cnt >= 0);

        if (cnt == 0) {
            if (listener_) {
                listener_->sensorExit(it->second);
            }
            counts_.erase(it->second);
        }

        contacts_.erase(it);
    }

    void CollisionSensorComponent::onRegister()
    {
    }

    void CollisionSensorComponent::onUnregister()
    {
        counts_.clear();
        contacts_.clear();
        setListener(SensorListenerPtr());
    }
}
