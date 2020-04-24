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
#include "AObject.h"
#include <sstream>
#include <algorithm>

namespace af3d
{
    APropertyValue::APropertyValue()
    : type_(None)
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

    APropertyValue::APropertyValue(const char* val)
    : type_(String),
      str_(val)
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

    APropertyValue::APropertyValue(const btVector3& val)
    : type_(Vec3f),
      pod_(toVector3f(val))
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

    APropertyValue::APropertyValue(const AWeakObject& val)
    : type_(WeakObject),
      wobj_(val)
    {
    }

    APropertyValue::APropertyValue(const btTransform& val)
    : type_(Transform),
      xf_(val)
    {
    }

    APropertyValue::APropertyValue(const std::vector<APropertyValue>& val)
    : type_(Array),
      arr_(val)
    {
    }

    bool APropertyValue::convertibleTo(Type other) const
    {
        switch (type_) {
        case None:
            return false;
        case Int:
            return (other == Int) || (other == Float) || (other == String);
        case Float:
            return (other == Int) || (other == Float) || (other == String);
        case String:
            return (other == String);
        case Vec2f:
        case Vec3f:
        case Vec4f:
            return (other == Vec2f) || (other == Vec3f) || (other == Vec4f) || (other == String);
        case Object:
        case WeakObject:
            return (other == Object) || (other == WeakObject) || (other == String);
        case Transform:
            return (other == Transform) || (other == String);
        case Array:
            return (other == Array) || (other == String);
        default:
            btAssert(false);
            return false;
        }
    }

    APropertyValue APropertyValue::convertTo(Type other) const
    {
        switch (other) {
        case None:
            return APropertyValue();
        case Int:
            return toInt();
        case Float:
            return toFloat();
        case String:
            return toString();
        case Vec2f:
            return toVec2f();
        case Vec3f:
            return toVec3f();
        case Vec4f:
            return toVec4f();
        case Object:
            return toObject();
        case WeakObject:
            return toWeakObject();
        case Transform:
            return toTransform();
        case Array:
            return toArray();
        default:
            btAssert(false);
            return APropertyValue();
        }
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
        case None:
            return "";
        case Int:
            return std::to_string(pod_.intVal);
        case Float:
            return std::to_string(pod_.floatVal);
        case String:
            return str_;
        case Vec2f: {
            std::ostringstream os;
            os << pod_.vec2fVal;
            return os.str();
        }
        case Vec3f: {
            std::ostringstream os;
            os << pod_.vec3fVal;
            return os.str();
        }
        case Vec4f: {
            std::ostringstream os;
            os << pod_.vec4fVal;
            return os.str();
        }
        case Object: {
            std::ostringstream os;
            os << "<" << obj_.get() << ">";
            return os.str();
        }
        case WeakObject: {
            std::ostringstream os;
            os << "<<" << wobj_.cookie() << ">>";
            return os.str();
        }
        case Transform: {
            std::ostringstream os;
            os << xf_;
            return os.str();
        }
        case Array: {
            std::ostringstream os;
            os << "(";
            for (size_t i = 0; i < arr_.size(); ++i) {
                if (i != 0) {
                    os << ",";
                }
                os << arr_[i].toString();
            }
            os << ")";
            return os.str();
        }
        default:
            btAssert(false);
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

    btVector3 APropertyValue::toVec3() const
    {
        switch (type_) {
        case Vec2f:
            return btVector3(pod_.vec2fVal.x(), pod_.vec2fVal.y(), 0.0f);
        case Vec3f:
            return fromVector3f(pod_.vec3fVal);
        case Vec4f:
            return btVector3(pod_.vec4fVal.x(), pod_.vec4fVal.y(), pod_.vec4fVal.z());
        default:
            return btVector3_zero;
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
        case WeakObject:
            return wobj_.lock();
        default:
            return AObjectPtr();
        }
    }

    AWeakObject APropertyValue::toWeakObject() const
    {
        switch (type_) {
        case Object:
            return AWeakObject(obj_);
        case WeakObject:
            return wobj_;
        default:
            return AWeakObject();
        }
    }

    btTransform APropertyValue::toTransform() const
    {
        switch (type_) {
        case Transform:
            return xf_;
        default:
            return btTransform::getIdentity();
        }
    }

    std::vector<APropertyValue> APropertyValue::toArray() const
    {
        switch (type_) {
        case Array:
            return arr_;
        default:
            return std::vector<APropertyValue>();
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
        case WeakObject:
            return wobj_.cookie() < rhs.wobj_.cookie();
        case Transform:
            btAssert(false);
            return false;
        case Array:
            if (arr_.size() != rhs.arr_.size()) {
                return arr_.size() < rhs.arr_.size();
            }
            for (size_t i = 0; i < arr_.size(); ++i) {
                if (arr_[i] != rhs.arr_[i]) {
                    return arr_[i] < rhs.arr_[i];
                }
            }
            return false;
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
        case WeakObject:
            return wobj_ == rhs.wobj_;
        case Transform:
            return xf_ == rhs.xf_;
        case Array:
            if (arr_.size() != rhs.arr_.size()) {
                return false;
            }
            for (size_t i = 0; i < arr_.size(); ++i) {
                if (arr_[i] != rhs.arr_[i]) {
                    return false;
                }
            }
            return true;
        default:
            btAssert(false);
            return false;
        }
    }

    void APropertyValue::refreshObjects()
    {
        switch (type_) {
        case Object:
            // If object was re-created, then re-assign it.
            obj_ = AWeakObject(obj_).lock();
            break;
        case Array:
            for (auto& e : arr_) {
                e.refreshObjects();
            }
            break;
        default:
            break;
        }
    }
}
