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
#include "AssetManager.h"
#include "Mesh.h"
#include "Logger.h"
#include "assimp/postprocess.h"
#include "log4cplus/ndc.h"

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

        MaterialTypeName baseMatTypeName = assetManager.getAssetModel(path_)->materialTypeName();

        log4cplus::NDCContextCreator ndc(path_);

        std::vector<MaterialPtr> mats(scene_->mNumMaterials);

        for (std::uint32_t i = 0; i < scene_->mNumMaterials; ++i) {
            auto matData = scene_->mMaterials[i];
            std::string matName = path_ + "/" + matData->GetName().C_Str();
            auto mat = materialManager.getMaterial(matName);
            if (!mat) {
                LOG4CPLUS_TRACE(logger(), "-- " << matData->GetName().C_Str() << "--");

                if (baseMatTypeName == MaterialTypeBasic) {
                    mat = createMaterialBasic(matName, matData);
                } else if (baseMatTypeName == MaterialTypePBR) {
                    mat = createMaterialPBR(matName, matData);
                } else {
                    LOG4CPLUS_WARN(logger(), "Bad material = " << baseMatTypeName << ", defaulting to \"Basic\"");
                    mat = createMaterialBasic(matName, matData);
                }

                int twoSided = 0;
                std::uint32_t mx = 1;
                if ((aiGetMaterialIntegerArray(matData, AI_MATKEY_TWOSIDED, &twoSided, &mx) == aiReturn_SUCCESS) && twoSided) {
                    LOG4CPLUS_TRACE(logger(), "CullFaceMode: 0");
                    mat->setCullFaceMode(0);
                }
            }
            mats[i] = mat;
        }

        std::uint32_t numVertices[2] = {0, 0};

        std::map<std::uint32_t, std::uint32_t> slices;

        for (std::uint32_t i = 0; i < scene_->mNumMeshes; ++i) {
            auto meshData = scene_->mMeshes[i];

            if (mats[meshData->mMaterialIndex]->type()->hasNM()) {
                numVertices[0] += meshData->mNumVertices;
            } else {
                numVertices[1] += meshData->mNumVertices;
            }

            slices[meshData->mMaterialIndex] = 0;
        }

        HardwareVertexBufferPtr vbo[2];

        if (numVertices[0] > 0) {
            vbo[0] = hwManager.createVertexBuffer(HardwareBuffer::Usage::StaticDraw, 56);
        }
        if (numVertices[1] > 0) {
            vbo[1] = hwManager.createVertexBuffer(HardwareBuffer::Usage::StaticDraw, 32);
        }

        for (std::uint32_t i = 0; i < scene_->mNumMeshes; ++i) {
            auto meshData = scene_->mMeshes[i];

            if (i == 0) {
                aabb.lowerBound = fromAssimp(meshData->mVertices[0]);
                aabb.upperBound = fromAssimp(meshData->mVertices[0]);
            }

            for (std::uint32_t j = 0; j < meshData->mNumVertices; ++j) {
                aabb.combine(fromAssimp(meshData->mVertices[j]));
            }

            slices[meshData->mMaterialIndex] += meshData->mNumFaces * 3;
        }

        for (const auto& kv : slices) {
            VertexArrayLayout vaLayout;
            vaLayout.addEntry(VertexArrayEntry(VertexAttribName::Pos, GL_FLOAT_VEC3, 0, 0));
            vaLayout.addEntry(VertexArrayEntry(VertexAttribName::UV, GL_FLOAT_VEC2, 12, 0));
            vaLayout.addEntry(VertexArrayEntry(VertexAttribName::Normal, GL_FLOAT_VEC3, 20, 0));

            if (mats[kv.first]->type()->hasNM()) {
                vaLayout.addEntry(VertexArrayEntry(VertexAttribName::Tangent, GL_FLOAT_VEC3, 32, 0));
                vaLayout.addEntry(VertexArrayEntry(VertexAttribName::Bitangent, GL_FLOAT_VEC3, 44, 0));
                auto ebo = hwManager.createIndexBuffer(HardwareBuffer::Usage::StaticDraw,
                      (numVertices[0] > std::numeric_limits<std::uint16_t>::max()) ? HardwareIndexBuffer::UInt32 : HardwareIndexBuffer::UInt16);
                auto va = std::make_shared<VertexArray>(hwManager.createVertexArray(), vaLayout, VBOList{vbo[0]}, ebo);
                subMeshes.push_back(std::make_shared<SubMesh>(mats[kv.first], VertexArraySlice(va, 0, kv.second, 0)));
            } else {
                auto ebo = hwManager.createIndexBuffer(HardwareBuffer::Usage::StaticDraw,
                      (numVertices[1] > std::numeric_limits<std::uint16_t>::max()) ? HardwareIndexBuffer::UInt32 : HardwareIndexBuffer::UInt16);
                auto va = std::make_shared<VertexArray>(hwManager.createVertexArray(), vaLayout, VBOList{vbo[1]}, ebo);
                subMeshes.push_back(std::make_shared<SubMesh>(mats[kv.first], VertexArraySlice(va, 0, kv.second, 0)));
            }
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

        std::map<std::uint32_t, SubMeshPtr> slices;

        for (std::uint32_t i = 0; i < scene_->mNumMeshes; ++i) {
            auto meshData = scene_->mMeshes[i];
            slices[meshData->mMaterialIndex] = SubMeshPtr();
        }

        size_t i = 0;
        for (auto& kv : slices) {
            kv.second = mesh.subMeshes()[i];
            ++i;
        }
        btAssert(i == mesh.subMeshes().size());

        HardwareVertexBufferPtr vbo[2];
        std::uint32_t numVertices[2] = {0, 0};

        for (std::uint32_t i = 0; i < scene_->mNumMeshes; ++i) {
            auto meshData = scene_->mMeshes[i];
            if (slices[meshData->mMaterialIndex]->material()->type()->hasNM()) {
                numVertices[0] += meshData->mNumVertices;
                vbo[0] = slices[meshData->mMaterialIndex]->vaSlice().va()->vbos()[0];
            } else {
                numVertices[1] += meshData->mNumVertices;
                vbo[1] = slices[meshData->mMaterialIndex]->vaSlice().va()->vbos()[0];
            }
        }

        float *allVerts[2], *vertsStart[2];
        std::map<std::uint32_t, GLvoid*> allIndices;
        std::map<std::uint32_t, GLvoid*> indicesStart;

        for (int i = 0; i < 2; ++i) {
            if (vbo[i]) {
                vbo[i]->resize(numVertices[i], ctx);
                allVerts[i] = vertsStart[i] = (float*)vbo[i]->lock(HardwareBuffer::WriteOnly, ctx);
            }
        }

        for (const auto& kv : slices) {
            auto ebo = kv.second->vaSlice().va()->ebo();
            ebo->resize(kv.second->vaSlice().count(), ctx);
            allIndices[kv.first] = indicesStart[kv.first] = ebo->lock(HardwareBuffer::WriteOnly, ctx);
        }

        numVertices[0] = 0;
        numVertices[1] = 0;

        for (std::uint32_t i = 0; i < scene_->mNumMeshes; ++i) {
            auto meshData = scene_->mMeshes[i];
            bool withTangent = slices[meshData->mMaterialIndex]->material()->type()->hasNM();

            float*& verts = allVerts[withTangent ? 0 : 1];

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
                if (withTangent) {
                    *verts = meshData->mTangents[j].x;
                    ++verts;
                    *verts = meshData->mTangents[j].y;
                    ++verts;
                    *verts = meshData->mTangents[j].z;
                    ++verts;
                    *verts = meshData->mBitangents[j].x;
                    ++verts;
                    *verts = meshData->mBitangents[j].y;
                    ++verts;
                    *verts = meshData->mBitangents[j].z;
                    ++verts;
                }
            }

            auto cva = slices[meshData->mMaterialIndex]->vaSlice().va();
            std::uint32_t idxOffset = numVertices[withTangent ? 0 : 1];

            if (cva->ebo()->dataType() == HardwareIndexBuffer::UInt16) {
                std::uint16_t*& indices = (std::uint16_t*&)allIndices[meshData->mMaterialIndex];

                for (std::uint32_t j = 0; j < meshData->mNumFaces; ++j) {
                    auto face = meshData->mFaces[j];
                    btAssert(face.mNumIndices == 3);
                    btAssert(face.mIndices[0] + idxOffset <= std::numeric_limits<std::uint16_t>::max());
                    btAssert(face.mIndices[1] + idxOffset <= std::numeric_limits<std::uint16_t>::max());
                    btAssert(face.mIndices[2] + idxOffset <= std::numeric_limits<std::uint16_t>::max());
                    *indices = face.mIndices[0] + idxOffset;
                    ++indices;
                    *indices = face.mIndices[1] + idxOffset;
                    ++indices;
                    *indices = face.mIndices[2] + idxOffset;
                    ++indices;
                }
            } else {
                std::uint32_t*& indices = (std::uint32_t*&)allIndices[meshData->mMaterialIndex];

                for (std::uint32_t j = 0; j < meshData->mNumFaces; ++j) {
                    auto face = meshData->mFaces[j];
                    btAssert(face.mNumIndices == 3);
                    *indices = face.mIndices[0] + idxOffset;
                    ++indices;
                    *indices = face.mIndices[1] + idxOffset;
                    ++indices;
                    *indices = face.mIndices[2] + idxOffset;
                    ++indices;
                }
            }

            numVertices[withTangent ? 0 : 1] += meshData->mNumVertices;
        }

        for (int i = 0; i < 2; ++i) {
            if (vbo[i]) {
                btAssert((allVerts[i] - vertsStart[i]) * 4 == vbo[i]->sizeInBytes(ctx));
                vbo[i]->unlock(ctx);
            }
        }

        for (const auto& kv : slices) {
            auto ebo = kv.second->vaSlice().va()->ebo();
            btAssert(((char*)allIndices[kv.first] - (char*)indicesStart[kv.first]) == ebo->sizeInBytes(ctx));
            ebo->unlock(ctx);
        }

        scene_.reset();
    }

    AssimpScenePtr AssimpMeshLoader::loadScene(Assimp::Importer& importer)
    {
        std::uint32_t flags = 0;
        if (assetManager.getAssetModel(path_)->flipUV()) {
            flags |= aiProcess_FlipUVs;
        }
        return assimpImport(importer, path_, flags | aiProcess_CalcTangentSpace |
            aiProcess_JoinIdenticalVertices |
            aiProcess_Triangulate |
            aiProcess_SortByPType);
    }

    MaterialPtr AssimpMeshLoader::createMaterialBasic(const std::string& matName, aiMaterial* matData)
    {
        aiString texPath;

        bool haveNormalMap = (matData->GetTexture(aiTextureType_NORMALS, 0, &texPath) == aiReturn_SUCCESS) &&
            scene_->mMeshes[0]->mTangents;
        MaterialTypeName matTypeName = (haveNormalMap ? MaterialTypeBasicNM : MaterialTypeBasic);

        auto mat = materialManager.createMaterial(matTypeName, matName);
        runtime_assert(mat);

        if (haveNormalMap) {
            LOG4CPLUS_TRACE(logger(), "NormalTex: " << texPath.C_Str());
            mat->setTextureBinding(SamplerName::Normal,
                TextureBinding(textureManager.loadTexture(texPath.C_Str())));
        }

        if (matData->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == aiReturn_SUCCESS) {
            LOG4CPLUS_TRACE(logger(), "DiffuseTex: " << texPath.C_Str());
            mat->setTextureBinding(SamplerName::Main,
                TextureBinding(textureManager.loadTexture(texPath.C_Str())));
        }

        bool haveSpecularTex = false;

        if (matData->GetTexture(aiTextureType_SPECULAR, 0, &texPath) == aiReturn_SUCCESS) {
            LOG4CPLUS_TRACE(logger(), "SpecularTex: " << texPath.C_Str());
            mat->setTextureBinding(SamplerName::Specular,
                TextureBinding(textureManager.loadTexture(texPath.C_Str())));
            haveSpecularTex = true;
        } else if (mat->textureBinding(SamplerName::Main).tex) {
            mat->setTextureBinding(SamplerName::Specular, mat->textureBinding(SamplerName::Main));
        }

        aiColor4D color;
        if (!mat->textureBinding(SamplerName::Main).tex &&
            (aiGetMaterialColor(matData, AI_MATKEY_COLOR_DIFFUSE, &color) == aiReturn_SUCCESS)) {
            LOG4CPLUS_TRACE(logger(), "MainColor: " << fromAssimp(color));
            mat->params().setUniform(UniformName::MainColor, gammaToLinear(fromAssimp(color)));
        }

        float val;
        std::uint32_t mx = 1;
        bool haveShininess = false;
        if (aiGetMaterialFloatArray(matData, AI_MATKEY_SHININESS, &val, &mx) == aiReturn_SUCCESS) {
            float val2;
            mx = 1;
            if (aiGetMaterialFloatArray(matData, AI_MATKEY_SHININESS_STRENGTH, &val2, &mx) == aiReturn_SUCCESS) {
                if (val * val2 > SIMD_EPSILON) {
                    LOG4CPLUS_TRACE(logger(), "Shininess: " << val * val2);
                    mat->params().setUniform(UniformName::Shininess, val * val2);
                    haveShininess = true;
                    if (!haveSpecularTex && (aiGetMaterialColor(matData, AI_MATKEY_COLOR_SPECULAR, &color) == aiReturn_SUCCESS)) {
                        LOG4CPLUS_TRACE(logger(), "SpecularColor: " << fromAssimp(color));
                        mat->params().setUniform(UniformName::SpecularColor, gammaToLinear(fromAssimp(color)));
                    } else if (haveSpecularTex) {
                        LOG4CPLUS_TRACE(logger(), "SpecularColor: one");
                        mat->params().setUniform(UniformName::SpecularColor, gammaToLinear(Color_one));
                    }
                }
            }
        }

        if (haveSpecularTex && !haveShininess) {
            LOG4CPLUS_WARN(logger(), "Have specular texture, but no shininess! Check your model, probably it wasn't saved correctly");
        }

        return mat;
    }

    MaterialPtr AssimpMeshLoader::createMaterialPBR(const std::string& matName, aiMaterial* matData)
    {
        aiString texPath;

        bool haveNormalMap = (matData->GetTexture(aiTextureType_NORMALS, 0, &texPath) == aiReturn_SUCCESS) &&
            scene_->mMeshes[0]->mTangents;
        MaterialTypeName matTypeName = (haveNormalMap ? MaterialTypePBRNM : MaterialTypePBR);

        auto mat = materialManager.createMaterial(matTypeName, matName);
        runtime_assert(mat);

        if (haveNormalMap) {
            LOG4CPLUS_TRACE(logger(), "NormalTex: " << texPath.C_Str());
            mat->setTextureBinding(SamplerName::Normal,
                TextureBinding(textureManager.loadTexture(texPath.C_Str())));
        }

        if (matData->GetTexture(aiTextureType_DIFFUSE, 0, &texPath) == aiReturn_SUCCESS) {
            LOG4CPLUS_TRACE(logger(), "DiffuseTex: " << texPath.C_Str());
            mat->setTextureBinding(SamplerName::Main,
                TextureBinding(textureManager.loadTexture(texPath.C_Str())));
        }

        aiColor4D color;
        if (!mat->textureBinding(SamplerName::Main).tex &&
            (aiGetMaterialColor(matData, AI_MATKEY_COLOR_DIFFUSE, &color) == aiReturn_SUCCESS)) {
            LOG4CPLUS_TRACE(logger(), "MainColor: " << fromAssimp(color));
            mat->params().setUniform(UniformName::MainColor, gammaToLinear(fromAssimp(color)));
        }

        if (matData->GetTexture(aiTextureType_SHININESS, 0, &texPath) == aiReturn_SUCCESS) {
            LOG4CPLUS_TRACE(logger(), "RoughnessTex: " << texPath.C_Str());
            mat->setTextureBinding(SamplerName::Roughness,
                TextureBinding(textureManager.loadTexture(texPath.C_Str())));
        }

        if (matData->Get("$raw.ReflectionFactor|file", aiTextureType_UNKNOWN, 0, texPath) == aiReturn_SUCCESS) {
            LOG4CPLUS_TRACE(logger(), "MetalnessTex: " << texPath.C_Str());
            mat->setTextureBinding(SamplerName::Metalness,
                TextureBinding(textureManager.loadTexture(texPath.C_Str())));
        }

        if (matData->GetTexture(aiTextureType_SPECULAR, 0, &texPath) == aiReturn_SUCCESS) {
            LOG4CPLUS_TRACE(logger(), "AOTex: " << texPath.C_Str());
            mat->setTextureBinding(SamplerName::AO,
                TextureBinding(textureManager.loadTexture(texPath.C_Str())));
        }

        if (matData->GetTexture(aiTextureType_EMISSIVE, 0, &texPath) == aiReturn_SUCCESS) {
            LOG4CPLUS_TRACE(logger(), "EmissiveTex: " << texPath.C_Str());
            mat->setTextureBinding(SamplerName::Emissive,
                TextureBinding(textureManager.loadTexture(texPath.C_Str())));
            float val;
            std::uint32_t mx = 1;
            if (aiGetMaterialFloatArray(matData, "$raw.EmissionFactor", 0, 0, &val, &mx) == aiReturn_SUCCESS) {
                LOG4CPLUS_TRACE(logger(), "EmissiveFactor: " << val);
                mat->params().setUniform(UniformName::EmissiveFactor, val);
            }
        } else {
            mat->setTextureBinding(SamplerName::Emissive,
                TextureBinding(textureManager.black1x1(), SamplerParams(GL_NEAREST, GL_NEAREST)));
        }

        return mat;
    }
}
