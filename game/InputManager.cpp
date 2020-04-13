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

#include "InputManager.h"
#include "Settings.h"
#include "Logger.h"

namespace af3d
{
    InputManager inputManager;

    template <>
    Single<InputManager>* Single<InputManager>::single = nullptr;

    bool InputManager::init()
    {
        LOG4CPLUS_DEBUG(logger(), "inputManager: init...");
        return true;
    }

    void InputManager::shutdown()
    {
        LOG4CPLUS_DEBUG(logger(), "inputManager: shutdown...");
    }

    void InputManager::update()
    {
        if (keyboard().triggered(KI_J)) {
            gameDebugPressed_ = !gameDebugPressed_;
        }
        if (keyboard().triggered(KI_P)) {
            physicsDebugPressed_ = !physicsDebugPressed_;
        }
        if (keyboard().triggered(KI_M)) {
            slowmoPressed_ = !slowmoPressed_;
        }
        if (keyboard().triggered(KI_B)) {
            cullPressed_ = !cullPressed_;
        }
    }

    Vector2f InputManager::lookPos(bool& relative)
    {
        if (lookRelative_) {
            if (gamepad().pos(false) != Vector2f_zero) {
                lookGamepadPos_ = gamepad().pos(false);
            }
            if (mouse().pos() == lookMousePos_) {
                relative = true;
                return lookGamepadPos_;
            } else {
                lookMousePos_ = mouse().pos();
                lookRelative_ = false;
                relative = false;
                return lookMousePos_;
            }
        } else {
            lookMousePos_ = mouse().pos();
            if (gamepad().pos(false) == Vector2f_zero) {
                relative = false;
                return lookMousePos_;
            } else {
                lookRelative_ = true;
                lookGamepadPos_ = gamepad().pos(false);
                relative = true;
                return lookGamepadPos_;
            }
        }
    }

    void InputManager::processed()
    {
        keyboard_.processed();
        mouse_.processed();
        gamepad_.processed();
    }

    void InputManager::proceed()
    {
        keyboard_.proceed();
        mouse_.proceed();
        gamepad_.proceed();
    }
}
