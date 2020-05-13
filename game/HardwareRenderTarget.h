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
#include "HardwareRenderbuffer.h"
#include "Utils.h"
#include <boost/optional.hpp>

namespace af3d
{
    class HardwareRenderTarget
    {
    public:
        HardwareRenderTarget() = default;
        explicit HardwareRenderTarget(const HardwareTexturePtr& texture,
            GLint level = 0,
            TextureCubeFace cubeFace = TextureCubeXP)
        : texType_(texture->type()),
          res_(texture),
          level_(level),
          cubeFace_(cubeFace),
          width_(texture->width()),
          height_(texture->height())
        {
        }
        explicit HardwareRenderTarget(const HardwareRenderbufferPtr& rb)
        : res_(rb),
          width_(rb->width()),
          height_(rb->height())
        {
        }
        ~HardwareRenderTarget() = default;

        inline const boost::optional<TextureType>& texType() const { return texType_; }
        inline const HardwareResourcePtr& res() const { return res_; }
        inline GLint level() const { return level_; }
        inline TextureCubeFace cubeFace() const { return cubeFace_; }

        inline std::uint32_t fullWidth() const { return width_; }
        inline std::uint32_t fullHeight() const { return height_; }

        typedef void (*unspecified_bool_type)();
        static void unspecified_bool_true() {}

        operator unspecified_bool_type() const
        {
            return res_ ? unspecified_bool_true : 0;
        }

        bool operator!() const
        {
            return !res_;
        }

        inline bool operator==(const HardwareRenderTarget& other) const
        {
            return (res_ == other.res_) &&
                (level_ == other.level_) &&
                (cubeFace_ == other.cubeFace_);
        }

        inline bool operator!=(const HardwareRenderTarget& other) const
        {
            return !(*this == other);
        }

    private:
        boost::optional<TextureType> texType_;
        HardwareResourcePtr res_;
        GLint level_ = 0;
        TextureCubeFace cubeFace_ = TextureCubeXP;
        std::uint32_t width_ = 0;
        std::uint32_t height_ = 0;
    };
}

#endif
