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
    struct InputTextCallback_UserData
    {
        std::string* Str;
        ImGuiInputTextCallback ChainCallback;
        void* ChainCallbackUserData;
    };

    class PropertyEditVisitor : public APropertyTypeVisitor
    {
    public:
        PropertyEditVisitor(APropertyValue& value, bool readOnly)
        : value_(value),
          readOnly_(readOnly)
        {
        }

        ~PropertyEditVisitor() = default;

        void visitBool(const APropertyTypeBool& type) override
        {
        }

        void visitInt(const APropertyTypeInt& type) override
        {
        }

        void visitFloat(const APropertyTypeFloat& type) override
        {
        }

        void visitString(const APropertyTypeString& type) override
        {
            ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;

            if (readOnly_) {
                flags |= ImGuiInputTextFlags_ReadOnly;
            }

            std::string val = value_.toString();

            if (inputText("##val", val, flags) && !readOnly_) {
                ret_ = true;
                value_ = APropertyValue(val);
            }
        }

        void visitVec2f(const APropertyTypeVec2f& type) override
        {
        }

        void visitVec3f(const APropertyTypeVec3f& type) override
        {
            ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;

            if (readOnly_) {
                flags |= ImGuiInputTextFlags_ReadOnly;
            }

            Vector3f v = value_.toVec3f();

            if (ImGui::InputFloat3("##val", &v.v[0], "%.3f", flags) && !readOnly_) {
                ret_ = true;
                value_ = APropertyValue(v);
            }
        }

        void visitVec4f(const APropertyTypeVec4f& type) override
        {
        }

        void visitColor(const APropertyTypeColor& type) override
        {
            ImGuiColorEditFlags flags = 0;

            Color c = value_.toColor();

            bool ch;
            if (type.hasAlpha()) {
                ch = ImGui::ColorEdit4("##val", &c.v[0], flags);
            } else {
                ch = ImGui::ColorEdit3("##val", &c.v[0], flags);
            }

            if (ch && !readOnly_) {
                ret_ = true;
                value_ = APropertyValue(c);
            }
        }

        void visitEnum(const APropertyTypeEnum& type) override
        {
            std::vector<const char*> items;
            items.reserve(type.enumerators().size());

            for (const auto& str : type.enumerators()) {
                items.push_back(str.data());
            }

            int cur = value_.toInt();

            if (ImGui::Combo("##val", &cur, &items[0], items.size()) && !readOnly_) {
                ret_ = true;
                value_ = APropertyValue(cur);
            }
        }

        void visitObject(const APropertyTypeObject& type) override
        {
        }

        void visitTransform(const APropertyTypeTransform& type) override
        {
            ImGuiInputTextFlags flags = ImGuiInputTextFlags_EnterReturnsTrue;

            if (readOnly_) {
                flags |= ImGuiInputTextFlags_ReadOnly;
            }

            btTransform xf = value_.toTransform();

            btVector3 origin = xf.getOrigin();
            btVector3 euler;
            xf.getBasis().getEulerZYX(euler[2], euler[1], euler[0]);

            euler[0] = btDegrees(euler[0]);
            euler[1] = btDegrees(euler[1]);
            euler[2] = btDegrees(euler[2]);

            bool ch1 = ImGui::InputFloat3("origin", &origin.m_floats[0], "%.3f", flags);
            bool ch2 = ImGui::InputFloat3("euler", &euler.m_floats[0], "%.3f°", flags);
            if ((ch1 || ch2) && !readOnly_) {
                ret_ = true;
                xf.setOrigin(origin);
                euler[0] = btRadians(euler[0]);
                euler[1] = btRadians(euler[1]);
                euler[2] = btRadians(euler[2]);
                xf.getBasis().setEulerZYX(euler[0], euler[1], euler[2]);
                value_ = APropertyValue(xf);
            }
        }

        void visitArray(const APropertyTypeArray& type) override
        {
        }

        inline bool ret() const { return ret_; }

    private:
        APropertyValue& value_;
        bool readOnly_ = false;
        bool ret_ = false;
    };

    bool APropertyEdit(const APropertyType& type, APropertyValue& val, bool readOnly)
    {
        PropertyEditVisitor visitor(val, readOnly);
        type.accept(visitor);
        return visitor.ret();
    }

    static int inputTextCallback(ImGuiInputTextCallbackData* data)
    {
        InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)data->UserData;
        if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
            // Resize string callback
            // If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
            std::string* str = user_data->Str;
            IM_ASSERT(data->Buf == str->c_str());
            str->resize(data->BufTextLen);
            data->Buf = (char*)str->c_str();
        } else if (user_data->ChainCallback) {
            // Forward to user callback, if any
            data->UserData = user_data->ChainCallbackUserData;
            return user_data->ChainCallback(data);
        }
        return 0;
    }

    bool inputText(const char* label, std::string& str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
    {
        btAssert((flags & ImGuiInputTextFlags_CallbackResize) == 0);
        flags |= ImGuiInputTextFlags_CallbackResize;

        InputTextCallback_UserData cb_user_data;
        cb_user_data.Str = &str;
        cb_user_data.ChainCallback = callback;
        cb_user_data.ChainCallbackUserData = user_data;
        return ImGui::InputText(label, (char*)str.c_str(), str.capacity() + 1, flags, inputTextCallback, &cb_user_data);
    }

    bool inputTextMultiline(const char* label, std::string& str, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
    {
        btAssert((flags & ImGuiInputTextFlags_CallbackResize) == 0);
        flags |= ImGuiInputTextFlags_CallbackResize;

        InputTextCallback_UserData cb_user_data;
        cb_user_data.Str = &str;
        cb_user_data.ChainCallback = callback;
        cb_user_data.ChainCallbackUserData = user_data;
        return ImGui::InputTextMultiline(label, (char*)str.c_str(), str.capacity() + 1, size, flags, inputTextCallback, &cb_user_data);
    }

    bool inputTextWithHint(const char* label, const char* hint, std::string& str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
    {
        btAssert((flags & ImGuiInputTextFlags_CallbackResize) == 0);
        flags |= ImGuiInputTextFlags_CallbackResize;

        InputTextCallback_UserData cb_user_data;
        cb_user_data.Str = &str;
        cb_user_data.ChainCallback = callback;
        cb_user_data.ChainCallbackUserData = user_data;
        return ImGui::InputTextWithHint(label, hint, (char*)str.c_str(), str.capacity() + 1, flags, inputTextCallback, &cb_user_data);
    }
} }
