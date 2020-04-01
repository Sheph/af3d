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

#include "Resource.h"
#include "Renderer.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN_ABSTRACT(Resource, AObject)
    ACLASS_DEFINE_END(Resource)

    Resource::Resource(const AClass& klass, const std::string& name,
        const ResourceLoaderPtr& loader)
    : AObject(klass),
      loader_(loader),
      state_(Unloaded)
    {
        setName(name);
    }

    const AClass& Resource::staticKlass()
    {
        return AClass_Resource;
    }

    void Resource::invalidate()
    {
        state_ = Unloaded;
        doInvalidate();
    }

    void Resource::load(const ResourceLoaderPtr& loader)
    {
        if (!loader && !loader_) {
            return;
        }

        State old = Unloaded;
        if (state_.compare_exchange_strong(old, Loading) || loader) {
            ResourcePtr res = std::static_pointer_cast<Resource>(sharedThis());
            ResourceLoaderPtr l = loader;
            if (!l) {
                l = loader_;
            }
            renderer.scheduleHwOp([res, l](HardwareContext& ctx) {
                l->load(*res, ctx);
                res->state_ = Loaded;
            });
        }
    }

    void Resource::doInvalidate()
    {
    }
}
