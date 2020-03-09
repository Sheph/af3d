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

#ifndef _TEXTURE_H_
#define _TEXTURE_H_

#include "Resource.h"
#include "HardwareTexture.h"

namespace af3d
{
    class TextureManager;

    class Texture : public Resource
    {
    public:
        Texture(TextureManager* mgr, const std::string& name,
            const HardwareTexturePtr& hwTex,
            const ResourceLoaderPtr& loader = ResourceLoaderPtr());
        ~Texture();

        inline const HardwareTexturePtr& hwTex() const { return hwTex_; }

        inline std::uint32_t width() const { return hwTex_->width(); }

        inline std::uint32_t height() const { return hwTex_->height(); }

        void upload(GLint internalFormat, GLenum format, GLenum type, std::vector<Byte>&& pixels);

    private:
        TextureManager* mgr_;
        HardwareTexturePtr hwTex_;
    };

    using TexturePtr = std::shared_ptr<Texture>;
}

#endif
