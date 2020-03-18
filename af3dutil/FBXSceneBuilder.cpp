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

#include "af3d/FBXSceneBuilder.h"
#include "Logger.h"

namespace af3d
{
    void FBXSceneBuilder::PropertyTemplateBuilder::addValue(const std::string& value)
    {
        if (parent_->objType == "Material" && value == "FbxSurfacePhong") {
            childBuilder_ = &mtlTemplateBuilder;
        } else {
            //LOG4CPLUS_WARN(af3dutil::logger(), "PropertyTemplateBuilder: unknown template " << parent_->objType << ":" << value);
        }
    }

    FBXDomBuilder* FBXSceneBuilder::PropertyTemplateBuilder::childBegin(const std::string& name)
    {
        if (name == "Properties70") {
            return childBuilder_;
        }
        return nullptr;
    }

    void FBXSceneBuilder::PropertyTemplateBuilder::childEnd(const std::string& name, FBXDomBuilder* builder)
    {
        childBuilder_ = nullptr;
    }

    void FBXSceneBuilder::ObjectTypeBuilder::addValue(const std::string& value)
    {
        objType = value;
    }

    FBXDomBuilder* FBXSceneBuilder::ObjectTypeBuilder::childBegin(const std::string& name)
    {
        if (name == "PropertyTemplate") {
            ptBuilder.reset();
            return &ptBuilder;
        }
        return nullptr;
    }

    FBXDomBuilder* FBXSceneBuilder::DefinitionsBuilder::childBegin(const std::string& name)
    {
        if (name == "ObjectType") {
            return &otBuilder;
        }
        return nullptr;
    }

    FBXSceneBuilder::FBXSceneBuilder()
    {
        defBuilder_.otBuilder.ptBuilder.mtlTemplateBuilder.setTarget(&mtlTemplate_);
    }

    FBXDomBuilder* FBXSceneBuilder::childBegin(const std::string& name)
    {
        if (name == "Definitions") {
            return &defBuilder_;
        }
        return nullptr;
    }
}
