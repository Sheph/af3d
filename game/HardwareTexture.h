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

#ifndef _HARDWARE_TEXTURE_H_
#define _HARDWARE_TEXTURE_H_

#include "HardwareResource.h"

namespace af3d
{
    enum TextureType
    {
        TextureType2D = 0,
        TextureTypeCubeMap,
        TextureTypeCubeMapArray,
        TextureTypeMax = TextureTypeCubeMapArray
    };

    enum TextureCubeFace
    {
        TextureCubeXP = 0,
        TextureCubeXN = 1,
        TextureCubeYP = 2,
        TextureCubeYN = 3,
        TextureCubeZP = 4,
        TextureCubeZN = 5,
        TextureCubeFaceMax = TextureCubeZN
    };

    enum TextureFormat
    {
        TextureFormatAny = 0,
        TextureFormatRG = 1,
        TextureFormatRGBA = 2,
        TextureFormatMax = TextureFormatRGBA
    };

    const btMatrix3x3& textureCubeFaceBasis(TextureCubeFace face);

    class HardwareTexture : public HardwareResource
    {
    public:
        HardwareTexture(HardwareResourceManager* mgr, TextureType type, std::uint32_t width, std::uint32_t height, std::uint32_t depth,
            TextureFormat format);
        ~HardwareTexture();

        static GLenum glType(TextureType type);

        static GLenum glCubeFace(TextureCubeFace face);

        inline TextureType type() const { return type_; }

        inline std::uint32_t width() const { return width_; }

        inline std::uint32_t height() const { return height_; }

        inline std::uint32_t depth() const { return depth_; }

        inline TextureFormat format() const { return format_; }

        GLuint id(HardwareContext& ctx) const override;

        void upload(GLint internalFormat, GLenum format, GLenum dataType, const GLvoid* pixels, bool genMipmap, GLint level, HardwareContext& ctx);

        void uploadCompressed(GLint internalFormat, const GLvoid* data, GLsizei dataSize, bool genMipmap, GLint level, HardwareContext& ctx);

        void update(GLenum format, GLenum dataType, const GLvoid* pixels, GLint level, GLint layer, HardwareContext& ctx);

        void download(GLenum format, GLenum dataType, GLvoid* pixels, HardwareContext& ctx);

        void generateMipmap(HardwareContext& ctx);

    private:
        void doInvalidate(HardwareContext& ctx) override;

        void createTexture();

        void uploadCubeFace(TextureCubeFace face, GLint internalFormat, GLenum format, GLenum dataType, const GLvoid* pixels, bool genMipmap, GLint level, HardwareContext& ctx);

        TextureType type_;
        std::uint32_t width_;
        std::uint32_t height_;
        std::uint32_t depth_;
        TextureFormat format_;
        GLuint id_ = 0;
    };

    using HardwareTexturePtr = std::shared_ptr<HardwareTexture>;
}

#endif
