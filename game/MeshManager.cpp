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

#include "MeshManager.h"
#include "HardwareResourceManager.h"
#include "Logger.h"
#include "af3d/Assert.h"

namespace af3d
{
    namespace
    {
        class BoxMeshGenerator : public ResourceLoader
        {
        public:
            BoxMeshGenerator(const btVector3& size, const std::array<Color, 6>& colors)
            : size_(size),
              colors_(colors)
            {
            }

            void load(Resource& res, HardwareContext& ctx) override
            {
                Mesh& mesh = static_cast<Mesh&>(res);

                auto vbo = mesh.va()->vbos()[0];
                auto ebo = mesh.va()->ebo();

                vbo->resize(4 * 6, ctx); // 4 verts per face
                ebo->resize(6 * 6, ctx); // 6 indices per face

                float* verts = (float*)vbo->lock(HardwareBuffer::WriteOnly, ctx);
                std::uint16_t* indices = (std::uint16_t*)ebo->lock(HardwareBuffer::WriteOnly, ctx);

                for (int face = 0; face < 6; ++face) {
                }

                ebo->unlock(ctx);
                vbo->unlock(ctx);
            }

        private:
            btVector3 size_;
            std::array<Color, 6> colors_;
        };
    }

    MeshManager meshManager;

    template <>
    Single<MeshManager>* Single<MeshManager>::single = nullptr;

    MeshManager::~MeshManager()
    {
        runtime_assert(cachedMeshes_.empty());
        runtime_assert(immediateMeshes_.empty());
    }

    bool MeshManager::init()
    {
        LOG4CPLUS_DEBUG(logger(), "meshManager: init...");
        return true;
    }

    void MeshManager::shutdown()
    {
        LOG4CPLUS_DEBUG(logger(), "meshManager: shutdown...");
        runtime_assert(cachedMeshes_.empty());
        runtime_assert(immediateMeshes_.empty());
    }

    void MeshManager::reload()
    {
        LOG4CPLUS_DEBUG(logger(), "meshManager: reload...");
        for (const auto& kv : cachedMeshes_) {
            kv.second->invalidate();
            kv.second->load();
        }
        for (auto mesh : immediateMeshes_) {
            mesh->invalidate();
            mesh->load();
        }
    }

    bool MeshManager::renderReload(HardwareContext& ctx)
    {
        LOG4CPLUS_DEBUG(logger(), "meshManager: render reload...");
        return true;
    }

    MeshPtr MeshManager::loadMesh(const std::string& path)
    {
        auto it = cachedMeshes_.find(path);
        if (it != cachedMeshes_.end()) {
            return it->second;
        }

        MeshPtr mesh; // = ...;
        mesh->load();
        cachedMeshes_.emplace(path, mesh);

        return mesh;
    }

    MeshPtr MeshManager::createMesh(const AABB& aabb,
        const std::vector<SubMeshPtr>& subMeshes,
        const ResourceLoaderPtr& loader)
    {
        auto mesh = std::make_shared<Mesh>(this, "", aabb, subMeshes, subMeshes[0]->vaSlice().va(), loader);
        mesh->load();
        immediateMeshes_.insert(mesh.get());
        return mesh;
    }

    MeshPtr MeshManager::createBoxMesh(const btVector3& size,
        const MaterialPtr& material, const std::array<Color, 6>& colors)
    {
        VertexArrayLayout vaLayout;

        vaLayout.addEntry(VertexArrayEntry(VertexAttribName::Pos, GL_FLOAT_VEC3, 0, 0));
        vaLayout.addEntry(VertexArrayEntry(VertexAttribName::Color, GL_FLOAT_VEC4, 0, 12));

        auto vbo = hwManager.createVertexBuffer(HardwareBuffer::Usage::StaticDraw, 28);
        auto ebo = hwManager.createIndexBuffer(HardwareBuffer::Usage::StaticDraw, HardwareIndexBuffer::UInt16);

        auto va = std::make_shared<VertexArray>(vaLayout, VertexArray::VBOList{vbo}, ebo);

        auto subMesh = std::make_shared<SubMesh>(material, VertexArraySlice(va));

        return createMesh(AABB(btVector3_zero - (size / 2), btVector3_zero + (size / 2)),
            std::vector<SubMeshPtr>{subMesh}, std::make_shared<BoxMeshGenerator>(size, colors));
    }

    void MeshManager::onMeshDestroy(Mesh* mesh)
    {
        immediateMeshes_.erase(mesh);
    }
}
