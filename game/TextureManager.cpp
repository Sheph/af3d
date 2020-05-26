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
#include "AssetManager.h"
#include "Platform.h"
#include "Logger.h"
#include "Settings.h"
#include "af3d/Assert.h"
#include "af3d/ImageReader.h"

namespace af3d
{
    namespace
    {
        class TextureGenerator : public ResourceLoader
        {
        public:
            TextureGenerator(const std::string& path, bool isSRGB)
            : path_(path),
              isSRGB_(isSRGB)
            {
            }

            bool init(std::uint32_t& width, std::uint32_t& height)
            {
                bool res = initImpl();

                if (res) {
                    width = info_.width;
                    // FIXME: Currently assume that all hdr files are equirect cubemaps...
                    height = (((info_.flags & ImageReader::FlagHDR) != 0) && ((info_.flags & ImageReader::FlagSRGB) == 0)) ? (info_.width / 2) : info_.height;
                }

                return res;
            }

            void load(Resource& res, HardwareContext& ctx) override
            {
                Texture& texture = static_cast<Texture&>(res);

                if (!reader_) {
                    runtime_assert(initImpl());
                }

                // FIXME: Currently assume that all hdr files are equirect cubemaps...
                std::uint32_t newHeight = (((info_.flags & ImageReader::FlagHDR) != 0) && ((info_.flags & ImageReader::FlagSRGB) == 0)) ? (info_.width / 2) : info_.height;

                if ((info_.width != texture.width()) && (newHeight != texture.height())) {
                    LOG4CPLUS_DEBUG(logger(), "textureManager: loading (recreate) " << info_.width << "x" << info_.height
                        << " " << path_ << ", format = " << ImageReader::glFormatStr(info_.format) << ", SRGB = " << ((info_.flags & ImageReader::FlagSRGB) != 0) << "...");
                    auto hwTex = hwManager.createTexture(texture.type(), info_.width, newHeight);
                    texture.setHwTex(hwTex);
                } else {
                    LOG4CPLUS_DEBUG(logger(), "textureManager: loading " << info_.width << "x" << info_.height
                        << " " << path_ << ", format = " << ImageReader::glFormatStr(info_.format) << ", SRGB = " << ((info_.flags & ImageReader::FlagSRGB) != 0) << "...");
                }

                std::vector<Byte> data;
                GLint internalFormat = 0;

                if ((info_.flags & ImageReader::FlagHDR) != 0) {
                    if (info_.format == GL_RGB) {
                        internalFormat = GL_RGB16F;
                    } else {
                        runtime_assert(false);
                    }

                    runtime_assert(info_.numMipLevels == 1);

                    if (!reader_->read(0, data)) {
                        reader_.reset();
                        is_.reset();
                        return;
                    }

                    if (newHeight != info_.height) {
                        texture.hwTex()->upload(internalFormat, info_.format, GL_FLOAT,
                            nullptr, true, 0, ctx);
                    }

                    // FIXME: Currently assume that all hdr files are equirect cubemaps...
                    std::uint32_t numLevels = 0;
                    size_t sz = 0;
                    while (true) {
                        size_t curSz = textureMipSize(info_.width, numLevels) * textureMipSize(newHeight, numLevels) * 3 * sizeof(float);
                        if ((sz + curSz > data.size()) || (curSz == 0)) {
                            break;
                        }
                        texture.hwTex()->upload(internalFormat, info_.format, GL_FLOAT,
                            reinterpret_cast<const GLvoid*>(&data[0] + sz), false, numLevels, ctx);
                        sz += curSz;
                        ++numLevels;
                    }
                } else {
                    bool compressed = false;
                    if (info_.format == GL_RED) {
                        internalFormat = GL_RED;
                    } else if (info_.format == GL_RGB) {
                        internalFormat = ((info_.flags & ImageReader::FlagSRGB) != 0) ? GL_SRGB : GL_RGB;
                    } else if (info_.format == GL_RGBA) {
                        internalFormat = ((info_.flags & ImageReader::FlagSRGB) != 0) ? GL_SRGB_ALPHA : GL_RGBA;
                    } else if (info_.format == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) {
                        internalFormat = ((info_.flags & ImageReader::FlagSRGB) != 0) ? GL_COMPRESSED_SRGB_S3TC_DXT1_EXT : info_.format;
                        compressed = true;
                    } else if (info_.format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) {
                        internalFormat = ((info_.flags & ImageReader::FlagSRGB) != 0) ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT : info_.format;
                        compressed = true;
                    } else if (info_.format == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT) {
                        internalFormat = ((info_.flags & ImageReader::FlagSRGB) != 0) ? GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT : info_.format;
                        compressed = true;
                    } else if (info_.format == GL_COMPRESSED_RG_RGTC2) {
                        btAssert((info_.flags & ImageReader::FlagSRGB) == 0);
                        internalFormat = info_.format;
                        compressed = true;
                    } else {
                        runtime_assert(false);
                    }

                    if (info_.numMipLevels > 1) {
                        for (std::uint32_t mip = 0; mip < info_.numMipLevels; ++mip) {
                            if (!reader_->read(mip, data)) {
                                reader_.reset();
                                is_.reset();
                                return;
                            }
                            if (compressed) {
                                texture.hwTex()->uploadCompressed(internalFormat,
                                    reinterpret_cast<const GLvoid*>(&data[0]), data.size(), false, mip, ctx);
                            } else {
                                texture.hwTex()->upload(internalFormat, info_.format, GL_UNSIGNED_BYTE,
                                    reinterpret_cast<const GLvoid*>(&data[0]), false, mip, ctx);
                            }
                        }
                    } else {
                        if (!reader_->read(0, data)) {
                            reader_.reset();
                            is_.reset();
                            return;
                        }

                        if (compressed) {
                            texture.hwTex()->uploadCompressed(internalFormat,
                                reinterpret_cast<const GLvoid*>(&data[0]), data.size(), true, 0, ctx);
                        } else {
                            texture.hwTex()->upload(internalFormat, info_.format, GL_UNSIGNED_BYTE,
                                reinterpret_cast<const GLvoid*>(&data[0]), true, 0, ctx);
                        }
                    }
                }

                reader_.reset();
                is_.reset();
            }

        private:
            bool initImpl()
            {
                auto is = std::make_shared<PlatformIFStream>(path_);
                auto reader = std::make_shared<ImageReader>(path_, *is);

                if (!reader->init(info_)) {
                    return false;
                }

                if (isSRGB_) {
                    info_.flags |= ImageReader::FlagSRGB;
                }

                is_ = is;
                reader_ = reader;

                return true;
            }

            std::string path_;
            bool isSRGB_;
            std::shared_ptr<PlatformIFStream> is_;
            std::shared_ptr<ImageReader> reader_;
            ImageReader::Info info_;
        };

        class OffscreenTextureGenerator : public ResourceLoader
        {
        public:
            OffscreenTextureGenerator(float scale, GLint internalFormat,
                GLenum format,
                GLenum dataType,
                bool genMipmap,
                std::vector<Byte>&& pixels)
            : scale_(scale),
              internalFormat_(internalFormat),
              format_(format),
              dataType_(dataType),
              genMipmap_(genMipmap),
              pixels_(std::move(pixels))
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
                        auto hwTex = hwManager.createTexture(texture.type(), newWidth, newHeight);
                        texture.setHwTex(hwTex);
                    } else {
                        LOG4CPLUS_DEBUG(logger(), "textureManager: offscreen scaled texture " << newWidth << "x" << newHeight << "...");
                    }
                    texture.hwTex()->upload(internalFormat_, format_, dataType_, (pixels_.empty() ? nullptr : &pixels_[0]), genMipmap_, 0, ctx);
                } else {
                    LOG4CPLUS_DEBUG(logger(), "textureManager: offscreen fixed texture " << texture.width() << "x" << texture.height() << "...");
                    texture.hwTex()->upload(internalFormat_, format_, dataType_, (pixels_.empty() ? nullptr : &pixels_[0]), genMipmap_, 0, ctx);
                }
            }

        private:
            float scale_ = 0.0f;
            GLint internalFormat_;
            GLenum format_;
            GLenum dataType_;
            bool genMipmap_;
            std::vector<Byte> pixels_;
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
        white1x1_ = createTexture(TextureType2D, 1, 1);
        black1x1_ = createTexture(TextureType2D, 1, 1);
        return true;
    }

    void TextureManager::shutdown()
    {
        LOG4CPLUS_DEBUG(logger(), "textureManager: shutdown...");
        white1x1_.reset();
        black1x1_.reset();
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
        std::vector<Byte> dataWhite{255, 255, 255, 255};
        white1x1_->upload(GL_SRGB_ALPHA, GL_RGBA, GL_UNSIGNED_BYTE, std::move(dataWhite), false);
        std::vector<Byte> dataBlack{0, 0, 0, 0};
        black1x1_->upload(GL_SRGB_ALPHA, GL_RGBA, GL_UNSIGNED_BYTE, std::move(dataBlack), false);
    }

    bool TextureManager::renderReload(HardwareContext& ctx)
    {
        LOG4CPLUS_DEBUG(logger(), "textureManager: render reload...");
        return true;
    }

    TexturePtr TextureManager::loadTexture(const std::string& path, bool fallback)
    {
        auto it = cachedTextures_.find(path);
        if (it != cachedTextures_.end()) {
            return it->second;
        }

        auto loader = std::make_shared<TextureGenerator>(path,
            assetManager.getAssetTexture(path)->isSRGB());

        std::uint32_t width;
        std::uint32_t height;

        if (!loader->init(width, height)) {
            runtime_assert(path != "bad.png");
            return fallback ? loadTexture("bad.png") : TexturePtr();
        }

        auto tex = std::make_shared<Texture>(this, path,
            hwManager.createTexture(TextureType2D, width, height), loader);
        tex->load();
        cachedTextures_.emplace(path, tex);

        return tex;
    }

    TexturePtr TextureManager::createTexture(TextureType type, std::uint32_t width, std::uint32_t height,
        const ResourceLoaderPtr& loader)
    {
        auto tex = std::make_shared<Texture>(this, "",
            hwManager.createTexture(type, width, height), loader);
        tex->load();
        immediateTextures_.insert(tex.get());
        return tex;
    }

    TexturePtr TextureManager::createRenderTextureScaled(TextureType type, float scale, GLint internalFormat, GLenum format, GLenum dataType,
        bool genMipmap, std::vector<Byte>&& pixels)
    {
        btAssert(scale > 0.0f);

        std::uint32_t width = static_cast<float>(settings.viewWidth) / scale;
        std::uint32_t height = static_cast<float>(settings.viewHeight) / scale;

        return createTexture(type, width, height,
            std::make_shared<OffscreenTextureGenerator>(scale, internalFormat, format, dataType, genMipmap, std::move(pixels)));
    }

    TexturePtr TextureManager::createRenderTexture(TextureType type, std::uint32_t width, std::uint32_t height, GLint internalFormat, GLenum format, GLenum dataType,
        bool genMipmap, std::vector<Byte>&& pixels)
    {
        return createTexture(type, width, height,
            std::make_shared<OffscreenTextureGenerator>(0.0f, internalFormat, format, dataType, genMipmap, std::move(pixels)));
    }

    void TextureManager::onTextureDestroy(Texture* tex)
    {
        immediateTextures_.erase(tex);
    }
}
