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

#ifndef _AF3D_FBX_SCENE_BUILDER_H_
#define _AF3D_FBX_SCENE_BUILDER_H_

#include "af3d/FBXDomBuilder.h"
#include "af3d/FBXMaterialTemplateBuilder.h"
#include <boost/noncopyable.hpp>

namespace af3d
{
    class FBXSceneBuilder : public FBXDomBuilder,
        boost::noncopyable
    {
    public:
        FBXSceneBuilder();
        ~FBXSceneBuilder() = default;

        FBXDomBuilder* childBegin(const std::string& name) override;

    private:
        class ObjectTypeBuilder;

        class PropertyTemplateBuilder : public FBXDomBuilder
        {
        public:
            explicit PropertyTemplateBuilder(ObjectTypeBuilder* parent)
            : parent_(parent) {}

            void reset() { childBuilder_ = nullptr; }

            void addValue(const std::string& value) override;

            FBXDomBuilder* childBegin(const std::string& name) override;
            void childEnd(const std::string& name, FBXDomBuilder* builder) override;

            FBXMaterialTemplateBuilder mtlTemplateBuilder;

        private:
            ObjectTypeBuilder* parent_;
            FBXDomBuilder* childBuilder_ = nullptr;
        };

        class DefinitionsBuilder;

        class ObjectTypeBuilder : public FBXDomBuilder
        {
        public:
            explicit ObjectTypeBuilder(DefinitionsBuilder* parent)
            : ptBuilder(this),
              parent_(parent) {}

            void addValue(const std::string& value) override;

            FBXDomBuilder* childBegin(const std::string& name) override;

            std::string objType;

            PropertyTemplateBuilder ptBuilder;

        private:
            DefinitionsBuilder* parent_;
        };

        class DefinitionsBuilder : public FBXDomBuilder
        {
        public:
            DefinitionsBuilder()
            : otBuilder(this) {}

            FBXDomBuilder* childBegin(const std::string& name) override;

            ObjectTypeBuilder otBuilder;
        };

        DefinitionsBuilder defBuilder_;

        FBXMaterialTemplate mtlTemplate_;
    };
}

#endif
