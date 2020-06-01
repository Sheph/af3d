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
    VertexArrayWriter::VertexArrayWriter()
    : data_(std::make_shared<Data>())
    {
        VertexArrayLayout vaLayout;

        vaLayout.addEntry(VertexArrayEntry(VertexAttribName::Pos, GL_FLOAT_VEC3, 0, 0));
        vaLayout.addEntry(VertexArrayEntry(VertexAttribName::UV, GL_FLOAT_VEC2, 12, 0));
        vaLayout.addEntry(VertexArrayEntry(VertexAttribName::Color, GL_UNSIGNED_INT8_VEC4_NV, 20, 0, true));

        auto vbo = hwManager.createDataBuffer(HardwareBuffer::Usage::StreamDraw, sizeof(VertexImm));
        auto ebo = hwManager.createIndexBuffer(HardwareBuffer::Usage::StreamDraw, HardwareIndexBuffer::UInt16);
        VBOList vbos{vbo};

        va_ = std::make_shared<VertexArray>(hwManager.createVertexArray(), vaLayout, vbos, ebo);
        vaNoEbo_ = std::make_shared<VertexArray>(hwManager.createVertexArray(), vaLayout, vbos, HardwareIndexBufferPtr());
    }

    void VertexArrayWriter::upload()
    {
        auto va = va_;
        auto data = data_;
        renderer.scheduleHwOp([va, data](HardwareContext& ctx) {
            if (!data->vertices.empty()) {
                va->vbos()[0]->reload(data->vertices.size(), &data->vertices[0], ctx);
            }
            if (!data->indices.empty()) {
                va->ebo()->reload(data->indices.size(), &data->indices[0], ctx);
            }
        });
        data_ = std::make_shared<Data>();
    }
}
