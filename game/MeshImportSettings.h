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

#ifndef _MESHIMPORTSETTINGS_H_
#define _MESHIMPORTSETTINGS_H_

#include "AObject.h"

namespace af3d
{
    class MeshImportSettings : public std::enable_shared_from_this<MeshImportSettings>,
        public AObject
    {
    public:
        struct Entry
        {
            Entry() = default;
            virtual ~Entry() = default;

            std::string name;
            std::string path;
        };

        struct MeshEntry : public Entry
        {
        };

        struct ObjectEntry : public Entry
        {
            std::vector<ObjectEntry> subObjs;
            std::vector<MeshEntry> meshes;
        };

        MeshImportSettings();
        ~MeshImportSettings() = default;

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        inline float scale() const { return scale_; }
        inline void setScale(float value) { scale_ = value; }

        inline const ObjectEntry& root() const { return root_; }
        inline void setRoot(const ObjectEntry& value) { root_ = value; }

        APropertyValue propertyScaleGet(const std::string&) const { return scale(); }
        void propertyScaleSet(const std::string&, const APropertyValue& value) { setScale(value.toFloat()); }

        APropertyValue propertyRootGet(const std::string&) const;
        void propertyRootSet(const std::string&, const APropertyValue& value);

    private:
        float scale_ = 1.0f;
        ObjectEntry root_;
    };

    extern const APropertyTypeObject APropertyType_MeshImportSettings;

    using MeshImportSettingsPtr = std::shared_ptr<MeshImportSettings>;

    ACLASS_DECLARE(MeshImportSettings)
}

#endif
