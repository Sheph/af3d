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

#include "ImGuiFileDialog.h"
#include "Platform.h"
#include <boost/filesystem.hpp>
#include <boost/tokenizer.hpp>
#include <sstream>

namespace af3d
{
    std::unordered_map<ImGuiID, std::unique_ptr<ImGuiFileDialog>> ImGuiFileDialog::dialogs_;

    ImGuiFileDialog::ImGuiFileDialog(ImGuiID id)
    : id_(id)
    {
    }

    ImGuiFileDialog* ImGuiFileDialog::begin(const char* name,
        const std::string& root,
        const std::string& path, const char* filters)
    {
        ImGuiIO& io = ImGui::GetIO();

        bool isOpen = true;

        ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
            ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(550, 350), ImGuiCond_FirstUseEver);
        ImGui::Begin(name, &isOpen);

        // Window case, this might be top-level window, so call GetID from within
        // the window.
        auto id = ImGui::GetID("");
        auto res = beginImpl(id, root, path, filters, isOpen);
        if (!res) {
            ImGui::End();
        }
        return res;
    }

    ImGuiFileDialog* ImGuiFileDialog::beginImpl(ImGuiID id, const std::string& root, const std::string& path, const char* filters, bool isOpen)
    {
        auto it = dialogs_.find(id);
        if (it == dialogs_.end()) {
            it = dialogs_.emplace(id,
                std::unique_ptr<ImGuiFileDialog>(new ImGuiFileDialog(id))).first;
        }

        if (!isOpen) {
            it->second->ok_ = false;
            return it->second.get();
        }

        if (it->second->beginImpl(root, path, filters)) {
            return it->second.get();
        } else {
            return nullptr;
        }
    }

    ImGuiFileDialog* ImGuiFileDialog::beginAssets(const char* name,
        const std::string& path, const char* filters)
    {
        return begin(name, platform->assetsPath(), path, filters);
    }

    ImGuiFileDialog* ImGuiFileDialog::beginModal(const char* name,
        const std::string& root,
        const std::string& path, const char* filters)
    {
        bool isOpen = true;

        if (!ImGui::IsPopupOpen(name)) {
            // Handle not opened case separately.
            runtime_assert(!ImGui::BeginPopupModal(name, &isOpen));
            return nullptr;
        }

        // Modal popup case, call GetID before, imgui may close it later...
        auto id = ImGui::GetID(name);

        ImGui::SetNextWindowSize(ImVec2(550, 350), ImGuiCond_FirstUseEver);
        bool isOpen2 = ImGui::BeginPopupModal(name, &isOpen);

        auto res = beginImpl(id, root, path, filters, isOpen2);
        if (!res) {
            runtime_assert(isOpen2);
            ImGui::EndPopup();
        } else if (!isOpen2) {
            // Popup already closed by imgui.
            res->skipEnd_ = true;
        }
        return res;
    }

    ImGuiFileDialog* ImGuiFileDialog::beginAssetsModal(const char* name,
        const std::string& path, const char* filters)
    {
        return beginModal(name, platform->assetsPath(), path, filters);
    }

    void ImGuiFileDialog::setup(const std::string& root, const std::string& path, const char* filters)
    {
        if (!init_ && (root_ == root) && (initialPath_ == path) && (initialFilters_ == filters)) {
            return;
        }

        init_ = false;

        root_ = root;
        initialPath_ = path;
        initialFilters_ = filters;

        path_.clear();
        fileName_.clear();
        pathParts_.clear();
        fileList_.clear();

        filters_.clear();
        filterPtrs_.clear();
        filterIdx_ = 0;
        ok_ = false;

        if (filters) {
            std::string filtersStr(filters);

            boost::tokenizer<boost::char_separator<char>> tokens(filtersStr,
                boost::char_separator<char>(";"));

            for (auto it = tokens.begin(); it != tokens.end(); ++it) {
                FilterInfo fi;

                boost::tokenizer<boost::char_separator<char> > entry(*it,
                    boost::char_separator<char>(","));
                auto jt = entry.begin();

                std::ostringstream os;

                os << *jt << " (";
                bool first = true;
                for (++jt; jt != entry.end(); ++jt) {
                    if (!first) {
                        os << ", ";
                    }
                    os << *jt;
                    fi.exts.insert(*jt);
                    first = false;
                }
                if (first) {
                    os << ".*";
                }
                os << ")";

                fi.text = os.str();

                filters_.push_back(fi);
            }

            for (const auto& fi : filters_) {
                filterPtrs_.push_back(fi.text.c_str());
            }
        }

        changeDir(path, true);
    }

    bool ImGuiFileDialog::beginImpl(const std::string& root, const std::string& path, const char* filters)
    {
        setup(root, path, filters);

        std::string newDir;
        bool keepFileName = false;

        for (auto it = pathParts_.begin(); it != pathParts_.end(); ++it) {
            if (it != pathParts_.begin()) {
                ImGui::SameLine();
            }
            if (ImGui::Button(it->c_str())) {
                auto it2 = it;
                ++it2;
                if (it2 == pathParts_.end()) {
                    keepFileName = true;
                }
                boost::filesystem::path p;
                for (auto jt = pathParts_.begin(); jt != it2; ++jt) {
                    p /= *jt;
                }
                newDir = p.generic_string();
            }
        }

        ImVec2 size = ImGui::GetContentRegionMax();
        size.y -= 100.0f;

        ImGui::BeginChild("##FileList", size);

        for (const auto& fi : fileList_) {
            bool show = true;

            std::string str;

            if (fi.isDir) {
                str = "[Dir]  " + fi.fileName;
            } else {
                str = "[File] " + fi.fileName;
                if (!filters_.empty() &&
                    !filters_[filterIdx_].exts.empty() &&
                    (filters_[filterIdx_].exts.count(fi.ext) == 0)) {
                    show = false;
                }
            }

            if (!show) {
                continue;
            }

            if (ImGui::Selectable(str.c_str(), (fi.fileName == fileName_))) {
                if (fi.isDir) {
                    boost::filesystem::path p;
                    if (fi.fileName == "..") {
                        auto jt = pathParts_.end();
                        --jt;
                        for (auto it = pathParts_.begin(); it != jt; ++it) {
                            p /= *it;
                        }
                    } else {
                        for (const auto& part : pathParts_) {
                            p /= part;
                        }
                        p /= fi.fileName;
                    }
                    newDir = p.generic_string();
                } else {
                    fileName_ = fi.fileName;
                }
            }
        }

        ImGui::EndChild();

        ImGui::AlignTextToFramePadding();
        ImGui::Text("File Name :");

        ImGui::SameLine();

        float width = ImGui::GetContentRegionAvailWidth();
        if (!filters_.empty()) {
            width -= 250.0f;
        }

        ImGui::PushItemWidth(width);
        ImGuiUtils::inputText("##FileName", fileName_);
        ImGui::PopItemWidth();

        if (!filters_.empty()) {
            ImGui::SameLine();
            ImGui::PushItemWidth(230.0f);
            ImGui::Combo("##Filters", &filterIdx_, filterPtrs_.data(), filterPtrs_.size());
            ImGui::PopItemWidth();
        }

        if (!newDir.empty()) {
            std::string fn = fileName_;
            changeDir(newDir, false);
            if (keepFileName) {
                fileName_ = fn;
            }
        }

        bool res = false;

        if (ImGui::Button("Cancel")) {
            res = true;
        }

        ImGui::SameLine();

        if (ImGui::Button("Ok")) {
            ok_ = true;
            res = true;
        }

        return res;
    }

    void ImGuiFileDialog::changeDir(const std::string& path, bool force)
    {
        boost::filesystem::path fsPath(path);

        boost::filesystem::path fsRoot;
        if (!root_.empty()) {
            fsRoot = boost::filesystem::absolute(root_);
            fsRoot = fsRoot.lexically_normal();
            if (fsRoot.filename_is_dot()) {
                fsRoot.remove_filename();
            }
            if (fsPath.is_relative()) {
                // If path is relative, assume it's relative to root, if any.
                fsPath = fsRoot / fsPath;
            }
        }

        fsPath = boost::filesystem::absolute(fsPath);
        fsPath = fsPath.lexically_normal();
        if (fsPath.filename_is_dot()) {
            fsPath.remove_filename();
        }

        boost::system::error_code ec;

        if (!boost::filesystem::is_directory(fsPath, ec)) {
            fileName_ = fsPath.filename().generic_string();
            fsPath = fsPath.parent_path();
        } else {
            fileName_.clear();
        }

        boost::filesystem::directory_iterator it(fsPath, ec);
        if (ec) {
            if (!force) {
                return;
            }
            // No such path, reset to root, if any, or current path.
            if (root_.empty()) {
                fsPath = boost::filesystem::current_path();
            } else {
                fsPath = fsRoot;
            }
            it = boost::filesystem::directory_iterator(fsPath, ec);
        }

        pathParts_.clear();
        while (true)  {
            if (fsPath == fsRoot) {
                break;
            }
            pathParts_.push_front(fsPath.filename().generic_string());
            if (!fsPath.has_parent_path()) {
                if (!root_.empty()) {
                    // Reached the end, but no root path met, reset.
                    pathParts_.clear();
                    it = boost::filesystem::directory_iterator(fsRoot, ec);
                }
                break;
            }
            fsPath = fsPath.parent_path();
        }

        fsPath.clear();
        for (const auto& f : pathParts_) {
            fsPath /= f;
        }
        path_ = fsPath.generic_string();

        bool addDotDot = false;

        fileList_.clear();
        if (!pathParts_.empty()) {
            if (!root_.empty() || (pathParts_.size() > 1)) {
                addDotDot = true;
            }
        }

        if (!root_.empty()) {
            pathParts_.push_front(".");
        }

        for (; !ec && (it != boost::filesystem::directory_iterator()); ++it) {
            auto status = it->status(ec);
            if (ec) {
                ec.clear();
                continue;
            }

            if (boost::filesystem::is_regular_file(status)) {
                fileList_.emplace_back(it->path().filename().generic_string(),
                    it->path().extension().generic_string(), false);
            } else if (boost::filesystem::is_directory(status)) {
                fileList_.emplace_back(it->path().filename().generic_string(),
                    "", true);
            }
        }

        std::sort(fileList_.begin(), fileList_.end(), [](const FileInfo& a, const FileInfo& b) {
            if (a.isDir != b.isDir) {
                return a.isDir > b.isDir;
            }
            return std::strcoll(a.fileName.c_str(), b.fileName.c_str()) < 0;
        });

        if (addDotDot) {
            fileList_.insert(fileList_.begin(), FileInfo("..", "", true));
        }
    }

    std::string ImGuiFileDialog::filePath() const
    {
        if (path_.empty()) {
            return fileName_;
        } else {
            return fileName_.empty() ? path_ : (path_ + "/" + fileName_);
        }
    }

    void ImGuiFileDialog::end()
    {
        if (!skipEnd_) {
            ImGui::End();
        }
        dialogs_.erase(id_); // Basically "delete this"
    }

    void ImGuiFileDialog::endModal()
    {
        if (!skipEnd_) {
            ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }
        dialogs_.erase(id_); // Basically "delete this"
    }
}
