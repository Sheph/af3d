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

#include "InputKeyboard.h"
#include "InputManager.h"
#include "Settings.h"
#include "af3d/Utils.h"

namespace af3d
{
    int InputKeyboard::kiToChar(KeyIdentifier ki, bool* isTextInput)
    {
        if (isTextInput) {
            *isTextInput = true;
        }
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
            if (isTextInput) {
                *isTextInput = false;
            }
            return ki;
        }
    }

    void InputKeyboard::press(KeyIdentifier ki)
    {
        if (!keyMap_[ki].pressed) {
            keyMap_[ki].triggered = true;
        }
        keyMap_[ki].pressed = true;

        inputManager.setUsingGamepad(false);
    }

    void InputKeyboard::release(KeyIdentifier ki)
    {
        keyMap_[ki].pressed = false;
        keyMap_[ki].triggered = false;

        inputManager.setUsingGamepad(false);
    }

    bool InputKeyboard::pressed(KeyIdentifier ki) const
    {
        return keyMap_[ki].pressed;
    }

    bool InputKeyboard::triggered(KeyIdentifier ki) const
    {
        return keyMap_[ki].triggered;
    }

    void InputKeyboard::processed()
    {
        for (auto& kv : keyMap_) {
            kv.second.savedTriggered = kv.second.triggered;
            kv.second.triggered = false;
        }
    }

    void InputKeyboard::proceed()
    {
        for (auto& kv : keyMap_) {
            kv.second.triggered = kv.second.savedTriggered;
        }
    }
}
