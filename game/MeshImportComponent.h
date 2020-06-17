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

#ifndef _MESHIMPORTCOMPONENT_H_
#define _MESHIMPORTCOMPONENT_H_

#include "PhasedComponent.h"
#include "MeshImportSettings.h"

namespace af3d
{
    class MeshImportComponent : public std::enable_shared_from_this<MeshImportComponent>,
        public PhasedComponent
    {
    public:
        explicit MeshImportComponent();
        ~MeshImportComponent() = default;

        static const AClass& staticKlass();

        static AObjectPtr create(const APropertyValueMap& propVals);

        AObjectPtr sharedThis() override { return shared_from_this(); }

        inline const MeshImportSettingsPtr& importSettings() const { return importSettings_; }
        inline void setImportSettings(const MeshImportSettingsPtr& value) { importSettings_ = value; }

        APropertyValue propertyImportSettingsGet(const std::string&) const { return APropertyValue(importSettings()); }
        void propertyImportSettingsSet(const std::string&, const APropertyValue& value) { setImportSettings(value.toObject<MeshImportSettings>()); }

        APropertyValue propertyUpdateGet(const std::string&) const { return false; }
        ACommandPtr propertyUpdateSet(const std::string&, const APropertyValue& value);

    private:
        void onRegister() override;

        void onUnregister() override;

        MeshImportSettingsPtr importSettings_;
    };

    using MeshImportComponentPtr = std::shared_ptr<MeshImportComponent>;

    ACLASS_DECLARE(MeshImportComponent)
}

#endif
