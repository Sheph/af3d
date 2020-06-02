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

#include "HardwareVertexArray.h"

namespace af3d
{
    HardwareVertexArray::HardwareVertexArray(HardwareResourceManager* mgr)
    : HardwareResource(mgr)
    {
    }

    HardwareVertexArray::~HardwareVertexArray()
    {
        GLuint id = id_;
        if (id != 0) {
            cleanup([id](HardwareContext& ctx) {
                ogl.DeleteVertexArrays(1, &id);
            });
        } else {
            cleanup();
        }
    }

    void HardwareVertexArray::doInvalidate(HardwareContext& ctx)
    {
        layout_ = VertexArrayLayout();
        vbos_.clear();
        ebo_.reset();
        id_ = 0;
    }

    GLuint HardwareVertexArray::id(HardwareContext& ctx) const
    {
        return id_;
    }

    void HardwareVertexArray::setup(const VertexArrayLayout& layout,
        const VBOList& vbos,
        const HardwareIndexBufferPtr& ebo,
        HardwareContext& ctx)
    {
        if (id_ != 0) {
            ogl.DeleteVertexArrays(1, &id_);
            id_ = 0;
        }

        layout_ = layout;
        vbos_ = vbos;
        ebo_ = ebo;

        if (id_ == 0) {
            ogl.GenVertexArrays(1, &id_);
            btAssert(id_ != 0);
            setValid();
        }
        ogl.BindVertexArray(id_);

        for (const auto& entry : layout_.entries()) {
            const auto& vbo = vbos_[entry.bufferIdx];
            if (!vbo->id(ctx)) {
                continue;
            }
            ogl.BindBuffer(GL_ARRAY_BUFFER, vbo->id(ctx));
            GLint location = HardwareProgram::getVertexAttribLocation(entry.name);
            ogl.EnableVertexAttribArray(location);
            const auto& ti = HardwareProgram::getTypeInfo(entry.type);
            ogl.VertexAttribPointer(location, ti.numComponents, ti.baseType, entry.normalize, vbo->elementSize(), (const GLvoid*)entry.offset);
        }

        if (ebo_) {
            if (ebo->id(ctx)) {
                ogl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo->id(ctx));
            }
        }

        ogl.BindVertexArray(0);
    }
}
