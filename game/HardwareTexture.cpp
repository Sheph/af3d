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

    HardwareTexture::HardwareTexture(HardwareResourceManager* mgr, TextureType type, std::uint32_t width, std::uint32_t height, std::uint32_t depth,
        TextureFormat format)
    : HardwareResource(mgr),
      type_(type),
      width_(width),
      height_(height),
      depth_(depth),
      format_(format)
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
        switch (type) {
        case TextureTypeCubeMap:
            return GL_TEXTURE_CUBE_MAP;
        case TextureType2DArray:
            return GL_TEXTURE_2D_ARRAY;
        case TextureTypeCubeMapArray:
            return GL_TEXTURE_CUBE_MAP_ARRAY;
        default:
            btAssert(false);
        case TextureType2D:
            return GL_TEXTURE_2D;
        }
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

    void HardwareTexture::doInvalidate(HardwareContext& ctx)
    {
        id_ = 0;
    }

    GLuint HardwareTexture::id(HardwareContext& ctx) const
    {
        return id_;
    }

    void HardwareTexture::upload(GLint internalFormat, GLenum format, GLenum dataType, const GLvoid* pixels, bool genMipmap, GLint level, HardwareContext& ctx)
    {
        createTexture();
        ctx.bindTexture(type_, id_);
        if (type_ == TextureType2D) {
            ogl.TexImage2D(GL_TEXTURE_2D, level, internalFormat, textureMipSize(width_, level), textureMipSize(height_, level), 0, format, dataType, pixels);
            if (genMipmap) {
                ogl.GenerateMipmap(GL_TEXTURE_2D);
            }
        } else if (type_ == TextureTypeCubeMap) {
            uploadCubeFace(TextureCubeXP, internalFormat, format, dataType, pixels, false, level, ctx);
            uploadCubeFace(TextureCubeXN, internalFormat, format, dataType, pixels, false, level, ctx);
            uploadCubeFace(TextureCubeYP, internalFormat, format, dataType, pixels, false, level, ctx);
            uploadCubeFace(TextureCubeYN, internalFormat, format, dataType, pixels, false, level, ctx);
            uploadCubeFace(TextureCubeZP, internalFormat, format, dataType, pixels, false, level, ctx);
            uploadCubeFace(TextureCubeZN, internalFormat, format, dataType, pixels, false, level, ctx);
            if (genMipmap) {
                ogl.GenerateMipmap(GL_TEXTURE_CUBE_MAP);
            }
        } else if (type_ == TextureTypeCubeMapArray) {
            ogl.TexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, level, internalFormat, textureMipSize(width_, level), textureMipSize(height_, level), depth_ * 6, 0, format, dataType, pixels);
            if (genMipmap) {
                ogl.GenerateMipmap(GL_TEXTURE_CUBE_MAP_ARRAY);
            }
        } else {
            btAssert(type_ == TextureType2DArray);
            ogl.TexImage3D(GL_TEXTURE_2D_ARRAY, level, internalFormat, textureMipSize(width_, level), textureMipSize(height_, level), depth_, 0, format, dataType, pixels);
            if (genMipmap) {
                ogl.GenerateMipmap(GL_TEXTURE_2D_ARRAY);
            }
        }
    }

    void HardwareTexture::uploadCubeFace(TextureCubeFace face, GLint internalFormat, GLenum format, GLenum dataType, const GLvoid* pixels, bool genMipmap, GLint level, HardwareContext& ctx)
    {
        createTexture();
        ctx.bindTexture(type_, id_);
        btAssert(type_ == TextureTypeCubeMap);
        ogl.TexImage2D(glCubeFace(face), level, internalFormat, textureMipSize(width_, level), textureMipSize(height_, level), 0, format, dataType, pixels);
        if (genMipmap) {
            ogl.GenerateMipmap(GL_TEXTURE_CUBE_MAP);
        }
    }

    void HardwareTexture::uploadCompressed(GLint internalFormat, const GLvoid* data, GLsizei dataSize, bool genMipmap, GLint level, HardwareContext& ctx)
    {
        createTexture();
        ctx.bindTexture(type_, id_);
        btAssert(type_ == TextureType2D);
        ogl.CompressedTexImage2D(GL_TEXTURE_2D, level, internalFormat, textureMipSize(width_, level), textureMipSize(height_, level), 0, dataSize, data);
        if (genMipmap) {
            ogl.GenerateMipmap(GL_TEXTURE_2D);
        }
    }

    void HardwareTexture::update(GLenum format, GLenum dataType, const GLvoid* pixels, GLint level, GLint layer, HardwareContext& ctx)
    {
        createTexture();
        ctx.bindTexture(type_, id_);
        btAssert(type_ == TextureTypeCubeMapArray);
        ogl.TexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, level, 0, 0, layer * (TextureCubeFaceMax + 1),
            textureMipSize(width_, level), textureMipSize(height_, level), (TextureCubeFaceMax + 1), format, dataType, pixels);
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
            setValid();
        }
    }
}
