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

#ifndef _VERTEX_ARRAY_WRITER_H_
#define _VERTEX_ARRAY_WRITER_H_

#include "VertexArray.h"

namespace af3d
{
    class VertexArrayWriter
    {
    public:
        struct Data
        {
            std::vector<float> vertices;
            std::vector<std::uint16_t> indices;
        };

        VertexArrayWriter() = default;
        VertexArrayWriter(VertexArrayLayout vaLayout, const VBOList& vbos,
            const HardwareIndexBufferPtr& ebo);
        ~VertexArrayWriter() = default;

        inline const VertexArrayPtr& va() const { return va_; }
        inline const VertexArrayPtr& vaNoEbo() const { return vaNoEbo_; }
        inline Data& data() { return *data_; }

        void upload();

    private:
        VertexArrayPtr va_;
        VertexArrayPtr vaNoEbo_;
        std::shared_ptr<Data> data_;
    };
}

#endif
