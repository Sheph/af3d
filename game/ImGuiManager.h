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

#ifndef _IMGUIMANAGER_H_
#define _IMGUIMANAGER_H_

#include "InputManager.h"
#include "Material.h"
#include "af3d/Single.h"
#include "imgui.h"

struct ImGuiSettingsHandler;

namespace af3d
{
    class ImGuiManager : public Single<ImGuiManager>
    {
    public:
        ImGuiManager() = default;
        ~ImGuiManager();

        bool init();

        void shutdown();

        void reload();

        static constexpr const char* strCommandHistoryOpened = "CommandHistoryOpened";
        static constexpr const char* strPropertyEditorOpened = "PropertyEditorOpened";
        static constexpr const char* strPropertyEditorColumnWidth = "PropertyEditorColumnWidth";
        static constexpr const char* strToolBoxOpened = "ToolBoxOpened";
        static constexpr const char* strCollisionMatrixEditorOpened = "CollisionMatrixEditorOpened";

        std::string cfgGet(const std::string& key, const std::string& def = "") const;
        void cfgSet(const std::string& key, const std::string& value);

        bool cfgGetBool(const std::string& key, bool def = false) const;
        void cfgSetBool(const std::string& key, bool value);

        int cfgGetInt(const std::string& key, int def = 0) const;
        void cfgSetInt(const std::string& key, int value);

        float cfgGetFloat(const std::string& key, float def = 0.0f) const;
        void cfgSetFloat(const std::string& key, float value);

        ImTextureID toTextureId(const TexturePtr& tex);
        TexturePtr fromTextureId(ImTextureID texId);

        void frameStart(float dt);
        void frameEnd();

        void keyPress(KeyIdentifier ki, std::uint32_t keyModifiers);

        void keyRelease(KeyIdentifier ki, std::uint32_t keyModifiers);

        void textInputUCS2(std::uint32_t ch);

        void mouseDown(bool left);

        void mouseUp(bool left);

        void mouseWheel(int delta);

        void mouseMove(const Vector2f& point);

    private:
        using Cfg = std::unordered_map<std::string, std::string>;
        using TextureCache = std::unordered_set<TexturePtr>;

        static const char* getClipboardText(void* userData);
        static void setClipboardText(void* userData, const char* text);
        static int kiToChar(KeyIdentifier ki);

        static void* CfgHandler_ReadOpen(ImGuiContext*, ImGuiSettingsHandler*, const char* name);
        static void CfgHandler_ReadLine(ImGuiContext*, ImGuiSettingsHandler*, void* entry, const char* line);
        static void CfgHandler_WriteAll(ImGuiContext* ctx, ImGuiSettingsHandler* handler, ImGuiTextBuffer* buf);

        Cfg cfg_;

        TexturePtr fontsTex_;
        TextureCache textureCache_;
    };

    extern ImGuiManager imGuiManager;
}

#endif
