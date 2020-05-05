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

#ifndef _AOBJECT_H_
#define _AOBJECT_H_

#include "AClass.h"

namespace af3d
{
    enum AObjectFlags
    {
        AObjectEditable = 1 << 0,
        AObjectMarkerObject = 1 << 1,
        AObjectMarkerLight = 1 << 2,
        AObjectMarkerJoint = 1 << 3,
        AObjectMarkerOther = 1 << 4
    };

    class AObject : boost::noncopyable
    {
    public:
        virtual ~AObject();

        static const AClass& staticKlass();

        static AObject* getByCookie(ACookie value);

        static size_t getCount();

        virtual AObjectPtr sharedThis() = 0;

        inline const AClass& klass() const { return *klass_; }

        // WARNING! Only call this if you're 100% sure what you're doing.
        inline void setKlass(const AClass& value) { klass_ = &value; }

        bool isSubClassOf(const AClass& value) const;

        inline ACookie cookie() const { return cookie_; }
        void setCookie(ACookie value);

        inline const std::string& name() const { return name_; }
        inline void setName(const std::string& value) { name_ = value; }

        inline std::uint32_t aflags() const { return aflags_; }
        inline void aflagsSet(std::uint32_t value) { aflags_ |= value; }
        inline void aflagsClear(std::uint32_t value) { aflags_ &= ~value; }

        inline bool isMarker() const { return (aflags_ & (AObjectMarkerObject | AObjectMarkerLight | AObjectMarkerJoint | AObjectMarkerOther)) != 0; }

        bool propertyCanGet(const std::string& key) const;
        bool propertyCanSet(const std::string& key) const;
        const AProperty* propertyFind(const std::string& key) const;

        APropertyValue propertyGet(const std::string& key) const;
        void propertySet(const std::string& key, const APropertyValue& value);

        void propertiesSet(const APropertyValueMap& propVals);

        APropertyValue propertyNameGet(const std::string&) const { return name(); }
        void propertyNameSet(const std::string&, const APropertyValue& value) { setName(value.toString()); }

    protected:
        explicit AObject(const AClass& klass);

    private:
        const AClass* klass_;

        ACookie cookie_;
        std::string name_;
        std::uint32_t aflags_ = 0;
    };

    extern const APropertyTypeObject APropertyType_AObject;
    extern const APropertyTypeArray APropertyType_ArrayAObject;

    ACLASS_DECLARE(AObject)

    template <class T>
    inline T* aobjectCast(AObject* obj)
    {
        return (obj && obj->isSubClassOf(T::staticKlass())) ?
            static_cast<T*>(obj) : nullptr;
    }

    template <class T>
    inline std::shared_ptr<T> aobjectCast(const AObjectPtr& obj)
    {
        return (obj && obj->isSubClassOf(T::staticKlass())) ?
            std::static_pointer_cast<T>(obj) : std::shared_ptr<T>();
    }

    template <class T>
    inline std::shared_ptr<T> aweakObjectCast(const AWeakObject& weakObj)
    {
        return aobjectCast<T>(weakObj.lock());
    }

    template <class T>
    std::shared_ptr<T> APropertyValue::toObject() const
    {
        switch (type_) {
        case Object:
            return aobjectCast<T>(obj_);
        case WeakObject:
            return aweakObjectCast<T>(wobj_);
        default:
            return std::shared_ptr<T>();
        }
    }
}

#endif
