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

#include "Game.h"
#include "Settings.h"
#include "Logger.h"
#include "SceneObjectFactory.h"
#include "Platform.h"
#include "GameShell.h"
#include "HardwareResourceManager.h"
#include "Renderer.h"
#include "TextureManager.h"
#include "MaterialManager.h"
#include "MeshManager.h"
#include "ImGuiManager.h"
#include "AssetManager.h"
#include "AClassRegistry.h"
#include "af3d/Utils.h"
#include "af3d/StreamAppConfig.h"

namespace af3d
{
    template <>
    Single<Game>* Single<Game>::single = nullptr;

    Game::Game()
    {
    }

    Game::~Game()
    {
    }

    bool Game::init(const std::string& startAsset)
    {
        AClassRegistry::instance().dump();

        if (!hwManager.init()) {
            return false;
        }

        if (!renderer.init()) {
            return false;
        }

        if (!textureManager.init()) {
            return false;
        }

        if (!materialManager.init()) {
            return false;
        }

        if (!meshManager.init()) {
            return false;
        }

        if (!inputManager.init()) {
            return false;
        }

        if (!imGuiManager.init()) {
            return false;
        }

        LOG4CPLUS_DEBUG(logger(), "Supported desktop video modes:");

        int i = 0;

        for (const auto& mode : platform->desktopVideoModes()) {
            LOG4CPLUS_DEBUG(logger(), i << " = " << mode.width << "x" << mode.height);
            ++i;
        }

        LOG4CPLUS_DEBUG(logger(), "Supported windowed video modes:");

        i = 0;

        for (const auto& mode : platform->winVideoModes()) {
            LOG4CPLUS_DEBUG(logger(), i << " = " << mode.width << "x" << mode.height);
            ++i;
        }

        LOG4CPLUS_DEBUG(logger(), "Supported MSAA modes:");

        i = 0;

        for (auto mode : platform->msaaModes()) {
            LOG4CPLUS_DEBUG(logger(), i << " = x" << mode);
            ++i;
        }

        LOG4CPLUS_DEBUG(logger(), "Default windowed video mode: " << platform->winVideoModes()[platform->defaultVideoMode()].width << "x" << platform->winVideoModes()[platform->defaultVideoMode()].height);

        if (platform->desktopVideoMode() < 0) {
            LOG4CPLUS_WARN(logger(), "Fullscreen mode not supported!");
        } else {
            LOG4CPLUS_DEBUG(logger(), "Desktop video mode: " << platform->desktopVideoModes()[platform->desktopVideoMode()].width << "x" << platform->desktopVideoModes()[platform->desktopVideoMode()].height);
        }

        if (!platform->vsyncSupported()) {
            LOG4CPLUS_WARN(logger(), "vsync not supported!");
        }

        std::shared_ptr<StreamAppConfig> userConfig;

        std::string userConfigData = platform->readUserConfig();

        if (!userConfigData.empty()) {
            userConfig = std::make_shared<StreamAppConfig>();

            std::istringstream is(userConfigData);

            if (!userConfig->load(is)) {
                LOG4CPLUS_WARN(logger(), "Error parsing user config file!");
                userConfig.reset();
            }
        }

        if (userConfig) {
            if (!setupVideo(*userConfig)) {
                writeUserConfig(true);
                return false;
            }
        } else {
            if (!platform->changeVideoMode(false, platform->defaultVideoMode(), 0, false, true)) {
                return false;
            }
        }

        if (userConfig) {
            setupAudio(*userConfig);
        }

        if (!assetManager.init()) {
            return false;
        }

        if (!sceneObjectFactory.init()) {
            return false;
        }

        return loadLevel(startAsset);
    }

    bool Game::renderReload(HardwareContext& ctx)
    {
        if (!hwManager.renderReload(ctx)) {
            return false;
        }
        if (!renderer.reload(ctx)) {
            return false;
        }
        if (!textureManager.renderReload(ctx)) {
            return false;
        }
        if (!materialManager.renderReload(ctx)) {
            return false;
        }
        if (!meshManager.renderReload(ctx)) {
            return false;
        }
        return true;
    }

    void Game::reload()
    {
        textureManager.reload();
        materialManager.reload();
        meshManager.reload();
        imGuiManager.reload();
        gameShell->reload();
    }

    bool Game::update()
    {
        static std::string assetPath;

        std::uint64_t timeUs = getTimeUs();
        std::uint32_t deltaUs;

        if (lastTimeUs_ == 0) {
            lastProfileReportTimeUs_ = timeUs;
            deltaUs = 16000; // pretend that very first frame lasted 16ms
        } else {
            deltaUs = static_cast<std::uint32_t>(timeUs - lastTimeUs_);
        }

        lastTimeUs_ = timeUs;

        float dt = static_cast<float>(deltaUs) / 1000000.0f;

        imGuiManager.frameStart(dt);

        level_->scene()->update(dt);

        imGuiManager.frameEnd();

        std::uint64_t timeUs2 = getTimeUs();

        accumRenderTimeUs_ += static_cast<std::uint32_t>(timeUs2 - timeUs);
        accumTimeUs_ += deltaUs;
        ++numFrames_;

        if ((timeUs2 - lastProfileReportTimeUs_) > settings.profileReportTimeoutMs * 1000) {
            lastProfileReportTimeUs_ = timeUs2;

            LOG4CPLUS_TRACE(logger(),
                "FPS: " << (numFrames_ * 1000000) / accumTimeUs_
                << " Time: " << accumRenderTimeUs_ / (numFrames_ * 1000));

            accumRenderTimeUs_ = 0;
            accumTimeUs_ = 0;
            numFrames_ = 0;
        }

        if (level_->scene()->quit()) {
            if (settings.editor.playing && editorLevel_) {
                level_.reset();
                level_ = editorLevel_;
                editorLevel_.reset();
                settings.editor.playing = false;
                return true;
            } else {
                level_->scene()->setQuit(false);
                return false;
            }
        }

        if (level_->scene()->getNextLevel(assetPath)) {
            if (!loadLevel(assetPath)) {
                return false;
            }
        }

        return true;
    }

    bool Game::render(HardwareContext& ctx)
    {
        return renderer.render(ctx);
    }

    void Game::cancelUpdate(HardwareContext& ctx)
    {
        renderer.cancelSwap(ctx);
    }

    void Game::cancelRender()
    {
        renderer.cancelRender();
    }

    void Game::shutdown()
    {
        writeUserConfig(false);

        level_.reset();
        editorLevel_.reset();

        sceneObjectFactory.shutdown();

        assetManager.shutdown();

        imGuiManager.shutdown();

        inputManager.shutdown();

        meshManager.shutdown();

        materialManager.shutdown();

        textureManager.shutdown();

        renderer.shutdown();

        hwManager.shutdown();
    }

    void Game::keyPress(KeyIdentifier ki, std::uint32_t keyModifiers)
    {
        inputManager.keyboard().press(ki, keyModifiers);
        imGuiManager.keyPress(ki, keyModifiers);
    }

    void Game::keyRelease(KeyIdentifier ki, std::uint32_t keyModifiers)
    {
        inputManager.keyboard().release(ki, keyModifiers);
        imGuiManager.keyRelease(ki, keyModifiers);
    }

    void Game::textInputUCS2(std::uint32_t ch)
    {
        imGuiManager.textInputUCS2(ch);
    }

    void Game::mouseDown(bool left)
    {
        inputManager.mouse().press(left);
        imGuiManager.mouseDown(left);
    }

    void Game::mouseUp(bool left)
    {
        inputManager.mouse().release(left);
        imGuiManager.mouseUp(left);
    }

    void Game::mouseWheel(int delta)
    {
        imGuiManager.mouseWheel(delta);
    }

    void Game::mouseMove(const Vector2f& point)
    {
        Vector2f pt = point - Vector2f(settings.viewX, settings.viewY);

        inputManager.mouse().move(pt);
        imGuiManager.mouseMove(pt);
    }

    void Game::gamepadMoveStick(bool left, const Vector2f& value)
    {
    }

    void Game::gamepadMoveTrigger(bool left, float value)
    {
    }

    void Game::gamepadPress(GamepadButton button)
    {
    }

    void Game::gamepadRelease(GamepadButton button)
    {
    }

    bool Game::loadLevel(const std::string& assetPath)
    {
        int cp = 0;

        if (level_) {
            cp = level_->scene()->checkpoint();
        }

        if (settings.editor.playing && !editorLevel_) {
            editorLevel_ = level_;
        }

        level_.reset();

        LOG4CPLUS_INFO(logger(), "loading level (\"" << assetPath << "\")...");

        LevelPtr level = std::make_shared<Level>(assetPath, cp);

        if (!level->init()) {
            if (editorLevel_) {
                level_ = editorLevel_;
                editorLevel_.reset();
                settings.editor.playing = false;
                return true;
            } else {
                return false;
            }
        }

        level_ = level;

        LOG4CPLUS_INFO(logger(), "level loaded");

        return true;
    }

    bool Game::setupVideo(const AppConfig& userConfig)
    {
        bool fullscreen = false;
        int videoMode = -1;
        int msaaMode = -1;
        bool vsync = false;
        bool trilinearFilter = true;
        bool failed = false;

        if (userConfig.haveKey("video.fullscreen")) {
            fullscreen = userConfig.getBool("video.fullscreen");
        } else {
            failed = true;
        }

        if (userConfig.haveKey("video.mode")) {
            Vector2f res = userConfig.getVector2f("video.mode");

            std::vector<VideoMode> modes;

            if (fullscreen) {
                modes = platform->desktopVideoModes();
            } else {
                modes = platform->winVideoModes();
            }

            VideoMode m(res.x(), res.y());

            for (size_t i = 0; i < modes.size(); ++i) {
                if (m == modes[i]) {
                    videoMode = i;
                    break;
                }
            }

            if (videoMode == -1) {
                failed = true;
            }
        } else {
            failed = true;
        }

        if (userConfig.haveKey("video.msaa")) {
            std::uint32_t msaa = userConfig.getInt("video.msaa");

            for (size_t i = 0; i < platform->msaaModes().size(); ++i) {
                if (msaa == platform->msaaModes()[i]) {
                    msaaMode = i;
                    break;
                }
            }

            if (msaaMode == -1) {
                failed = true;
            }
        } else {
            failed = true;
        }

        if (userConfig.haveKey("video.vsync")) {
            vsync = userConfig.getBool("video.vsync");
            if (vsync && !platform->vsyncSupported()) {
                failed = true;
            }
        } else {
            failed = true;
        }

        if (userConfig.haveKey("video.trilinear")) {
            trilinearFilter = userConfig.getBool("video.trilinear");
        } else {
            failed = true;
        }

        if (failed) {
            return platform->changeVideoMode(false, platform->defaultVideoMode(), 0, false, true);
        } else {
            return platform->changeVideoMode(fullscreen, videoMode, msaaMode, vsync, trilinearFilter);
        }
    }

    void Game::setupAudio(const AppConfig& userConfig)
    {
    }

    void Game::writeUserConfig(bool controlsOnly)
    {
        std::ostringstream os;

        if (!controlsOnly) {
            os << "[video]" << std::endl;
            os << "fullscreen=" << (settings.fullscreen ? "true" : "false") << std::endl;

            VideoMode vm;

            if (settings.fullscreen) {
                vm = platform->desktopVideoModes()[settings.videoMode];
            } else {
                vm = platform->winVideoModes()[settings.videoMode];
            }

            os << "mode=" << vm.width << "," << vm.height << std::endl;
            os << "msaa=" << platform->msaaModes()[settings.msaaMode] << std::endl;
            os << "vsync=" << (settings.vsync ? "true" : "false") << std::endl;
            os << "trilinear=" << (settings.trilinearFilter ? "true" : "false") << std::endl;
        }

        platform->writeUserConfig(os.str());
    }
}
