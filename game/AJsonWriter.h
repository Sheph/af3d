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

#ifndef _AJSONWRITER_H_
#define _AJSONWRITER_H_

#include "AJsonSerializer.h"
#include <list>

namespace af3d
{
    class AJsonWriter : boost::noncopyable
    {
    public:
        AJsonWriter(Json::Value& jsonValue, AJsonSerializer& serializer);
        ~AJsonWriter() = default;

        inline AJsonSerializer& serializer() { return serializer_; }

        void write(const AObjectPtr& obj);

        std::uint32_t registerObject(const AObjectPtr& obj);

    private:
        using ObjectMap = std::unordered_map<AObjectPtr, std::uint32_t>;

        Json::Value& jsonValue_;
        AJsonSerializer& serializer_;

        std::uint32_t nextId_ = 1;
        ObjectMap objectMap_;
        std::list<AObjectPtr> objectsToProcess_;
    };
}

#endif
