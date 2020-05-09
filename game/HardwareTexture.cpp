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

#include "HardwareTexture.h"
#include "HardwareContext.h"

namespace af3d
{
    const btMatrix3x3& textureCubeFaceBasis(TextureCubeFace face)
    {
        static const std::array<btMatrix3x3, TextureCubeFaceMax + 1> arr = {
            makeLookBasis(btVector3(1.0f, 0.0f, 0.0f), btVector3(0.0f, -1.0f, 0.0f)),
            makeLookBasis(btVector3(-1.0f, 0.0f, 0.0f), btVector3(0.0f, -1.0f, 0.0f)),
            makeLookBasis(btVector3(0.0f, 1.0f, 0.0f), btVector3(0.0f, 0.0f, 1.0f)),
            makeLookBasis(btVector3(0.0f, -1.0f, 0.0f), btVector3(0.0f, 0.0f, -1.0f)),
            makeLookBasis(btVector3(0.0f, 0.0f, 1.0f), btVector3(0.0f, -1.0f, 0.0f)),
            makeLookBasis(btVector3(0.0f, 0.0f, -1.0f), btVector3(0.0f, -1.0f, 0.0f))
        };
        return arr[face];
    }

    HardwareTexture::HardwareTexture(HardwareResourceManager* mgr, TextureType type, std::uint32_t width, std::uint32_t height)
    : HardwareResource(mgr),
      type_(type),
      width_(width),
      height_(height)
    {
    }

    HardwareTexture::~HardwareTexture()
    {
        GLuint id = id_;
        if (id != 0) {
            cleanup([id](HardwareContext& ctx) {
                ctx.deleteTexture(id);
            });
        } else {
            cleanup();
        }
    }

    GLenum HardwareTexture::glType(TextureType type)
    {
        return (type == TextureType2D) ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP;
    }

    GLenum HardwareTexture::glCubeFace(TextureCubeFace face)
    {
        switch (face) {
        case TextureCubeXN: return GL_TEXTURE_CUBE_MAP_NEGATIVE_X;
        case TextureCubeYP: return GL_TEXTURE_CUBE_MAP_POSITIVE_Y;
        case TextureCubeYN: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Y;
        case TextureCubeZP: return GL_TEXTURE_CUBE_MAP_POSITIVE_Z;
        case TextureCubeZN: return GL_TEXTURE_CUBE_MAP_NEGATIVE_Z;
        default:
            btAssert(false);
        case TextureCubeXP: return GL_TEXTURE_CUBE_MAP_POSITIVE_X;
        }
    }

    void HardwareTexture::invalidate(HardwareContext& ctx)
    {
        id_ = 0;
    }

    GLuint HardwareTexture::id(HardwareContext& ctx) const
    {
        return id_;
    }

    void HardwareTexture::upload(GLint internalFormat, GLenum format, GLenum dataType, const GLvoid* pixels, bool genMipmap, HardwareContext& ctx)
    {
        createTexture();
        ctx.bindTexture(type_, id_);
        if (type_ == TextureType2D) {
            ogl.TexImage2D(GL_TEXTURE_2D, 0, internalFormat, width_, height_, 0, format, dataType, pixels);
            if (genMipmap) {
                ogl.GenerateMipmap(GL_TEXTURE_2D);
            }
        } else {
            uploadCubeFace(TextureCubeXP, internalFormat, format, dataType, pixels, false, ctx);
            uploadCubeFace(TextureCubeXN, internalFormat, format, dataType, pixels, false, ctx);
            uploadCubeFace(TextureCubeYP, internalFormat, format, dataType, pixels, false, ctx);
            uploadCubeFace(TextureCubeYN, internalFormat, format, dataType, pixels, false, ctx);
            uploadCubeFace(TextureCubeZP, internalFormat, format, dataType, pixels, false, ctx);
            uploadCubeFace(TextureCubeZN, internalFormat, format, dataType, pixels, false, ctx);
            if (genMipmap) {
                ogl.GenerateMipmap(GL_TEXTURE_CUBE_MAP);
            }
        }
    }

    void HardwareTexture::uploadCubeFace(TextureCubeFace face, GLint internalFormat, GLenum format, GLenum dataType, const GLvoid* pixels, bool genMipmap, HardwareContext& ctx)
    {
        createTexture();
        ctx.bindTexture(type_, id_);
        btAssert(type_ == TextureTypeCubeMap);
        ogl.TexImage2D(glCubeFace(face), 0, internalFormat, width_, height_, 0, format, dataType, pixels);
        if (genMipmap) {
            ogl.GenerateMipmap(GL_TEXTURE_CUBE_MAP);
        }
    }

    void HardwareTexture::download(GLenum format, GLenum dataType, GLvoid* pixels, HardwareContext& ctx)
    {
        createTexture();
        ctx.bindTexture(type_, id_);
        btAssert(type_ == TextureType2D);
        ogl.GetTexImage(GL_TEXTURE_2D, 0, format, dataType, pixels);
    }

    void HardwareTexture::generateMipmap(HardwareContext& ctx)
    {
        createTexture();
        ctx.bindTexture(type_, id_);
        ogl.GenerateMipmap(glType(type_));
    }

    void HardwareTexture::createTexture()
    {
        if (id_ == 0) {
            ogl.GenTextures(1, &id_);
            btAssert(id_ != 0);
        }
    }
}
