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

#include "AClass.h"
#include "AClassRegistry.h"

namespace af3d
{
    const AClass AClass_Null;

    AClass::AClass()
    : super_(*this)
    {
    }

    AClass::AClass(const std::string& name,
        const AClass& super,
        CreateFn createFn,
        const std::vector<PropertyDef>& propertyDefs)
    : name_(name),
      super_(super),
      createFn_(createFn)
    {
        properties_.reserve(propertyDefs.size());
        for (const auto& def : propertyDefs) {
            properties_.emplace_back(def.prop);
            runtime_assert(funcs_.emplace(def.prop.name(), def.funcs).second);
        }
        AClassRegistry::instance().classRegister(*this);
    }

    AClass::~AClass()
    {
        // Nothing we can do here actually...
    }

    const AClass* AClass::super() const
    {
        return (&super_ == &AClass_Null) ? nullptr : &super_;
    }

    APropertyList AClass::getProperties() const
    {
        if (&super_ == this) {
            return properties_;
        } else {
            auto superProps = super_.getProperties();
            superProps.insert(superProps.end(), properties_.begin(), properties_.end());
            return superProps;
        }
    }

    APropertyValue AClass::propertyGet(const AObject* obj, const std::string& key) const
    {
        if (&super_ == this) {
            return APropertyValue();
        }

        auto it = funcs_.find(key);
        if (it == funcs_.end()) {
            return super_.propertyGet(obj, key);
        }

        if (!it->second.getter) {
            return APropertyValue();
        }

        return (obj->*(it->second.getter))(key);
    }

    void AClass::propertySet(AObject* obj, const std::string& key, const APropertyValue& value) const
    {
        if (&super_ == this) {
            return;
        }

        auto it = funcs_.find(key);
        if (it == funcs_.end()) {
            super_.propertySet(obj, key, value);
            return;
        }

        if (!it->second.setter) {
            return;
        }

        (obj->*(it->second.setter))(key, value);
    }

    AObjectPtr AClass::create(const APropertyValueMap& propVals) const
    {
        return createFn_ ? createFn_(propVals) : AObjectPtr();
    }
}
