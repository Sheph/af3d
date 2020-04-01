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

#include "AssetManager.h"
#include "TextureManager.h"
#include "Logger.h"
#include "Platform.h"
#include "AJsonReader.h"
#include <log4cplus/ndc.h>

namespace af3d
{
    AssetManager assetManager;

    template <>
    Single<AssetManager>* Single<AssetManager>::single = NULL;

    bool AssetManager::init()
    {
        LOG4CPLUS_DEBUG(logger(), "assetManager: init...");
        return true;
    }

    void AssetManager::shutdown()
    {
        LOG4CPLUS_DEBUG(logger(), "assetManager: shutdown...");
        tpsMap_.clear();
        sceneAssetMap_.clear();
    }

    Image AssetManager::getImage(const std::string& name)
    {
        std::string::size_type pos = name.find('/');

        if (pos == std::string::npos) {
            auto texture = textureManager.loadTexture(name);

            return Image(texture, 0, 0, texture->width(), texture->height());
        } else {
            std::string tpsName = name.substr(0, pos) + ".json";
            std::string fileName = name.substr(pos + 1);

            auto it = tpsMap_.find(tpsName);

            if (it == tpsMap_.end()) {
                PlatformIFStream is(tpsName);

                if (!is) {
                    it = tpsMap_.insert(std::make_pair(tpsName, TPSPtr())).first;
                } else {
                    TPSPtr tps = TPS::fromStream(tpsName, is);

                    it = tpsMap_.insert(std::make_pair(tpsName, tps)).first;
                }
            }

            if (!it->second) {
                auto texture = textureManager.loadTexture(name);

                return Image(texture, 0, 0, texture->width(), texture->height());
            }

            auto texture = textureManager.loadTexture(it->second->imageFileName());

            auto entry = it->second->entry(fileName, false);

            if (entry.valid()) {
                return Image(texture, entry.x, entry.y, entry.width, entry.height);
            } else {
                return Image(texture, 0, 0, texture->width(), texture->height());
            }
        }
    }

    DrawablePtr AssetManager::getDrawable(const std::string& name)
    {
        return std::make_shared<Drawable>(getImage(name));
    }

    SceneAssetPtr AssetManager::getSceneAsset(const std::string& name)
    {
        log4cplus::NDCContextCreator ndc(name);

        auto it = sceneAssetMap_.find(name);

        if (it == sceneAssetMap_.end()) {
            Json::Value jsonValue;

            PlatformIFStream is(name);

            if (is) {
                std::string jsonStr;
                if (readStream(is, jsonStr)) {
                    Json::Reader reader;
                    if (!reader.parse(jsonStr, jsonValue)) {
                        LOG4CPLUS_ERROR(logger(), "Failed to parse JSON: " << reader.getFormattedErrorMessages());
                    }
                } else {
                    LOG4CPLUS_ERROR(logger(), "Error reading file");
                }
            } else {
                LOG4CPLUS_ERROR(logger(), "Cannot open file");
            }

            it = sceneAssetMap_.emplace(name, jsonValue).first;
        }

        AJsonSerializerDefault defS;

        AJsonReader reader(defS);
        auto res = reader.read(it->second);

        if (res.size() != 1) {
            if (!it->second.isNull()) {
                if (res.empty()) {
                    LOG4CPLUS_ERROR(logger(), "No objects inside ?");
                } else {
                    LOG4CPLUS_ERROR(logger(), "Multiple scenes in a single file ?");
                }
            }
            return SceneAssetPtr();
        }

        auto asset = aobjectCast<SceneAsset>(res.back());
        if (!asset) {
            LOG4CPLUS_ERROR(logger(), "Not a scene asset");
        }

        return asset;
    }
}
