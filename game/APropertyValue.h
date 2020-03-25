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

#ifndef _APROPERTY_VALUE_H_
#define _APROPERTY_VALUE_H_

#include "af3d/Types.h"
#include "af3d/Vector2.h"
#include "af3d/Vector3.h"
#include "af3d/Vector4.h"
#include <boost/operators.hpp>
#include <memory>
#include <unordered_map>

namespace af3d
{
    class AObject;
    using AObjectPtr = std::shared_ptr<AObject>;

    class APropertyValue : public boost::totally_ordered<APropertyValue>
    {
    public:
        enum Type
        {
            None = 0,
            Int,
            Float,
            String,
            Vec2f,
            Vec3f,
            Vec4f,
            Object,
        };

        APropertyValue();
        APropertyValue(bool val);
        APropertyValue(int val);
        APropertyValue(float val);
        APropertyValue(const std::string& val);
        APropertyValue(const Vector2f& val);
        APropertyValue(const Vector3f& val);
        APropertyValue(const Vector4f& val);
        APropertyValue(const AObjectPtr& val);

        ~APropertyValue() = default;

        inline Type type() const { return type_; }

        inline bool empty() const { return type_ == None; }

        bool toBool() const;
        int toInt() const;
        float toFloat() const;
        std::string toString() const;
        Vector2f toVec2f() const;
        Vector3f toVec3f() const;
        Vector4f toVec4f() const;
        AObjectPtr toObject() const;

        template <class T>
        std::shared_ptr<T> toObject() const
        {
            switch (type_) {
            case Object:
                return std::dynamic_pointer_cast<T>(obj_);
            default:
                return std::shared_ptr<T>();
            }
        }

        bool operator<(const APropertyValue& rhs) const;
        bool operator==(const APropertyValue& rhs) const;

    private:
        union Pod
        {
            Pod() = default;
            explicit Pod(int value) : intVal(value) {}
            explicit Pod(float value) : floatVal(value) {}
            explicit Pod(const Vector2f& value) : vec2fVal(value) {}
            explicit Pod(const Vector3f& value) : vec3fVal(value) {}
            explicit Pod(const Vector4f& value) : vec4fVal(value) {}

            int intVal;
            float floatVal;
            Vector2f vec2fVal;
            Vector3f vec3fVal;
            Vector4f vec4fVal;
        };

        Type type_;
        Pod pod_;
        std::string str_;
        AObjectPtr obj_;
    };

    using APropertyValueMap = std::unordered_map<std::string, APropertyValue>;
}

#endif
