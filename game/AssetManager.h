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

#ifndef _ASSETMANAGER_H_
#define _ASSETMANAGER_H_

#include "Drawable.h"
#include "SceneAsset.h"
#include "af3d/Single.h"
#include "af3d/TPS.h"
#include "json/json.h"

namespace af3d
{
    class AssetManager : public Single<AssetManager>
    {
    public:
        AssetManager() = default;
        ~AssetManager() = default;

        bool init();

        void shutdown();

        Image getImage(const std::string& name);

        DrawablePtr getDrawable(const std::string& name);

        SceneAssetPtr getSceneAsset(const std::string& name, bool editor = false);

        SceneAssetPtr getSceneObjectAsset(const std::string& name);

    private:
        using TPSMap = std::unordered_map<std::string, TPSPtr>;
        using SceneAssetMap = std::unordered_map<std::string, Json::Value>;

        TPSMap tpsMap_;
        SceneAssetMap sceneAssetMap_;
    };

    extern AssetManager assetManager;
}

#endif
