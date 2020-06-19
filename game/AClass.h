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

#ifndef _ACLASS_H_
#define _ACLASS_H_

#include "AProperty.h"
#include <unordered_map>

namespace af3d
{
    class AClass : boost::noncopyable
    {
    public:
        using CreateFn = AObjectPtr (*)(const APropertyValueMap& propVals);

        struct PropertyFuncs
        {
            PropertyFuncs(APropertyGetter getter,
                APropertySetter setter, bool undoable)
            : getter(getter),
              setter(setter),
              undoableSetter(undoable ? (APropertyUndoableSetter)setter : nullptr)
            {
            }

            APropertyGetter getter;
            APropertySetter setter;
            APropertyUndoableSetter undoableSetter;
        };

        struct PropertyDef
        {
            PropertyDef(const std::string& name,
                const std::string& tooltip,
                const APropertyType& type,
                const APropertyValue& def,
                APropertyCategory category,
                std::uint32_t flags,
                APropertyGetter getter,
                APropertySetter setter)
            : prop(name, tooltip, type, def, category, flags),
              funcs(getter, setter, ((flags & APropertyUndoable) != 0))
            {
            }

            AProperty prop;
            PropertyFuncs funcs;
        };

        AClass();
        AClass(const std::string& name,
            const AClass& super,
            CreateFn createFn,
            const std::vector<PropertyDef>& propertyDefs);
        ~AClass();

        inline const std::string& name() const { return name_; }

        const AClass* super() const;

        bool isSubClassOf(const AClass& value) const;

        inline const APropertyList& thisProperties() const { return properties_; }

        APropertyList getProperties() const;

        bool propertyCanGet(const std::string& key) const;
        bool propertyCanSet(const std::string& key) const;
        const AProperty* propertyFind(const std::string& key) const;

        APropertyValue propertyGet(const AObject* obj, const std::string& key) const;
        ACommandPtr propertySet(AObject* obj, const std::string& key, const APropertyValue& value) const;

        AObjectPtr create(const APropertyValueMap& propVals = APropertyValueMap()) const;

    private:
        std::string name_;
        const AClass& super_;
        CreateFn createFn_ = nullptr;
        APropertyList properties_;
        std::unordered_map<std::string, PropertyFuncs> funcs_;
    };

    #define ACLASS_DECLARE(Name) extern const AClass AClass_##Name;

    #define ACLASS_NS_DECLARE(NS, Name) extern const AClass AClass_##NS##Name;

    #define ACLASS_DEFINE_BEGIN_ABSTRACT(Name, Super) \
        const AClass AClass_##Name{#Name, AClass_##Super, nullptr, {

    #define ACLASS_NS_DEFINE_BEGIN_ABSTRACT(NS, Name, Super) \
        const AClass AClass_##NS##Name{#NS "::" #Name, AClass_##Super, nullptr, {

    #define ACLASS_DEFINE_BEGIN(Name, Super) \
        const AClass AClass_##Name{#Name, AClass_##Super, &Name::create, {

    #define ACLASS_NS_DEFINE_BEGIN(NS, Name, Super) \
        const AClass AClass_##NS##Name{#NS "::" #Name, AClass_##Super, &NS::Name::create, {

    #define ACLASS_PROPERTY(ClassName, Member, Name, Tooltip, Type, Def, Category, Flags) \
        {Name, Tooltip, APropertyType_##Type, APropertyValue(Def), APropertyCategory::Category, APropertyReadable|APropertyWritable|Flags, (APropertyGetter)&ClassName::property##Member##Get, (APropertySetter)&ClassName::property##Member##Set},

    #define ACLASS_PROPERTY_UNDOABLE(ClassName, Member, Name, Tooltip, Type, Def, Category, Flags) \
        {Name, Tooltip, APropertyType_##Type, APropertyValue(Def), APropertyCategory::Category, APropertyReadable|APropertyWritable|APropertyUndoable|Flags, (APropertyGetter)&ClassName::property##Member##Get, (APropertySetter)(APropertyUndoableSetter)&ClassName::property##Member##Set},

    #define ACLASS_PROPERTY_RO(ClassName, Member, Name, Tooltip, Type, Category, Flags) \
        {Name, Tooltip, APropertyType_##Type, APropertyValue(), APropertyCategory::Category, APropertyReadable|Flags, (APropertyGetter)&ClassName::property##Member##Get, nullptr},

    #define ACLASS_DEFINE_END(Name) }};

    #define ACLASS_NS_DEFINE_END(NS, Name) }};

    ACLASS_DECLARE(Null)
}

#endif
