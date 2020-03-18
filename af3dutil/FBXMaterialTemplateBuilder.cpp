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

#include "af3d/FBXMaterialTemplateBuilder.h"
#include "Logger.h"

namespace af3d
{
    void FBXMaterialTemplateBuilder::onProperty(const std::string& key, bool value)
    {
        //LOG4CPLUS_WARN(af3dutil::logger(), "MtlTemplate: unknown key " << key);
    }

    void FBXMaterialTemplateBuilder::onProperty(const std::string& key, std::int32_t value)
    {
        //LOG4CPLUS_WARN(af3dutil::logger(), "MtlTemplate: unknown key " << key);
    }

    void FBXMaterialTemplateBuilder::onProperty(const std::string& key, float value)
    {
        if (key == "ShininessExponent") {
            target_->setShininess(value);
        } else if (key == "AmbientFactor") {
            auto c = target_->ambientColor();
            c.setW(value);
            target_->setAmbientColor(c);
        } else if (key == "DiffuseFactor") {
            auto c = target_->diffuseColor();
            c.setW(value);
            target_->setDiffuseColor(c);
        } else if (key == "EmissiveFactor") {
            auto c = target_->emissiveColor();
            c.setW(value);
            target_->setEmissiveColor(c);
        } else if (key == "SpecularFactor") {
            auto c = target_->specularColor();
            c.setW(value);
            target_->setSpecularColor(c);
        } else {
            //LOG4CPLUS_WARN(af3dutil::logger(), "MtlTemplate: unknown key " << key);
        }
    }

    void FBXMaterialTemplateBuilder::onProperty(const std::string& key, const std::string& value)
    {
        //LOG4CPLUS_WARN(af3dutil::logger(), "MtlTemplate: unknown key " << key);
    }

    void FBXMaterialTemplateBuilder::onProperty(const std::string& key, const btVector3& value)
    {
        //LOG4CPLUS_WARN(af3dutil::logger(), "MtlTemplate: unknown key " << key);
    }

    void FBXMaterialTemplateBuilder::onProperty(const std::string& key, const Color& value)
    {
        if (key == "AmbientColor") {
            auto w = target_->ambientColor().w();
            Color c = value;
            c.setW(w);
            target_->setAmbientColor(c);
        } else if (key == "DiffuseColor") {
            auto w = target_->diffuseColor().w();
            Color c = value;
            c.setW(w);
            target_->setDiffuseColor(c);
        } else if (key == "EmissiveColor") {
            auto w = target_->emissiveColor().w();
            Color c = value;
            c.setW(w);
            target_->setEmissiveColor(c);
        } else if (key == "SpecularColor") {
            auto w = target_->specularColor().w();
            Color c = value;
            c.setW(w);
            target_->setSpecularColor(c);
        } else {
            //LOG4CPLUS_WARN(af3dutil::logger(), "MtlTemplate: unknown key " << key);
        }
    }
}
