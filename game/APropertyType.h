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

#ifndef _APROPERTY_TYPE_H_
#define _APROPERTY_TYPE_H_

#include "APropertyTypeVisitor.h"
#include "af3d/Assert.h"
#include <boost/noncopyable.hpp>
#include <limits>

namespace af3d
{
    class AClass;

    enum class APropertyUnit
    {
        Other = 0,
        Radian = 1,
        Mesh = 2
    };

    class APropertyType : boost::noncopyable
    {
    public:
        virtual ~APropertyType() = default;

        virtual void accept(APropertyTypeVisitor& visitor) const = 0;

        inline const char* name() const { return name_; }
        inline APropertyUnit unit() const { return unit_; }

    protected:
        explicit APropertyType(const char* name, APropertyUnit unit = APropertyUnit::Other)
        {
            name_ = name;
            unit_ = unit;
        }

    private:
        const char* name_;
        APropertyUnit unit_;
    };

    class APropertyTypeBool : public APropertyType
    {
    public:
        APropertyTypeBool();

        void accept(APropertyTypeVisitor& visitor) const override;
    };

    template <class T>
    class APropertyTypeNumeric : public APropertyType
    {
    public:
        inline T vMin() const { return vMin_; }
        inline T vMax() const { return vMax_; }

    protected:
        APropertyTypeNumeric(const char* name, T vMin, T vMax, APropertyUnit unit)
        : APropertyType(name, unit),
          vMin_(vMin),
          vMax_(vMax) {}

    private:
        T vMin_;
        T vMax_;
    };

    class APropertyTypeInt : public APropertyTypeNumeric<int>
    {
    public:
        explicit APropertyTypeInt(int vMin = (std::numeric_limits<int>::min)(),
            int vMax = (std::numeric_limits<int>::max)(), APropertyUnit unit = APropertyUnit::Other);
        explicit APropertyTypeInt(APropertyUnit unit);

        void accept(APropertyTypeVisitor& visitor) const override;
    };

    class APropertyTypeFloat : public APropertyTypeNumeric<float>
    {
    public:
        explicit APropertyTypeFloat(float vMin = -(std::numeric_limits<float>::max)(),
            float vMax = (std::numeric_limits<float>::max)(), APropertyUnit unit = APropertyUnit::Other);
        explicit APropertyTypeFloat(APropertyUnit unit);

        void accept(APropertyTypeVisitor& visitor) const override;
    };

    class APropertyTypeString : public APropertyType
    {
    public:
        explicit APropertyTypeString(APropertyUnit unit = APropertyUnit::Other);

        void accept(APropertyTypeVisitor& visitor) const override;
    };

    class APropertyTypeVec2f : public APropertyType
    {
    public:
        APropertyTypeVec2f();

        void accept(APropertyTypeVisitor& visitor) const override;
    };

    class APropertyTypeVec3f : public APropertyType
    {
    public:
        APropertyTypeVec3f();

        void accept(APropertyTypeVisitor& visitor) const override;
    };

    class APropertyTypeVec3i : public APropertyType
    {
    public:
        APropertyTypeVec3i();

        void accept(APropertyTypeVisitor& visitor) const override;
    };

    class APropertyTypeVec4f : public APropertyType
    {
    public:
        APropertyTypeVec4f();

        void accept(APropertyTypeVisitor& visitor) const override;
    };

    class APropertyTypeColor : public APropertyType
    {
    public:
        explicit APropertyTypeColor(bool hasAlpha);

        inline bool hasAlpha() const { return hasAlpha_; }

        void accept(APropertyTypeVisitor& visitor) const override;

    private:
        bool hasAlpha_;
    };

    class APropertyTypeEnum : public APropertyType
    {
    public:
        using Enumerators = std::vector<std::string>;

        inline const Enumerators& enumerators() const { return enumerators_; }

        void accept(APropertyTypeVisitor& visitor) const override;

    protected:
        APropertyTypeEnum(const char* name, const Enumerators& enumerators);

    private:
        Enumerators enumerators_;
    };

    template <class T, int N = static_cast<int>(T::Max) + 1>
    class APropertyTypeEnumImpl : public APropertyTypeEnum
    {
    public:
        APropertyTypeEnumImpl(const char* name,
            const Enumerators& enumerators)
        : APropertyTypeEnum(name, enumerators)
        {
            runtime_assert(enumerators.size() == N);
        }
    };

    class APropertyTypeObject : public APropertyType
    {
    public:
        APropertyTypeObject(const char* name, const AClass& klass, bool isWeak = false);

        inline const AClass& klass() const { return klass_; }
        inline bool isWeak() const { return isWeak_; }

        void accept(APropertyTypeVisitor& visitor) const override;

    private:
        const AClass& klass_;
        bool isWeak_ = false;
    };

    class APropertyTypeTransform : public APropertyType
    {
    public:
        APropertyTypeTransform();

        void accept(APropertyTypeVisitor& visitor) const override;
    };

    class APropertyTypeQuaternion : public APropertyType
    {
    public:
        APropertyTypeQuaternion();

        void accept(APropertyTypeVisitor& visitor) const override;
    };

    class APropertyTypeArray : public APropertyType
    {
    public:
        APropertyTypeArray(const char* name, const APropertyType& type);

        inline const APropertyType& type() const { return type_; }

        void accept(APropertyTypeVisitor& visitor) const override;

    private:
        const APropertyType& type_;
    };
}

#endif
