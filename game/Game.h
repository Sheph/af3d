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

#ifndef _GAME_H_
#define _GAME_H_

#include "af3d/Types.h"
#include "af3d/Single.h"
#include "af3d/AppConfig.h"
#include "Level.h"
#include "InputManager.h"
#include "HardwareContext.h"

namespace af3d
{
    class Game : public Single<Game>
    {
    public:
        Game();
        ~Game();

        bool init(const std::string& startAsset);

        bool renderReload(HardwareContext& ctx);

        void reload();

        bool update();

        bool render(HardwareContext& ctx);

        void cancelUpdate(HardwareContext& ctx);

        void cancelRender();

        void shutdown();

        void keyPress(KeyIdentifier ki, std::uint32_t keyModifiers);

        void keyRelease(KeyIdentifier ki, std::uint32_t keyModifiers);

        void textInputUCS2(std::uint32_t ch);

        void mouseDown(bool left);

        void mouseUp(bool left);

        void mouseWheel(int delta);

        void mouseMove(const Vector2f& point);

        void gamepadMoveStick(bool left, const Vector2f& value);

        void gamepadMoveTrigger(bool left, float value);

        void gamepadPress(GamepadButton button);

        void gamepadRelease(GamepadButton button);

    private:
        bool loadLevel(const std::string& assetPath);

        bool setupVideo(const AppConfig& userConfig);

        void setupAudio(const AppConfig& userConfig);

        void writeUserConfig(bool controlsOnly);

        LevelPtr level_;
        LevelPtr editorLevel_;

        std::uint64_t lastTimeUs_ = 0;
        std::uint32_t numFrames_ = 0;
        std::uint32_t accumRenderTimeUs_ = 0;
        std::uint32_t accumTimeUs_ = 0;
        std::uint64_t lastProfileReportTimeUs_ = 0;
    };
}

#endif
