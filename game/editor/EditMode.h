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

#ifndef _EDITOR_EDITMODE_H_
#define _EDITOR_EDITMODE_H_

#include "AObject.h"
#include "af3d/Types.h"
#include "af3d/Ray.h"
#include "af3d/Frustum.h"
#include <boost/noncopyable.hpp>
#include <list>

namespace af3d { namespace editor
{
    class EditMode : boost::noncopyable
    {
    public:
        template <class ObjT>
        struct ItemBase;

        using Item = ItemBase<AObject>;

        class WItem
        {
        public:
            WItem() = default;
            explicit WItem(const AWeakObject& obj,
                const AWeakObject& part = AWeakObject())
            : obj_(obj),
              part_((part.empty() || obj.empty()) ? obj : part) {}
            explicit WItem(const AObjectPtr& obj,
                const AObjectPtr& part = AObjectPtr())
            : WItem(AWeakObject(obj), AWeakObject(part)) {}

            inline bool empty() const { return obj_.empty(); }

            inline Item lock() const;

            template <class ObjT>
            inline ItemBase<ObjT> lock() const;

            inline const AWeakObject& obj() const { return obj_; }
            inline const AWeakObject& part() const { return part_; }

            inline bool operator==(const WItem& other) const
            {
                return (obj_ == other.obj_) && (part_ == other.part_);
            }

            inline bool operator!=(const WItem& other) const
            {
                return !(*this == other);
            }

        private:
            AWeakObject obj_;
            AWeakObject part_;
        };

        template <class ObjT>
        class ItemBase
        {
        public:
            ItemBase() = default;
            explicit ItemBase(const std::shared_ptr<ObjT>& obj,
                const AObjectPtr& part = AObjectPtr())
            : obj_(obj),
              part_((part && obj) ? part : obj) {}

            inline bool empty() const { return !obj_; }

            inline WItem toWItem() const { return WItem(obj_, part_); }

            inline const std::shared_ptr<ObjT>& obj() const { return obj_; }
            inline const AObjectPtr& part() const { return part_; }

            inline bool operator==(const ItemBase<ObjT>& other) const
            {
                return (obj_ == other.obj_) && (part_ == other.part_);
            }

            inline bool operator!=(const ItemBase<ObjT>& other) const
            {
                return !(*this == other);
            }

        private:
            std::shared_ptr<ObjT> obj_;
            AObjectPtr part_;
        };

        using AWeakList = std::list<WItem>;
        using AList = std::list<Item>;

        EditMode() = default;
        virtual ~EditMode() = default;

        virtual const std::string& name() const = 0;

        virtual bool active() const = 0;

        virtual void activate() = 0;

        virtual const AWeakList& hovered() const = 0;

        virtual AList hoveredStrong() const = 0;

        virtual const AWeakList& selected() const = 0;

        virtual AList selectedStrong() const = 0;

        virtual bool isHovered(const AObjectPtr& obj) const = 0;

        virtual bool isSelected(const AObjectPtr& obj) const = 0;

        virtual void select(AList&& items) = 0;

        virtual void setHovered(const AWeakList& witems) = 0;

        virtual Item rayCast(const Frustum& frustum, const Ray& ray) const = 0;

        virtual bool isValid(const Item& item) const = 0;

        virtual bool isAlive(const Item& item) const = 0;
    };

    inline EditMode::Item EditMode::WItem::lock() const
    {
        return EditMode::Item(obj_.lock(), part_.lock());
    }

    template <class ObjT>
    inline EditMode::ItemBase<ObjT> EditMode::WItem::lock() const
    {
        return EditMode::ItemBase<ObjT>(aweakObjectCast<ObjT>(obj_), part_.lock());
    }
} }

#endif
