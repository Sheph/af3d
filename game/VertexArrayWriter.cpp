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

#include "VertexArrayWriter.h"
#include "HardwareResourceManager.h"
#include "Renderer.h"

namespace af3d
{
    VertexArrayWriter::VertexArrayWriter(VertexArrayLayout vaLayout, const VBOList& vbos,
        const HardwareIndexBufferPtr& ebo)
    : va_(std::make_shared<VertexArray>(hwManager.createVertexArray(), vaLayout, vbos, ebo)),
      vaNoEbo_(std::make_shared<VertexArray>(hwManager.createVertexArray(), vaLayout, vbos, HardwareIndexBufferPtr())),
      data_(std::make_shared<Data>())
    {
    }

    void VertexArrayWriter::upload()
    {
        auto va = va_;
        auto data = data_;
        renderer.scheduleHwOp([va, data](HardwareContext& ctx) {
            if (!data->vertices.empty()) {
                GLsizei sz = data->vertices.size() * sizeof(data->vertices[0]);
                btAssert((sz % va->vbos()[0]->elementSize()) == 0);
                int cnt = sz / va->vbos()[0]->elementSize();
                va->vbos()[0]->reload(cnt, &data->vertices[0], ctx);
            }
            if (!data->indices.empty()) {
                GLsizei sz = data->indices.size() * sizeof(data->indices[0]);
                btAssert((sz % va->ebo()->elementSize()) == 0);
                int cnt = sz / va->ebo()->elementSize();
                va->ebo()->reload(cnt, &data->indices[0], ctx);
            }
        });
        data_ = std::make_shared<Data>();
    }
}
