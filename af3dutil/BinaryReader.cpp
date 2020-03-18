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

#include "af3d/BinaryReader.h"
#include "af3d/ByteOrder.h"

namespace af3d
{
    BinaryReader::BinaryReader(std::istream& is)
    : is_(is)
    {
    }

    void BinaryReader::skipBytes(std::uint32_t count)
    {
        is_.seekg(count, std::ios_base::cur);
        offset_ += count;
    }

    void BinaryReader::readBytes(Byte* first, Byte* last)
    {
        std::uint32_t count = last - first;
        is_.read(reinterpret_cast<char*>(first), count);
        offset_ += count;
    }

    std::uint8_t BinaryReader::readUInt8()
    {
        std::uint8_t ret;
        readPOD(ret);
        return ret;
    }

    std::int8_t BinaryReader::readInt8()
    {
        std::int8_t ret;
        readPOD(ret);
        return ret;
    }

    std::uint16_t BinaryReader::readUInt16Le()
    {
        std::uint16_t ret;
        readPOD(ret);
        ret = fromLittleEndian(ret);
        return ret;
    }

    std::int16_t BinaryReader::readInt16Le()
    {
        std::int16_t ret;
        readPOD(ret);
        ret = fromLittleEndian(ret);
        return ret;
    }

    std::uint32_t BinaryReader::readUInt32Le()
    {
        std::uint32_t ret;
        readPOD(ret);
        ret = fromLittleEndian(ret);
        return ret;
    }

    std::int32_t BinaryReader::readInt32Le()
    {
        std::int32_t ret;
        readPOD(ret);
        ret = fromLittleEndian(ret);
        return ret;
    }

    std::uint64_t BinaryReader::readUInt64Le()
    {
        std::uint64_t ret;
        readPOD(ret);
        ret = fromLittleEndian(ret);
        return ret;
    }

    std::int64_t BinaryReader::readInt64Le()
    {
        std::int64_t ret;
        readPOD(ret);
        ret = fromLittleEndian(ret);
        return ret;
    }

    std::string BinaryReader::readString(uint32_t length)
    {
        std::string str(length, '\0');
        readBytes(&str[0], &str[0] + length);
        return str;
    }

    float BinaryReader::readFloatLe()
    {
        union
        {
            std::uint32_t v;
            float v2;
        } un;
        un.v = readUInt32Le();

        return un.v2;
    }

    double BinaryReader::readDoubleLe()
    {
        union
        {
            std::uint64_t v;
            double v2;
        } un;
        un.v = readUInt64Le();

        return un.v2;
    }

    bool BinaryReader::isOk()
    {
        return (bool)is_;
    }

    bool BinaryReader::isEOF()
    {
        is_.peek();
        return is_.eof();
    }
}
