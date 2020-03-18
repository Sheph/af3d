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

#ifndef _AF3D_FBX_PARSER_H_
#define _AF3D_FBX_PARSER_H_

#include "af3d/Types.h"
#include "af3d/BinaryReader.h"
#include "af3d/FBXNodeBuilder.h"
#include <boost/noncopyable.hpp>
#include <iostream>

namespace af3d
{
    class FBXParser : boost::noncopyable
    {
    public:
        FBXParser(const std::string& path, std::istream& is);
        ~FBXParser() = default;

        bool parse(FBXNodeBuilder* rootBuilder);

    private:
        bool readHeader();

        bool readNode(FBXNodeBuilder* builder, bool& isEmpty);

        bool readProperty(FBXNodeBuilder* builder);

        bool decompress(std::uint32_t compressedLength, Byte* data, std::uint32_t dataSize);

        std::string path_;
        BinaryReader reader_;
        std::vector<Byte> tmp_;
    };
}

#endif
