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

#ifndef _APROPERTY_H_
#define _APROPERTY_H_

#include "APropertyTypes.h"
#include "APropertyValueMap.h"

namespace af3d
{
    enum APropertyFlags
    {
        APropertyReadable = 1 << 0,
        APropertyWritable = 1 << 1,
        APropertyEditable = 1 << 2,
        APropertyTransient = 1 << 3
    };

    enum class APropertyCategory
    {
        General = 0,
        Hierarchy,
        Position,
        Params,
        Max = Params
    };

    class AProperty
    {
    public:
        AProperty() = default;
        AProperty(const std::string& name,
            const std::string& tooltip,
            const APropertyType& type,
            const APropertyValue& def,
            APropertyCategory category,
            std::uint32_t flags)
        : name_(name),
          tooltip_(tooltip),
          type_(&type),
          def_(def),
          category_(category),
          flags_(flags)
        {
        }

        inline const std::string& name() const { return name_; }
        inline const std::string& tooltip() const { return tooltip_; }
        inline const APropertyType& type() const { return *type_; }
        inline const APropertyValue& def() const { return def_; }
        inline APropertyCategory category() const { return category_; }
        inline std::uint32_t flags() const { return flags_; }

    private:
        std::string name_;
        std::string tooltip_;
        const APropertyType* type_;
        APropertyValue def_;
        APropertyCategory category_;
        std::uint32_t flags_;
    };

    using APropertySetter = void (AObject::*)(const std::string&, const APropertyValue&);
    using APropertyGetter = APropertyValue (AObject::*)(const std::string&) const;

    using APropertyList = std::vector<AProperty>;

    #define AProperty_Children "children"
    #define AProperty_Parent "parent"
    #define AProperty_Name "name"
    #define AProperty_Visible "visible"
    #define AProperty_LocalTransform "local transform"
    #define AProperty_WorldTransform "world transform"
    #define AProperty_Scale "scale"
    #define AProperty_Position "position"
    #define AProperty_Angle "angle"
    #define AProperty_Size "size"
    #define AProperty_Width "width"
    #define AProperty_Height "height"
}

inline std::ostream& operator <<(std::ostream& os, af3d::APropertyCategory cat)
{
    return (os << static_cast<int>(cat));
}

#endif
