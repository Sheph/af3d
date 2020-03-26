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

#include "MaterialManager.h"
#include "HardwareResourceManager.h"
#include "TextureManager.h"
#include "Logger.h"
#include "Platform.h"
#include "af3d/Assert.h"

namespace af3d
{
    const std::string MaterialManager::materialUnlitDefault = "_builtin_unlitDefault";

    static const struct {
        const char* vert;
        const char* frag;
        bool usesLight;
    } shaders[MaterialTypeMax + 1] = {
        {"shaders/unlit.vert", "shaders/unlit.frag", false},
        {"shaders/basic.vert", "shaders/basic.frag", true},
        {"shaders/imm.vert", "shaders/imm.frag", false}
    };

    MaterialManager materialManager;

    template <>
    Single<MaterialManager>* Single<MaterialManager>::single = nullptr;

    MaterialManager::~MaterialManager()
    {
        runtime_assert(cachedMaterials_.empty());
        runtime_assert(immediateMaterials_.empty());
    }

    bool MaterialManager::init()
    {
        LOG4CPLUS_DEBUG(logger(), "materialManager: init...");
        for (int i = MaterialTypeFirst; i <= MaterialTypeMax; ++i) {
            MaterialTypeName name = static_cast<MaterialTypeName>(i);
            materialTypes_[name] = std::make_shared<MaterialType>(name, hwManager.createProgram(), shaders[name].usesLight);
        }

        return true;
    }

    void MaterialManager::shutdown()
    {
        LOG4CPLUS_DEBUG(logger(), "materialManager: shutdown...");
        runtime_assert(immediateMaterials_.empty());
        cachedMaterials_.clear();
        for (int i = MaterialTypeFirst; i <= MaterialTypeMax; ++i) {
            MaterialTypeName name = static_cast<MaterialTypeName>(i);
            materialTypes_[name].reset();
        }
    }

    void MaterialManager::reload()
    {
        LOG4CPLUS_DEBUG(logger(), "materialManager: reload...");
    }

    bool MaterialManager::renderReload(HardwareContext& ctx)
    {
        LOG4CPLUS_DEBUG(logger(), "materialManager: render reload...");

        for (const auto& mat : materialTypes_) {
            LOG4CPLUS_DEBUG(logger(), "materialManager: loading " << shaders[mat->name()].vert << "...");

            PlatformIFStream isVert(shaders[mat->name()].vert);

            std::string vertSource;

            if (!readStream(isVert, vertSource)) {
                LOG4CPLUS_ERROR(logger(), "Unable to read \"" << shaders[mat->name()].vert << "\"");
                return false;
            }

            LOG4CPLUS_DEBUG(logger(), "materialManager: loading " << shaders[mat->name()].frag << "...");

            PlatformIFStream isFrag(shaders[mat->name()].frag);

            std::string fragSource;

            if (!readStream(isFrag, fragSource)) {
                LOG4CPLUS_ERROR(logger(), "Unable to read \"" << shaders[mat->name()].frag << "\"");
                return false;
            }

            if (!mat->reload(vertSource, fragSource, ctx)) {
                return false;
            }

            mat->setDefaultUniform(UniformName::MainColor, Color_one);
            mat->setDefaultUniform(UniformName::SpecularColor, Color(0.0f, 0.0f, 0.0f, 1.0f));
            mat->setDefaultUniform(UniformName::Shininess, 1.0f);
        }

        if (first_) {
            first_ = false;
            createMaterial(MaterialTypeUnlit, materialUnlitDefault);
        }

        return true;
    }

    MaterialTypePtr MaterialManager::getMaterialType(MaterialTypeName name)
    {
        return materialTypes_[name];
    }

    MaterialPtr MaterialManager::getMaterial(const std::string& name)
    {
        auto it = cachedMaterials_.find(name);
        if (it == cachedMaterials_.end()) {
            return MaterialPtr();
        }
        return it->second;
    }

    MaterialPtr MaterialManager::loadImmMaterial(const std::string& texturePath, const SamplerParams& params)
    {
        std::string matName = "_bmatImm/" + texturePath + "@" + params.toString();

        auto it = cachedMaterials_.find(matName);
        if (it != cachedMaterials_.end()) {
            return it->second;
        }

        auto tex = textureManager.loadTexture(texturePath);

        auto mat = createMaterial(MaterialTypeImm, matName);

        mat->setTextureBinding(SamplerName::Main, TextureBinding(tex, params));
        mat->setBlendingParams(BlendingParams(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        mat->setDepthTest(false);

        return mat;
    }

    MaterialPtr MaterialManager::createMaterial(MaterialTypeName typeName, const std::string& name)
    {
        auto material = std::make_shared<Material>(this, name, getMaterialType(typeName));

        if (name.empty()) {
            immediateMaterials_.insert(material.get());
        } else {
            if (!cachedMaterials_.emplace(name, material).second) {
                LOG4CPLUS_ERROR(logger(), "Material " << name << " already exists");
                return MaterialPtr();
            }

            LOG4CPLUS_DEBUG(logger(), "materialManager: creating material (" << typeName << ") " << name << "...");
        }

        return material;
    }

    bool MaterialManager::onMaterialClone(const MaterialPtr& material)
    {
        if (material->name().empty()) {
            immediateMaterials_.insert(material.get());
            return true;
        }

        if (!cachedMaterials_.emplace(material->name(), material).second) {
            LOG4CPLUS_ERROR(logger(), "Material " << material->name() << " already exists");
            return false;
        }

        return true;
    }

    void MaterialManager::onMaterialDestroy(Material* material)
    {
        immediateMaterials_.erase(material);
    }
}
