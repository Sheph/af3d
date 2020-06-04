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

        subKeys.clear();

        subKeys.push_back("none");
        subKeys.push_back("fxaa");
        subKeys.push_back("taa");

        aaMode = static_cast<AAMode>(appConfig->getStringIndex(".aaMode", subKeys));
        bloom = appConfig->getBool(".bloom");

        LOG4CPLUS_INFO(logger(), "AA mode : " << subKeys[static_cast<int>(aaMode)]);
        LOG4CPLUS_INFO(logger(), "Bloom : " << bloom);

        /*
         * physics.
         */

        physics.fixedTimestep = appConfig->getFloat("physics.fixedTimestep");
        physics.maxSteps = appConfig->getInt("physics.maxSteps");
        physics.slowmoFactor = appConfig->getFloat("physics.slowmoFactor");
        physics.debugWireframe = appConfig->getBool("physics.debug.wireframe");
        physics.debugAabb = appConfig->getBool("physics.debug.aabb");
        physics.debugContactPoints = appConfig->getBool("physics.debug.contactPoints");
        physics.debugNoDeactivation = appConfig->getBool("physics.debug.noDeactivation");
        physics.debugJoints = appConfig->getBool("physics.debug.joints");
        physics.debugJointLimits = appConfig->getBool("physics.debug.jointLimits");
        physics.debugNormals = appConfig->getBool("physics.debug.normals");
        physics.debugFrames = appConfig->getBool("physics.debug.frames");

        /*
         * imGui.
         */

        imGui.drawCursor = appConfig->getBool("ImGui.drawCursor");

        /*
         * editor.
         */

        editor.enabled = false;
        editor.playing = false;
        editor.disableSimulation = appConfig->getBool("editor.disableSimulation");
        editor.objMarkerSizeWorld = appConfig->getFloat("editor.objMarkerSizeWorld");
        editor.objMarkerSizePixels = appConfig->getInt("editor.objMarkerSizePixels");
        editor.objMarkerColorInactive = appConfig->getColor("editor.objMarkerColorInactive");
        editor.objMarkerColorHovered = appConfig->getColor("editor.objMarkerColorHovered");
        editor.objMarkerColorSelected = appConfig->getColor("editor.objMarkerColorSelected");
        editor.outlineColorInactive = appConfig->getColor("editor.outlineColorInactive");
        editor.outlineColorHovered = appConfig->getColor("editor.outlineColorHovered");
        editor.outlineColorSelected = appConfig->getColor("editor.outlineColorSelected");
        editor.lightMarkerSizePixels = appConfig->getInt("editor.lightMarkerSizePixels");
        editor.lightMarkerAlphaOff = appConfig->getFloat("editor.lightMarkerAlphaOff");
        editor.lightMarkerAlphaInactive = appConfig->getFloat("editor.lightMarkerAlphaInactive");
        editor.lightMarkerAlphaHovered = appConfig->getFloat("editor.lightMarkerAlphaHovered");
        editor.lightMarkerAlphaSelected = appConfig->getFloat("editor.lightMarkerAlphaSelected");
        editor.collisionColorOff = appConfig->getColor("editor.collisionColorOff");
        editor.collisionColorInactive = appConfig->getColor("editor.collisionColorInactive");
        editor.collisionColorHovered = appConfig->getColor("editor.collisionColorHovered");
        editor.collisionColorSelected = appConfig->getColor("editor.collisionColorSelected");
        editor.jointMarkerSizeWorld = appConfig->getFloat("editor.jointMarkerSizeWorld");
        editor.jointMarkerSizePixels = appConfig->getInt("editor.jointMarkerSizePixels");
        editor.jointMarkerColorInactive = appConfig->getColor("editor.jointMarkerColorInactive");
        editor.jointMarkerColorHovered = appConfig->getColor("editor.jointMarkerColorHovered");
        editor.jointMarkerColorSelected = appConfig->getColor("editor.jointMarkerColorSelected");
        editor.jointMarkerColorOff = appConfig->getColor("editor.jointMarkerColorOff");

        /*
         * cluster.
         */

        auto v = appConfig->getVector3f("cluster.gridSize");
        cluster.gridSize = Vector3i(v.x(), v.y(), v.z());
        v = appConfig->getVector3f("cluster.cullNumGroups");
        cluster.cullNumGroups = Vector3i(v.x(), v.y(), v.z());
        cluster.numTiles = cluster.gridSize.x() * cluster.gridSize.y() * cluster.gridSize.z();
        cluster.maxLights = appConfig->getInt("cluster.maxLights");
        cluster.maxLightsPerTile = appConfig->getInt("cluster.maxLightsPerTile");
        cluster.maxProbes = appConfig->getInt("cluster.maxProbes");
        cluster.maxProbesPerTile = appConfig->getInt("cluster.maxProbesPerTile");

        /*
         * light probe.
         */

        lightProbe.irradianceResolution = appConfig->getInt("light probe.irradianceResolution");
        lightProbe.specularResolution = appConfig->getInt("light probe.specularResolution");
        lightProbe.specularMipLevels = appConfig->getInt("light probe.specularMipLevels");
    }
}
