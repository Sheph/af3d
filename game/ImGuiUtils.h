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

#ifndef _IMGUIUTILS_H_
#define _IMGUIUTILS_H_

#include "Utils.h"
#include "AProperty.h"
#include "imgui.h"

namespace af3d { namespace ImGuiUtils
{
    class APropertyEdit : public APropertyTypeVisitor
    {
    public:
        APropertyEdit() = default;
        explicit APropertyEdit(const APropertyType& type);
        ~APropertyEdit() = default;

        bool update(APropertyValue& val, bool readOnly);

        void visitBool(const APropertyTypeBool& type) override;
        void visitInt(const APropertyTypeInt& type) override;
        void visitFloat(const APropertyTypeFloat& type) override;
        void visitString(const APropertyTypeString& type) override;
        void visitVec2f(const APropertyTypeVec2f& type) override;
        void visitVec3f(const APropertyTypeVec3f& type) override;
        void visitVec4f(const APropertyTypeVec4f& type) override;
        void visitColor(const APropertyTypeColor& type) override;
        void visitEnum(const APropertyTypeEnum& type) override;
        void visitObject(const APropertyTypeObject& type) override;
        void visitTransform(const APropertyTypeTransform& type) override;
        void visitArray(const APropertyTypeArray& type) override;

    private:
        struct Transform
        {
            Vector3f pos;
            Vector3f euler;
        };

        union Pod
        {
            bool boolVal;
            int intVal;
            float floatVal;
            Vector2f vec2fVal;
            Vector3f vec3fVal;
            Vector4f vec4fVal;
            Transform xfVal;
        };

        const APropertyType* type_ = nullptr;
        APropertyValue* curVal_ = nullptr;
        bool curReadOnly_ = false;
        bool curRet_ = false;

        Pod podVal_;
        std::string strVal_;
        std::vector<APropertyEdit> arrayVal_;
    };

    bool inputText(const char* label, std::string& str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    bool inputTextMultiline(const char* label, std::string& str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
    bool inputTextWithHint(const char* label, const char* hint, std::string& str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = NULL, void* user_data = NULL);
} }

#endif
