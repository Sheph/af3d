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

#ifndef _AF3D_BINARY_READER_H_
#define _AF3D_BINARY_READER_H_

#include "af3d/Types.h"
#include <boost/noncopyable.hpp>
#include <istream>

namespace af3d
{
    class BinaryReader : boost::noncopyable
    {
    public:
        explicit BinaryReader(std::istream& is);
        ~BinaryReader() = default;

        void skipBytes(std::uint32_t count);
        void readBytes(Byte* first, Byte* last);
        inline void readBytes(char* first, char* last)
        {
            readBytes(reinterpret_cast<Byte*>(first), reinterpret_cast<Byte*>(last));
        }
        template <class T>
        inline void readPOD(T& res)
        {
            readBytes(reinterpret_cast<Byte*>(&res),
                reinterpret_cast<Byte*>(&res + 1));
        }

        std::uint8_t readUInt8();
        std::int8_t readInt8();
        std::uint16_t readUInt16Le();
        std::int16_t readInt16Le();
        std::uint32_t readUInt32Le();
        std::int32_t readInt32Le();
        std::uint64_t readUInt64Le();
        std::int64_t readInt64Le();
        std::string readString(uint32_t length);
        float readFloatLe();
        double readDoubleLe();

        bool isOk();
        bool isEOF();

        inline std::uint32_t offset() const { return offset_; }

    private:
        std::istream& is_;
        std::uint32_t offset_ = 0;
    };
}

#endif
