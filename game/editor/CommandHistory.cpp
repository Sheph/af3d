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

#include "editor/CommandHistory.h"
#include "Logger.h"

namespace af3d { namespace editor
{
    void CommandHistory::add(const CommandPtr& cmd)
    {
        if (!cmd->redo()) {
            return;
        }
        LOG4CPLUS_DEBUG(logger(), "CommandHistory: add (" << pos_ + 1 << ") " << cmd->description());
        list_.erase(list_.begin() + pos_, list_.end());
        list_.push_back(cmd);
        ++pos_;
    }

    void CommandHistory::undo(int n)
    {
        while ((pos_ > 0) && (n-- > 0)) {
            --pos_;
            LOG4CPLUS_DEBUG(logger(), "CommandHistory: undo (" << pos_ + 1 << ") " << list_[pos_]->description());
            if (!list_[pos_]->undo()) {
                ++pos_;
                break;
            }
        }
    }

    void CommandHistory::redo(int n)
    {
        while ((pos_ < static_cast<int>(list_.size())) && (n-- > 0)) {
            LOG4CPLUS_DEBUG(logger(), "CommandHistory: redo (" << pos_ + 1 << ") " << list_[pos_]->description());
            if (!list_[pos_]->redo()) {
                break;
            }
            ++pos_;
        }
    }
} }
