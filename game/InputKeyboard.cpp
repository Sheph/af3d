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
    static std::string ki2str[256];

    static bool ki2strInit()
    {
        ki2str[KI_BACK] = "BACKSPACE";
        ki2str[KI_TAB] = "TAB";
        ki2str[KI_RETURN] = "ENTER";
        ki2str[KI_PAUSE] = "PAUSE";
        ki2str[KI_SCROLL] = "SCROLL";
        ki2str[KI_ESCAPE] = "ESC";
        ki2str[KI_DELETE] = "DELETE";
        ki2str[KI_HOME] = "HOME";
        ki2str[KI_LEFT] = "LEFT";
        ki2str[KI_UP] = "UP";
        ki2str[KI_RIGHT] = "RIGHT";
        ki2str[KI_DOWN] = "DOWN";
        ki2str[KI_PRIOR] = "PAGEUP";
        ki2str[KI_NEXT] = "PAGEDOWN";
        ki2str[KI_END] = "END";
        ki2str[KI_SNAPSHOT] = "PRNTSCRN";
        ki2str[KI_INSERT] = "INSERT";
        ki2str[KI_NUMLOCK] = "NUMLOCK";
        ki2str[KI_SPACE] = "SPACE";
        ki2str[KI_NUMPADENTER] = "KP_ENTER";
        ki2str[KI_F1] = "F1";
        ki2str[KI_F2] = "F2";
        ki2str[KI_F3] = "F3";
        ki2str[KI_F4] = "F4";
        ki2str[KI_NUMPAD7] = "KP_7";
        ki2str[KI_NUMPAD4] = "KP_4";
        ki2str[KI_NUMPAD8] = "KP_8";
        ki2str[KI_NUMPAD6] = "KP_6";
        ki2str[KI_NUMPAD2] = "KP_2";
        ki2str[KI_NUMPAD9] = "KP_9";
        ki2str[KI_NUMPAD3] = "KP_3";
        ki2str[KI_NUMPAD1] = "KP_1";
        ki2str[KI_NUMPAD5] = "KP_5";
        ki2str[KI_NUMPAD0] = "KP_0";
        ki2str[KI_DECIMAL] = "KP_PERIOD";
        ki2str[KI_OEM_NEC_EQUAL] = "KP_EQUAL";
        ki2str[KI_MULTIPLY] = "KP_MUL";
        ki2str[KI_ADD] = "KP_ADD";
        ki2str[KI_SEPARATOR] = "KP_COMMA";
        ki2str[KI_SUBTRACT] = "KP_SUB";
        ki2str[KI_DIVIDE] = "KP_DIV";
        ki2str[KI_F5] = "F5";
        ki2str[KI_F6] = "F6";
        ki2str[KI_F7] = "F7";
        ki2str[KI_F8] = "F8";
        ki2str[KI_F9] = "F9";
        ki2str[KI_F10] = "F10";
        ki2str[KI_F11] = "F11";
        ki2str[KI_F12] = "F12";
        ki2str[KI_F13] = "F13";
        ki2str[KI_F14] = "F14";
        ki2str[KI_F15] = "F15";
        ki2str[KI_F16] = "F16";
        ki2str[KI_F17] = "F17";
        ki2str[KI_F18] = "F18";
        ki2str[KI_F19] = "F19";
        ki2str[KI_F20] = "F20";
        ki2str[KI_F21] = "F21";
        ki2str[KI_F22] = "F22";
        ki2str[KI_F23] = "F23";
        ki2str[KI_F24] = "F24";
        ki2str[KI_LSHIFT] = "LSHIFT";
        ki2str[KI_RSHIFT] = "RSHIFT";
        ki2str[KI_LCONTROL] = "LCTRL";
        ki2str[KI_RCONTROL] = "RCTRL";
        ki2str[KI_CAPITAL] = "CAPSLOCK";
        ki2str[KI_LMENU] = "LALT";
        ki2str[KI_RMENU] = "RALT";
        ki2str[KI_OEM_7] = "\"";
        ki2str[KI_OEM_COMMA] = ",";
        ki2str[KI_OEM_MINUS] = "-";
        ki2str[KI_OEM_PERIOD] = ".";
        ki2str[KI_OEM_2] = "/";
        ki2str[KI_0] = "0";
        ki2str[KI_1] = "1";
        ki2str[KI_2] = "2";
        ki2str[KI_3] = "3";
        ki2str[KI_4] = "4";
        ki2str[KI_5] = "5";
        ki2str[KI_6] = "6";
        ki2str[KI_7] = "7";
        ki2str[KI_8] = "8";
        ki2str[KI_9] = "9";
        ki2str[KI_OEM_1] = ":";
        ki2str[KI_OEM_PLUS] = "+";
        ki2str[KI_OEM_4] = "[";
        ki2str[KI_OEM_5] = "\\";
        ki2str[KI_OEM_6] = "]";
        ki2str[KI_OEM_3] = "~";
        ki2str[KI_A] = "A";
        ki2str[KI_B] = "B";
        ki2str[KI_C] = "C";
        ki2str[KI_D] = "D";
        ki2str[KI_E] = "E";
        ki2str[KI_F] = "F";
        ki2str[KI_G] = "G";
        ki2str[KI_H] = "H";
        ki2str[KI_I] = "I";
        ki2str[KI_J] = "J";
        ki2str[KI_K] = "K";
        ki2str[KI_L] = "L";
        ki2str[KI_M] = "M";
        ki2str[KI_N] = "N";
        ki2str[KI_O] = "O";
        ki2str[KI_P] = "P";
        ki2str[KI_Q] = "Q";
        ki2str[KI_R] = "R";
        ki2str[KI_S] = "S";
        ki2str[KI_T] = "T";
        ki2str[KI_U] = "U";
        ki2str[KI_V] = "V";
        ki2str[KI_W] = "W";
        ki2str[KI_X] = "X";
        ki2str[KI_Y] = "Y";
        ki2str[KI_Z] = "Z";
        return true;
    }

    static bool ki2strInitialized = ki2strInit();

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

    const char* InputKeyboard::kiToStr(KeyIdentifier ki)
    {
        return ki2str[ki].c_str();
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
