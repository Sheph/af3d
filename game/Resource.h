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

#ifndef _RESOURCE_H_
#define _RESOURCE_H_

#include "HardwareContext.h"
#include "AObject.h"
#include "af3d/Utils.h"
#include <memory>
#include <atomic>

namespace af3d
{
    class Resource;

    class ResourceLoader : boost::noncopyable
    {
    public:
        ResourceLoader() = default;
        virtual ~ResourceLoader() = default;

        virtual void load(Resource& res, HardwareContext& ctx) = 0;
    };

    using ResourceLoaderPtr = std::shared_ptr<ResourceLoader>;

    class Resource;
    using ResourcePtr = std::shared_ptr<Resource>;

    class Resource : public AObject
    {
    public:
        enum State
        {
            Unloaded = 0,
            Loading,
            Loaded
        };

        Resource(const AClass& klass, const std::string& name,
            const ResourceLoaderPtr& loader = ResourceLoaderPtr());
        virtual ~Resource() = default;

        static const AClass& staticKlass();

        void invalidate();

        void load(const ResourceLoaderPtr& loader = ResourceLoaderPtr());

        inline const ResourceLoaderPtr& loader() const { return loader_; }

        inline bool valid() const { return state_ != Unloaded; }

    private:
        virtual void doInvalidate();

        ResourceLoaderPtr loader_;
        std::atomic<State> state_;
    };

    ACLASS_DECLARE(Resource)
}

#endif
