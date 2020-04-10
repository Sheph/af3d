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

#include "Action.h"
#include "ImGuiUtils.h"

namespace af3d { namespace editor
{
    Action::Action(const std::string& text, const StateFn& stateFn, const TriggerFn& triggerFn, const Image& icon, const KeySequence& shortcut)
    : text_(text),
      tooltip_(shortcut.empty() ? text : text + " (" + shortcut.str() + ")"),
      icon_(icon),
      shortcut_(shortcut),
      stateFn_(stateFn),
      triggerFn_(triggerFn)
    {
    }

    void Action::trigger()
    {
        auto s = state();
        if (s.enabled && !s.checked) {
            triggerFn_();
        }
    }

    void Action::doMenu()
    {
        auto s = state();
        if (ImGui::BeginMenu(text_.c_str(), s.enabled && !s.checked)) {
            trigger();
            ImGui::EndMenu();
        }
    }

    void Action::doMenuItem()
    {
        auto s = state();
        if (ImGui::MenuItem(text_.c_str(), (shortcut_.empty() ? nullptr : shortcut_.str().c_str()), false, s.enabled && !s.checked)) {
            trigger();
        }
    }

    void Action::doButton(float size)
    {
        auto s = state();
        if (ImGuiUtils::imageButtonTooltip(text_.c_str(), icon(), size, tooltip_.c_str(), s.enabled, s.checked)) {
            trigger();
        }
    }
} }
