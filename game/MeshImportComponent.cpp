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

#include "editor/CommandAdd.h"
#include "editor/CommandSetProperty.h"
#include "MeshImportComponent.h"
#include "RenderMeshComponent.h"
#include "MeshManager.h"
#include "Scene.h"
#include "Logger.h"
#include "log4cplus/ndc.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(MeshImportComponent, PhasedComponent)
    ACLASS_PROPERTY(MeshImportComponent, ImportSettings, "import settings", "Import settings", MeshImportSettings, MeshImportSettingsPtr(), General, APropertyEditable)
    ACLASS_PROPERTY(MeshImportComponent, Update, "update", "Update", Bool, false, General, APropertyEditable|APropertyTransient|APropertyUndoable)
    ACLASS_DEFINE_END(MeshImportComponent)

    MeshImportComponent::MeshImportComponent()
    : PhasedComponent(AClass_MeshImportComponent, 0)
    {
    }

    const AClass& MeshImportComponent::staticKlass()
    {
        return AClass_MeshImportComponent;
    }

    AObjectPtr MeshImportComponent::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<MeshImportComponent>();
        obj->propertiesSet(propVals);
        return obj;
    }

    ACommandPtr MeshImportComponent::propertyUpdateSet(const std::string&, const APropertyValue& value)
    {
        if (!value.toBool() || !importSettings_ || !parent() || !scene()->workspace()) {
            // Currently only supported in editor.
            return ACommandPtr();
        }

        std::vector<ACommandPtr> cmds;

        log4cplus::NDCContextCreator ndc("import " + importSettings_->name());

        apply(importSettings_->root(), parent(), parent()->cookie(), parent()->transform(), cmds);

        return std::make_shared<AInplaceCommand>([cmds](bool isRedo) {
            for (const auto& cmd : cmds) {
                if (isRedo) {
                    cmd->redo();
                } else {
                    cmd->undo();
                }
            }
            return true;
        });
    }

    void MeshImportComponent::onRegister()
    {
    }

    void MeshImportComponent::onUnregister()
    {
    }

    void MeshImportComponent::apply(const MeshImportSettings::ObjectEntry& entry, SceneObject* obj, ACookie parentCookie,
        const btTransform& parentXf,
        std::vector<ACommandPtr>& cmds)
    {
        for (const auto& kv : entry.subObjs) {
            auto mesh = meshManager.loadMesh(kv.first);
            if (!mesh) {
                continue;
            }
            std::vector<SceneObjectPtr> objs;
            if (obj) {
                objs = obj->getObjects(kv.second.name);
            }
            SceneObject* nextObj = nullptr;
            ACookie nextCookie = 0;
            btTransform nextXf = parent()->transform() * toTransform(mesh->aabb().scaledAt0(importSettings_->scale()).getCenter());
            if (objs.empty()) {
                APropertyValueMap initVals;
                initVals.set(AProperty_Name, kv.second.name);
                initVals.set(AProperty_WorldTransform, nextXf);
                nextCookie = AObject::allocCookie();
                cmds.push_back(std::make_shared<editor::CommandAdd>(scene(), parentCookie, AClass_SceneObject, "", initVals, nextCookie));
                LOG4CPLUS_DEBUG(logger(), "new object " + kv.second.name + " from " + kv.first);
            } else {
                if (objs.size() > 1) {
                    LOG4CPLUS_WARN(logger(), "ambiguous obj name " + kv.second.name);
                }
                nextObj = objs[0].get();
                nextCookie = nextObj->cookie();
                cmds.push_back(std::make_shared<editor::CommandSetProperty>(scene(), nextObj->sharedThis(), AProperty_WorldTransform, nextXf));
            }
            apply(kv.second, nextObj, nextCookie, nextXf, cmds);
        }

        for (const auto& kv : entry.meshes) {
            auto mesh = meshManager.loadMesh(kv.first);
            if (!mesh) {
                continue;
            }
            std::vector<RenderMeshComponentPtr> rcs;
            if (obj) {
                rcs = obj->findComponents<RenderMeshComponent>(kv.second.name);
            }
            if (rcs.empty()) {
                APropertyValueMap initVals;
                initVals.set(AProperty_Name, kv.second.name);
                initVals.set("mesh", APropertyValue(mesh));
                initVals.set(AProperty_Scale, importSettings_->scale());
                initVals.set(AProperty_LocalTransform, (parent()->transform().inverse() * parentXf).inverse());
                cmds.push_back(std::make_shared<editor::CommandAdd>(scene(), parentCookie, AClass_RenderMeshComponent, "", initVals));
                LOG4CPLUS_DEBUG(logger(), "new component " + kv.second.name + " from " + kv.first);
            } else {
                if (rcs.size() > 1) {
                    LOG4CPLUS_WARN(logger(), "ambiguous component name " + kv.second.name);
                }
                cmds.push_back(std::make_shared<editor::CommandSetProperty>(scene(), rcs[0], "mesh", APropertyValue(mesh)));
                cmds.push_back(std::make_shared<editor::CommandSetProperty>(scene(), rcs[0], AProperty_Scale, importSettings_->scale()));
                cmds.push_back(std::make_shared<editor::CommandSetProperty>(scene(), rcs[0],
                    AProperty_LocalTransform, (parent()->transform().inverse() * parentXf).inverse()));
            }
        }
    }
}
