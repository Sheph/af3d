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

#include "HardwareBuffer.h"

namespace af3d
{
    HardwareBuffer::HardwareBuffer(HardwareResourceManager* mgr, Usage usage, GLsizeiptr elementSize)
    : HardwareResource(mgr),
      usage_(usage),
      elementSize_(elementSize)
    {
    }

    HardwareBuffer::~HardwareBuffer()
    {
        GLuint id = id_;
        if (id != 0) {
            cleanup([id](HardwareContext& ctx) {
                ogl.DeleteBuffers(1, &id);
            });
        } else {
            cleanup();
        }
    }

    GLbitfield HardwareBuffer::glAccess(Access access)
    {
        switch (access) {
        default:
            btAssert(false);
        case WriteOnly:
            return GL_MAP_WRITE_BIT;
        }
    }

    GLenum HardwareBuffer::glUsage() const
    {
        switch (usage_) {
        case Usage::DynamicDraw:
            return GL_DYNAMIC_DRAW;
        case Usage::StreamDraw:
            return GL_STREAM_DRAW;
        default:
            btAssert(false);
        case Usage::StaticDraw:
            return GL_STATIC_DRAW;
        }
    }

    void HardwareBuffer::invalidate(HardwareContext& ctx)
    {
        id_ = 0;
        count_ = 0;
    }

    GLuint HardwareBuffer::id(HardwareContext& ctx) const
    {
        return id_;
    }

    void HardwareBuffer::resize(GLsizeiptr cnt, HardwareContext& ctx)
    {
        createBuffer();
        count_ = cnt;
        doResize(ctx);
    }

    void HardwareBuffer::reload(GLsizeiptr cnt, const GLvoid* data, HardwareContext& ctx)
    {
        createBuffer();
        count_ = cnt;
        doReload(cnt, data, ctx);
    }

    void HardwareBuffer::upload(GLintptr offset, GLsizeiptr cnt, const GLvoid* data, HardwareContext& ctx)
    {
        createBuffer();
        doUpload(offset, cnt, data, ctx);
    }

    GLvoid* HardwareBuffer::lock(GLintptr offset, GLsizeiptr cnt, Access access, HardwareContext& ctx)
    {
        createBuffer();
        GLvoid* ptr = doLock(offset, cnt, access, ctx);
        if (ptr) {
            locked_ = true;
        }
        return ptr;
    }

    GLvoid* HardwareBuffer::lock(Access access, HardwareContext& ctx)
    {
        return lock(0, count_, access, ctx);
    }

    void HardwareBuffer::unlock(HardwareContext& ctx)
    {
        btAssert(locked_);
        doUnlock(ctx);
        locked_ = false;
    }

    void HardwareBuffer::createBuffer()
    {
        if (id_ == 0) {
            ogl.GenBuffers(1, &id_);
            btAssert(id_ != 0);
        }
    }
}
