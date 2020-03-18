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

#ifndef _AF3D_FBX_MATERIAL_TEMPLATE_H_
#define _AF3D_FBX_MATERIAL_TEMPLATE_H_

#include "af3d/Types.h"

namespace af3d
{
    class FBXMaterialTemplate
    {
    public:
        FBXMaterialTemplate() = default;
        ~FBXMaterialTemplate() = default;

        inline const Color& ambientColor() const { return ambientColor_; }
        inline void setAmbientColor(const Color& value) { ambientColor_ = value; }

        inline const Color& diffuseColor() const { return diffuseColor_; }
        inline void setDiffuseColor(const Color& value) { diffuseColor_ = value; }

        inline const Color& emissiveColor() const { return emissiveColor_; }
        inline void setEmissiveColor(const Color& value) { emissiveColor_ = value; }

        inline const Color& specularColor() const { return specularColor_; }
        inline void setSpecularColor(const Color& value) { specularColor_ = value; }

        inline float shininess() const { return shininess_; }
        inline void setShininess(float value) { shininess_ = value; }

    private:
        Color ambientColor_ = Color_one;
        Color diffuseColor_ = Color_one;
        Color emissiveColor_ = Color_zero;
        Color specularColor_ = Color_zero;
        float shininess_ = 1.0f;
    };
}

#endif
