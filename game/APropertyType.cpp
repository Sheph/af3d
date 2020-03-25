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

#include "APropertyType.h"
#include "AClass.h"

namespace af3d
{
    APropertyTypeNull::APropertyTypeNull()
    : APropertyType("Null")
    {
    }

    void APropertyTypeNull::accept(APropertyTypeVisitor& visitor) const
    {
        visitor.visitNull(*this);
    }

    APropertyTypeBool::APropertyTypeBool()
    : APropertyType("Bool")
    {
    }

    void APropertyTypeBool::accept(APropertyTypeVisitor& visitor) const
    {
        visitor.visitBool(*this);
    }

    APropertyTypeInt::APropertyTypeInt(int vMin, int vMax)
    : APropertyTypeNumeric<int>("Int", vMin, vMax)
    {
    }

    void APropertyTypeInt::accept(APropertyTypeVisitor& visitor) const
    {
        visitor.visitInt(*this);
    }

    APropertyTypeFloat::APropertyTypeFloat(float vMin, float vMax)
    : APropertyTypeNumeric<float>("Float", vMin, vMax)
    {
    }

    void APropertyTypeFloat::accept(APropertyTypeVisitor& visitor) const
    {
        visitor.visitFloat(*this);
    }

    APropertyTypeString::APropertyTypeString()
    : APropertyType("String")
    {
    }

    void APropertyTypeString::accept(APropertyTypeVisitor& visitor) const
    {
        visitor.visitString(*this);
    }

    APropertyTypeVec2f::APropertyTypeVec2f()
    : APropertyType("Vec2f")
    {
    }

    void APropertyTypeVec2f::accept(APropertyTypeVisitor& visitor) const
    {
        visitor.visitVec2f(*this);
    }

    APropertyTypeVec3f::APropertyTypeVec3f()
    : APropertyType("Vec3f")
    {
    }

    void APropertyTypeVec3f::accept(APropertyTypeVisitor& visitor) const
    {
        visitor.visitVec3f(*this);
    }

    APropertyTypeVec4f::APropertyTypeVec4f()
    : APropertyType("Vec4f")
    {
    }

    void APropertyTypeVec4f::accept(APropertyTypeVisitor& visitor) const
    {
        visitor.visitVec4f(*this);
    }

    APropertyTypeColor::APropertyTypeColor(bool hasAlpha)
    : APropertyType("Color"),
      hasAlpha_(hasAlpha)
    {
    }

    void APropertyTypeColor::accept(APropertyTypeVisitor& visitor) const
    {
        visitor.visitColor(*this);
    }

    APropertyTypeEnum::APropertyTypeEnum(const char* name, const Enumerators& enumerators)
    : APropertyType(name),
      enumerators_(enumerators)
    {
    }

    void APropertyTypeEnum::accept(APropertyTypeVisitor& visitor) const
    {
        visitor.visitEnum(*this);
    }

    APropertyTypeObject::APropertyTypeObject(const char* name, const AClass& klass)
    : APropertyType(name),
      klass_(klass)
    {
    }

    void APropertyTypeObject::accept(APropertyTypeVisitor& visitor) const
    {
        visitor.visitObject(*this);
    }

    APropertyTypeTransform::APropertyTypeTransform()
    : APropertyType("Transform")
    {
    }

    void APropertyTypeTransform::accept(APropertyTypeVisitor& visitor) const
    {
        visitor.visitTransform(*this);
    }
}
