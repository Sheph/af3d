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

#ifndef _INPUTMANAGER_H_
#define _INPUTMANAGER_H_

#include "af3d/Single.h"
#include "InputKeyboard.h"
#include "InputMouse.h"
#include "InputGamepad.h"

namespace af3d
{
    class InputManager : public Single<InputManager>
    {
    public:
        InputManager() = default;
        ~InputManager() = default;

        bool init();

        void shutdown();

        void update();

        void processed();

        void proceed();

        inline InputKeyboard& keyboard() { return keyboard_; }

        inline InputMouse& mouse() { return mouse_; }

        inline InputGamepad& gamepad() { return gamepad_; }

        Vector2f lookPos(bool& relative);

        inline bool usingGamepad() const { return usingGamepad_; }
        inline void setUsingGamepad(bool value) { usingGamepad_ = value; }

        inline bool gameDebugPressed() const { return gameDebugPressed_; }

        inline bool physicsDebugPressed() const { return physicsDebugPressed_; }

        inline bool slowmoPressed() const { return slowmoPressed_; }

        inline bool cullPressed() const { return cullPressed_; }

    private:
        InputKeyboard keyboard_;
        InputMouse mouse_;
        InputGamepad gamepad_;

        bool lookRelative_ = false;
        Vector2f lookMousePos_ = Vector2f_zero;
        Vector2f lookGamepadPos_ = Vector2f_zero;

        bool usingGamepad_ = false;

        bool gameDebugPressed_ = false;
        bool physicsDebugPressed_ = false;
        bool slowmoPressed_ = false;
        bool cullPressed_ = false;
    };

    extern InputManager inputManager;
}

#endif
