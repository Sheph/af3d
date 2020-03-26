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

#include "af3d/TPS.h"
#include "af3d/Utils.h"
#include "Logger.h"
#include "json/json.h"
#include "log4cplus/ndc.h"

namespace af3d
{
    static const TPSEntry dummyEntry;

    TPS::TPS(const std::string& imageFileName,
        std::uint32_t width,
        std::uint32_t height)
    : imageFileName_(imageFileName),
      width_(width),
      height_(height)
    {
    }

    TPSPtr TPS::fromStream(const std::string& fileName, std::istream& is)
    {
        std::string json;

        {
            log4cplus::NDCContextCreator ndc(fileName);

            if (!is) {
                LOG4CPLUS_ERROR(af3dutil::logger(), "Cannot open file");

                return TPSPtr();
            }

            if (!readStream(is, json)) {
                LOG4CPLUS_ERROR(af3dutil::logger(), "Error reading file");

                return TPSPtr();
            }
        }

        return fromString(fileName, json);
    }

    TPSPtr TPS::fromString(const std::string& fileName, const std::string& json)
    {
        Json::Value jsonValue;
        Json::Reader reader;

        log4cplus::NDCContextCreator ndc(fileName);

        if (!reader.parse(json, jsonValue)) {
            LOG4CPLUS_ERROR(af3dutil::logger(),
                "Failed to parse JSON: " << reader.getFormattedErrorMessages());
            return TPSPtr();
        }

        LOG4CPLUS_DEBUG(af3dutil::logger(), "Processing...");

        return fromJsonValue(jsonValue);
    }

    TPSPtr TPS::fromJsonValue(const Json::Value& jsonValue)
    {
        std::string imageFileName = jsonValue["meta"]["image"].asString();
        std::uint32_t width = jsonValue["meta"]["size"]["w"].asUInt();
        std::uint32_t height = jsonValue["meta"]["size"]["h"].asUInt();

        TPSPtr tps = std::make_shared<TPS>(imageFileName, width, height);

        int i = 0;
        Json::Value tmp = jsonValue["frames"][i];
        while (!tmp.isNull()) {
            std::ostringstream os;
            os << "frame " << i++;
            log4cplus::NDCContextCreator ndc(os.str());

            std::string fileName = tmp["filename"].asString();
            std::uint32_t x = tmp["frame"]["x"].asUInt();
            std::uint32_t y = tmp["frame"]["y"].asUInt();
            std::uint32_t width = tmp["frame"]["w"].asUInt();
            std::uint32_t height = tmp["frame"]["h"].asUInt();

            TPSEntry entry(x, y, width, height);

            tps->addEntry(fileName, entry);

            tmp = jsonValue["frames"][i];
        }

        return tps;
    }

    void TPS::addEntry(const std::string& fileName, const TPSEntry& entry)
    {
        if (!entries_.insert(std::make_pair(fileName, entry)).second) {
            LOG4CPLUS_WARN(af3dutil::logger(),
                "File \"" << fileName << "\" already in TPS \"" << imageFileName_ << "\"");
        }
    }

    const TPSEntry& TPS::entry(const std::string& fileName, bool quiet) const
    {
        Entries::const_iterator it = entries_.find(fileName);

        if (it == entries_.end()) {
            if (!quiet) {
                LOG4CPLUS_WARN(af3dutil::logger(),
                   "File \"" << fileName << "\" not found in TPS \"" << imageFileName_ << "\"");
            }
            return dummyEntry;
        }

        return it->second;
    }
}
