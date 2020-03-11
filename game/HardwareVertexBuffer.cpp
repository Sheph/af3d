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

#include "HardwareVertexBuffer.h"

namespace af3d
{
    HardwareVertexBuffer::HardwareVertexBuffer(HardwareResourceManager* mgr, Usage usage, GLsizeiptr elementSize)
    : HardwareBuffer(mgr, usage, elementSize)
    {
    }

    void HardwareVertexBuffer::doResize(HardwareContext& ctx)
    {
        ogl.BindBuffer(GL_ARRAY_BUFFER, id(ctx));
        ogl.BufferData(GL_ARRAY_BUFFER, sizeInBytes(ctx), NULL, glUsage());
    }

    void HardwareVertexBuffer::doUpload(GLintptr offset, GLsizeiptr cnt, const GLvoid* data, HardwareContext& ctx)
    {
        ogl.BindBuffer(GL_ARRAY_BUFFER, id(ctx));
        ogl.BufferSubData(GL_ARRAY_BUFFER, offset * elementSize(), cnt * elementSize(), data);
    }

    GLvoid* HardwareVertexBuffer::doLock(GLintptr offset, GLsizeiptr cnt, Access access, HardwareContext& ctx)
    {
        ogl.BindBuffer(GL_ARRAY_BUFFER, id(ctx));
        return ogl.MapBufferRange(GL_ARRAY_BUFFER, offset * elementSize(), cnt * elementSize(), glAccess(access));
    }

    void HardwareVertexBuffer::doUnlock(HardwareContext& ctx)
    {
        ogl.BindBuffer(GL_ARRAY_BUFFER, id(ctx));
        ogl.UnmapBuffer(GL_ARRAY_BUFFER);
    }
}
