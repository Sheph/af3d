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

#include "Texture.h"
#include "TextureManager.h"
#include "Renderer.h"
#include "Logger.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(Texture, Resource)
    ACLASS_DEFINE_END(Texture)

    namespace
    {
        class TextureUploader : public ResourceLoader
        {
        public:
            TextureUploader(GLint internalFormat,
                GLenum format,
                GLenum type,
                std::vector<Byte>&& pixels,
                bool genMipmap)
            : internalFormat_(internalFormat),
              format_(format),
              type_(type),
              pixels_(std::move(pixels)),
              genMipmap_(genMipmap)
            {
            }

            void load(Resource& res, HardwareContext& ctx) override
            {
                Texture& texture = static_cast<Texture&>(res);

                LOG4CPLUS_DEBUG(logger(), "textureManager: loading " << texture.width() << "x" << texture.height() << (genMipmap_ ? " + mipmap..." : "..."));

                texture.hwTex()->upload(internalFormat_, format_, type_,
                    reinterpret_cast<const GLvoid*>(&pixels_[0]), genMipmap_, ctx);

                pixels_.clear();
            }

        private:
            GLint internalFormat_;
            GLenum format_;
            GLenum type_;
            std::vector<Byte> pixels_;
            bool genMipmap_;
        };
    }

    Texture::Texture(TextureManager* mgr, const std::string& name,
        const HardwareTexturePtr& hwTex,
        const ResourceLoaderPtr& loader)
    : Resource(AClass_Texture, name, loader),
      mgr_(mgr),
      hwTex_(hwTex)
    {
    }

    Texture::~Texture()
    {
        mgr_->onTextureDestroy(this);
    }

    const AClass& Texture::staticKlass()
    {
        return AClass_Texture;
    }

    AObjectPtr Texture::create(const APropertyValueMap& propVals)
    {
        return TexturePtr();
    }

    void Texture::upload(GLint internalFormat, GLenum format, GLenum type, std::vector<Byte>&& pixels, bool genMipmap)
    {
        load(std::make_shared<TextureUploader>(internalFormat, format, type, std::move(pixels), genMipmap));
    }

    void Texture::download(GLenum format, GLenum type, std::vector<Byte>& pixels)
    {
        auto tex = hwTex_;
        renderer.scheduleHwOpSync([tex, format, type, &pixels](HardwareContext& ctx) {
            tex->download(format, type, &pixels[0], ctx);
        });
    }
}
