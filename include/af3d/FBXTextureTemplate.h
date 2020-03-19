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

#ifndef _AF3D_FBX_TEXTURE_TEMPLATE_H_
#define _AF3D_FBX_TEXTURE_TEMPLATE_H_

#include "af3d/Types.h"

namespace af3d
{
    enum class FBXTextureUse
    {
        Standard = 0,
        ShadowMap,
        LightMap,
        SphericalReflectionMap,
        SphereReflectionMap,
        BumpNormalMap,
        Max = BumpNormalMap
    };

    enum class FBXWrapMode
    {
        Repeat = 0,
        Clamp,
        Max = Clamp
    };

    enum class FBXBlendMode
    {
        Translucent = 0,
        Additive,
        Modulate,
        Modulate2,
        Over,
        Max = Over
    };

    class FBXTextureTemplate
    {
    public:
        FBXTextureTemplate() = default;
        ~FBXTextureTemplate() = default;

        inline FBXTextureUse textureUse() const { return textureUse_; }
        inline void setTextureUse(FBXTextureUse value) { textureUse_ = value; }

        inline float alpha() const { return alpha_; }
        inline void setAlpha(float value) { alpha_ = value; }

        inline FBXWrapMode wrapU() const { return wrapU_; }
        inline void setWrapU(FBXWrapMode value) { wrapU_ = value; }

        inline FBXWrapMode wrapV() const { return wrapV_; }
        inline void setWrapV(FBXWrapMode value) { wrapV_ = value; }

        inline bool premultiplyAlpha() const { return premultiplyAlpha_; }
        inline void setPremultiplyAlpha(bool value) { premultiplyAlpha_ = value; }

        inline FBXBlendMode blendMode() const { return blendMode_; }
        inline void setBlendMode(FBXBlendMode value) { blendMode_ = value; }

    private:
        FBXTextureUse textureUse_ = FBXTextureUse::Standard;
        float alpha_ = 1.0f;
        FBXWrapMode wrapU_ = FBXWrapMode::Repeat;
        FBXWrapMode wrapV_ = FBXWrapMode::Repeat;
        bool premultiplyAlpha_ = false;
        FBXBlendMode blendMode_ = FBXBlendMode::Additive;
    };
}

#endif
