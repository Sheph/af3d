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

#ifndef _AWEAKOBJECT_H_
#define _AWEAKOBJECT_H_

#include "AObject.h"

namespace af3d
{
    class AWeakObject
    {
    public:
        AWeakObject() = default;
        explicit AWeakObject(const AObjectPtr& obj);
        ~AWeakObject() = default;

        inline bool empty() const { return cookie_ == 0; }

        inline ACookie cookie() const { return cookie_; }

        inline bool operator==(const AWeakObject& other) const
        {
            return cookie_ == other.cookie_;
        }

        inline bool operator!=(const AWeakObject& other) const
        {
            return !(*this == other);
        }

        AObjectPtr lock() const;

        void reset(const AObjectPtr& obj = AObjectPtr());

    private:
        ACookie cookie_ = 0;
    };

    template <class T>
    inline std::shared_ptr<T> aweakObjectCast(const AWeakObject& weakObj)
    {
        auto obj = weakObj.lock();
        return (obj && obj->isSubClassOf(T::staticKlass())) ?
            std::static_pointer_cast<T>(obj) : std::shared_ptr<T>();
    }
}

#endif
