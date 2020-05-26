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
#include "Settings.h"
#include <log4cplus/ndc.h>
#include <fstream>

namespace af3d
{
    AssetManager assetManager;

    template <>
    Single<AssetManager>* Single<AssetManager>::single = nullptr;

    bool AssetManager::init()
    {
        LOG4CPLUS_DEBUG(logger(), "assetManager: init...");
        processAssetsJson("assets-textures.json", [this](const std::string& name, AssetData& data, const Json::Value& v) {
            data.tex = std::make_shared<AssetTexture>();
            data.tex->setName(name);
            if (v["srgb"].isBool()) {
                data.tex->setSRGB(v["srgb"].asBool());
            }
            LOG4CPLUS_TRACE(logger(), "texture \"" << name << "\" sRGB = " << data.tex->isSRGB());
        });
        return true;
    }

    void AssetManager::shutdown()
    {
        LOG4CPLUS_DEBUG(logger(), "assetManager: shutdown...");
        assetMap_.clear();
        tpsMap_.clear();
        sceneAssetMap_.clear();
        collisionMatrixMap_.clear();
    }

    const AssetTexturePtr& AssetManager::getAssetTexture(const std::string& name)
    {
        AssetData& data = assetMap_[name];
        if (!data.tex) {
            data.tex = std::make_shared<AssetTexture>();
            data.tex->setName(name);
        }
        return data.tex;
    }

    const AssetModelPtr& AssetManager::getAssetModel(const std::string& name)
    {
        AssetData& data = assetMap_[name];
        if (!data.model) {
            data.model = std::make_shared<AssetModel>();
            data.model->setName(name);
        }
        return data.model;
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

    SceneAssetPtr AssetManager::getSceneAsset(const std::string& name, bool editor)
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

        SceneAssetPtr asset;

        {
            AJsonSerializerDefault defS;

            AJsonReader reader(defS, editor);
            auto res = reader.read(it->second);

            if (res.size() != 1) {
                if (!it->second.isNull()) {
                    if (res.empty()) {
                        LOG4CPLUS_ERROR(logger(), "No objects inside ?");
                    } else {
                        LOG4CPLUS_ERROR(logger(), "Multiple scenes in a single file ?");
                    }
                }
            } else {
                asset = aobjectCast<SceneAsset>(res.back());
                if (!asset) {
                    LOG4CPLUS_ERROR(logger(), "Not a scene asset");
                }
            }
        }

        if (settings.editor.enabled) {
            // Don't cache scenes in editor.
            sceneAssetMap_.erase(it);
        }

        return asset;
    }

    SceneAssetPtr AssetManager::getSceneObjectAsset(const std::string& name)
    {
        auto sa = getSceneAsset(name);
        if (!sa) {
            return SceneAssetPtr();
        }

        SceneObjectPtr obj;

        if (sa->root()) {
            obj = sa->root();
        } else {
            obj = std::make_shared<SceneObject>();
        }

        sa->apply(obj);
        sa->setRoot(obj);

        return sa;
    }

    CollisionMatrixPtr AssetManager::getCollisionMatrix(const std::string& name)
    {
        auto it = collisionMatrixMap_.find(name);
        if (it == collisionMatrixMap_.end()) {
            PlatformIFStream is(name);
            auto cm = CollisionMatrix::fromStream(name, is);
            if (!cm) {
                return cm;
            }
            cm->setName(name);
            it = collisionMatrixMap_.insert(std::make_pair(name, cm)).first;
        }
        return it->second;
    }

    void AssetManager::saveCollisionMatrix(const CollisionMatrixPtr& cm)
    {
        collisionMatrixMap_[cm->name()] = cm;
        std::ofstream os(platform->assetsPath() + "/" + cm->name(),
            std::ios_base::binary | std::ios_base::out | std::ios_base::trunc);
        os << Json::FastWriter().write(cm->toJsonValue());
    }

    void AssetManager::processAssetsJson(const std::string& path, const AssetsJsonFn& fn)
    {
        PlatformIFStream is(path);

        log4cplus::NDCContextCreator ndc(path);

        if (!is) {
            LOG4CPLUS_ERROR(logger(), "Cannot open file");
            return;
        }

        std::string jsonStr;
        if (!readStream(is, jsonStr)) {
            LOG4CPLUS_ERROR(logger(), "Error reading file");
            return;
        }

        Json::Value jsonValue;
        Json::Reader reader;
        if (!reader.parse(jsonStr, jsonValue)) {
            LOG4CPLUS_ERROR(logger(), "Failed to parse JSON: " << reader.getFormattedErrorMessages());
            return;
        }

        if (!jsonValue.isObject()) {
            LOG4CPLUS_ERROR(logger(), "Not a json object");
            return;
        }

        for (auto it = jsonValue.begin(); it != jsonValue.end(); ++it) {
            const Json::Value& key = it.key();
            if (!key.isString()) {
                LOG4CPLUS_WARN(logger(), "\"" << key << "\" is not a string");
                continue;
            }
            const Json::Value& v = *it;
            if (!v.isObject()) {
                LOG4CPLUS_WARN(logger(), "\"" << key.asString() << "\" value is not an object");
                continue;
            }
            fn(key.asString(), assetMap_[key.asString()], v);
        }
    }
}
