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
            io.KeyMap[i] = InputKeyboard::kiToChar(static_cast<KeyIdentifier>(io.KeyMap[i]));
        }

        //io.SetClipboardTextFn = &ImGuiManager::setClipboardText;
        //io.GetClipboardTextFn = &ImGuiManager::getClipboardText;
        //io.ClipboardUserData = this;

        Byte* pixels;
        int width, height;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

        fontsTex_ = textureManager.createTexture(width, height);

        io.Fonts->TexID = fontsTex_.get();

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
            std::vector<Byte>(pixels, pixels + (fontsTex_->width() * fontsTex_->height() * 4)));
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
        io.KeysDown[InputKeyboard::kiToChar(ki)] = true;
        io.KeyCtrl = (keyModifiers & KM_CTRL) != 0;
        io.KeyShift = (keyModifiers & KM_SHIFT) != 0;
        io.KeyAlt = (keyModifiers & KM_ALT) != 0;
        io.KeySuper = (keyModifiers & KM_META) != 0;
    }

    void ImGuiManager::keyRelease(KeyIdentifier ki, std::uint32_t keyModifiers)
    {
        ImGuiIO& io = ImGui::GetIO();
        io.KeysDown[InputKeyboard::kiToChar(ki)] = false;
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
}
