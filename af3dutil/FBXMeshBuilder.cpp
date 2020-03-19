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

#include "af3d/FBXMeshBuilder.h"
#include "Logger.h"

namespace af3d
{
    double* FBXMeshBuilder::VerticesBuilder::addArrayDouble(std::uint32_t count)
    {
        if ((count % 3) != 0) {
            LOG4CPLUS_WARN(af3dutil::logger(), "MeshVerticesBuilder: bad count " << count);
        }

        parent->tmp_.resize(count * 8);

        return (double*)&parent->tmp_[0];
    }

    void FBXMeshBuilder::VerticesBuilder::endArrayDouble(double* value, std::uint32_t count)
    {
        count /= 3;
        auto& verts = parent->target_->vertices();
        verts.resize(count);
        for (std::uint32_t i = 0; i < count; ++i) {
            verts[i].setX(*value);
            ++value;
            verts[i].setY(*value);
            ++value;
            verts[i].setZ(*value);
            ++value;
        }
    }

    std::int32_t* FBXMeshBuilder::IndicesBuilder::addArrayInt32(std::uint32_t count)
    {
        if ((count % 4) != 0) {
            LOG4CPLUS_WARN(af3dutil::logger(), "MeshIndicesBuilder: bad count " << count);
        }

        parent->tmp_.resize(count * 4);

        return (std::int32_t*)&parent->tmp_[0];
    }

    void FBXMeshBuilder::IndicesBuilder::endArrayInt32(std::int32_t* value, std::uint32_t count)
    {
        auto& idxs = parent->target_->indices();
        idxs.reserve((count / 4) * 3);
        for (std::uint32_t i = 0; i < count; ++i) {
            if (value[i] >= 0) {
                btAssert(value[i] <= std::numeric_limits<std::uint16_t>::max());
                idxs.push_back(value[i]);
            }
        }
        idxs.resize((idxs.size() / 3) * 3);
    }

    double* FBXMeshBuilder::NormalsBuilder::addArrayDouble(std::uint32_t count)
    {
        if ((count % 3) != 0) {
            LOG4CPLUS_WARN(af3dutil::logger(), "MeshNormalsBuilder: bad count " << count);
        }

        parent->tmp_.resize(count * 8);

        return (double*)&parent->tmp_[0];
    }

    void FBXMeshBuilder::NormalsBuilder::endArrayDouble(double* value, std::uint32_t count)
    {
        count /= 3;
        auto& norms = parent->target_->normals();
        norms.resize(count);
        for (std::uint32_t i = 0; i < count; ++i) {
            norms[i].setX(*value);
            ++value;
            norms[i].setY(*value);
            ++value;
            norms[i].setZ(*value);
            ++value;
        }
    }

    double* FBXMeshBuilder::UVBuilder::addArrayDouble(std::uint32_t count)
    {
        if ((count % 2) != 0) {
            LOG4CPLUS_WARN(af3dutil::logger(), "MeshUVBuilder: bad count " << count);
        }

        parent->tmp_.resize(count * 8);

        return (double*)&parent->tmp_[0];
    }

    void FBXMeshBuilder::UVBuilder::endArrayDouble(double* value, std::uint32_t count)
    {
        count /= 2;
        auto& uvs = parent->target_->uvs();
        uvs.resize(count);
        for (std::uint32_t i = 0; i < count; ++i) {
            uvs[i].setX(*value);
            ++value;
            uvs[i].setY(*value);
            ++value;
        }
    }

    FBXMeshBuilder::FBXMeshBuilder()
    {
        vertsBuilder_.parent = this;
        idxBuilder_.parent = this;
        leNormalBuilder_.normalsBuilder.parent = this;
        leUVBuilder_.uvBuilder.parent = this;
    }

    void FBXMeshBuilder::setTarget(FBXMesh* target)
    {
        target_ = target;
        i_ = 0;
    }

    void FBXMeshBuilder::addValue(std::int64_t value)
    {
        if (i_ == 0) {
            target_->setUID(value);
        }
        ++i_;
    }

    void FBXMeshBuilder::addValue(const std::string& value)
    {
        if (i_ == 1) {
            target_->setName(value);
        }
        ++i_;
    }

    FBXDomBuilder* FBXMeshBuilder::childBegin(const std::string& name)
    {
        if (name == "Vertices") {
            return &vertsBuilder_;
        } else if (name == "PolygonVertexIndex") {
            return &idxBuilder_;
        } else if (name == "LayerElementNormal") {
            return &leNormalBuilder_;
        } else if (name == "LayerElementUV") {
            return &leUVBuilder_;
        }
        return nullptr;
    }
}
