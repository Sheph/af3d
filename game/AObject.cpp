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

#include "AObject.h"

namespace af3d
{
    const APropertyTypeObject APropertyType_AObject{"AObject", AClass_AObject};
    const APropertyTypeArray APropertyType_ArrayAObject{"ArrayAObject", APropertyType_AObject};

    ACLASS_DEFINE_BEGIN_ABSTRACT(AObject, Null)
    ACLASS_PROPERTY(AObject, Name, AProperty_Name, "Object name", String, "", General)
    ACLASS_DEFINE_END(AObject)

    AObject::AObject(const AClass& klass)
    : klass_(&klass)
    {
    }

    const AClass& AObject::staticKlass()
    {
        return AClass_AObject;
    }

    APropertyValue AObject::propertyGet(const std::string& key) const
    {
        return klass_->propertyGet(this, key);
    }

    void AObject::propertySet(const std::string& key, const APropertyValue& value)
    {
        klass_->propertySet(this, key, value);
    }

    void AObject::propertiesSet(const APropertyValueMap& propVals)
    {
        for (const auto& kv : propVals) {
            propertySet(kv.first, kv.second);
        }
    }
}