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

#include "AClassRegistry.h"
#include "Logger.h"

namespace af3d
{
    AClassRegistry& AClassRegistry::instance()
    {
        static AClassRegistry inst;
        return inst;
    }

    void AClassRegistry::dump()
    {
        LOG4CPLUS_DEBUG(logger(), "AClassRegistry::dump():");
        for (const auto& kv : map_) {
            LOG4CPLUS_DEBUG(logger(), "class " << kv.second->name() << "(" << (kv.second->super() ? kv.second->super()->name() : "")  << "):");
            const auto& props = kv.second->thisProperties();
            for (const auto& prop : props) {
                LOG4CPLUS_DEBUG(logger(), "  " << prop.name() << "(" << prop.type().name() << "): tooltip = \"" <<
                    prop.tooltip() << "\", def = " << prop.def().toString() <<
                    ", cat = " << prop.category() << ", flags = " << prop.flags());
            }
        }
    }

    void AClassRegistry::classRegister(const AClass& klass)
    {
        bool res = map_.emplace(klass.name(), &klass).second;
        runtime_assert(res);
    }

    const AClass* AClassRegistry::classFind(const std::string& name) const
    {
        auto it = map_.find(name);
        if (it == map_.end()) {
            return nullptr;
        } else {
            return it->second;
        }
    }

    AObjectPtr AClassRegistry::classCreateObj(const std::string& name, const APropertyValueMap& propVals) const
    {
        auto klass = classFind(name);
        if (!klass) {
            return AObjectPtr();
        }
        return klass->create(propVals);
    }
}
