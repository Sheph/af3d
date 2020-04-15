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

#include "Mesh.h"
#include "MeshManager.h"
#include "Renderer.h"

namespace af3d
{
    const APropertyTypeObject APropertyType_Mesh{"Mesh", AClass_Mesh};

    ACLASS_DEFINE_BEGIN(Mesh, Resource)
    ACLASS_DEFINE_END(Mesh)

    SubMeshData::SubMeshData()
    : loaded_(false)
    {
    }

    void SubMeshData::invalidate()
    {
        loaded_ = false;
        vertices_.clear();
        faces_.clear();
    }

    void SubMeshData::load(const VertexArraySlice& vaSlice)
    {
        bool old = false;
        if (loaded_.compare_exchange_strong(old, true)) {
            renderer.scheduleHwOpSync([this, &vaSlice](HardwareContext& ctx) {
                const auto& entries = vaSlice.va()->layout().entries();
                for (const auto& entry : entries) {
                    if (entry.name == VertexAttribName::Pos) {
                        const auto& vbo = vaSlice.va()->vbos()[entry.bufferIdx];
                        const auto& ebo = vaSlice.va()->ebo();

                        int minIdx = std::numeric_limits<int>::max();
                        int maxIdx = 0;

                        if (ebo) {
                            std::uint32_t cnt = vaSlice.count() ? vaSlice.count() : (ebo->count(ctx) - vaSlice.start());
                            std::uint16_t* indices = (std::uint16_t*)ebo->lock(vaSlice.start(), cnt, HardwareBuffer::ReadOnly, ctx);
                            if (ebo->dataType() == HardwareIndexBuffer::UInt16) {
                                for (std::uint32_t i = 0; i < cnt; i += 3) {
                                    faces_.emplace_back(vaSlice.baseVertex() + *indices,
                                        vaSlice.baseVertex() + *(indices + 1),
                                        vaSlice.baseVertex() + *(indices + 2));
                                    minIdx = btMin(minIdx, faces_.back().x());
                                    minIdx = btMin(minIdx, faces_.back().y());
                                    minIdx = btMin(minIdx, faces_.back().z());
                                    maxIdx = btMax(maxIdx, faces_.back().x());
                                    maxIdx = btMax(maxIdx, faces_.back().y());
                                    maxIdx = btMax(maxIdx, faces_.back().z());
                                    indices += 3;
                                }
                            } else {
                                runtime_assert(false);
                            }
                            ebo->unlock(ctx);
                            for (auto& face : faces_) {
                                face.setValue(face.x() - minIdx,
                                    face.y() - minIdx,
                                    face.z() - minIdx);
                            }
                        } else {
                            minIdx = vaSlice.start();
                            maxIdx = vaSlice.count() ? (vaSlice.start() + vaSlice.count()) : vbo->count(ctx);
                            --maxIdx;
                            for (int i = 0; i < (maxIdx - minIdx + 1); i += 3) {
                                faces_.emplace_back(i, i + 1, i + 2);
                            }
                        }

                        char* verts = (char*)vbo->lock(minIdx, maxIdx - minIdx + 1, HardwareBuffer::ReadOnly, ctx);
                        if (entry.type == GL_FLOAT_VEC3) {
                            for (int i = minIdx; i <= maxIdx; ++i) {
                                float* pos = (float*)(verts + entry.offset);
                                vertices_.emplace_back(*pos, *(pos + 1), *(pos + 2));
                                verts += vbo->elementSize();
                            }
                        } else {
                            runtime_assert(false);
                        }
                        vbo->unlock(ctx);

                        break;
                    }
                }
            });
        }
    }

    Mesh::Mesh(MeshManager* mgr,
        const std::string& name,
        const AABB& aabb,
        const std::vector<SubMeshPtr>& subMeshes,
        const ResourceLoaderPtr& loader)
    : Resource(AClass_Mesh, name, loader),
      mgr_(mgr),
      aabb_(aabb),
      subMeshes_(subMeshes),
      subMeshesData_(subMeshes.size())
    {
        for (size_t i = 0; i < subMeshesData_.size(); ++i) {
            subMeshesData_[i] = std::make_shared<SubMeshData>();
        }
    }

    Mesh::Mesh(MeshManager* mgr,
        const std::string& name,
        const AABB& aabb,
        const std::vector<SubMeshPtr>& subMeshes,
        const std::vector<SubMeshDataPtr>& subMeshesData,
        const ResourceLoaderPtr& loader)
    : Resource(AClass_Mesh, name, loader),
      mgr_(mgr),
      aabb_(aabb),
      subMeshes_(subMeshes),
      subMeshesData_(subMeshesData)
    {
    }

    Mesh::~Mesh()
    {
        mgr_->onMeshDestroy(this);
    }

    const AClass& Mesh::staticKlass()
    {
        return AClass_Mesh;
    }

    AObjectPtr Mesh::create(const APropertyValueMap& propVals)
    {
        auto name = propVals.get("name").toString();
        return meshManager.loadMesh(name);
    }

    MeshPtr Mesh::clone() const
    {
        std::vector<SubMeshPtr> subMeshes;
        for (const auto& subMesh : subMeshes_) {
            subMeshes.push_back(std::make_shared<SubMesh>(
                subMesh->material()->clone(), subMesh->vaSlice()));
        }
        return meshManager.createMesh(aabb(), subMeshes, subMeshesData_);
    }

    SubMeshDataPtr Mesh::getSubMeshData(int idx)
    {
        btAssert(idx >= 0 && idx < static_cast<int>(subMeshesData_.size()));
        subMeshesData_[idx]->load(subMeshes_[idx]->vaSlice());
        return subMeshesData_[idx];
    }
}
