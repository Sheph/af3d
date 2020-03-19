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

#ifndef _ASSIMPIOSTREAM_H_
#define _ASSIMPIOSTREAM_H_

#include "Platform.h"
#include "af3d/Types.h"
#include "assimp/IOStream.hpp"

namespace af3d
{
    class AssimpIOStream : public Assimp::IOStream
    {
    public:
        explicit AssimpIOStream(std::unique_ptr<PlatformIFStream> is);
        ~AssimpIOStream() = default;

        size_t Read(void* pvBuffer, size_t pSize, size_t pCount) override;
        size_t Write(const void* pvBuffer, size_t pSize, size_t pCount) override;
        aiReturn Seek(size_t pOffset, aiOrigin pOrigin) override;
        size_t Tell() const override;
        size_t FileSize() const override;
        void Flush() override;

    private:
        std::unique_ptr<PlatformIFStream> is_;
    };
}

#endif
