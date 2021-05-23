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

#ifndef _SCRIPTCONVERTERS_H_
#define _SCRIPTCONVERTERS_H_

#include "af3d/Types.h"
#include "af3d/EnumSet.h"
#include "AssetManager.h"
#include "Platform.h"
#include "luainc.h"
#include <luabind/luabind.hpp>

namespace
{
    static inline void call0Wrapper(const luabind::object& obj)
    {
        try {
            luabind::call_function<void>(obj);
        } catch (const luabind::error& e) {
            ::lua_pop(e.state(), 1);
        } catch (const std::exception& e) {
            LOG4CPLUS_ERROR(af3d::logger(), e.what());
        }
    }

    template <class T>
    struct basic_converter
    {
        static luabind::object to_object(lua_State* L, const T& value)
        {
            return luabind::object(L, value);
        }
    };

    template <>
    struct basic_converter<af3d::VideoMode>
    {
        static luabind::object to_object(lua_State* L, const af3d::VideoMode& value)
        {
            luabind::object list = luabind::newtable(L);

            list[1] = value.width;
            list[2] = value.height;

            return list;
        }
    };

    template <class T>
    struct basic_converter< std::vector<T> >
    {
        static luabind::object to_object(lua_State* L, const std::vector<T>& value)
        {
            luabind::object list = luabind::newtable(L);

            for (int i = 0; i < value.size(); ++i) {
                list[i + 1] = basic_converter<T>::to_object(L, value[i]);
            }

            return list;
        }
    };
}

namespace luabind
{
    #define AF3D_ENUM_CONVERTER(T) \
        template <> \
        struct default_converter<T> : native_converter_base<T> \
        { \
            static int compute_score(lua_State* L, int index) \
            { \
                return ::lua_type(L, index) == LUA_TNUMBER ? 0 : -1; \
            } \
            T from(lua_State* L, int index) \
            { \
                luabind::object tmp(luabind::from_stack(L, index)); \
                return static_cast<T>(luabind::object_cast<int>(tmp)); \
            } \
            void to(lua_State* L, T value) \
            { \
                basic_converter<int>::to_object(L, static_cast<int>(value)).push(L); \
            } \
        }

    AF3D_ENUM_CONVERTER(af3d::BodyType);
    AF3D_ENUM_CONVERTER(af3d::SceneObjectType);

    template <class T>
    struct default_converter< std::vector<T> > : native_converter_base< std::vector<T> >
    {
        static int compute_score(lua_State* L, int index)
        {
            return ::lua_type(L, index) == LUA_TTABLE ? 0 : -1;
        }

        std::vector<T> from(lua_State* L, int index)
        {
            std::vector<T> list;

            for (luabind::iterator it(luabind::object(luabind::from_stack(L, index))), end;
                 it != end; ++it) {
                list.push_back(luabind::object_cast<T>(*it));
            }

            return list;
        }

        void to(lua_State* L, const std::vector<T>& value)
        {
            basic_converter< std::vector<T> >::to_object(L, value).push(L);
        }
    };

    template <class T>
    struct default_converter< const std::vector<T>& > : default_converter< std::vector<T> >
    {};

    template <>
    struct default_converter<af3d::VideoMode> : native_converter_base<af3d::VideoMode>
    {
        static int compute_score(lua_State* L, int index)
        {
            return ::lua_type(L, index) == LUA_TTABLE ? 0 : -1;
        }

        af3d::VideoMode from(lua_State* L, int index)
        {
            luabind::object tmp(luabind::from_stack(L, index));
            return af3d::VideoMode(luabind::object_cast<std::uint32_t>(tmp[1]),
                luabind::object_cast<std::uint32_t>(tmp[2]));
        }

        void to(lua_State* L, const af3d::VideoMode& value)
        {
            basic_converter<af3d::VideoMode>::to_object(L, value).push(L);
        }
    };

    template <>
    struct default_converter<const af3d::VideoMode&> : default_converter<af3d::VideoMode>
    {};

    template <class T>
    struct default_converter<std::shared_ptr<T>>
        : detail::default_converter_generator<std::shared_ptr<T>>::type
    {
        template <class U>
        typename std::enable_if<std::is_base_of<af3d::AObject, U>::value, int>::type match(lua_State* L, detail::by_reference<std::shared_ptr<U>> tmp, int index)
        {
            if (lua_isnil(L, index)) {
                return 0;
            }

            detail::object_rep* obj = detail::get_instance(L, index);
            if (obj == 0) {
                return -1;
            }

            std::pair<void*, int> s = obj->get_instance(detail::registered_class<af3d::AObjectPtr>::id);
            this->result = s.first;
            if (this->result && !(*static_cast<af3d::AObjectPtr*>(this->result))->isSubClassOf(T::staticKlass())) {
                return -1;
            }
            return s.second;
        }

        template <class U>
        typename std::enable_if<!std::is_base_of<af3d::AObject, U>::value, int>::type match(lua_State* L, detail::by_reference<std::shared_ptr<U>> tmp, int index)
        {
            if (lua_isnil(L, index)) {
                return 0;
            }
            return detail::default_converter_generator<std::shared_ptr<T> >::type::match(L, tmp, index);
        }

        template <class U>
        typename std::enable_if<std::is_base_of<af3d::AObject, U>::value, std::shared_ptr<T>>::type apply(lua_State* L, detail::by_reference<std::shared_ptr<U>> tmp, int index)
        {
            if (lua_isnil(L, index)) {
                return std::shared_ptr<T>();
            }
            return af3d::aobjectCast<T>(*static_cast<af3d::AObjectPtr*>(this->result));
        }

        template <class U>
        typename std::enable_if<!std::is_base_of<af3d::AObject, U>::value, std::shared_ptr<T>>::type apply(lua_State* L, detail::by_reference<std::shared_ptr<U>> tmp, int index)
        {
            if (lua_isnil(L, index)) {
                return std::shared_ptr<T>();
            }
            return detail::default_converter_generator<std::shared_ptr<T> >::type::apply(L, tmp, index);
        }

        void apply(lua_State* L, const std::shared_ptr<T>& p)
        {
            detail::default_converter_generator<std::shared_ptr<T> >::type::apply(L, p);
        }
    };

    template <class T>
    struct default_converter<const std::shared_ptr<T>&>
        : detail::default_converter_generator<const std::shared_ptr<T>&>::type
    {
        template <class U>
        typename std::enable_if<std::is_base_of<af3d::AObject, U>::value, int>::type match(lua_State* L, detail::by_const_reference<std::shared_ptr<U>> tmp, int index)
        {
            if (lua_isnil(L, index)) {
                return 0;
            }

            detail::object_rep* obj = detail::get_instance(L, index);
            if (obj == 0) {
                return -1;
            }

            std::pair<void*, int> s = obj->get_instance(detail::registered_class<af3d::AObjectPtr>::id);
            if (s.second >= 0 && !obj->is_const()) {
                s.second += 10;
            }
            this->result = s.first;
            if (this->result && !(*static_cast<af3d::AObjectPtr*>(this->result))->isSubClassOf(T::staticKlass())) {
                return -1;
            }
            return s.second;
        }

        template <class U>
        typename std::enable_if<!std::is_base_of<af3d::AObject, U>::value, int>::type match(lua_State* L, detail::by_const_reference<std::shared_ptr<U>> tmp, int index)
        {
            if (lua_isnil(L, index)) {
                return 0;
            }
            return detail::default_converter_generator<const std::shared_ptr<T>&>::type::match(L, tmp, index);
        }

        template <class U>
        typename std::enable_if<std::is_base_of<af3d::AObject, U>::value, std::shared_ptr<T>>::type apply(lua_State* L, detail::by_const_reference<std::shared_ptr<U>> tmp, int index)
        {
            if (lua_isnil(L, index)) {
               return std::shared_ptr<T>();
            }
            return af3d::aobjectCast<T>(*static_cast<af3d::AObjectPtr*>(this->result));
        }

        template <class U>
        typename std::enable_if<!std::is_base_of<af3d::AObject, U>::value, std::shared_ptr<T>>::type apply(lua_State* L, detail::by_const_reference<std::shared_ptr<U>> tmp, int index)
        {
            if (lua_isnil(L, index)) {
                return std::shared_ptr<T>();
            }
            return detail::default_converter_generator<const std::shared_ptr<T>&>::type::apply(L, tmp, index);
        }

        void apply(lua_State* L, const std::shared_ptr<T>& p)
        {
            detail::default_converter_generator<const std::shared_ptr<T>&>::type::apply(L, p);
        }
    };

    template <class T1, class T2>
    struct default_converter< std::pair<T1, T2> > : native_converter_base< std::pair<T1, T2> >
    {
        static int compute_score(lua_State* L, int index)
        {
            return -1;
        }

        std::pair<T1, T2> from(lua_State* L, int index)
        {
            throw luabind::cast_failed(L, typeid(value));
        }

        void to(lua_State* L, const std::pair<T1, T2>& value)
        {
            basic_converter<T1>::to_object(L, value.first).push(L);
            basic_converter<T2>::to_object(L, value.second).push(L);
        }
    };

    template <class T1, class T2>
    struct default_converter< const std::pair<T1, T2>& > : default_converter< std::pair<T1, T2> >
    {};

    template <class T, int N>
    struct default_converter< af3d::EnumSet<T, N> > : native_converter_base< af3d::EnumSet<T, N> >
    {
        static int compute_score(lua_State* L, int index)
        {
            return ::lua_type(L, index) == LUA_TTABLE ? 0 : -1;
        }

        af3d::EnumSet<T, N> from(lua_State* L, int index)
        {
            af3d::EnumSet<T, N> res;

            for (luabind::iterator it(luabind::object(luabind::from_stack(L, index))), end;
                 it != end; ++it) {
                res.set(luabind::object_cast<T>(*it));
            }

            return res;
        }

        void to(lua_State* L, const af3d::EnumSet<T, N>& value)
        {
            throw luabind::cast_failed(L, typeid(value));
        }
    };

    template <class T, int N>
    struct default_converter< const af3d::EnumSet<T, N>& > : default_converter< af3d::EnumSet<T, N> >
    {};

    template <>
    struct default_converter< std::function<void()> > : native_converter_base< std::function<void()> >
    {
        static int compute_score(lua_State* L, int index)
        {
            return ::lua_type(L, index) == LUA_TFUNCTION ? 0 : -1;
        }

        std::function<void()> from(lua_State* L, int index)
        {
            return std::bind(&call0Wrapper, luabind::object(luabind::from_stack(L, index)));
        }

        void to(lua_State* L, const std::function<void()>& value)
        {
            throw luabind::cast_failed(L, typeid(value));
        }
    };

    template <>
    struct default_converter<const std::function<void()>&> : default_converter< std::function<void()> >
    {};
}

#endif
