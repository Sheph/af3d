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
#include "Platform.h"
#include "Logger.h"
#include "Settings.h"
#include "af3d/Assert.h"
#include "af3d/PNGDecoder.h"

namespace af3d
{
    namespace
    {
        class TextureGenerator : public ResourceLoader
        {
        public:
            explicit TextureGenerator(const std::string& path)
            : path_(path)
            {
            }

            bool init(std::uint32_t& width, std::uint32_t& height)
            {
                bool res = initImpl();

                if (res) {
                    width = decoder_->width();
                    height = decoder_->height();
                }

                return res;
            }

            void load(Resource& res, HardwareContext& ctx) override
            {
                LOG4CPLUS_DEBUG(logger(), "textureManager: loading " << path_ << "...");

                Texture& texture = static_cast<Texture&>(res);

                if (!decoder_) {
                    runtime_assert(initImpl());
                }

                std::vector<Byte> data;

                if (decoder_->decode(data)) {
                    texture.hwTex()->upload(GL_RGBA, decoder_->hasAlpha() ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE,
                        reinterpret_cast<const GLvoid*>(&data[0]), true, ctx);
                }

                decoder_.reset();
                is_.reset();
            }

        private:
            bool initImpl()
            {
                auto is = std::make_shared<PlatformIFStream>(path_);
                auto decoder = std::make_shared<PNGDecoder>(path_, *is);

                if (!decoder->init(false)) {
                    return false;
                }

                is_ = is;
                decoder_ = decoder;

                return true;
            }

            std::string path_;
            std::shared_ptr<PlatformIFStream> is_;
            std::shared_ptr<PNGDecoder> decoder_;
        };

        class OffscreenTextureGenerator : public ResourceLoader
        {
        public:
            explicit OffscreenTextureGenerator(float scale)
            : scale_(scale)
            {
            }

            void load(Resource& res, HardwareContext& ctx) override
            {
                Texture& texture = static_cast<Texture&>(res);

                if (scale_ > 0.0f) {
                    std::uint32_t newWidth = static_cast<float>(settings.viewWidth) / scale_;
                    std::uint32_t newHeight = static_cast<float>(settings.viewHeight) / scale_;

                    if ((newWidth != texture.width()) && (newHeight != texture.height())) {
                        LOG4CPLUS_DEBUG(logger(), "textureManager: offscreen scaled texture (recreate) " << newWidth << "x" << newHeight << "...");
                        auto hwTex = hwManager.createTexture(newWidth, newHeight);
                        texture.setHwTex(hwTex);
                    } else {
                        LOG4CPLUS_DEBUG(logger(), "textureManager: offscreen scaled texture " << newWidth << "x" << newHeight << "...");
                    }
                    texture.hwTex()->upload(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, NULL, false, ctx);
                } else {
                    LOG4CPLUS_DEBUG(logger(), "textureManager: offscreen fixed texture " << texture.width() << "x" << texture.height() << "...");
                    texture.hwTex()->upload(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, NULL, false, ctx);
                }
            }

        private:
            float scale_ = 0.0f;
        };
    }

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
        white1x1_ = createTexture(1, 1);
        return true;
    }

    void TextureManager::shutdown()
    {
        LOG4CPLUS_DEBUG(logger(), "textureManager: shutdown...");
        white1x1_.reset();
        runtime_assert(immediateTextures_.empty());
        cachedTextures_.clear();
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
        std::vector<Byte> data{255, 255, 255, 255};
        white1x1_->upload(GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, std::move(data), false);
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

        auto loader = std::make_shared<TextureGenerator>(path);

        std::uint32_t width;
        std::uint32_t height;

        if (!loader->init(width, height)) {
            runtime_assert(path != "bad.png");
            return loadTexture("bad.png");
        }

        auto tex = std::make_shared<Texture>(this, path,
            hwManager.createTexture(width, height), loader);
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

    TexturePtr TextureManager::createRenderTexture(float scale)
    {
        btAssert(scale > 0.0f);

        std::uint32_t width = static_cast<float>(settings.viewWidth) / scale;
        std::uint32_t height = static_cast<float>(settings.viewHeight) / scale;

        return createTexture(width, height, std::make_shared<OffscreenTextureGenerator>(scale));
    }

    TexturePtr TextureManager::createRenderTexture(std::uint32_t width, std::uint32_t height)
    {
        return createTexture(width, height, std::make_shared<OffscreenTextureGenerator>(0.0f));
    }

    void TextureManager::onTextureDestroy(Texture* tex)
    {
        immediateTextures_.erase(tex);
    }
}
