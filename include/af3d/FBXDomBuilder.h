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

#ifndef _AF3D_FBX_DOM_BUILDER_H_
#define _AF3D_FBX_DOM_BUILDER_H_

#include "af3d/Types.h"

namespace af3d
{
    class FBXDomBuilder
    {
    public:
        FBXDomBuilder() = default;
        virtual ~FBXDomBuilder() = default;

        virtual void addValue(std::int16_t value) {}
        virtual void addValue(bool value) {}
        virtual void addValue(std::int32_t value) {}
        virtual void addValue(float value) {}
        virtual void addValue(double value) {}
        virtual void addValue(std::int64_t value) {}
        virtual void addValue(const std::string& value) {}

        virtual Byte* addValueRaw(std::uint32_t count) { return nullptr; }
        virtual void endValueRaw(Byte* value, std::uint32_t count) {}

        virtual std::int16_t* addArrayInt16(std::uint32_t count) { return nullptr; }
        virtual void endArrayInt16(std::int16_t* value, std::uint32_t count) {}

        virtual bool* addArrayBool(std::uint32_t count) { return nullptr; }
        virtual void endArrayBool(bool* value, std::uint32_t count) {}

        virtual std::int32_t* addArrayInt32(std::uint32_t count) { return nullptr; }
        virtual void endArrayInt32(std::int32_t* value, std::uint32_t count) {}

        virtual float* addArrayFloat(std::uint32_t count) { return nullptr; }
        virtual void endArrayFloat(float* value, std::uint32_t count) {}

        virtual double* addArrayDouble(std::uint32_t count) { return nullptr; }
        virtual void endArrayDouble(double* value, std::uint32_t count) {}

        virtual std::int64_t* addArrayInt64(std::uint32_t count) { return nullptr; }
        virtual void endArrayInt64(std::int64_t* value, std::uint32_t count) {}

        virtual FBXDomBuilder* childBegin(const std::string& name) { return nullptr; }
        virtual void childEnd(const std::string& name, FBXDomBuilder* builder) {}
    };
}

#endif
