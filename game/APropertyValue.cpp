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

#include "APropertyValue.h"

namespace af3d
{
    APropertyValue::APropertyValue()
    : type_(None)
    {
    }

    APropertyValue::APropertyValue(bool val)
    : type_(Int),
      pod_(val)
    {
    }

    APropertyValue::APropertyValue(int val)
    : type_(Int),
      pod_(val)
    {
    }

    APropertyValue::APropertyValue(float val)
    : type_(Float),
      pod_(val)
    {
    }

    APropertyValue::APropertyValue(const std::string& val)
    : type_(String),
      str_(val)
    {
    }

    APropertyValue::APropertyValue(const Vector2f& val)
    : type_(Vec2f),
      pod_(val)
    {
    }

    APropertyValue::APropertyValue(const Vector3f& val)
    : type_(Vec3f),
      pod_(val)
    {
    }

    APropertyValue::APropertyValue(const Vector4f& val)
    : type_(Vec4f),
      pod_(val)
    {
    }

    APropertyValue::APropertyValue(const AObjectPtr& val)
    : type_(Object),
      obj_(val)
    {
    }

    bool APropertyValue::toBool() const
    {
        switch (type_) {
        case Int:
            return static_cast<bool>(pod_.intVal);
        case Float:
            return static_cast<bool>(pod_.floatVal);
        default:
            return false;
        }
    }

    int APropertyValue::toInt() const
    {
        switch (type_) {
        case Int:
            return pod_.intVal;
        case Float:
            return pod_.floatVal;
        default:
            return 0;
        }
    }

    float APropertyValue::toFloat() const
    {
        switch (type_) {
        case Int:
            return pod_.intVal;
        case Float:
            return pod_.floatVal;
        default:
            return 0.0f;
        }
    }

    std::string APropertyValue::toString() const
    {
        switch (type_) {
        case Int:
            return std::to_string(pod_.intVal);
        case Float:
            return std::to_string(pod_.floatVal);
        case String:
            return str_;
        default:
            return "";
        }
    }

    Vector2f APropertyValue::toVec2f() const
    {
        switch (type_) {
        case Vec2f:
            return pod_.vec2fVal;
        case Vec3f:
            return Vector2f(pod_.vec3fVal.x(), pod_.vec3fVal.y());
        case Vec4f:
            return Vector2f(pod_.vec4fVal.x(), pod_.vec4fVal.y());
        default:
            return Vector2f_zero;
        }
    }

    Vector3f APropertyValue::toVec3f() const
    {
        switch (type_) {
        case Vec2f:
            return Vector3f(pod_.vec2fVal.x(), pod_.vec2fVal.y(), 0.0f);
        case Vec3f:
            return pod_.vec3fVal;
        case Vec4f:
            return Vector3f(pod_.vec4fVal.x(), pod_.vec4fVal.y(), pod_.vec4fVal.z());
        default:
            return Vector3f_zero;
        }
    }

    Vector4f APropertyValue::toVec4f() const
    {
        switch (type_) {
        case Vec2f:
            return Vector4f(pod_.vec2fVal.x(), pod_.vec2fVal.y(), 0.0f, 0.0f);
        case Vec3f:
            return Vector4f(pod_.vec3fVal.x(), pod_.vec3fVal.y(), pod_.vec3fVal.z(), 0.0f);
        case Vec4f:
            return pod_.vec4fVal;
        default:
            return Vector4f_zero;
        }
    }

    AObjectPtr APropertyValue::toObject() const
    {
        switch (type_) {
        case Object:
            return obj_;
        default:
            return AObjectPtr();
        }
    }

    bool APropertyValue::operator<(const APropertyValue& rhs) const
    {
        if (type_ != rhs.type_) {
            return type_ < rhs.type_;
        }
        switch (type_) {
        case None:
            return false;
        case Int:
            return pod_.intVal < rhs.pod_.intVal;
        case Float:
            return pod_.floatVal < rhs.pod_.floatVal;
        case String:
            return str_ < rhs.str_;
        case Vec2f:
            return pod_.vec2fVal < rhs.pod_.vec2fVal;
        case Vec3f:
            return pod_.vec3fVal < rhs.pod_.vec3fVal;
        case Vec4f:
            return pod_.vec4fVal < rhs.pod_.vec4fVal;
        case Object:
            return obj_ < rhs.obj_;
        default:
            btAssert(false);
            return false;
        }
    }

    bool APropertyValue::operator==(const APropertyValue& rhs) const
    {
        if (type_ != rhs.type_) {
            return false;
        }
        switch (type_) {
        case None:
            return true;
        case Int:
            return pod_.intVal == rhs.pod_.intVal;
        case Float:
            return pod_.floatVal == rhs.pod_.floatVal;
        case String:
            return str_ == rhs.str_;
        case Vec2f:
            return pod_.vec2fVal == rhs.pod_.vec2fVal;
        case Vec3f:
            return pod_.vec3fVal == rhs.pod_.vec3fVal;
        case Vec4f:
            return pod_.vec4fVal == rhs.pod_.vec4fVal;
        case Object:
            return obj_ == rhs.obj_;
        default:
            btAssert(false);
            return false;
        }
    }
}
