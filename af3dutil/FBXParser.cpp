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

#include "af3d/FBXParser.h"
#include "af3d/Assert.h"
#include "af3d/ByteOrder.h"
#include "Logger.h"
#include <log4cplus/ndc.h>
#include <zlib.h>

namespace af3d
{
    FBXParser::FBXParser(const std::string& path, std::istream& is)
    : path_(path),
      reader_(is)
    {
    }

    bool FBXParser::parse(FBXDomBuilder* rootBuilder)
    {
        log4cplus::NDCContextCreator ndc(path_);

        LOG4CPLUS_DEBUG(af3dutil::logger(), "Parsing...");

        if (!readHeader()) {
            LOG4CPLUS_ERROR(af3dutil::logger(), "Bad header");
            return false;
        }

        std::uint32_t version = reader_.readUInt32Le();

        std::uint32_t maxVersion = 7400;
        if (version > maxVersion) {
            LOG4CPLUS_ERROR(af3dutil::logger(), "Unsupported FBX version " << version);
            return false;
        }

        bool isEmpty = false;

        while (!isEmpty) {
            bool res = readNode(rootBuilder, isEmpty);
            if (!res) {
                return false;
            }
        }

        return true;
    }

    bool FBXParser::readHeader()
    {
        std::string magic("Kaydara FBX Binary  ");
        for (auto c : magic) {
            if (reader_.readUInt8() != c) {
                return false;
            }
        }
        if (reader_.readUInt8() != 0x00) {
            return false;
        }
        if (reader_.readUInt8() != 0x1A) {
            return false;
        }
        if (reader_.readUInt8() != 0x00) {
            return false;
        }
        return reader_.isOk();
    }

    bool FBXParser::readNode(FBXDomBuilder* builder, bool& isEmpty)
    {
        std::uint32_t endOffset = reader_.readUInt32Le();
        std::uint32_t numProperties = reader_.readUInt32Le();
        reader_.readUInt32Le();
        std::uint8_t nameLength = reader_.readUInt8();
        std::string name = reader_.readString(nameLength);

        if (!reader_.isOk()) {
            LOG4CPLUS_ERROR(af3dutil::logger(), "Unexpected EOF at node header");
            return false;
        }

        if (name.empty() && (numProperties == 0) && (endOffset <= reader_.offset())) {
            isEmpty = true;
            return true;
        }

        auto b = builder->childBegin(name);

        if (!b) {
            reader_.skipBytes(endOffset - reader_.offset());
            return true;
        }

        for (std::uint32_t i = 0; i < numProperties; i++) {
            if (!readProperty(b)) {
                builder->childEnd(name, b);
                return false;
            }
        }

        bool res = true;
        bool tmp = false;

        while (reader_.offset() < endOffset) {
            res = readNode(b, tmp);
            if (!res) {
                break;
            }
        }

        builder->childEnd(name, b);

        return res;
    }

    bool FBXParser::readProperty(FBXDomBuilder* builder)
    {
        auto type = reader_.readUInt8();

        if (!reader_.isOk()) {
            LOG4CPLUS_ERROR(af3dutil::logger(), "Unexpected EOF at property");
            return false;
        }

        if (type == 'S') {
            uint32_t count = reader_.readUInt32Le();
            auto s = reader_.readString(count);
            builder->addValue(s);
        } else if (type == 'R') {
            uint32_t count = reader_.readUInt32Le();
            auto ptr = builder->addValueRaw(count);
            if (ptr) {
                reader_.readBytes(ptr, ptr + count);
                builder->endValueRaw(ptr, count);
            } else {
                reader_.skipBytes(count);
            }
        } else if (type == 'Y') {
            std::int16_t val = reader_.readInt16Le();
            builder->addValue(val);
        } else if (type == 'C') {
            bool val = reader_.readUInt8() & 0x1;
            builder->addValue(val);
        } else if (type == 'I') {
            std::int32_t val = reader_.readInt32Le();
            builder->addValue(val);
        } else if (type == 'F') {
            float val = reader_.readFloatLe();
            builder->addValue(val);
        } else if (type == 'D') {
            double val = reader_.readDoubleLe();
            builder->addValue(val);
        } else if (type == 'L') {
            std::int64_t val = reader_.readInt64Le();
            builder->addValue(val);
        } else {
            std::uint32_t count = reader_.readUInt32Le(); // number of elements in array
            std::uint32_t encoding = reader_.readUInt32Le(); // 0 .. uncompressed, 1 .. zlib-compressed
            std::uint32_t compressedLength = reader_.readUInt32Le();
            if (encoding) {
                if (type == 'f') {
                    auto ptr = builder->addArrayFloat(count);
                    if (ptr) {
                        if (!decompress(compressedLength, (Byte*)ptr, count * 4)) {
                            builder->endArrayFloat(ptr, count);
                            return false;
                        }
                        if (!isCpuLittleEndian()) {
                            // TODO: swap bytes.
                            runtime_assert(false);
                        }
                        builder->endArrayFloat(ptr, count);
                    } else {
                        reader_.skipBytes(compressedLength);
                    }
                } else if (type == 'd') {
                    auto ptr = builder->addArrayDouble(count);
                    if (ptr) {
                        if (!decompress(compressedLength, (Byte*)ptr, count * 8)) {
                            builder->endArrayDouble(ptr, count);
                            return false;
                        }
                        if (!isCpuLittleEndian()) {
                            // TODO: swap bytes.
                            runtime_assert(false);
                        }
                        builder->endArrayDouble(ptr, count);
                    } else {
                        reader_.skipBytes(compressedLength);
                    }
                } else if (type == 'l') {
                    auto ptr = builder->addArrayInt64(count);
                    if (ptr) {
                        if (!decompress(compressedLength, (Byte*)ptr, count * 8)) {
                            builder->endArrayInt64(ptr, count);
                            return false;
                        }
                        if (!isCpuLittleEndian()) {
                            // TODO: swap bytes.
                            runtime_assert(false);
                        }
                        builder->endArrayInt64(ptr, count);
                    } else {
                        reader_.skipBytes(compressedLength);
                    }
                } else if (type == 'i') {
                    auto ptr = builder->addArrayInt32(count);
                    if (ptr) {
                        if (!decompress(compressedLength, (Byte*)ptr, count * 4)) {
                            builder->endArrayInt32(ptr, count);
                            return false;
                        }
                        if (!isCpuLittleEndian()) {
                            // TODO: swap bytes.
                            runtime_assert(false);
                        }
                        builder->endArrayInt32(ptr, count);
                    } else {
                        reader_.skipBytes(compressedLength);
                    }
                } else if (type == 'b') {
                    auto ptr = builder->addArrayBool(count);
                    if (ptr) {
                        if (!decompress(compressedLength, (Byte*)ptr, count)) {
                            builder->endArrayBool(ptr, count);
                            return false;
                        }
                        if (!isCpuLittleEndian()) {
                            // TODO: swap bytes.
                            runtime_assert(false);
                        }
                        builder->endArrayBool(ptr, count);
                    } else {
                        reader_.skipBytes(compressedLength);
                    }
                } else {
                    LOG4CPLUS_ERROR(af3dutil::logger(), "Bad property type - " << (int)type);
                    return false;
                }
            } else {
                if (type == 'f') {
                    auto ptr = builder->addArrayFloat(count);
                    if (ptr) {
                        reader_.readBytes((Byte*)ptr, (Byte*)ptr + count * 4);
                        if (!isCpuLittleEndian()) {
                            // TODO: swap bytes.
                            runtime_assert(false);
                        }
                        builder->endArrayFloat(ptr, count);
                    } else {
                        reader_.skipBytes(count * 4);
                    }
                } else if (type == 'd') {
                    auto ptr = builder->addArrayDouble(count);
                    if (ptr) {
                        reader_.readBytes((Byte*)ptr, (Byte*)ptr + count * 8);
                        if (!isCpuLittleEndian()) {
                            // TODO: swap bytes.
                            runtime_assert(false);
                        }
                        builder->endArrayDouble(ptr, count);
                    } else {
                        reader_.skipBytes(count * 8);
                    }
                } else if (type == 'l') {
                    auto ptr = builder->addArrayInt64(count);
                    if (ptr) {
                        reader_.readBytes((Byte*)ptr, (Byte*)ptr + count * 8);
                        if (!isCpuLittleEndian()) {
                            // TODO: swap bytes.
                            runtime_assert(false);
                        }
                        builder->endArrayInt64(ptr, count);
                    } else {
                        reader_.skipBytes(count * 8);
                    }
                } else if (type == 'i') {
                    auto ptr = builder->addArrayInt32(count);
                    if (ptr) {
                        reader_.readBytes((Byte*)ptr, (Byte*)ptr + count * 4);
                        if (!isCpuLittleEndian()) {
                            // TODO: swap bytes.
                            runtime_assert(false);
                        }
                        builder->endArrayInt32(ptr, count);
                    } else {
                        reader_.skipBytes(count * 4);
                    }
                } else if (type == 'b') {
                    auto ptr = builder->addArrayBool(count);
                    if (ptr) {
                        reader_.readBytes((Byte*)ptr, (Byte*)ptr + count);
                        if (!isCpuLittleEndian()) {
                            // TODO: swap bytes.
                            runtime_assert(false);
                        }
                        builder->endArrayBool(ptr, count);
                    } else {
                        reader_.skipBytes(count);
                    }
                } else {
                    LOG4CPLUS_ERROR(af3dutil::logger(), "Bad property type - " << (int)type);
                    return false;
                }
            }
        }

        return true;
    }

    bool FBXParser::decompress(std::uint32_t compressedLength, Byte* data, std::uint32_t dataSize)
    {
        tmp_.resize(compressedLength);
        reader_.readBytes(&tmp_[0], &tmp_[0] + compressedLength);
        unsigned long destLen = dataSize;
        ::uncompress(data, &destLen, &tmp_[0], compressedLength);
        if (destLen != dataSize) {
            LOG4CPLUS_ERROR(af3dutil::logger(), "dataSize does not match data");
            return false;
        }
        return true;
    }
}
