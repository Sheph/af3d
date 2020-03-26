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

#ifndef _AF3D_TPS_H_
#define _AF3D_TPS_H_

#include "af3d/Types.h"
#include <boost/noncopyable.hpp>
#include <unordered_map>
#include <iostream>
#include <memory>
#include "json/json-forwards.h"

namespace af3d
{
    class TPS;
    using TPSPtr = std::shared_ptr<TPS>;

    struct TPSEntry
    {
        TPSEntry() = default;
        TPSEntry(std::uint32_t x,
            std::uint32_t y,
            std::uint32_t width,
            std::uint32_t height)
        : x(x),
          y(y),
          width(width),
          height(height)
        {
        }

        inline bool valid() const { return (width > 0) && (height > 0); }

        std::uint32_t x = 0;
        std::uint32_t y = 0;
        std::uint32_t width = 0;
        std::uint32_t height = 0;
    };

    class TPS : boost::noncopyable
    {
    public:
        TPS(const std::string& imageFileName,
            std::uint32_t width,
            std::uint32_t height);
        ~TPS() = default;

        static TPSPtr fromStream(const std::string& fileName, std::istream& is);

        static TPSPtr fromString(const std::string& fileName, const std::string& json);

        static TPSPtr fromJsonValue(const Json::Value& jsonValue);

        inline const std::string& imageFileName() const { return imageFileName_; }
        inline std::uint32_t width() const { return width_; }
        inline std::uint32_t height() const { return height_; }

        void addEntry(const std::string& fileName, const TPSEntry& entry);

        const TPSEntry& entry(const std::string& fileName, bool quiet = false) const;

    private:
        using Entries = std::unordered_map<std::string, TPSEntry>;

        std::string imageFileName_;
        std::uint32_t width_;
        std::uint32_t height_;
        Entries entries_;
    };
}

#endif
