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

#ifndef _IMGUIFILEDIALOG_H_
#define _IMGUIFILEDIALOG_H_

#include "ImGuiUtils.h"
#include <list>

namespace af3d
{
    class ImGuiFileDialog
    {
    public:
        ~ImGuiFileDialog() = default;

        // filters - "Model files,.fbx,.obj;All files"
        static ImGuiFileDialog* begin(const char* name,
            const std::string& root = ".",
            const std::string& path = ".", const char* filters = nullptr);

        static ImGuiFileDialog* beginAssets(const char* name,
            const std::string& path = ".", const char* filters = nullptr);

        inline bool ok() const { return ok_; }
        inline const std::string& fileName() const { return fileName_; }
        std::string filePath() const;

        void end();

    private:
        struct FileInfo
        {
            FileInfo() = default;
            FileInfo(const std::string& fileName,
                const std::string& ext,
                bool isDir)
            : fileName(fileName),
              ext(ext),
              isDir(isDir) {}

            std::string fileName;
            std::string ext;
            bool isDir = false;
        };

        struct FilterInfo
        {
            std::string text;
            std::unordered_set<std::string> exts;
        };

        explicit ImGuiFileDialog(ImGuiID id);

        void setup(const std::string& root, const std::string& path, const char* filters);

        bool beginImpl(const std::string& root, const std::string& path, const char* filters);

        void changeDir(const std::string& path, bool force);

        static std::unordered_map<ImGuiID, std::unique_ptr<ImGuiFileDialog>> dialogs_;

        bool init_ = true;

        ImGuiID id_;

        std::string root_;
        std::string initialPath_;
        const char* initialFilters_ = nullptr;

        std::string path_;
        std::string fileName_;
        std::list<std::string> pathParts_;
        std::vector<FileInfo> fileList_;

        std::vector<FilterInfo> filters_;
        std::vector<const char*> filterPtrs_;
        int filterIdx_ = 0;
        bool ok_ = false;
    };
}

#endif
