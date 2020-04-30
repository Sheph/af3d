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

#include "ImGuiManager.h"
#include "TextureManager.h"
#include "VertexArrayWriter.h"
#include "Settings.h"
#include "Logger.h"
#include <boost/tokenizer.hpp>
#include "imgui_internal.h"

namespace af3d
{
    static_assert(sizeof(VertexImm) == sizeof(ImDrawVert), "Bad ImDrawVert size");

    ImGuiManager imGuiManager;

    template <>
    Single<ImGuiManager>* Single<ImGuiManager>::single = nullptr;

    ImGuiManager::~ImGuiManager()
    {
        runtime_assert(!fontsTex_);
        runtime_assert(textureCache_.empty());
    }

    bool ImGuiManager::init()
    {
        LOG4CPLUS_DEBUG(logger(), "imGuiManager: init...");

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        ImGuiContext* ctx = ImGui::GetCurrentContext();

        ImGuiSettingsHandler cfgHandler;
        cfgHandler.TypeName = "Cfg";
        cfgHandler.TypeHash = ImHashStr("Cfg");
        cfgHandler.ReadOpenFn = CfgHandler_ReadOpen;
        cfgHandler.ReadLineFn = CfgHandler_ReadLine;
        cfgHandler.WriteAllFn = CfgHandler_WriteAll;
        ctx->SettingsHandlers.push_back(cfgHandler);

        ImGuiIO& io = ImGui::GetIO();

        ImGui::StyleColorsDark();

        io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset; // We can honor the ImDrawCmd::VtxOffset field, allowing for large meshes.
        io.BackendPlatformName = "AirForce3D";
        io.BackendRendererName = "AirForce3D";

        io.KeyMap[ImGuiKey_Tab] = KI_TAB;
        io.KeyMap[ImGuiKey_LeftArrow] = KI_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = KI_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = KI_UP;
        io.KeyMap[ImGuiKey_DownArrow] = KI_DOWN;
        io.KeyMap[ImGuiKey_PageUp] = KI_PRIOR;
        io.KeyMap[ImGuiKey_PageDown] = KI_NEXT;
        io.KeyMap[ImGuiKey_Home] = KI_HOME;
        io.KeyMap[ImGuiKey_End] = KI_END;
        io.KeyMap[ImGuiKey_Insert] = KI_INSERT;
        io.KeyMap[ImGuiKey_Delete] = KI_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = KI_BACK;
        io.KeyMap[ImGuiKey_Space] = KI_SPACE;
        io.KeyMap[ImGuiKey_Enter] = KI_RETURN;
        io.KeyMap[ImGuiKey_Escape] = KI_ESCAPE;
        io.KeyMap[ImGuiKey_KeyPadEnter] = KI_NUMPADENTER;
        io.KeyMap[ImGuiKey_A] = KI_A;
        io.KeyMap[ImGuiKey_C] = KI_C;
        io.KeyMap[ImGuiKey_V] = KI_V;
        io.KeyMap[ImGuiKey_X] = KI_X;
        io.KeyMap[ImGuiKey_Y] = KI_Y;
        io.KeyMap[ImGuiKey_Z] = KI_Z;

        for (int i = 0; i < ImGuiKey_COUNT; ++i) {
            io.KeyMap[i] = kiToChar(static_cast<KeyIdentifier>(io.KeyMap[i]));
        }

        //io.SetClipboardTextFn = &ImGuiManager::setClipboardText;
        //io.GetClipboardTextFn = &ImGuiManager::getClipboardText;
        //io.ClipboardUserData = this;

        Byte* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        fontsTex_ = textureManager.createTexture(width, height);

        io.Fonts->TexID = fontsTex_.get();

        if (!ctx->SettingsLoaded) {
            IM_ASSERT(ctx->SettingsWindows.empty());
            if (ctx->IO.IniFilename) {
                ImGui::LoadIniSettingsFromDisk(ctx->IO.IniFilename);
            }
            ctx->SettingsLoaded = true;
        }

        return true;
    }

    void ImGuiManager::shutdown()
    {
        LOG4CPLUS_DEBUG(logger(), "imGuiManager: shutdown...");

        ImGuiIO& io = ImGui::GetIO();

        io.Fonts->TexID = nullptr;

        ImGui::DestroyContext();

        fontsTex_.reset();
        runtime_assert(textureCache_.empty());
    }

    void ImGuiManager::reload()
    {
        LOG4CPLUS_DEBUG(logger(), "imGuiManager: reload...");
        ImGuiIO& io = ImGui::GetIO();

        Byte* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        fontsTex_->upload(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE,
            std::vector<Byte>(pixels, pixels + (fontsTex_->width() * fontsTex_->height() * 4)), false);
    }

    std::string ImGuiManager::cfgGet(const std::string& key, const std::string& def) const
    {
        auto it = cfg_.find(key);
        return (it == cfg_.end()) ? def : it->second;
    }

    void ImGuiManager::cfgSet(const std::string& key, const std::string& value)
    {
        auto it = cfg_.find(key);
        if ((it == cfg_.end()) || (it->second != value)) {
            cfg_[key] = value;
            ImGui::MarkIniSettingsDirty();
        }
    }

    bool ImGuiManager::cfgGetBool(const std::string& key, bool def) const
    {
        auto valStr = cfgGet(key);
        std::istringstream is(valStr);
        int val = 0;
        if (!(is >> val) || !is.eof()) {
            return def;
        } else {
            return val;
        }
    }

    void ImGuiManager::cfgSetBool(const std::string& key, bool value)
    {
        cfgSet(key, std::to_string(static_cast<int>(value)));
    }

    int ImGuiManager::cfgGetInt(const std::string& key, int def) const
    {
        auto valStr = cfgGet(key);
        std::istringstream is(valStr);
        int val = 0;
        if (!(is >> val) || !is.eof()) {
            return def;
        } else {
            return val;
        }
    }

    void ImGuiManager::cfgSetInt(const std::string& key, int value)
    {
        cfgSet(key, std::to_string(value));
    }

    float ImGuiManager::cfgGetFloat(const std::string& key, float def) const
    {
        auto valStr = cfgGet(key);
        std::istringstream is(valStr);
        float val = 0.0f;
        if (!(is >> val) || !is.eof()) {
            return def;
        } else {
            return val;
        }
    }

    void ImGuiManager::cfgSetFloat(const std::string& key, float value)
    {
        cfgSet(key, std::to_string(value));
    }

    ImTextureID ImGuiManager::toTextureId(const TexturePtr& tex)
    {
        if (!tex) {
            return nullptr;
        }
        textureCache_.insert(tex);
        return tex.get();
    }

    TexturePtr ImGuiManager::fromTextureId(ImTextureID texId)
    {
        if (!texId) {
            return TexturePtr();
        }
        return static_cast<Texture*>(texId)->shared_from_this();
    }

    void ImGuiManager::frameStart(float dt)
    {
        ImGuiIO& io = ImGui::GetIO();

        io.DisplaySize = ImVec2(settings.viewWidth, settings.viewHeight);
        io.DeltaTime = dt;
        io.MouseDrawCursor = settings.imGui.drawCursor;

        ImGui::NewFrame();
    }

    void ImGuiManager::frameEnd()
    {
        ImGui::EndFrame();
        textureCache_.clear();
    }

    void ImGuiManager::keyPress(KeyIdentifier ki, std::uint32_t keyModifiers)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.KeysDown[kiToChar(ki)] = true;
        io.KeyCtrl = (keyModifiers & KM_CTRL) != 0;
        io.KeyShift = (keyModifiers & KM_SHIFT) != 0;
        io.KeyAlt = (keyModifiers & KM_ALT) != 0;
        io.KeySuper = (keyModifiers & KM_META) != 0;
    }

    void ImGuiManager::keyRelease(KeyIdentifier ki, std::uint32_t keyModifiers)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.KeysDown[kiToChar(ki)] = false;
        io.KeyCtrl = (keyModifiers & KM_CTRL) != 0;
        io.KeyShift = (keyModifiers & KM_SHIFT) != 0;
        io.KeyAlt = (keyModifiers & KM_ALT) != 0;
        io.KeySuper = (keyModifiers & KM_META) != 0;
    }

    void ImGuiManager::textInputUCS2(std::uint32_t ch)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddInputCharacter(ch);
    }

    void ImGuiManager::mouseDown(bool left)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDown[left ? 0 : 1] = true;
    }

    void ImGuiManager::mouseUp(bool left)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDown[left ? 0 : 1] = false;
    }

    void ImGuiManager::mouseWheel(int delta)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseWheel -= delta;
    }

    void ImGuiManager::mouseMove(const Vector2f& point)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.MousePos.x = point.x();
        io.MousePos.y = point.y();
    }

    const char* ImGuiManager::getClipboardText(void* userData)
    {
        //TODO: integrate with OS
        return nullptr;
    }

    void ImGuiManager::setClipboardText(void* userData, const char* text)
    {
        //TODO: integrate with OS
    }

    int ImGuiManager::kiToChar(KeyIdentifier ki)
    {
        if (ki >= KI_0 && ki <= KI_9) {
            return (ki - KI_0) + '0';
        } else if (ki >= KI_A && ki <= KI_Z) {
            return (ki - KI_A) + 'a';
        } else if (ki == KI_OEM_1) {
            return ';';
        } else if (ki == KI_OEM_PLUS) {
            return '=';
        } else if (ki == KI_OEM_COMMA) {
            return ',';
        } else if (ki == KI_OEM_MINUS) {
            return '-';
        } else if (ki == KI_OEM_PERIOD) {
            return '.';
        } else if (ki == KI_OEM_2) {
            return '/';
        } else if (ki == KI_OEM_3) {
            return '`';
        } else if (ki == KI_OEM_4) {
            return '[';
        } else if (ki == KI_OEM_5) {
            return '\\';
        } else if (ki == KI_OEM_6) {
            return ']';
        } else if (ki == KI_OEM_7) {
            return '\'';
        } else if (ki == KI_SPACE) {
            return ' ';
        } else {
            ImGuiIO& io = ImGui::GetIO();
            runtime_assert(ki + 0xFF < static_cast<int>(sizeof(io.KeysDown) / sizeof(io.KeysDown[0])));
            return ki + 0xFF;
        }
    }

    void* ImGuiManager::CfgHandler_ReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* name)
    {
        return (void*)&imGuiManager;
    }

    void ImGuiManager::CfgHandler_ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line)
    {
        std::string s(line);
        boost::tokenizer<boost::char_separator<char>> tok(s,
            boost::char_separator<char>("="));
        auto it = tok.begin();
        if (it == tok.end()) {
            return;
        }
        std::string key = *it;
        ++it;
        if (it == tok.end()) {
            return;
        }
        imGuiManager.cfg_[key] = *it;
    }

    void ImGuiManager::CfgHandler_WriteAll(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf)
    {
        buf->appendf("[%s][Default]\n", handler->TypeName);
        for (const auto& kv : imGuiManager.cfg_) {
            buf->appendf("%s=%s\n", kv.first.c_str(), kv.second.c_str());
        }
        buf->append("\n");
    }
}
