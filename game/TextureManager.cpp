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

#include "TextureManager.h"
#include "HardwareResourceManager.h"
#include "Logger.h"
#include "af3d/Assert.h"

namespace af3d
{
    TextureManager textureManager;

    template <>
    Single<TextureManager>* Single<TextureManager>::single = nullptr;

    TextureManager::~TextureManager()
    {
        runtime_assert(cachedTextures_.empty());
        runtime_assert(immediateTextures_.empty());
    }

    bool TextureManager::init()
    {
        LOG4CPLUS_DEBUG(logger(), "textureManager: init...");
        return true;
    }

    void TextureManager::shutdown()
    {
        LOG4CPLUS_DEBUG(logger(), "textureManager: shutdown...");
        runtime_assert(cachedTextures_.empty());
        runtime_assert(immediateTextures_.empty());
    }

    void TextureManager::reload()
    {
        LOG4CPLUS_DEBUG(logger(), "textureManager: reload...");
        for (const auto& kv : cachedTextures_) {
            kv.second->invalidate();
            kv.second->load();
        }
        for (auto tex : immediateTextures_) {
            tex->invalidate();
            tex->load();
        }
    }

    bool TextureManager::renderReload(HardwareContext& ctx)
    {
        LOG4CPLUS_DEBUG(logger(), "textureManager: render reload...");
        return true;
    }

    TexturePtr TextureManager::loadTexture(const std::string& path)
    {
        auto it = cachedTextures_.find(path);
        if (it != cachedTextures_.end()) {
            return it->second;
        }

        std::uint32_t width = 320;
        std::uint32_t height = 240;
        ResourceLoaderPtr myPngLoader; // = ...

        auto tex = std::make_shared<Texture>(this, path,
            hwManager.createTexture(width, height), myPngLoader);
        tex->load();
        cachedTextures_.emplace(path, tex);

        return tex;
    }

    TexturePtr TextureManager::createTexture(std::uint32_t width, std::uint32_t height,
        const ResourceLoaderPtr& loader)
    {
        auto tex = std::make_shared<Texture>(this, "",
            hwManager.createTexture(width, height), loader);
        tex->load();
        immediateTextures_.insert(tex.get());
        return tex;
    }

    void TextureManager::onTextureDestroy(Texture* tex)
    {
        immediateTextures_.erase(tex);
    }
}
