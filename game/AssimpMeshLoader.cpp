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

#include "AssimpMeshLoader.h"
#include "MaterialManager.h"
#include "HardwareResourceManager.h"
#include "TextureManager.h"
#include "Mesh.h"
#include "Logger.h"
#include "assimp/postprocess.h"

namespace af3d
{
    AssimpMeshLoader::AssimpMeshLoader(const std::string& path)
    : path_(path)
    {
    }

    bool AssimpMeshLoader::init(Assimp::Importer& importer, AABB& aabb, std::vector<SubMeshPtr>& subMeshes)
    {
        scene_ = loadScene(importer);
        if (!scene_) {
            return false;
        }

        std::vector<MaterialPtr> mats(scene_->mNumMaterials);

        for (std::uint32_t i = 0; i < scene_->mNumMaterials; ++i) {
            auto matData = scene_->mMaterials[i];
            std::string matName = path_ + "/" + matData->GetName().C_Str();
            auto mat = materialManager.getMaterial(matName);
            if (!mat) {
                mat = materialManager.createMaterial(MaterialTypeBasic, matName);
                runtime_assert(mat);
                aiString texPath;
                if (matData->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == aiReturn_SUCCESS) {
                    mat->setTextureBinding(SamplerName::Main,
                        TextureBinding(textureManager.loadTexture(texPath.C_Str())));
                }
                aiColor4D color;
                if (aiGetMaterialColor(matData, AI_MATKEY_COLOR_DIFFUSE, &color) == aiReturn_SUCCESS) {
                    mat->params().setUniform(UniformName::MainColor, fromAssimp(color));
                }
                if (aiGetMaterialColor(matData, AI_MATKEY_COLOR_SPECULAR, &color) == aiReturn_SUCCESS) {
                    mat->params().setUniform(UniformName::SpecularColor, fromAssimp(color));
                }
                float val;
                std::uint32_t mx = 1;
                if (aiGetMaterialFloatArray(matData, AI_MATKEY_SHININESS, &val, &mx) == aiReturn_SUCCESS) {
                    mat->params().setUniform(UniformName::Shininess, val);
                }
            }
            mats[i] = mat;
        }

        VertexArrayLayout vaLayout;

        vaLayout.addEntry(VertexArrayEntry(VertexAttribName::Pos, GL_FLOAT_VEC3, 0, 0));
        vaLayout.addEntry(VertexArrayEntry(VertexAttribName::UV, GL_FLOAT_VEC2, 12, 0));
        vaLayout.addEntry(VertexArrayEntry(VertexAttribName::Normal, GL_FLOAT_VEC3, 20, 0));

        for (std::uint32_t i = 0; i < scene_->mNumMeshes; ++i) {
            auto meshData = scene_->mMeshes[i];

            if (i == 0) {
                aabb.lowerBound = fromAssimp(meshData->mVertices[0]);
                aabb.upperBound = fromAssimp(meshData->mVertices[0]);
            }

            for (std::uint32_t j = 0; j < meshData->mNumVertices; ++j) {
                aabb.combine(fromAssimp(meshData->mVertices[j]));
            }

            auto vbo = hwManager.createVertexBuffer(HardwareBuffer::Usage::StaticDraw, 32);
            auto ebo = hwManager.createIndexBuffer(HardwareBuffer::Usage::StaticDraw, HardwareIndexBuffer::UInt16);

            auto va = std::make_shared<VertexArray>(hwManager.createVertexArray(), vaLayout, VBOList{vbo}, ebo);

            auto subMesh = std::make_shared<SubMesh>(mats[meshData->mMaterialIndex], VertexArraySlice(va));

            subMeshes.push_back(subMesh);
        }

        return true;
    }

    void AssimpMeshLoader::load(Resource& res, HardwareContext& ctx)
    {
        Mesh& mesh = static_cast<Mesh&>(res);

        if (!scene_) {
            scene_ = loadScene(ctx.importer());
            runtime_assert(scene_);
        }

        for (std::uint32_t i = 0; i < scene_->mNumMeshes; ++i) {
            auto meshData = scene_->mMeshes[i];
            const auto& va = mesh.subMeshes()[i]->vaSlice().va();
            auto vbo = va->vbos()[0];
            auto ebo = va->ebo();

            vbo->resize(meshData->mNumVertices, ctx);
            ebo->resize(meshData->mNumFaces * 3, ctx);

            float *verts, *vertsStart;
            std::uint16_t *indices, *indicesStart;

            verts = vertsStart = (float*)vbo->lock(HardwareBuffer::WriteOnly, ctx);
            indices = indicesStart = (std::uint16_t*)ebo->lock(HardwareBuffer::WriteOnly, ctx);

            btAssert(meshData->mTextureCoords[0]);

            for (std::uint32_t j = 0; j < meshData->mNumVertices; ++j) {
                *verts = meshData->mVertices[j].x;
                ++verts;
                *verts = meshData->mVertices[j].y;
                ++verts;
                *verts = meshData->mVertices[j].z;
                ++verts;
                *verts = meshData->mTextureCoords[0][j].x;
                ++verts;
                *verts = meshData->mTextureCoords[0][j].y;
                ++verts;
                *verts = meshData->mNormals[j].x;
                ++verts;
                *verts = meshData->mNormals[j].y;
                ++verts;
                *verts = meshData->mNormals[j].z;
                ++verts;
            }

            for (std::uint32_t j = 0; j < meshData->mNumFaces; ++j) {
                auto face = meshData->mFaces[j];
                btAssert(face.mNumIndices == 3);
                btAssert(face.mIndices[0] <= std::numeric_limits<std::uint16_t>::max());
                btAssert(face.mIndices[1] <= std::numeric_limits<std::uint16_t>::max());
                btAssert(face.mIndices[2] <= std::numeric_limits<std::uint16_t>::max());
                *indices = face.mIndices[0];
                ++indices;
                *indices = face.mIndices[1];
                ++indices;
                *indices = face.mIndices[2];
                ++indices;
            }

            btAssert((verts - vertsStart) * 4 == vbo->sizeInBytes(ctx));
            btAssert((indices - indicesStart) == ebo->count(ctx));

            ebo->unlock(ctx);
            vbo->unlock(ctx);
        }

        scene_.reset();
    }

    AssimpScenePtr AssimpMeshLoader::loadScene(Assimp::Importer& importer)
    {
        return assimpImport(importer, path_, aiProcess_Triangulate);
    }
}
