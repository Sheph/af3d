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

#ifndef _ACOMMAND_H_
#define _ACOMMAND_H_

#include "af3d/Types.h"
#include <boost/noncopyable.hpp>
#include <memory>

namespace af3d
{
    class ACommand : boost::noncopyable
    {
    public:
        ACommand() = default;
        virtual ~ACommand() = default;

        virtual bool redo() = 0;

        virtual bool undo() = 0;
    };

    using ACommandPtr = std::shared_ptr<ACommand>;

    class AInplaceCommand : public ACommand
    {
    public:
        using Fn = std::function<bool(bool isRedo)>;

        explicit AInplaceCommand(const Fn& fn) : fn_(fn) {}
        ~AInplaceCommand() = default;

        virtual bool redo() override { return fn_(true); }

        virtual bool undo() override { return fn_(false); }

    private:
        Fn fn_;
    };

    using AInplaceCommandPtr = std::shared_ptr<AInplaceCommand>;
}

#endif
