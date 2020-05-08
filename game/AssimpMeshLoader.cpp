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

        log4cplus::NDCContextCreator ndc(path_);

        std::vector<MaterialPtr> mats(scene_->mNumMaterials);

        for (std::uint32_t i = 0; i < scene_->mNumMaterials; ++i) {
            auto matData = scene_->mMaterials[i];
            std::string matName = path_ + "/" + matData->GetName().C_Str();
            auto mat = materialManager.getMaterial(matName);
            if (!mat) {
                LOG4CPLUS_TRACE(logger(), "-- " << matData->GetName().C_Str() << "--");

                aiString texPath, texPath2, texPath3;
                if (matData->GetTexture(aiTextureType_SHININESS, 0, &texPath) != aiReturn_SUCCESS) {
                    texPath.Clear();
                }
                if (matData->Get("$raw.ReflectionFactor|file", aiTextureType_UNKNOWN, 0, texPath2) != aiReturn_SUCCESS) {
                    texPath2.Clear();
                }

                bool isPBR = (texPath.length > 0) && (texPath2.length > 0);
                bool haveNormalMap = (matData->GetTexture(aiTextureType_NORMALS, 0, &texPath3) == aiReturn_SUCCESS) && scene_->mMeshes[0]->mTangents;
                MaterialTypeName matTypeName;
                if (isPBR) {
                    matTypeName = (haveNormalMap ? MaterialTypePBRNM : MaterialTypePBR);
                } else {
                    matTypeName = (haveNormalMap ? MaterialTypeBasicNM : MaterialTypeBasic);
                }

                mat = materialManager.createMaterial(matTypeName, matName);
                runtime_assert(mat);

                if (isPBR) {
                    LOG4CPLUS_TRACE(logger(), "RoughnessTex: " << texPath.C_Str());
                    mat->setTextureBinding(SamplerName::Roughness,
                        TextureBinding(textureManager.loadTexture(texPath.C_Str())));

                    LOG4CPLUS_TRACE(logger(), "MetalnessTex: " << texPath2.C_Str());
                    mat->setTextureBinding(SamplerName::Metalness,
                        TextureBinding(textureManager.loadTexture(texPath2.C_Str())));
                }

                if (haveNormalMap) {
                    LOG4CPLUS_TRACE(logger(), "NormalTex: " << texPath3.C_Str());
                    mat->setTextureBinding(SamplerName::Normal,
                        TextureBinding(textureManager.loadTexture(texPath3.C_Str())));
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
                    mat->params().setUniform(UniformName::MainColor, fromAssimp(color));
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
                                mat->params().setUniform(UniformName::SpecularColor, fromAssimp(color));
                            } else if (haveSpecularTex) {
                                LOG4CPLUS_TRACE(logger(), "SpecularColor: one");
                                mat->params().setUniform(UniformName::SpecularColor, Color_one);
                            }
                        }
                    }
                }

                if (haveSpecularTex && !haveShininess) {
                    LOG4CPLUS_WARN(logger(), "Have specular texture, but no shininess! Check your model, probably it wasn't saved correctly");
                }

                int twoSided = 0;
                mx = 1;
                if ((aiGetMaterialIntegerArray(matData, AI_MATKEY_TWOSIDED, &twoSided, &mx) == aiReturn_SUCCESS) && twoSided) {
                    LOG4CPLUS_TRACE(logger(), "CullFaceMode: 0");
                    mat->setCullFaceMode(0);
                }
            }
            mats[i] = mat;
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

            VertexArrayLayout vaLayout;
            GLsizeiptr vboElSize = 32;

            vaLayout.addEntry(VertexArrayEntry(VertexAttribName::Pos, GL_FLOAT_VEC3, 0, 0));
            vaLayout.addEntry(VertexArrayEntry(VertexAttribName::UV, GL_FLOAT_VEC2, 12, 0));
            vaLayout.addEntry(VertexArrayEntry(VertexAttribName::Normal, GL_FLOAT_VEC3, 20, 0));

            if ((mats[meshData->mMaterialIndex]->type()->name() == MaterialTypeBasicNM) ||
                (mats[meshData->mMaterialIndex]->type()->name() == MaterialTypePBRNM)) {
                vaLayout.addEntry(VertexArrayEntry(VertexAttribName::Tangent, GL_FLOAT_VEC3, 32, 0));
                vaLayout.addEntry(VertexArrayEntry(VertexAttribName::Bitangent, GL_FLOAT_VEC3, 44, 0));
                vboElSize = 56;
            }

            auto vbo = hwManager.createVertexBuffer(HardwareBuffer::Usage::StaticDraw, vboElSize);
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
            bool withTangent = (mesh.subMeshes()[i]->material()->type()->name() == MaterialTypeBasicNM) ||
                (mesh.subMeshes()[i]->material()->type()->name() == MaterialTypePBRNM);

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
        return assimpImport(importer, path_, aiProcess_CalcTangentSpace |
            aiProcess_JoinIdenticalVertices |
            aiProcess_Triangulate |
            aiProcess_SortByPType);
    }
}
