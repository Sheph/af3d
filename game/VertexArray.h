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

#ifndef _VERTEX_ARRAY_H_
#define _VERTEX_ARRAY_H_

#include "VertexArrayLayout.h"
#include "HardwareVertexArray.h"

namespace af3d
{
    class VertexArray : boost::noncopyable
    {
    public:
        VertexArray(const HardwareVertexArrayPtr& vao,
            const VertexArrayLayout& layout,
            const VBOList& vbos,
            const HardwareIndexBufferPtr& ebo = HardwareIndexBufferPtr());
        ~VertexArray() = default;

        const HardwareVertexArrayPtr& vao(HardwareContext& ctx) const;

        inline const VertexArrayLayout& layout() const { return layout_; }

        inline const VBOList& vbos() const { return vbos_; }

        inline const HardwareIndexBufferPtr& ebo() const { return ebo_; }

    private:
        // This one is populated and owned by the rendering thread!
        // VAOs cannot be shared between contexts, so only rendering thread
        // can touch this!
        HardwareVertexArrayPtr vao_;

        VertexArrayLayout layout_;
        VBOList vbos_;
        HardwareIndexBufferPtr ebo_;
    };

    using VertexArrayPtr = std::shared_ptr<VertexArray>;
}

#endif
