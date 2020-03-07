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

#include "HardwareResourceManager.h"
#include "af3d/Assert.h"
#include "af3d/Utils.h"

namespace af3d
{
    template <>
    Single<HardwareResourceManager>* Single<HardwareResourceManager>::single = nullptr;

    HardwareResourceManager::~HardwareResourceManager()
    {
        runtime_assert(resources_.empty());
    }

    bool HardwareResourceManager::init()
    {
        return true;
    }

    void HardwareResourceManager::shutdown()
    {
        runtime_assert(resources_.empty());
    }

    void HardwareResourceManager::invalidate(HardwareContext& ctx)
    {
        ScopedLock lock(mtx_);
        for (auto res : resources_) {
            res->invalidate(ctx);
        }
    }

    HardwareTexturePtr HardwareResourceManager::createTexture(std::uint32_t width, std::uint32_t height)
    {
        auto res = std::make_shared<HardwareTexture>(this, width, height);
        onResourceCreate(res);
        return res;
    }

    HardwareVertexBufferPtr HardwareResourceManager::createVertexBuffer(HardwareBuffer::Usage usage, GLsizeiptr elementSize)
    {
    }

    HardwareIndexBufferPtr HardwareResourceManager::createIndexBuffer(HardwareBuffer::Usage usage, HardwareIndexBuffer::DataType dataType)
    {
    }

    HardwareShaderPtr HardwareResourceManager::createShader(HardwareShader::Type type)
    {
    }

    HardwareProgramPtr HardwareResourceManager::createProgram()
    {
    }

    void HardwareResourceManager::onResourceDestroy(HardwareResource* res, const HardwareResource::CleanupFn& cleanupFn)
    {
        {
            ScopedLock lock(mtx_);
            runtime_assert(resources_.erase(res) > 0);
        }

        if (cleanupFn) {
            //TODO: Schedule 'cleanupFn' on renderer.
        }
    }

    void HardwareResourceManager::onResourceCreate(const HardwareResourcePtr& res)
    {
        ScopedLock lock(mtx_);
        runtime_assert(resources_.insert(res.get()).second);
    }
}
