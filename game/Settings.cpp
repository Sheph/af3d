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

#include "Settings.h"
#include "Utils.h"
#include "Logger.h"
#include <cmath>

namespace af3d
{
    Settings settings;

    template <>
    Single<Settings>* Single<Settings>::single = nullptr;

    void Settings::init(const AppConfigPtr& appConfig)
    {
        std::vector<std::string> subKeys;

        /*
         * general.
         */

#ifdef _WIN32
        assets = appConfig->getString(".assets");
#endif
        developer = appConfig->getInt(".developer");
        viewWidth = appConfig->getInt(".viewWidth");
        viewHeight = appConfig->getInt(".viewHeight");
        profileReportTimeoutMs = appConfig->getInt(".profileReportTimeoutMs");

        float tmp = appConfig->getFloat(".maxFPS");

        if (tmp > 0) {
            minRenderDt = std::floor(1000000.0f / tmp);
        } else {
            minRenderDt = 0;
        }

        viewAspect = static_cast<float>(viewWidth) / viewHeight;
        videoMode = -1;
        msaaMode = -1;
        vsync = false;
        fullscreen = false;
        trilinearFilter = false;
        viewX = 0;
        viewY = 0;

        subKeys = appConfig->getSubKeys(".winVideoMode");

        if (subKeys.empty()) {
            LOG4CPLUS_ERROR(logger(), "Config values for \"winVideoMode\" not found");
        }

        for (const auto& key : subKeys) {
            Vector2f tmp = appConfig->getVector2f(std::string(".winVideoMode.") + key);
            winVideoModes.insert(VideoMode(tmp.x(), tmp.y()));
        }

        /*
         * editor.
         */

        editor.enabled = false;
        editor.objMarkerSizeWorld = appConfig->getFloat("editor.objMarkerSizeWorld");
        editor.objMarkerSizePixels = appConfig->getInt("editor.objMarkerSizePixels");
        editor.objMarkerColorInactive = appConfig->getColor("editor.objMarkerColorInactive");
    }
}
