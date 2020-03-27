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

#ifndef _IMAGE_H_
#define _IMAGE_H_

#include "Texture.h"

namespace af3d
{
    class Image
    {
    public:
        Image() = default;
        Image(const TexturePtr& texture,
            std::uint32_t x, std::uint32_t y,
            std::uint32_t width, std::uint32_t height);
        ~Image() = default;

        inline const TexturePtr& texture() const { return texture_; }

        inline const std::array<Vector2f, 4>& texCoords() const { return texCoords_; }

        inline float aspect() const { return aspect_; }

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

        inline std::uint32_t x() const { return x_; }
        inline std::uint32_t y() const { return y_; }

        inline std::uint32_t width() const { return width_; }
        inline std::uint32_t height() const { return height_; }

    private:
        TexturePtr texture_;
        std::uint32_t x_ = 0;
        std::uint32_t y_ = 0;
        std::uint32_t width_ = 0;
        std::uint32_t height_ = 0;
        std::array<Vector2f, 4> texCoords_;
        float aspect_ = 0.0f;
    };
}

#endif
