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

#include "ImGuiComponent.h"
#include "ImGuiManager.h"
#include "MaterialManager.h"

namespace af3d
{
    ACLASS_DEFINE_BEGIN(ImGuiComponent, UIComponent)
    ACLASS_DEFINE_END(ImGuiComponent)

    ImGuiComponent::ImGuiComponent(int zOrder)
    : UIComponent(AClass_ImGuiComponent, zOrder)
    {
        material_ = materialManager.createMaterial(MaterialTypeImm);
        material_->setBlendingParams(BlendingParams(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
        material_->setDepthTest(false);
        material_->setCullFaceMode(0);
    }

    const AClass& ImGuiComponent::staticKlass()
    {
        return AClass_ImGuiComponent;
    }

    AObjectPtr ImGuiComponent::create(const APropertyValueMap& propVals)
    {
        auto obj = std::make_shared<ImGuiComponent>();
        obj->propertiesSet(propVals);
        return obj;
    }

    void ImGuiComponent::render(RenderList& rl)
    {
        ImGui::Render();

        ImDrawData* drawData = ImGui::GetDrawData();

        int fbWidth = (int)(drawData->DisplaySize.x * drawData->FramebufferScale.x);
        int fbHeight = (int)(drawData->DisplaySize.y * drawData->FramebufferScale.y);

        if (fbWidth <= 0 || fbHeight <= 0) {
            return;
        }

        float depthValue = zOrder();

        MaterialPtr mat;
        ImTextureID lastTexId;

        ImVec2 clipOff = drawData->DisplayPos;
        ImVec2 clipScale = drawData->FramebufferScale;

        for (int n = 0; n < drawData->CmdListsCount; ++n) {
            const ImDrawList* cmdList = drawData->CmdLists[n];

            for (int i = 0; i < cmdList->VtxBuffer.Size; ++i) {
                cmdList->VtxBuffer.Data[i].posZ = 0.0f;
            }

            auto vaSlice = rl.createGeometry((const VertexImm*)cmdList->VtxBuffer.Data, cmdList->VtxBuffer.Size,
                cmdList->IdxBuffer.Data, cmdList->IdxBuffer.Size);

            for (int i = 0; i < cmdList->CmdBuffer.Size; ++i) {
                const ImDrawCmd* pcmd = &cmdList->CmdBuffer[i];
                if (pcmd->UserCallback != nullptr) {
                    if (pcmd->UserCallback == ImDrawCallback_ResetRenderState) {
                        // Rendering is not immediate in our engine, how to reset ?
                        runtime_assert(false);
                    } else {
                        pcmd->UserCallback(cmdList, pcmd);
                    }
                } else {
                    ImVec4 clipRect;
                    clipRect.x = (pcmd->ClipRect.x - clipOff.x) * clipScale.x;
                    clipRect.y = (pcmd->ClipRect.y - clipOff.y) * clipScale.y;
                    clipRect.z = (pcmd->ClipRect.z - clipOff.x) * clipScale.x;
                    clipRect.w = (pcmd->ClipRect.w - clipOff.y) * clipScale.y;

                    if (clipRect.x < fbWidth && clipRect.y < fbHeight && clipRect.z >= 0.0f && clipRect.w >= 0.0f) {
                        ScissorParams scissorParams;
                        scissorParams.enabled = true;
                        scissorParams.x = clipRect.x;
                        scissorParams.y = fbHeight - clipRect.w;
                        scissorParams.width = clipRect.z - clipRect.x;
                        scissorParams.height = clipRect.w - clipRect.y;

                        if (!mat || lastTexId != pcmd->TextureId) {
                            if (mat) {
                                // FIXME: dirty hack to make different materials render one by one.
                                // Will fix this once different render list compilers are in place.
                                depthValue += 1.0f;
                            }
                            mat = material_->clone();
                            mat->setTextureBinding(SamplerName::Main,
                                TextureBinding(imGuiManager.fromTextureId(pcmd->TextureId), SamplerParams(GL_LINEAR, GL_LINEAR)));
                        }

                        lastTexId = pcmd->TextureId;

                        rl.addGeometry(mat, vaSlice.subSlice(pcmd->IdxOffset,
                            pcmd->ElemCount, pcmd->VtxOffset),
                            GL_TRIANGLES, depthValue, scissorParams);
                    }
                }
            }
        }
    }

    void ImGuiComponent::onRegister()
    {
    }

    void ImGuiComponent::onUnregister()
    {
    }
}
