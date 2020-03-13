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

#include "Renderer.h"
#include "Settings.h"
#include "Logger.h"
#include <thread>
#include <chrono>

namespace af3d
{
    Renderer renderer;

    template <>
    Single<Renderer>* Single<Renderer>::single = nullptr;

    Renderer::~Renderer()
    {
    }

    bool Renderer::init()
    {
        LOG4CPLUS_DEBUG(logger(), "renderer: init...");
        return true;
    }

    void Renderer::shutdown()
    {
        LOG4CPLUS_DEBUG(logger(), "renderer: shutdown...");
    }

    bool Renderer::reload(HardwareContext& ctx)
    {
        LOG4CPLUS_DEBUG(logger(), "renderer: reload...");

        LOG4CPLUS_INFO(logger(), "OpenGL vendor: " << ogl.GetString(GL_VENDOR));
        LOG4CPLUS_INFO(logger(), "OpenGL renderer: " << ogl.GetString(GL_RENDERER));
        LOG4CPLUS_INFO(logger(), "OpenGL version: " << ogl.GetString(GL_VERSION));

        GLint sampleBuffers = 0;
        GLint samples = 0;

        ogl.GetIntegerv(GL_SAMPLE_BUFFERS, &sampleBuffers);
        ogl.GetIntegerv(GL_SAMPLES, &samples);

        LOG4CPLUS_INFO(logger(), "sample_buffers = " << sampleBuffers << ", samples = " << samples);
        LOG4CPLUS_INFO(logger(), "texture filter: " << (settings.trilinearFilter ? "trilinear" : "bilinear"));

        return true;
    }

    void Renderer::scheduleHwOp(const HwOpFn&& hwOp)
    {
    }

    void Renderer::swap(const RenderNodePtr& rn)
    {
    }

    void Renderer::cancelSwap()
    {
    }

    bool Renderer::render(HardwareContext& ctx)
    {
        /*
        if (canceled) {
            hwManager.invalidate(ctx);
            return false;
        }
        */

        std::this_thread::sleep_for(std::chrono::milliseconds(16));

        return true;
    }

    void Renderer::cancelRender()
    {
        LOG4CPLUS_DEBUG(logger(), "renderer: cancelRender");
    }
}
