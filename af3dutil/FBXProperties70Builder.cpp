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

#include "af3d/FBXProperties70Builder.h"
#include "Logger.h"

namespace af3d
{
    void FBXProperties70Builder::PBuilder::reset()
    {
        i_ = 0;
        type_ = Type::Unknown;
    }

    void FBXProperties70Builder::PBuilder::addValue(std::int16_t value)
    {
        addValue((std::int32_t)value);
    }

    void FBXProperties70Builder::PBuilder::addValue(bool value)
    {
        addValue((std::int32_t)value);
    }

    void FBXProperties70Builder::PBuilder::addValue(std::int32_t value)
    {
        if (i_ == 4) {
            switch (type_) {
            case Type::Bool:
                parent_->onProperty(key_, (bool)value);
                break;
            case Type::Int:
                parent_->onProperty(key_, (std::int32_t)value);
                break;
            default:
                break;
            }
        }

        ++i_;
    }

    void FBXProperties70Builder::PBuilder::addValue(float value)
    {
        if (i_ == 4) {
            switch (type_) {
            case Type::Float:
                parent_->onProperty(key_, value);
                break;
            default:
                break;
            }
        }

        if ((i_ >= 4) && (i_ <= 6)) {
            switch (type_) {
            case Type::Vector3:
            case Type::Color:
                floats_[i_ - 4] = value;
                break;
            default:
                break;
            }
        }

        if (i_ == 6) {
            switch (type_) {
            case Type::Vector3:
                parent_->onProperty(key_, btVector3(floats_[0], floats_[1], floats_[2]));
                break;
            case Type::Color:
                parent_->onProperty(key_, Color(floats_[0], floats_[1], floats_[2], 1.0f));
                break;
            default:
                break;
            }
        }

        ++i_;
    }

    void FBXProperties70Builder::PBuilder::addValue(double value)
    {
        addValue((float)value);
    }

    void FBXProperties70Builder::PBuilder::addValue(std::int64_t value)
    {
        addValue((std::int32_t)value);
    }

    void FBXProperties70Builder::PBuilder::addValue(const std::string& value)
    {
        if (i_ == 0) {
            key_ = value;
        } else if (i_ == 1) {
            if (value == "bool") {
                type_ = Type::Bool;
            } else if ((value == "int") || (value == "enum")) {
                type_ = Type::Int;
            } else if ((value == "double") || (value == "Number")) {
                type_ = Type::Float;
            } else if (value == "KString") {
                type_ = Type::String;
            } else if ((value == "Vector") || (value == "Vector3D")) {
                type_ = Type::Vector3;
            } else if ((value == "Color") || (value == "ColorRGB")) {
                type_ = Type::Color;
            } else {
                LOG4CPLUS_WARN(af3dutil::logger(), "Unknown prop type = " << value);
            }
        } else if (i_ == 4) {
            switch (type_) {
            case Type::String:
                parent_->onProperty(key_, value);
                break;
            default:
                break;
            }
        }
        ++i_;
    }

    FBXProperties70Builder::FBXProperties70Builder()
    : pBuilder_(this)
    {
    }

    FBXDomBuilder* FBXProperties70Builder::childBegin(const std::string& name)
    {
        if (name == "P") {
            pBuilder_.reset();
            return &pBuilder_;
        }
        return nullptr;
    }
}
