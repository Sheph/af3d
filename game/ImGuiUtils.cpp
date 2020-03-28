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

#include "ImGuiUtils.h"

namespace af3d { namespace ImGuiUtils
{
    APropertyEdit::APropertyEdit(const APropertyType& type)
    : type_(&type)
    {
    }

    bool APropertyEdit::update(APropertyValue& val, bool readOnly)
    {
        if (readOnly) {
            ImGui::Text("(ro) %s", val.toString().c_str());
        } else {
            ImGui::Text("%s", val.toString().c_str());
        }
        return false;
    }

    void APropertyEdit::visitBool(const APropertyTypeBool& type)
    {
    }

    void APropertyEdit::visitInt(const APropertyTypeInt& type)
    {
    }

    void APropertyEdit::visitFloat(const APropertyTypeFloat& type)
    {
    }

    void APropertyEdit::visitString(const APropertyTypeString& type)
    {
    }

    void APropertyEdit::visitVec2f(const APropertyTypeVec2f& type)
    {
    }

    void APropertyEdit::visitVec3f(const APropertyTypeVec3f& type)
    {
    }

    void APropertyEdit::visitVec4f(const APropertyTypeVec4f& type)
    {
    }

    void APropertyEdit::visitColor(const APropertyTypeColor& type)
    {
    }

    void APropertyEdit::visitEnum(const APropertyTypeEnum& type)
    {
    }

    void APropertyEdit::visitObject(const APropertyTypeObject& type)
    {
    }

    void APropertyEdit::visitTransform(const APropertyTypeTransform& type)
    {
    }

    void APropertyEdit::visitArray(const APropertyTypeArray& type)
    {
    }
} }
