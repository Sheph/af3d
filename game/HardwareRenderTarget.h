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

#ifndef _HARDWARE_RENDERTARGET_H_
#define _HARDWARE_RENDERTARGET_H_

#include "HardwareTexture.h"

namespace af3d
{
    class HardwareRenderTarget
    {
    public:
        HardwareRenderTarget() = default;
        explicit HardwareRenderTarget(const HardwareTexturePtr& texture,
            GLint level = 0,
            TextureCubeFace cubeFace = TextureCubeXP)
        : texture_(texture),
          level_(level),
          cubeFace_(cubeFace)
        {
        }
        ~HardwareRenderTarget() = default;

        inline const HardwareTexturePtr& texture() const { return texture_; }
        inline GLint level() const { return level_; }
        inline TextureCubeFace cubeFace() const { return cubeFace_; }

        typedef void (*unspecified_bool_type)();
        static void unspecified_bool_true() {}

        operator unspecified_bool_type() const
        {
            return texture_ ? unspecified_bool_true : 0;
        }

        bool operator!() const
        {
            return !texture_;
        }

    private:
        HardwareTexturePtr texture_;
        GLint level_ = 0;
        TextureCubeFace cubeFace_ = TextureCubeXP;
    };
}

#endif
