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

#include "ScriptImpl.h"
#include <luabind/discard_result_policy.hpp>

namespace af3d
{
    static int errorHandler(lua_State* L)
    {
        lua_Debug d;

        ::lua_getstack(L, 1, &d);
        ::lua_getinfo(L, "Sln", &d);

        std::string err = ::lua_tostring(L, -1);

        ::lua_pop(L, 1);

        std::ostringstream os;

        os << d.short_src << ": " << d.currentline;

        if (d.name != 0) {
            os << " (" << d.namewhat << " " << d.name << ")";
        }

        os << " - " << err;

        ::lua_pushstring(L, os.str().c_str());

        LOG4CPLUS_ERROR(logger(), os.str());

        return 1;
    }

    Script::Impl::Impl(const std::string& path,
         Scene* scene)
    : path_(path),
      scene_(scene),
      L_(nullptr)
    {
    }

    Script::Impl::~Impl()
    {
        if (L_) {
            ::lua_close(L_);
        }
    }

    void Script::Impl::bind()
    {
        /*
         * Eclipse takes forever parsing this file
         * because of the bindings, so bypass eclipse indexer
         * using this define hack which is visible to compiler, but
         * not to eclipse.
         */
#ifdef BYPASS_ECLIPSE_INDEXER
        luabind::module(L_)
        [
            luabind::class_<GlobalConstHolder>("const")
                .enum_("constants")
                [
                    #define enum_value(a, b) luabind::value(a, static_cast<int>(b))
                    enum_value("SceneObjectTypeOther", SceneObjectType::Other),
                    enum_value("SceneObjectTypePlayer", SceneObjectType::Player),
                    enum_value("SceneObjectTypeEnemy", SceneObjectType::Enemy),
                    enum_value("SceneObjectTypeEnemyBuilding", SceneObjectType::EnemyBuilding),
                    enum_value("SceneObjectTypePlayerMissile", SceneObjectType::PlayerMissile),
                    enum_value("SceneObjectTypeEnemyMissile", SceneObjectType::EnemyMissile),
                    enum_value("SceneObjectTypeTerrain", SceneObjectType::Terrain),
                    enum_value("SceneObjectTypeRock", SceneObjectType::Rock),
                    enum_value("SceneObjectTypeAlly", SceneObjectType::Ally),
                    enum_value("SceneObjectTypeAllyMissile", SceneObjectType::AllyMissile),
                    enum_value("SceneObjectTypeNeutralMissile", SceneObjectType::NeutralMissile),
                    enum_value("SceneObjectTypeGizmo", SceneObjectType::Gizmo),
                    enum_value("SceneObjectTypeGarbage", SceneObjectType::Garbage),
                    enum_value("SceneObjectTypeBlocker", SceneObjectType::Blocker),
                    enum_value("SceneObjectTypeVehicle", SceneObjectType::Vehicle),
                    enum_value("SceneObjectTypeDeadbody", SceneObjectType::Deadbody),

                    luabind::value("PhaseThink", phaseThink),
                    luabind::value("PhasePreRender", phasePreRender),

                    luabind::value("EaseLinear", EaseLinear),
                    luabind::value("EaseInQuad", EaseInQuad),
                    luabind::value("EaseOutQuad", EaseOutQuad),
                    luabind::value("EaseInOutQuad", EaseInOutQuad),

                    enum_value("BodyTypeStatic", BodyType::Static),
                    enum_value("BodyTypeKinematic", BodyType::Kinematic),
                    enum_value("BodyTypeDynamic", BodyType::Dynamic),

                    enum_value("GamepadButtonUnknown", GamepadButton::Unknown),
                    enum_value("GamepadButtonDPADUp", GamepadButton::DPADUp),
                    enum_value("GamepadButtonDPADDown", GamepadButton::DPADDown),
                    enum_value("GamepadButtonDPADLeft", GamepadButton::DPADLeft),
                    enum_value("GamepadButtonDPADRight", GamepadButton::DPADRight),
                    enum_value("GamepadButtonStart", GamepadButton::Start),
                    enum_value("GamepadButtonBack", GamepadButton::Back),
                    enum_value("GamepadButtonLeftStick", GamepadButton::LeftStick),
                    enum_value("GamepadButtonRightStick", GamepadButton::RightStick),
                    enum_value("GamepadButtonLeftBumper", GamepadButton::LeftBumper),
                    enum_value("GamepadButtonRightBumper", GamepadButton::RightBumper),
                    enum_value("GamepadButtonLeftTrigger", GamepadButton::LeftTrigger),
                    enum_value("GamepadButtonRightTrigger", GamepadButton::RightTrigger),
                    enum_value("GamepadButtonA", GamepadButton::A),
                    enum_value("GamepadButtonB", GamepadButton::B),
                    enum_value("GamepadButtonX", GamepadButton::X),
                    enum_value("GamepadButtonY", GamepadButton::Y),
                    enum_value("GamepadButtonMax", GamepadButton::Max)
                    #undef enum_value
                ],

            luabind::class_<Settings>("Settings")
                .def_readonly("developer", &Settings::developer)
                .def("setDeveloper", &Settings::setDeveloper)
                .def_readonly("videoMode", &Settings::videoMode)
                .def_readonly("msaaMode", &Settings::msaaMode)
                .def_readonly("vsync", &Settings::vsync)
                .def_readonly("fullscreen", &Settings::fullscreen)
                .def_readonly("trilinearFilter", &Settings::trilinearFilter),

            luabind::class_<AObject, AObjectPtr>("AObject")
                .def(luabind::const_self == luabind::const_self)
                .def(luabind::self == luabind::self)
                .property("cookie", &AObject::cookie)
                .property("name", &AObject::name, &AObject::setName),

            luabind::class_<Scene, AObject, AObjectPtr>("Scene")
                .def("addObject", &Scene::addObject)
                .def("getObjects", (std::vector<SceneObjectPtr> (Scene::*)() const)&Scene::getObjects)
                .def("getObjects", (std::vector<SceneObjectPtr> (Scene::*)(const std::string&) const)&Scene::getObjects)
                .def("reparent", &Scene::reparent)
                .def("addTimer", &Scene::addTimer)
                .def("removeTimer", &Scene::removeTimer)
                .def("setNextLevel", &Scene::setNextLevel)
                .def("restartLevel", &Scene::restartLevel)
                .property("camera", &Scene::camera)
                .property("respawnPoint", &Scene::respawnPoint, &Scene::setRespawnPoint)
                .property("checkpoint", &Scene::checkpoint, &Scene::setCheckpoint)
                .property("cutscene", &Scene::cutscene, &Scene::setCutscene)
                .property("quit", &Scene::quit, &Scene::setQuit)
                .property("paused", &Scene::paused, &Scene::setPaused)
                .property("playable", &Scene::playable)
                .property("assetPath", &Scene::assetPath)
                .property("timeScale", &Scene::timeScale, &Scene::setTimeScale),

            luabind::class_<SceneObjectFactory>("SceneObjectFactory")
                .def("createDummy", &SceneObjectFactory::createDummy),

            luabind::class_<Component, AObject, AObjectPtr>("Component")
                .def(luabind::const_self == luabind::const_self)
                .def(luabind::self == luabind::self)
                .property("parent", &Component::script_parent)
                .def("removeFromParent", &Component::removeFromParent),

            luabind::class_<PhasedComponent, Component, ScriptComponent, AObjectPtr>("PhasedComponent")
                .def(luabind::constructor<luabind::object, std::uint32_t, int>())
                .property("phases", &PhasedComponent::phases)
                .property("order", &PhasedComponent::order),

            luabind::class_<RenderComponent, Component, AObjectPtr>("RenderComponent")
                .property("visible", &RenderComponent::visible, &RenderComponent::setVisible),

            luabind::class_<PhysicsComponent, Component, AObjectPtr>("PhysicsComponent"),

            luabind::class_<UIComponent, Component, AObjectPtr>("UIComponent")
                .property("zOrder", &UIComponent::zOrder),

            luabind::class_<CameraComponent, PhasedComponent, AObjectPtr>("CameraComponent"),

            luabind::class_<UITimerComponent, UIComponent, ScriptUITimerComponent, AObjectPtr>("UITimerComponent")
                .def(luabind::constructor<luabind::object, float, int>()),

            luabind::class_<SceneObject, AObject, AObjectPtr>("SceneObject")
                .def(luabind::constructor<>())
                .def(luabind::const_self == luabind::const_self)
                .def(luabind::self == luabind::self)
                .def("scene", &SceneObject::scene)
                .property("parent", &SceneObject::script_parentObject)
                .def("getObjects", (std::vector<SceneObjectPtr> (SceneObject::*)() const)&SceneObject::getObjects)
                .def("getObjects", (std::vector<SceneObjectPtr> (SceneObject::*)(const std::string&) const)&SceneObject::getObjects)
                .def("reparent", &SceneObject::reparent)
                .def("addComponent", &SceneObject::addComponent)
                .def("removeComponent", &SceneObject::removeComponent)
                .def("removeFromParent", &SceneObject::removeFromParent)
                .def("removeFromParentRecursive", &SceneObject::removeFromParentRecursive)
                .property("type", &SceneObject::type, &SceneObject::setType)
                .def("findCameraComponent", &SceneObject::findComponent<CameraComponent>)
                .def("findPhysicsBodyComponent", &SceneObject::findComponent<PhysicsBodyComponent>)
                .property("bodyType", &SceneObject::bodyType, &SceneObject::setBodyType)
                .property("freezable", &SceneObject::freezable, &SceneObject::setFreezable)
                .property("freezeRadius", &SceneObject::freezeRadius, &SceneObject::setFreezeRadius)
                .property("freezePhysics", &SceneObject::freezePhysics, &SceneObject::setFreezePhysics),

            luabind::class_<Tweening, TweeningPtr>("Tweening")
                .property("duration", &Tweening::duration)
                .def("getValue", &Tweening::getValue)
                .def("finished", &Tweening::finished)
                .property("loop", &Tweening::loop, &Tweening::setLoop),

            luabind::class_<SingleTweening, Tweening, TweeningPtr>("SingleTweening")
                .def(luabind::constructor<float, Easing, bool>())
                .def(luabind::constructor<float, Easing, float, float, bool>())
                .def("start", &SingleTweening::start)
                .def("tend", &SingleTweening::end),

            luabind::class_<SequentialTweening, Tweening, TweeningPtr>("SequentialTweening")
                .def(luabind::constructor<bool>())
                .def("addTweening", &SequentialTweening::addTweening),

            luabind::class_<PhysicsBodyComponent, PhysicsComponent, AObjectPtr>("PhysicsBodyComponent"),

            luabind::class_<Drawable, DrawablePtr>("Drawable"),

            luabind::class_<Platform>("Platform")
                .property("desktopVideoModes", &Platform::desktopVideoModes)
                .property("winVideoModes", &Platform::winVideoModes)
                .property("defaultVideoMode", &Platform::defaultVideoMode)
                .property("desktopVideoMode", &Platform::desktopVideoMode)
                .property("msaaModes", &Platform::msaaModes)
                .property("vsyncSupported", &Platform::vsyncSupported)
                .def("changeVideoMode", &Platform::changeVideoMode),

            luabind::class_<GameShell>("GameShell"),

            luabind::class_<InputGamepad>("InputGamepad")
                .property("stickDeadzone", &InputGamepad::stickDeadzone, &InputGamepad::setStickDeadzone)
                .property("triggerDeadzone", &InputGamepad::triggerDeadzone, &InputGamepad::setTriggerDeadzone)
                .def("triggered", &InputGamepad::triggered),

            luabind::class_<InputManager>("InputManager")
                .property("gamepad", &InputManager::gamepad)
                .property("usingGamepad", &InputManager::usingGamepad)
        ];
#endif
    }

    int Script::Impl::loadPackage(lua_State* L)
    {
        Impl* impl = reinterpret_cast<Impl*>(lua_touserdata(L, lua_upvalueindex(1)));

        (void)impl;

        const char *name = luaL_checkstring(L, 1);

        std::string path = std::string("modules/") + name + ".lua";

        std::string str;

        boost::shared_ptr<PlatformIFStream> is =
            boost::make_shared<PlatformIFStream>(path);

        if (!*is) {
            std::string path2 = std::string(name) + ".lua";

            is = boost::make_shared<PlatformIFStream>(path2);

            if (!*is) {
                luaL_error(L, "Cannot open file \"%s\" or \"%s\"",
                    path.c_str(), path2.c_str());
                return 1;
            }

            path = path2;
        }

        if (!readStream(*is, str)) {
            luaL_error(L, "Error reading file \"%s\"", path.c_str());
            return 1;
        }

        if (::luaL_loadbuffer(L, &str[0], str.size(), path.c_str()) != 0) {
            const char* errText = lua_tostring(L, -1);

            std::string text;

            if (errText) {
                text = errText;
            } else {
                text = std::string("Cannot load lua chunk from file \"") +
                                    path + "\"";
            }

            lua_pop(L, 1);

            luaL_error(L, "%s", text.c_str());
        }

        return 1;
    }

    void Script::Impl::setPackageLoaders()
    {
        {
            luabind::object loaders = luabind::newtable(L_);

            luabind::globals(L_)["package"]["loaders"] = loaders;
        }

        lua_getfield(L_, LUA_GLOBALSINDEX, "package");
        lua_getfield(L_, -1, "loaders");
        lua_remove(L_, -2);

        lua_pushinteger(L_, 1);
        lua_pushlightuserdata(L_, this);
        lua_pushcclosure(L_, &Impl::loadPackage, 1);
        lua_rawset(L_, -3);
        lua_pop(L_, 1);
    }

    void Script::Impl::require(const std::string& name)
    {
        luabind::call_function<void>(L_, "require", name);
    }

    void Script::Impl::createGlobals()
    {
        luabind::globals(L_)["scene"] = scene_;
        luabind::globals(L_)["factory"] = &sceneObjectFactory;
        luabind::globals(L_)["settings"] = &settings;
        luabind::globals(L_)["platform"] = platform.get();
        luabind::globals(L_)["input"] = &inputManager;
        luabind::globals(L_)["gameShell"] = gameShell.get();
    }

    void Script::Impl::loadFile()
    {
        std::string str;

        PlatformIFStream is(path_);

        if (!is) {
            throw std::runtime_error("Cannot open file \"" + path_ + "\"");
        }

        if (!readStream(is, str)) {
            throw std::runtime_error("Error reading file \"" + path_ + "\"");
        }

        if (::luaL_loadbuffer(L_, &str[0], str.size(), path_.c_str()) != 0) {
            const char* errText = ::lua_tostring(L_, -1);

            std::string text;

            if (errText) {
                text = errText;
            } else {
                text = std::string("Cannot load lua chunk from file \"") +
                                    path_ + "\"";
            }

            ::lua_pop(L_, 1);

            throw std::runtime_error(text);
        }
    }

    int Script::Impl::print(lua_State* L)
    {
        Impl* impl = reinterpret_cast<Impl*>(lua_touserdata(L, lua_upvalueindex(1)));

        log4cplus::NDCContextCreator ndc(impl->path_);

        std::ostringstream os;

        int n = lua_gettop(L);
        int i;
        lua_getglobal(L, "tostring");
        for (i = 1; i <= n; ++i) {
            const char* s;
            lua_pushvalue(L, -1);
            lua_pushvalue(L, i);
            lua_call(L, 1, 1);
            s = lua_tostring(L, -1);
            if (s == nullptr) {
                return luaL_error(L, LUA_QL("tostring") " must return a string to "
                    LUA_QL("print"));
            }
            if (i > 1) {
                os << "\t";
            }
            os << s;
            lua_pop(L, 1);
        }

        LOG4CPLUS_DEBUG(logger(), os.str());

        return 0;
    }

    Script::Script(const std::string& path,
        Scene* scene)
    : impl_(new Impl(path, scene))
    {
    }

    Script::~Script()
    {
    }

    bool Script::init()
    {
        impl_->L_ = ::lua_open();

        if (!impl_->L_) {
            LOG4CPLUS_ERROR(logger(), "Unable to create lua_State");

            return false;
        }

        luaL_openlibs(impl_->L_);

        try {
            luabind::open(impl_->L_);

            luabind::set_pcall_callback(&errorHandler);

            impl_->bind();

            impl_->setPackageLoaders();

            impl_->require("strict");
            impl_->require("utils");
            impl_->require("ui_utils");

            impl_->createGlobals();

            lua_pushlightuserdata(impl_->L_, impl_.get());
            lua_pushcclosure(impl_->L_, &Impl::print, 1);
            lua_setglobal(impl_->L_, "print");
        } catch (const luabind::error& e) {
            ::lua_pop(e.state(), 1);

            return false;
        } catch (const std::exception& e) {
            LOG4CPLUS_ERROR(logger(), e.what());

            return false;
        }

        return true;
    }

    bool Script::run()
    {
        try {
            impl_->require("startup");
            impl_->loadFile();
        } catch (const luabind::error& e) {
            ::lua_pop(e.state(), 1);

            return false;
        } catch (const std::exception& e) {
            LOG4CPLUS_ERROR(logger(), e.what());

            return false;
        }

        int base = ::lua_gettop(impl_->L_);

        ::lua_pushcfunction(impl_->L_, &errorHandler);
        ::lua_insert(impl_->L_, base);

        int result = ::lua_pcall(impl_->L_, 0, 0, base);

        ::lua_remove(impl_->L_, base);

        if (result != 0) {
            ::lua_pop(impl_->L_, 1);
        }

        return (result == 0);
    }

    void Script::finalize()
    {
        luabind::object obj = luabind::globals(impl_->L_)["finalizer"];

        if (!obj || (luabind::type(obj) != LUA_TFUNCTION)) {
            return;
        }

        try {
            luabind::call_function<void>(impl_->L_, "finalizer");
        } catch (const luabind::error& e) {
            ::lua_pop(e.state(), 1);
        } catch (const std::exception& e) {
            LOG4CPLUS_ERROR(logger(), e.what());
        }
    }

    struct lua_State* Script::state()
    {
        return impl_->L_;
    }
}
