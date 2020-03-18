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

#ifndef _AF3D_FBX_PROPERTIES70_BUILDER_H_
#define _AF3D_FBX_PROPERTIES70_BUILDER_H_

#include "af3d/FBXNodeBuilder.h"
#include "af3d/Vector4.h"
#include <boost/noncopyable.hpp>

namespace af3d
{
    class FBXProperties70Builder : public FBXNodeBuilder,
        boost::noncopyable
    {
    public:
        FBXProperties70Builder();

        FBXNodeBuilder* childBegin(const std::string& name) override;

        virtual void onProperty(const std::string& key, bool value) = 0;
        virtual void onProperty(const std::string& key, std::int32_t value) = 0;
        virtual void onProperty(const std::string& key, float value) = 0;
        virtual void onProperty(const std::string& key, const std::string& value) = 0;
        virtual void onProperty(const std::string& key, const btVector3& value) = 0;
        virtual void onProperty(const std::string& key, const Color& value) = 0;

    private:
        class PBuilder : public FBXNodeBuilder
        {
        public:
            explicit PBuilder(FBXProperties70Builder* parent)
            : parent_(parent) {}

            void reset();

            void addValue(std::int16_t value) override;
            void addValue(bool value) override;
            void addValue(std::int32_t value) override;
            void addValue(float value) override;
            void addValue(double value) override;
            void addValue(std::int64_t value) override;
            void addValue(const std::string& value) override;

        private:
            enum class Type
            {
                Unknown = 0,
                Bool,
                Int,
                Float,
                String,
                Vector3,
                Color
            };

            FBXProperties70Builder* parent_;

            int i_ = 0;
            std::string key_;
            Type type_ = Type::Unknown;
            std::array<float, 3> floats_;
        };

        PBuilder pBuilder_;
    };
}

#endif
