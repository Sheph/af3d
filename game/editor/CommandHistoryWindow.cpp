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

#include "editor/CommandHistoryWindow.h"
#include "Const.h"
#include "Logger.h"
#include "Scene.h"
#include "imgui.h"

namespace af3d {
    ACLASS_NS_DEFINE_BEGIN(editor, CommandHistoryWindow, UIComponent)
    ACLASS_NS_DEFINE_END(editor, CommandHistoryWindow)

namespace editor {
    CommandHistoryWindow::CommandHistoryWindow()
    : UIComponent(AClass_editorCommandHistoryWindow, zOrderEditor)
    {
    }

    const AClass& CommandHistoryWindow::staticKlass()
    {
        return AClass_editorCommandHistoryWindow;
    }

    AObjectPtr CommandHistoryWindow::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<CommandHistoryWindow>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void CommandHistoryWindow::update(float dt)
    {
        if (!show_) {
            removeFromParent();
            return;
        }

        ImGui::SetNextWindowPos(ImVec2(0, 100), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(200, 400), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin("Command History", &show_)) {
            ImGui::End();
            return;
        }

        auto& cmdHistory = scene()->workspace()->cmdHistory();
        int pos = cmdHistory.pos();

        ImGui::PushID(-1);
        if (ImGui::Selectable("<empty>", pos == 0)) {
            cmdHistory.undo(cmdHistory.list().size());
        }
        ImGui::PopID();

        for (int i = 0; i < static_cast<int>(cmdHistory.list().size()); ++i) {
            ImGui::PushID(i);
            if (ImGui::Selectable(cmdHistory.list()[i]->description().c_str(), (pos - 1) == i)) {
                if (i < (pos - 1)) {
                    cmdHistory.undo((pos - 1) - i);
                } else if (i > (pos - 1)) {
                    cmdHistory.redo(i - (pos - 1));
                }
            }
            ImGui::PopID();
        }

        ImGui::End();
    }

    void CommandHistoryWindow::onRegister()
    {
        LOG4CPLUS_DEBUG(logger(), "CommandHistoryWindow open");
    }

    void CommandHistoryWindow::onUnregister()
    {
        LOG4CPLUS_DEBUG(logger(), "CommandHistoryWindow closed");
    }
} }
