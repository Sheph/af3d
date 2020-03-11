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

#ifndef _TEXTURE_MANAGER_H_
#define _TEXTURE_MANAGER_H_

#include "ResourceManager.h"
#include "Texture.h"
#include "af3d/Single.h"
#include <unordered_map>
#include <unordered_set>

namespace af3d
{
    class TextureManager : public ResourceManager,
                           public Single<TextureManager>
    {
    public:
        TextureManager() = default;
        ~TextureManager();

        bool init() override;

        void shutdown() override;

        void reload() override;

        bool renderReload(HardwareContext& ctx) override;

        TexturePtr loadTexture(const std::string& path);

        TexturePtr createTexture(std::uint32_t width, std::uint32_t height);

        void onTextureDestroy(Texture* tex);

    private:
        using CachedTextures = std::unordered_map<std::string, TexturePtr>;
        using ImmediateTextures = std::unordered_set<Texture*>;

        CachedTextures cachedTextures_;
        ImmediateTextures immediateTextures_;
    };

    extern TextureManager textureManager;
}

#endif