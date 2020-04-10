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

#include "editor/Tool.h"
#include "editor/Workspace.h"

namespace af3d { namespace editor
{
    Tool::Tool(Workspace* workspace, const std::string& name, const Image& icon,
        const KeySequence& shortcut)
    : workspace_(workspace),
      name_(name),
      tooltip_(shortcut.empty() ? name : name + " (" + shortcut.str() + ")"),
      icon_(icon),
      shortcut_(shortcut)
    {
    }

    void Tool::activate(bool value)
    {
        if (active_ == value) {
            return;
        }
        active_ = value;
        if (active_) {
            btAssert(canWork());
            onActivate();
        } else {
            onDeactivate();
        }
    }

    void Tool::update(float dt)
    {
        if (active_) {
            doUpdate(dt);
        }
    }

    void Tool::options()
    {
        if (active_) {
            doOptions();
        }
    }

    Scene* Tool::scene()
    {
        return workspace_->scene();
    }

    const Scene* Tool::scene() const
    {
        return workspace_->scene();
    }
} }
