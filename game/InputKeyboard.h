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

#ifndef _INPUTKEYBOARD_H_
#define _INPUTKEYBOARD_H_

#include "af3d/Utils.h"
#include "af3d/EnumSet.h"
#include <Rocket/Core/Input.h>
#include <boost/noncopyable.hpp>

namespace af3d
{
    using namespace Rocket::Core::Input;

    struct KeySequence
    {
        KeySequence() = default;
        explicit KeySequence(KeyIdentifier ki, bool ignoreModifiers = false)
        : ki(ki),
          kms(ignoreModifiers ? -1 : 0) {}
        KeySequence(KeyModifier km, KeyIdentifier ki)
        : ki(ki),
          kms(km) {}
        KeySequence(KeyModifier km1, KeyModifier km2, KeyIdentifier ki)
        : ki(ki),
          kms(km1 | km2) {}

        const std::string& str() const;

        inline bool empty() const { return ki == KI_UNKNOWN; }

        KeyIdentifier ki = KI_UNKNOWN;
        int kms = -1;
    };

    class InputKeyboard : boost::noncopyable
    {
    public:
        InputKeyboard() = default;
        ~InputKeyboard() = default;

        static int kiToChar(KeyIdentifier ki);

        static const char* kiToStr(KeyIdentifier ki);

        static const char* kmsToStr(std::uint32_t keyModifiers);

        void press(KeyIdentifier ki, std::uint32_t keyModifiers);

        void release(KeyIdentifier ki, std::uint32_t keyModifiers);

        bool pressed(const KeySequence& ks) const;

        bool triggered(const KeySequence& ks) const;

        inline bool pressed(KeyIdentifier ki) const
        {
            return pressed(KeySequence(ki, true));
        }

        inline bool triggered(KeyIdentifier ki) const
        {
            return triggered(KeySequence(ki, true));
        }

        void processed();

        void proceed();

    private:
        struct KeyState
        {
            bool pressed = false;
            bool triggered = false;
            bool savedTriggered = false;
        };

        using KeyMap = EnumUnorderedMap<KeyIdentifier, KeyState>;

        mutable KeyMap keyMap_;
        std::uint32_t keyModifiers_ = 0;
    };
}

#endif
