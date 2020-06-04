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

#ifndef _HARDWARE_RESOURCE_MANAGER_H_
#define _HARDWARE_RESOURCE_MANAGER_H_

#include "HardwareTexture.h"
#include "HardwareVertexArray.h"
#include "HardwareShader.h"
#include "HardwareProgram.h"
#include "HardwareSampler.h"
#include "HardwareFramebuffer.h"
#include "HardwareRenderbuffer.h"
#include "af3d/Single.h"
#include <mutex>
#include <unordered_set>

namespace af3d
{
    class HardwareResourceManager : public Single<HardwareResourceManager>
    {
    public:
        HardwareResourceManager() = default;
        ~HardwareResourceManager();

        bool init();

        void shutdown();

        void invalidate(HardwareContext& ctx);

        bool renderReload(HardwareContext& ctx);

        HardwareTexturePtr createTexture(TextureType type, std::uint32_t width, std::uint32_t height, std::uint32_t depth,
            TextureFormat format = TextureFormatAny);
        HardwareDataBufferPtr createDataBuffer(HardwareBuffer::Usage usage, GLsizeiptr elementSize);
        HardwareIndexBufferPtr createIndexBuffer(HardwareBuffer::Usage usage, HardwareIndexBuffer::DataType dataType);
        HardwareVertexArrayPtr createVertexArray();
        HardwareShaderPtr createShader(HardwareShader::Type type);
        HardwareProgramPtr createProgram();
        HardwareSamplerPtr createSampler();
        HardwareFramebufferPtr createFramebuffer();
        HardwareRenderbufferPtr createRenderbuffer(std::uint32_t width, std::uint32_t height);

        void onResourceDestroy(HardwareResource* res, const HardwareResource::CleanupFn& cleanupFn);

    private:
        using Resources = std::unordered_set<HardwareResource*>;

        void onResourceCreate(const HardwareResourcePtr& res);

        std::mutex mtx_;
        Resources resources_;
    };

    extern HardwareResourceManager hwManager;
}

#endif
