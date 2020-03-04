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

#ifndef _INPUTGAMEPAD_H_
#define _INPUTGAMEPAD_H_

#include "af3d/Utils.h"
#include "af3d/Vector2.h"
#include <boost/noncopyable.hpp>
#include <unordered_map>

namespace af3d
{
    enum class GamepadButton
    {
        Unknown = 0,
        DPADUp,
        DPADDown,
        DPADLeft,
        DPADRight,
        Start,
        Back,
        LeftStick,
        RightStick,
        LeftBumper,
        RightBumper,
        LeftTrigger,
        RightTrigger,
        A,
        B,
        X,
        Y,
        MAX = Y
    };

    class InputGamepad : boost::noncopyable
    {
    public:
        InputGamepad() = default;
        ~InputGamepad() = default;

        inline float stickDeadzone() const { return stickDeadzone_; }
        inline void setStickDeadzone(float value) { stickDeadzone_ = value; }

        inline float triggerDeadzone() const { return triggerDeadzone_; }
        inline void setTriggerDeadzone(float value) { triggerDeadzone_ = value; }

        void moveStick(bool left, const Vector2f& value);
        void moveTrigger(bool left, float value);
        void press(GamepadButton button);
        void release(GamepadButton button);

        bool pressed(GamepadButton button) const;
        bool triggered(GamepadButton button) const;
        inline const Vector2f& pos(bool left) const { return pos_[left]; }

        bool moveLeft() const;
        bool moveRight() const;
        bool moveUp() const;
        bool moveDown() const;

        void processed();

        void proceed();

    private:
        struct ButtonState
        {
            bool pressed = false;
            bool triggered = false;
            bool savedTriggered = false;
        };

        using ButtonMap = std::unordered_map<GamepadButton, ButtonState, EnumHash<GamepadButton>>;

        float stickDeadzone_ = 0.2f;
        float triggerDeadzone_ = 0.1f;

        mutable ButtonMap buttonMap_;
        Vector2f pos_[2] = {Vector2f_zero, Vector2f_zero};
    };
}

#endif
