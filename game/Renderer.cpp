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
#include "HardwareResourceManager.h"
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
        runtime_assert(ops_.empty());
    }

    bool Renderer::init()
    {
        LOG4CPLUS_DEBUG(logger(), "renderer: init...");
        return true;
    }

    void Renderer::shutdown()
    {
        LOG4CPLUS_DEBUG(logger(), "renderer: shutdown...");
        ops_.clear();
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

    void Renderer::scheduleHwOp(const HwOpFn& hwOp)
    {
        {
            ScopedLockA lock(mtx_);

            if (cancelSwap_) {
                return;
            }

            ops_.push_back([hwOp](HardwareContext& ctx) {
                hwOp(ctx);
                return false;
            });
        }

        cond_.notify_one();
    }

    void Renderer::swap(const RenderNodeList& rnl)
    {
        {
            ScopedLockA lock(mtx_);

            while (!cancelSwap_ && rendering_) {
                cond_.wait(lock);
            }

            if (cancelSwap_) {
                cancelSwap_ = false;
                RenderOpList ops;
                ops_.swap(ops);
                rendering_ = false;
                lock.unlock();
                ops.clear();
                return;
            }

            rendering_ = true;
            ops_.push_back([this, rnl](HardwareContext& ctx) {
                for (const auto& rn : rnl) {
                    doRender(rn, ctx);
                }
                {
                    ScopedLockA lock(mtx_);
                    rendering_ = false;
                }
                cond_.notify_one();
                return true;
            });
        }

        cond_.notify_one();
    }

    void Renderer::cancelSwap(HardwareContext& ctx)
    {
        RenderOpList ops;

        {
            ScopedLock lock(mtx_);
            cancelSwap_ = true;
            ops_.swap(ops);
        }

        ops.clear();

        cond_.notify_one();
    }

    bool Renderer::render(HardwareContext& ctx)
    {
        while (true) {
            RenderOpFn fn;

            {
                ScopedLockA lock(mtx_);

                while (!cancelRender_ && ops_.empty()) {
                    cond_.wait(lock);
                }

                if (cancelRender_) {
                    cancelRender_ = false;
                    rendering_ = false;
                    lock.unlock();

                    hwManager.invalidate(ctx);

                    lock.lock();
                    RenderOpList ops;
                    ops_.swap(ops);
                    lock.unlock();
                    ops.clear();

                    return false;
                }

                fn = std::move(ops_.front());
                ops_.pop_front();
            }

            if (fn(ctx)) {
                std::uint64_t timeUs = getTimeUs();

                std::uint32_t dt = settings.minRenderDt;

                if (lastTimeUs_ != 0) {
                    dt = static_cast<std::uint32_t>(timeUs - lastTimeUs_);
                }

                if (dt < settings.minRenderDt) {
                    std::this_thread::sleep_for(std::chrono::microseconds(settings.minRenderDt - dt));
                    timeUs = getTimeUs();
                }

                lastTimeUs_ = timeUs;

                break;
            }
        }

        return true;
    }

    void Renderer::cancelRender()
    {
        {
            ScopedLock lock(mtx_);
            cancelRender_ = true;
        }

        cond_.notify_one();
    }

    void Renderer::doRender(const RenderNodePtr& rn, HardwareContext& ctx)
    {
        rn->apply(ctx);
    }
}
