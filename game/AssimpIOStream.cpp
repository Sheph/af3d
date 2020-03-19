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

#include "AssimpIOStream.h"
#include "af3d/Assert.h"

namespace af3d
{
    AssimpIOStream::AssimpIOStream(std::unique_ptr<PlatformIFStream> is)
    : is_(std::move(is))
    {
    }

    size_t AssimpIOStream::Read(void* pvBuffer, size_t pSize, size_t pCount)
    {
        is_->read(reinterpret_cast<char*>(pvBuffer), pSize * pCount);
        return is_->gcount();
    }

    size_t AssimpIOStream::Write(const void* pvBuffer, size_t pSize, size_t pCount)
    {
        runtime_assert(false);
        return 0;
    }

    aiReturn AssimpIOStream::Seek(size_t pOffset, aiOrigin pOrigin)
    {
        is_->clear();

        if (pOrigin == aiOrigin_SET) {
            is_->seekg(pOffset, std::ios_base::beg);
        } else if (pOrigin == aiOrigin_CUR) {
            is_->seekg(pOffset, std::ios_base::cur);
        } else if (pOrigin == aiOrigin_END) {
            is_->seekg(pOffset, std::ios_base::end);
        }

        return aiReturn_SUCCESS;
    }

    size_t AssimpIOStream::Tell() const
    {
        return is_->tellg();
    }

    size_t AssimpIOStream::FileSize() const
    {
        size_t off = is_->tellg();
        is_->seekg(0, std::ios::end);
        size_t sz = is_->tellg();
        is_->seekg(off, std::ios::beg);
        return sz;
    }

    void AssimpIOStream::Flush()
    {
    }
}
