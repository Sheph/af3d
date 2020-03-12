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
#include "Logger.h"
#include "Platform.h"
#include "af3d/Assert.h"

namespace af3d
{
    const std::string MaterialManager::materialUnlitColoredDefault = "_builtin_unlitColoredDefault";

    static const struct {
        const char* vert;
        const char* frag;
    } shaderPaths[MaterialTypeMax + 1] = {
        {"shaders/2d-default.vert", "shaders/2d-default.frag"},
        {"shaders/2d-color.vert", "shaders/2d-color.frag"},
        {"shaders/unlit-default.vert", "shaders/unlit-default.frag"},
        {"shaders/unlit-color.vert", "shaders/unlit-color.frag"},
        {"shaders/basic-default.vert", "shaders/basic-default.frag"},
        {"shaders/basic-color.vert", "shaders/basic-color.frag"}
    };

    MaterialManager materialManager;

    template <>
    Single<MaterialManager>* Single<MaterialManager>::single = nullptr;

    MaterialManager::~MaterialManager()
    {
        runtime_assert(materialTypes_.empty());
        runtime_assert(cachedMaterials_.empty());
        runtime_assert(immediateMaterials_.empty());
    }

    bool MaterialManager::init()
    {
        LOG4CPLUS_DEBUG(logger(), "materialManager: init...");
        for (int i = MaterialTypeFirst; i <= MaterialTypeMax; ++i) {
            MaterialTypeName name = static_cast<MaterialTypeName>(i);
            materialTypes_[name] = std::make_shared<MaterialType>(name, hwManager.createProgram());
        }

        return true;
    }

    void MaterialManager::shutdown()
    {
        LOG4CPLUS_DEBUG(logger(), "materialManager: shutdown...");
        runtime_assert(materialTypes_.empty());
        runtime_assert(cachedMaterials_.empty());
        runtime_assert(immediateMaterials_.empty());
    }

    void MaterialManager::reload()
    {
        LOG4CPLUS_DEBUG(logger(), "materialManager: reload...");
    }

    bool MaterialManager::renderReload(HardwareContext& ctx)
    {
        LOG4CPLUS_DEBUG(logger(), "materialManager: render reload...");

        for (const auto& mat : materialTypes_) {
            PlatformIFStream isVert(shaderPaths[mat->name()].vert);

            std::string vertSource;

            if (!readStream(isVert, vertSource)) {
                LOG4CPLUS_ERROR(logger(), "Unable to read \"" << shaderPaths[mat->name()].vert << "\"");
                //return false;
            }

            PlatformIFStream isFrag(shaderPaths[mat->name()].frag);

            std::string fragSource;

            if (!readStream(isFrag, fragSource)) {
                LOG4CPLUS_ERROR(logger(), "Unable to read \"" << shaderPaths[mat->name()].frag << "\"");
                //return false;
            }

            if (!mat->reload(vertSource, fragSource, ctx)) {
                //return false;
            }
        }

        if (first_) {
            first_ = false;
            createMaterial(MaterialTypeUnlitColored, materialUnlitColoredDefault);
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
