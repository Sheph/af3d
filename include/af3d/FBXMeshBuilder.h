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

#ifndef _AF3D_FBX_MESH_BUILDER_H_
#define _AF3D_FBX_MESH_BUILDER_H_

#include "af3d/FBXDomBuilder.h"
#include "af3d/FBXMesh.h"

namespace af3d
{
    class FBXMeshBuilder : public FBXDomBuilder
    {
    public:
        FBXMeshBuilder();

        inline FBXMesh* target() { return target_; }
        void setTarget(FBXMesh* target);

        void addValue(std::int64_t value) override;
        void addValue(const std::string& value) override;

        FBXDomBuilder* childBegin(const std::string& name) override;

    private:
        class VerticesBuilder : public FBXDomBuilder
        {
        public:
            double* addArrayDouble(std::uint32_t count) override;
            void endArrayDouble(double* value, std::uint32_t count) override;

            FBXMeshBuilder* parent = nullptr;
        };

        class IndicesBuilder : public FBXDomBuilder
        {
        public:
            std::int32_t* addArrayInt32(std::uint32_t count) override;
            void endArrayInt32(std::int32_t* value, std::uint32_t count) override;

            FBXMeshBuilder* parent = nullptr;
        };

        class NormalsBuilder : public FBXDomBuilder
        {
        public:
            double* addArrayDouble(std::uint32_t count) override;
            void endArrayDouble(double* value, std::uint32_t count) override;

            FBXMeshBuilder* parent = nullptr;
        };

        class UVBuilder : public FBXDomBuilder
        {
        public:
            double* addArrayDouble(std::uint32_t count) override;
            void endArrayDouble(double* value, std::uint32_t count) override;

            FBXMeshBuilder* parent = nullptr;
        };

        class LayerElementNormalBuilder : public FBXDomBuilder
        {
        public:
            inline FBXDomBuilder* childBegin(const std::string& name) override
            {
                if (name == "Normals") {
                    return &normalsBuilder;
                }
                return nullptr;
            }

            NormalsBuilder normalsBuilder;
        };

        class LayerElementUVBuilder : public FBXDomBuilder
        {
        public:
            inline FBXDomBuilder* childBegin(const std::string& name) override
            {
                if (name == "UV") {
                    return &uvBuilder;
                }
                return nullptr;
            }

            UVBuilder uvBuilder;
        };

        FBXMesh* target_ = nullptr;
        int i_ = 0;

        VerticesBuilder vertsBuilder_;
        IndicesBuilder idxBuilder_;
        LayerElementNormalBuilder leNormalBuilder_;
        LayerElementUVBuilder leUVBuilder_;

        std::vector<Byte> tmp_;
    };
}

#endif
