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

#ifndef _EDITOR_ACTION_H_
#define _EDITOR_ACTION_H_

#include "Image.h"
#include "InputManager.h"
#include "af3d/Types.h"

namespace af3d { namespace editor
{
    class Action
    {
    public:
        struct State
        {
            State() = default;
            explicit State(bool enabled, bool checked = false)
            : enabled(enabled), checked(checked) {}

            bool enabled = false;
            bool checked = false;
        };

        using StateFn = std::function<State()>;
        using TriggerFn = std::function<void()>;

        Action() = default;
        Action(const std::string& text, const StateFn& stateFn, const TriggerFn& triggerFn, const Image& icon = Image(), KeyIdentifier shortcut = KI_UNKNOWN);
        ~Action() = default;

        inline const std::string& text() const { return text_; }
        inline const std::string& tooltip() const { return tooltip_; }
        inline const Image& icon() const { return icon_; }
        inline KeyIdentifier shortcut() const { return shortcut_; }
        inline State state() const { return stateFn_(); }

        void trigger();

        void doMenu();

        void doMenuItem();

        void doButton(float size);

    private:
        std::string text_;
        std::string tooltip_;
        Image icon_;
        KeyIdentifier shortcut_;

        StateFn stateFn_;
        TriggerFn triggerFn_;
    };
} }

#endif
