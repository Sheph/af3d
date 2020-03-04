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

#ifndef _AF3D_SEQUENTIALAPPCONFIG_H_
#define _AF3D_SEQUANTIALAPPCONFIG_H_

#include "af3d/AppConfig.h"

namespace af3d
{
    class SequentialAppConfig : public AppConfig
    {
    public:
        SequentialAppConfig();
        ~SequentialAppConfig();

        void add(const AppConfigPtr& config);

        virtual std::string getLoggerConfig() const override;

        virtual bool haveKey(const std::string& key) const override;

        virtual std::vector<std::string> getSubKeys(const std::string& key = "") const override;

        virtual std::string getString(const std::string& key) const override;

        virtual int getStringIndex(const std::string& key,
            const std::vector<std::string>& allowed) const override;

        virtual int getInt(const std::string& key) const override;

        virtual float getFloat(const std::string& key) const override;

        virtual bool getBool(const std::string& key) const override;

        virtual Vector2f getVector2f(const std::string& key) const override;

        virtual Vector3f getVector3f(const std::string& key) const override;

        virtual Vector4f getVector4f(const std::string& key) const override;

    private:
        using Configs = std::vector<AppConfigPtr>;

        Configs configs_;
    };
}

#endif
