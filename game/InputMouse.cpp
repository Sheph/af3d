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

#include "InputMouse.h"
#include "InputManager.h"

namespace af3d
{
    void InputMouse::move(const Vector2f& point)
    {
        pos_ = point;
        inputManager.setUsingGamepad(false);
    }

    void InputMouse::press(bool left)
    {
        int button = left ? 0 : 1;

        if (!buttons_[button].pressed) {
            buttons_[button].triggered = true;
        }

        buttons_[button].pressed = true;

        inputManager.setUsingGamepad(false);
    }

    void InputMouse::release(bool left)
    {
        int button = left ? 0 : 1;

        buttons_[button].pressed = false;
        buttons_[button].triggered = false;

        inputManager.setUsingGamepad(false);
    }

    bool InputMouse::pressed(bool left) const
    {
        return buttons_[left ? 0 : 1].pressed;
    }

    bool InputMouse::triggered(bool left) const
    {
        return buttons_[left ? 0 : 1].triggered;
    }

    void InputMouse::processed()
    {
        for (size_t i = 0; i < buttons_.size(); ++i) {
            buttons_[i].savedTriggered = buttons_[i].triggered;
            buttons_[i].triggered = false;
        }
    }

    void InputMouse::proceed()
    {
        for (size_t i = 0; i < buttons_.size(); ++i) {
            buttons_[i].triggered = buttons_[i].savedTriggered;
        }
    }
}
