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

#include "Logger.h"
#include "Game.h"
#include "InputKeyboard.h"
#include "OGL.h"
#include "Settings.h"
#include "PlatformLinux.h"
#include "GameLogAppender.h"
#include "DummyShell.h"
#include "AssimpLogStream.h"
#include "af3d/Types.h"
#include "af3d/StreamAppConfig.h"
#include "af3d/SequentialAppConfig.h"
#include "af3d/OAL.h"
#include "assimp/DefaultLogger.hpp"
#include <log4cplus/configurator.h>
#include <log4cplus/spi/factory.h>
#include <boost/make_shared.hpp>
#include <boost/thread.hpp>
#include <iostream>
#include <fstream>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#include <X11/XF86keysym.h>
#define XK_MISCELLANY
#define XK_LATIN1
#define XK_XKB_KEYS
#include <X11/keysymdef.h>
#include <X11/extensions/xf86vmode.h>
#include <GL/glx.h>
#include <dlfcn.h>
#include <linux/joystick.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include <fcntl.h>
#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define GLX_GET_PROC(func, sym) \
    do { \
        *(void**)(&func) = (void*)getProcAddress((const GLubyte*)#sym); \
        if (!func) { \
            LOG4CPLUS_ERROR(af3d::logger(), "Unable to load GLX symbol: " << ::dlerror()); \
            return false; \
        } \
    } while (0)

#define GLX_GET_PROC_OPT(extName, func, sym) \
    do { \
        if ((strstr(extStr, #extName " ") != nullptr)) { \
            *(void**)(&func) = (void*)getProcAddress((const GLubyte*)#sym); \
            if (!func) { \
                LOG4CPLUS_WARN(af3d::logger(), "Unable to load GLX symbol " #sym); \
            } \
        } else { \
            LOG4CPLUS_WARN(af3d::logger(), #extName " is not supported"); \
        } \
    } while (0)

#define GL_GET_PROC(func, sym) \
    do { \
        *(void**)(&af3d::ogl.func) = (void*)getProcAddress((const GLubyte*)#sym); \
        if (!af3d::ogl.func) { \
            *(void**)(&af3d::ogl.func) = ::dlsym(handle, #sym); \
            if (!af3d::ogl.func) { \
                LOG4CPLUS_ERROR(af3d::logger(), "Unable to load GL symbol: " << ::dlerror()); \
                return false; \
            } \
        } \
    } while (0)

#define AL_GET_PROC(func, sym) \
    do { \
        *(void**)(&af3d::oal.func) = ::dlsym(handle, #sym); \
        if (!af3d::oal.func) { \
            LOG4CPLUS_ERROR(af3d::logger(), "Unable to load OpenAL symbol: " << ::dlerror()); \
            return false; \
        } \
    } while (0)

static af3d::KeyIdentifier kiMap[256];

static void InitKIMap()
{
    memset(kiMap, 0, sizeof(kiMap));

    kiMap[XK_BackSpace & 0xFF] = Rocket::Core::Input::KI_BACK;
    kiMap[XK_Tab & 0xFF] = Rocket::Core::Input::KI_TAB;
    kiMap[XK_Clear & 0xFF] = Rocket::Core::Input::KI_CLEAR;
    kiMap[XK_Return & 0xFF] = Rocket::Core::Input::KI_RETURN;
    kiMap[XK_Pause & 0xFF] = Rocket::Core::Input::KI_PAUSE;
    kiMap[XK_Scroll_Lock & 0xFF] = Rocket::Core::Input::KI_SCROLL;
    kiMap[XK_Escape & 0xFF] = Rocket::Core::Input::KI_ESCAPE;
    kiMap[XK_Delete & 0xFF] = Rocket::Core::Input::KI_DELETE;
    kiMap[XK_Kanji & 0xFF] = Rocket::Core::Input::KI_KANJI;
    kiMap[XK_Touroku & 0xFF] = Rocket::Core::Input::KI_OEM_FJ_TOUROKU;
    kiMap[XK_Massyo & 0xFF] = Rocket::Core::Input::KI_OEM_FJ_MASSHOU;
    kiMap[XK_Home & 0xFF] = Rocket::Core::Input::KI_HOME;
    kiMap[XK_Left & 0xFF] = Rocket::Core::Input::KI_LEFT;
    kiMap[XK_Up & 0xFF] = Rocket::Core::Input::KI_UP;
    kiMap[XK_Right & 0xFF] = Rocket::Core::Input::KI_RIGHT;
    kiMap[XK_Down & 0xFF] = Rocket::Core::Input::KI_DOWN;
    kiMap[XK_Prior & 0xFF] = Rocket::Core::Input::KI_PRIOR;
    kiMap[XK_Next & 0xFF] = Rocket::Core::Input::KI_NEXT;
    kiMap[XK_End & 0xFF] = Rocket::Core::Input::KI_END;
    kiMap[XK_Begin & 0xFF] = Rocket::Core::Input::KI_HOME;
    kiMap[XK_Print & 0xFF] = Rocket::Core::Input::KI_SNAPSHOT;
    kiMap[XK_Insert & 0xFF] = Rocket::Core::Input::KI_INSERT;
    kiMap[XK_Num_Lock & 0xFF] = Rocket::Core::Input::KI_NUMLOCK;
    kiMap[XK_KP_Space & 0xFF] = Rocket::Core::Input::KI_SPACE;
    kiMap[XK_KP_Tab & 0xFF] = Rocket::Core::Input::KI_TAB;
    kiMap[XK_KP_Enter & 0xFF] = Rocket::Core::Input::KI_NUMPADENTER;
    kiMap[XK_KP_F1 & 0xFF] = Rocket::Core::Input::KI_F1;
    kiMap[XK_KP_F2 & 0xFF] = Rocket::Core::Input::KI_F2;
    kiMap[XK_KP_F3 & 0xFF] = Rocket::Core::Input::KI_F3;
    kiMap[XK_KP_F4 & 0xFF] = Rocket::Core::Input::KI_F4;
    kiMap[XK_KP_Home & 0xFF] = Rocket::Core::Input::KI_NUMPAD7;
    kiMap[XK_KP_Left & 0xFF] = Rocket::Core::Input::KI_NUMPAD4;
    kiMap[XK_KP_Up & 0xFF] = Rocket::Core::Input::KI_NUMPAD8;
    kiMap[XK_KP_Right & 0xFF] = Rocket::Core::Input::KI_NUMPAD6;
    kiMap[XK_KP_Down & 0xFF] = Rocket::Core::Input::KI_NUMPAD2;
    kiMap[XK_KP_Prior & 0xFF] = Rocket::Core::Input::KI_NUMPAD9;
    kiMap[XK_KP_Next & 0xFF] = Rocket::Core::Input::KI_NUMPAD3;
    kiMap[XK_KP_End & 0xFF] = Rocket::Core::Input::KI_NUMPAD1;
    kiMap[XK_KP_Begin & 0xFF] = Rocket::Core::Input::KI_NUMPAD5;
    kiMap[XK_KP_Insert & 0xFF] = Rocket::Core::Input::KI_NUMPAD0;
    kiMap[XK_KP_Delete & 0xFF] = Rocket::Core::Input::KI_DECIMAL;
    kiMap[XK_KP_Equal & 0xFF] = Rocket::Core::Input::KI_OEM_NEC_EQUAL;
    kiMap[XK_KP_Multiply & 0xFF] = Rocket::Core::Input::KI_MULTIPLY;
    kiMap[XK_KP_Add & 0xFF] = Rocket::Core::Input::KI_ADD;
    kiMap[XK_KP_Separator & 0xFF] = Rocket::Core::Input::KI_SEPARATOR;
    kiMap[XK_KP_Subtract & 0xFF] = Rocket::Core::Input::KI_SUBTRACT;
    kiMap[XK_KP_Decimal & 0xFF] = Rocket::Core::Input::KI_DECIMAL;
    kiMap[XK_KP_Divide & 0xFF] = Rocket::Core::Input::KI_DIVIDE;
    kiMap[XK_F1 & 0xFF] = Rocket::Core::Input::KI_F1;
    kiMap[XK_F2 & 0xFF] = Rocket::Core::Input::KI_F2;
    kiMap[XK_F3 & 0xFF] = Rocket::Core::Input::KI_F3;
    kiMap[XK_F4 & 0xFF] = Rocket::Core::Input::KI_F4;
    kiMap[XK_F5 & 0xFF] = Rocket::Core::Input::KI_F5;
    kiMap[XK_F6 & 0xFF] = Rocket::Core::Input::KI_F6;
    kiMap[XK_F7 & 0xFF] = Rocket::Core::Input::KI_F7;
    kiMap[XK_F8 & 0xFF] = Rocket::Core::Input::KI_F8;
    kiMap[XK_F9 & 0xFF] = Rocket::Core::Input::KI_F9;
    kiMap[XK_F10 & 0xFF] = Rocket::Core::Input::KI_F10;
    kiMap[XK_F11 & 0xFF] = Rocket::Core::Input::KI_F11;
    kiMap[XK_F12 & 0xFF] = Rocket::Core::Input::KI_F12;
    kiMap[XK_F13 & 0xFF] = Rocket::Core::Input::KI_F13;
    kiMap[XK_F14 & 0xFF] = Rocket::Core::Input::KI_F14;
    kiMap[XK_F15 & 0xFF] = Rocket::Core::Input::KI_F15;
    kiMap[XK_F16 & 0xFF] = Rocket::Core::Input::KI_F16;
    kiMap[XK_F17 & 0xFF] = Rocket::Core::Input::KI_F17;
    kiMap[XK_F18 & 0xFF] = Rocket::Core::Input::KI_F18;
    kiMap[XK_F19 & 0xFF] = Rocket::Core::Input::KI_F19;
    kiMap[XK_F20 & 0xFF] = Rocket::Core::Input::KI_F20;
    kiMap[XK_F21 & 0xFF] = Rocket::Core::Input::KI_F21;
    kiMap[XK_F22 & 0xFF] = Rocket::Core::Input::KI_F22;
    kiMap[XK_F23 & 0xFF] = Rocket::Core::Input::KI_F23;
    kiMap[XK_F24 & 0xFF] = Rocket::Core::Input::KI_F24;
    kiMap[XK_Shift_L & 0xFF] = Rocket::Core::Input::KI_LSHIFT;
    kiMap[XK_Shift_R & 0xFF] = Rocket::Core::Input::KI_RSHIFT;
    kiMap[XK_Control_L & 0xFF] = Rocket::Core::Input::KI_LCONTROL;
    kiMap[XK_Control_R & 0xFF] = Rocket::Core::Input::KI_RCONTROL;
    kiMap[XK_Caps_Lock & 0xFF] = Rocket::Core::Input::KI_CAPITAL;
    kiMap[XK_Alt_L & 0xFF] = Rocket::Core::Input::KI_LMENU;
    kiMap[XK_Alt_R & 0xFF] = Rocket::Core::Input::KI_RMENU;
    kiMap[XK_space & 0xFF] = Rocket::Core::Input::KI_SPACE;
    kiMap[XK_apostrophe & 0xFF] = Rocket::Core::Input::KI_OEM_7;
    kiMap[XK_comma & 0xFF] = Rocket::Core::Input::KI_OEM_COMMA;
    kiMap[XK_minus & 0xFF] = Rocket::Core::Input::KI_OEM_MINUS;
    kiMap[XK_period & 0xFF] = Rocket::Core::Input::KI_OEM_PERIOD;
    kiMap[XK_slash & 0xFF] = Rocket::Core::Input::KI_OEM_2;
    kiMap[XK_0 & 0xFF] = Rocket::Core::Input::KI_0;
    kiMap[XK_1 & 0xFF] = Rocket::Core::Input::KI_1;
    kiMap[XK_2 & 0xFF] = Rocket::Core::Input::KI_2;
    kiMap[XK_3 & 0xFF] = Rocket::Core::Input::KI_3;
    kiMap[XK_4 & 0xFF] = Rocket::Core::Input::KI_4;
    kiMap[XK_5 & 0xFF] = Rocket::Core::Input::KI_5;
    kiMap[XK_6 & 0xFF] = Rocket::Core::Input::KI_6;
    kiMap[XK_7 & 0xFF] = Rocket::Core::Input::KI_7;
    kiMap[XK_8 & 0xFF] = Rocket::Core::Input::KI_8;
    kiMap[XK_9 & 0xFF] = Rocket::Core::Input::KI_9;
    kiMap[XK_semicolon & 0xFF] = Rocket::Core::Input::KI_OEM_1;
    kiMap[XK_equal & 0xFF] = Rocket::Core::Input::KI_OEM_PLUS;
    kiMap[XK_bracketleft & 0xFF] = Rocket::Core::Input::KI_OEM_4;
    kiMap[XK_backslash & 0xFF] = Rocket::Core::Input::KI_OEM_5;
    kiMap[XK_bracketright & 0xFF] = Rocket::Core::Input::KI_OEM_6;
    kiMap[XK_grave & 0xFF] = Rocket::Core::Input::KI_OEM_3;
    kiMap[XK_a & 0xFF] = Rocket::Core::Input::KI_A;
    kiMap[XK_b & 0xFF] = Rocket::Core::Input::KI_B;
    kiMap[XK_c & 0xFF] = Rocket::Core::Input::KI_C;
    kiMap[XK_d & 0xFF] = Rocket::Core::Input::KI_D;
    kiMap[XK_e & 0xFF] = Rocket::Core::Input::KI_E;
    kiMap[XK_f & 0xFF] = Rocket::Core::Input::KI_F;
    kiMap[XK_g & 0xFF] = Rocket::Core::Input::KI_G;
    kiMap[XK_h & 0xFF] = Rocket::Core::Input::KI_H;
    kiMap[XK_i & 0xFF] = Rocket::Core::Input::KI_I;
    kiMap[XK_j & 0xFF] = Rocket::Core::Input::KI_J;
    kiMap[XK_k & 0xFF] = Rocket::Core::Input::KI_K;
    kiMap[XK_l & 0xFF] = Rocket::Core::Input::KI_L;
    kiMap[XK_m & 0xFF] = Rocket::Core::Input::KI_M;
    kiMap[XK_n & 0xFF] = Rocket::Core::Input::KI_N;
    kiMap[XK_o & 0xFF] = Rocket::Core::Input::KI_O;
    kiMap[XK_p & 0xFF] = Rocket::Core::Input::KI_P;
    kiMap[XK_q & 0xFF] = Rocket::Core::Input::KI_Q;
    kiMap[XK_r & 0xFF] = Rocket::Core::Input::KI_R;
    kiMap[XK_s & 0xFF] = Rocket::Core::Input::KI_S;
    kiMap[XK_t & 0xFF] = Rocket::Core::Input::KI_T;
    kiMap[XK_u & 0xFF] = Rocket::Core::Input::KI_U;
    kiMap[XK_v & 0xFF] = Rocket::Core::Input::KI_V;
    kiMap[XK_w & 0xFF] = Rocket::Core::Input::KI_W;
    kiMap[XK_x & 0xFF] = Rocket::Core::Input::KI_X;
    kiMap[XK_y & 0xFF] = Rocket::Core::Input::KI_Y;
    kiMap[XK_z & 0xFF] = Rocket::Core::Input::KI_Z;
}

static XF86VidModeModeInfo** vidmodes = nullptr;
static int numVidmodes = 0;
static XF86VidModeModeInfo desktopMode;

static Display* dpy = nullptr;
static GLXFBConfig config = nullptr;
static Window window = 0;
static Atom deleteMessage = 0;
static GLXContext context = nullptr;
static af3d::Game game;

typedef void (*PFNGLXSWAPBUFFERSPROC)(Display* dpy, GLXDrawable drawable);
typedef void (*PFNGLXDESTROYCONTEXTPROC)(Display* dpy, GLXContext ctx);
typedef Bool (*PFNGLXMAKECURRENTPROC)(Display* dpy, GLXDrawable drawable, GLXContext ctx);
typedef const char* (*PFNGLXQUERYEXTENSIONSSTRINGPROC)(Display* dpy, int screen);

static PFNGLXGETPROCADDRESSPROC getProcAddress = nullptr;
static PFNGLXCHOOSEFBCONFIGPROC chooseFBConfig = nullptr;
static PFNGLXGETFBCONFIGATTRIBPROC getFBConfigAttrib = nullptr;
static PFNGLXGETVISUALFROMFBCONFIGPROC getVisualFromFBConfig = nullptr;
static PFNGLXQUERYEXTENSIONSSTRINGPROC queryExtensionsString = nullptr;
static PFNGLXMAKECURRENTPROC makeCurrent = nullptr;
static PFNGLXSWAPBUFFERSPROC swapBuffers = nullptr;
static PFNGLXCREATENEWCONTEXTPROC createNewContext = nullptr;
static PFNGLXDESTROYCONTEXTPROC destroyContext = nullptr;

static PFNGLXCREATECONTEXTATTRIBSARBPROC createContextAttribsARB = nullptr;
static PFNGLXSWAPINTERVALSGIPROC swapIntervalSGI = nullptr;
static PFNGLXSWAPINTERVALEXTPROC swapIntervalEXT = nullptr;
static PFNGLXSWAPINTERVALMESAPROC swapIntervalMESA = nullptr;

static const int ctxAttribs[] =
{
    GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
    GLX_CONTEXT_MINOR_VERSION_ARB, 2,
    GLX_RENDER_TYPE, GLX_RGBA_TYPE,
    GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
    None
};

static const int configAttribs[] =
{
    GLX_X_RENDERABLE, True,
    GLX_DOUBLEBUFFER, True,
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
    GLX_DEPTH_SIZE, 24,
    GLX_STENCIL_SIZE, 8,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    None
};

static int msaaConfigAttribs[] =
{
    GLX_X_RENDERABLE, True,
    GLX_DOUBLEBUFFER, True,
    GLX_RENDER_TYPE, GLX_RGBA_BIT,
    GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
    GLX_RED_SIZE, 8,
    GLX_GREEN_SIZE, 8,
    GLX_BLUE_SIZE, 8,
    GLX_ALPHA_SIZE, 8,
    GLX_DEPTH_SIZE, 24,
    GLX_STENCIL_SIZE, 8,
    GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
    GLX_SAMPLES, 0,
    GLX_SAMPLE_BUFFERS, 1,
    None
};

static bool OGLPreInit()
{
    const char *extStr = nullptr;

    LOG4CPLUS_INFO(af3d::logger(), "Initializing OpenGL...");

    void* handle = ::dlopen("libGL.so.1", RTLD_NOW | RTLD_GLOBAL);

    if (!handle) {
        LOG4CPLUS_ERROR(af3d::logger(), "Unable to load libGL.so.1");
        return false;
    }

    getProcAddress = (PFNGLXGETPROCADDRESSPROC)::dlsym(handle, "glXGetProcAddressARB");

    if (!getProcAddress) {
        getProcAddress = (PFNGLXGETPROCADDRESSPROC)::dlsym(handle, "glXGetProcAddress");
    }

    if (!getProcAddress) {
        LOG4CPLUS_ERROR(af3d::logger(), "Unable to load symbol: " << ::dlerror());
        return false;
    }

    GLX_GET_PROC(chooseFBConfig, glXChooseFBConfig);
    GLX_GET_PROC(getFBConfigAttrib, glXGetFBConfigAttrib);
    GLX_GET_PROC(getVisualFromFBConfig, glXGetVisualFromFBConfig);
    GLX_GET_PROC(makeCurrent, glXMakeCurrent);
    GLX_GET_PROC(swapBuffers, glXSwapBuffers);
    GLX_GET_PROC(createNewContext, glXCreateNewContext);
    GLX_GET_PROC(destroyContext, glXDestroyContext);
    GLX_GET_PROC(queryExtensionsString, glXQueryExtensionsString);

    extStr = queryExtensionsString(dpy, DefaultScreen(dpy));

    if (!extStr) {
        LOG4CPLUS_ERROR(af3d::logger(), "Unable to obtain GLX extension string");
        return false;
    }

    GLX_GET_PROC_OPT(GLX_ARB_create_context, createContextAttribsARB, glXCreateContextAttribsARB);

    GLX_GET_PROC_OPT(GLX_EXT_swap_control, swapIntervalEXT, glXSwapIntervalEXT);
    if (!swapIntervalEXT) {
        GLX_GET_PROC_OPT(GLX_MESA_swap_control, swapIntervalMESA, glXSwapIntervalMESA);
        if (!swapIntervalMESA) {
            GLX_GET_PROC_OPT(GLX_SGI_swap_control, swapIntervalSGI, glXSwapIntervalSGI);
        }
    }

    GL_GET_PROC(DrawElementsBaseVertex, glDrawElementsBaseVertex);
    GL_GET_PROC(DepthMask, glDepthMask);
    GL_GET_PROC(DepthFunc, glDepthFunc);
    GL_GET_PROC(CullFace, glCullFace);
    GL_GET_PROC(DrawElements, glDrawElements);
    GL_GET_PROC(GenVertexArrays, glGenVertexArrays);
    GL_GET_PROC(DeleteVertexArrays, glDeleteVertexArrays);
    GL_GET_PROC(BindVertexArray, glBindVertexArray);
    GL_GET_PROC(GenBuffers, glGenBuffers);
    GL_GET_PROC(DeleteBuffers, glDeleteBuffers);
    GL_GET_PROC(BindBuffer, glBindBuffer);
    GL_GET_PROC(BufferData, glBufferData);
    GL_GET_PROC(BufferSubData, glBufferSubData);
    GL_GET_PROC(MapBufferRange, glMapBufferRange);
    GL_GET_PROC(UnmapBuffer, glUnmapBuffer);
    GL_GET_PROC(GenTextures, glGenTextures);
    GL_GET_PROC(DeleteTextures, glDeleteTextures);
    GL_GET_PROC(BindTexture, glBindTexture);
    GL_GET_PROC(ActiveTexture, glActiveTexture);
    GL_GET_PROC(TexImage2D, glTexImage2D);
    GL_GET_PROC(TexParameteri, glTexParameteri);
    GL_GET_PROC(ClearColor, glClearColor);
    GL_GET_PROC(Clear, glClear);
    GL_GET_PROC(Viewport, glViewport);
    GL_GET_PROC(AttachShader, glAttachShader);
    GL_GET_PROC(BindAttribLocation, glBindAttribLocation);
    GL_GET_PROC(CompileShader, glCompileShader);
    GL_GET_PROC(CreateProgram, glCreateProgram);
    GL_GET_PROC(CreateShader, glCreateShader);
    GL_GET_PROC(DeleteProgram, glDeleteProgram);
    GL_GET_PROC(DeleteShader, glDeleteShader);
    GL_GET_PROC(DetachShader, glDetachShader);
    GL_GET_PROC(DisableVertexAttribArray, glDisableVertexAttribArray);
    GL_GET_PROC(EnableVertexAttribArray, glEnableVertexAttribArray);
    GL_GET_PROC(LinkProgram, glLinkProgram);
    GL_GET_PROC(ShaderSource, glShaderSource);
    GL_GET_PROC(UseProgram, glUseProgram);
    GL_GET_PROC(GetProgramiv, glGetProgramiv);
    GL_GET_PROC(GetProgramInfoLog, glGetProgramInfoLog);
    GL_GET_PROC(GetActiveAttrib, glGetActiveAttrib);
    GL_GET_PROC(GetActiveUniform, glGetActiveUniform);
    GL_GET_PROC(GetShaderiv, glGetShaderiv);
    GL_GET_PROC(GetShaderInfoLog, glGetShaderInfoLog);
    GL_GET_PROC(GetAttribLocation, glGetAttribLocation);
    GL_GET_PROC(GetUniformLocation, glGetUniformLocation);
    GL_GET_PROC(VertexAttribPointer, glVertexAttribPointer);
    GL_GET_PROC(DrawArrays, glDrawArrays);
    GL_GET_PROC(Uniform1fv, glUniform1fv);
    GL_GET_PROC(Uniform3fv, glUniform3fv);
    GL_GET_PROC(Uniform4fv, glUniform4fv);
    GL_GET_PROC(Enable, glEnable);
    GL_GET_PROC(Disable, glDisable);
    GL_GET_PROC(BlendFunc, glBlendFunc);
    GL_GET_PROC(BlendFuncSeparate, glBlendFuncSeparate);
    GL_GET_PROC(UniformMatrix4fv, glUniformMatrix4fv);
    GL_GET_PROC(Uniform1i, glUniform1i);
    GL_GET_PROC(Uniform2i, glUniform2i);
    GL_GET_PROC(GetIntegerv, glGetIntegerv);
    GL_GET_PROC(GenerateMipmap, glGenerateMipmap);
    GL_GET_PROC(PointSize, glPointSize);
    GL_GET_PROC(LineWidth, glLineWidth);
    GL_GET_PROC(GenFramebuffers, glGenFramebuffersEXT);
    GL_GET_PROC(DeleteFramebuffers, glDeleteFramebuffersEXT);
    GL_GET_PROC(BindFramebuffer, glBindFramebufferEXT);
    GL_GET_PROC(FramebufferTexture2D, glFramebufferTexture2DEXT);
    GL_GET_PROC(CheckFramebufferStatus, glCheckFramebufferStatusEXT);
    GL_GET_PROC(Uniform1f, glUniform1f);
    GL_GET_PROC(Uniform2f, glUniform2f);
    GL_GET_PROC(ColorMask, glColorMask);
    GL_GET_PROC(StencilFunc, glStencilFunc);
    GL_GET_PROC(StencilOp, glStencilOp);
    GL_GET_PROC(GetString, glGetString);
    GL_GET_PROC(Scissor, glScissor);

    int n = 0;

    GLXFBConfig* configs = chooseFBConfig(dpy,
        DefaultScreen(dpy),
        configAttribs,
        &n);

    if (n <= 0) {
        LOG4CPLUS_ERROR(af3d::logger(), "Unable to choose config");
        return false;
    }

    std::set<std::uint32_t> ss;

    ss.insert(0);

    for (int i = 0; i < n; ++i) {
        int redSize = 0;
        int greenSize = 0;
        int blueSize = 0;
        int alphaSize = 0;
        int depthSize = 0;
        int samples = 0;

        getFBConfigAttrib(dpy, configs[i], GLX_RED_SIZE, &redSize);
        getFBConfigAttrib(dpy, configs[i], GLX_GREEN_SIZE, &greenSize);
        getFBConfigAttrib(dpy, configs[i], GLX_BLUE_SIZE, &blueSize);
        getFBConfigAttrib(dpy, configs[i], GLX_ALPHA_SIZE, &alphaSize);
        getFBConfigAttrib(dpy, configs[i], GLX_DEPTH_SIZE, &depthSize);
        getFBConfigAttrib(dpy, configs[i], GLX_SAMPLES, &samples);

        if ((redSize == 8) && (greenSize == 8) && (blueSize == 8) &&
            (alphaSize == 8) && (depthSize == 24) && (samples >= 0)) {
            ss.insert(samples);
        }
    }

    std::vector<std::uint32_t> msaaModes;

    std::copy(ss.begin(), ss.end(), std::back_inserter(msaaModes));

    ::XFree(configs);

    af3d::platform->setMsaaModes(msaaModes);

    if (swapIntervalEXT || swapIntervalMESA || swapIntervalSGI) {
        af3d::platform->setVSyncSupported(true);
    }

    return true;
}

static void PopulateVideoModes()
{
    static int majorVersion = 0;
    static int minorVersion = 0;

    std::uint32_t maxWinWidth = af3d::settings.viewWidth;
    std::uint32_t maxWinHeight = af3d::settings.viewHeight;

    if (!XF86VidModeQueryVersion(dpy, &majorVersion, &minorVersion)) {
        LOG4CPLUS_WARN(af3d::logger(), "XFree86-VidModeExtension not available");

        maxWinWidth *= 2;
        maxWinHeight *= 2;
    } else {
        std::set<af3d::VideoMode> tmp;

        XF86VidModeGetAllModeLines(dpy, DefaultScreen(dpy), &numVidmodes, &vidmodes);

        desktopMode = *vidmodes[0];

        for (int i = 0; i < numVidmodes; ++i) {
            if ((vidmodes[i]->hdisplay <= desktopMode.hdisplay) && (vidmodes[i]->vdisplay <= desktopMode.vdisplay)) {
                tmp.insert(af3d::VideoMode(vidmodes[i]->hdisplay, vidmodes[i]->vdisplay));
            }
        }

        if (desktopMode.hdisplay > maxWinWidth) {
            maxWinWidth = desktopMode.hdisplay;
        }

        if (desktopMode.vdisplay > maxWinHeight) {
            maxWinHeight = desktopMode.vdisplay;
        }

        int i = 0;

        for (const auto& vm : tmp) {
            if ((vm.width == desktopMode.hdisplay) && (vm.height == desktopMode.vdisplay)) {
                af3d::platform->setDesktopVideoMode(i);
                break;
            }
            ++i;
        }

        std::vector<af3d::VideoMode> videoModes;

        std::copy(tmp.begin(), tmp.end(), std::back_inserter(videoModes));

        af3d::platform->setDesktopVideoModes(videoModes);
    }

    std::set<af3d::VideoMode> tmp = af3d::settings.winVideoModes;

    tmp.insert(af3d::VideoMode(af3d::settings.viewWidth, af3d::settings.viewHeight));

    int i = 0;

    std::vector<af3d::VideoMode> videoModes;

    for (const auto& vm : tmp) {
        if ((vm.width > maxWinWidth) || (vm.height > maxWinHeight)) {
            continue;
        }

        if ((vm.width == af3d::settings.viewWidth) && (vm.height == af3d::settings.viewHeight)) {
            af3d::platform->setDefaultVideoMode(i);
        }

        videoModes.push_back(vm);

        ++i;
    }

    af3d::platform->setWinVideoModes(videoModes);
}

static bool OGLInit(bool vsync)
{
    if (createContextAttribsARB) {
        context = createContextAttribsARB(dpy,
            config,
            nullptr,
            True,
            ctxAttribs);
    } else {
        context = createNewContext(dpy, config, GLX_RGBA_TYPE, nullptr, True);
    }

    if (!context) {
        LOG4CPLUS_ERROR(af3d::logger(), "Unable to create context");
        return false;
    }

    if (!makeCurrent(dpy, window, context)) {
        LOG4CPLUS_ERROR(af3d::logger(), "Unable to make context current");
        return false;
    }

    if (swapIntervalEXT) {
        swapIntervalEXT(dpy, window, vsync ? 1 : 0);
    } else if (swapIntervalMESA) {
        swapIntervalMESA(vsync ? 1 : 0);
    } else if (swapIntervalSGI) {
        swapIntervalSGI(vsync ? 1 : 0);
    }

    LOG4CPLUS_INFO(af3d::logger(), "OpenGL initialized");

    return true;
}

static void OGLShutdown()
{
    LOG4CPLUS_INFO(af3d::logger(), "Shutting down OpenGL..");

    if (!makeCurrent(dpy, None, nullptr)) {
        LOG4CPLUS_WARN(af3d::logger(), "Unable to release current context");
    }

    destroyContext(dpy, context);

    LOG4CPLUS_INFO(af3d::logger(), "OpenGL shut down");
}

static bool createWindow(std::uint32_t width, std::uint32_t height, XVisualInfo* visualInfo, bool fullscreen)
{
    LOG4CPLUS_INFO(af3d::logger(), "Creating window " << width << "x" << height << ", fullscreen = " << fullscreen << "...");

    XSetWindowAttributes wa;

    memset(&wa, 0, sizeof(wa));

    wa.background_pixel = BlackPixel(dpy, DefaultScreen(dpy));
    wa.background_pixmap = None;
    wa.border_pixel = 0;
    wa.event_mask = StructureNotifyMask | ExposureMask;

    wa.colormap = ::XCreateColormap(dpy, RootWindow(dpy, DefaultScreen(dpy)),
                                    visualInfo->visual, AllocNone);

    unsigned long mask;

    if (fullscreen) {
        mask = CWBackPixel | CWColormap | CWEventMask;
    } else {
        mask = CWBackPixel | CWBorderPixel | CWEventMask | CWColormap;
    }

    window = ::XCreateWindow(dpy,
        RootWindow(dpy, DefaultScreen(dpy)),
        0,
        0,
        width,
        height,
        0,
        visualInfo->depth,
        InputOutput,
        visualInfo->visual,
        mask,
        &wa);

    if (!window) {
        LOG4CPLUS_ERROR(af3d::logger(), "Cannot create window");
        return false;
    }

    XSetStandardProperties(dpy,
        window,
        "AirForce3D",
        "AF3D",
        None,
        0,
        0,
        nullptr);

    if (fullscreen) {
        XSizeHints sizeHints;

        sizeHints.flags = 0;

       ::XSetWMNormalHints(dpy, window, &sizeHints);
    } else {
        XSizeHints sizeHints;

        sizeHints.flags = PMinSize | PMaxSize;
        sizeHints.min_width = sizeHints.max_width = width;
        sizeHints.min_height = sizeHints.max_height = height;

       ::XSetWMNormalHints(dpy, window, &sizeHints);
    }

    XMapWindow(dpy, window);

    if (fullscreen) {
        XEvent xev;
        memset(&xev, 0, sizeof(xev));
        xev.type = ClientMessage;
        xev.xclient.window = window;
        xev.xclient.message_type = ::XInternAtom(dpy, "_NET_WM_STATE", False);
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = 1;
        xev.xclient.data.l[1] = ::XInternAtom(dpy, "_NET_WM_STATE_FULLSCREEN", False);
        xev.xclient.data.l[2] = 0;

        XSendEvent (dpy, RootWindow(dpy, DefaultScreen(dpy)), False,
           SubstructureRedirectMask | SubstructureNotifyMask, &xev);

        XFlush(dpy);
    }

    deleteMessage = ::XInternAtom(dpy, "WM_DELETE_WINDOW", False);
    ::XSetWMProtocols(dpy, window, &deleteMessage, 1);

    XSync(dpy, False);

    if (fullscreen) {
        XWarpPointer(dpy, None, window, 0, 0, 0, 0, 0, 0);
        XGrabPointer(dpy, window,
            False,
            ButtonPressMask | ButtonReleaseMask | PointerMotionMask,
            GrabModeAsync, GrabModeAsync,
            window,
            None,
            CurrentTime);
        XGrabKeyboard(dpy, window,
            False,
            GrabModeAsync, GrabModeAsync,
            CurrentTime);
        XSync(dpy, False);
    }

    LOG4CPLUS_INFO(af3d::logger(), "Window created");

    return true;
}

static void destroyWindow()
{
    LOG4CPLUS_INFO(af3d::logger(), "Destroying window...");

    XUngrabPointer(dpy, CurrentTime);
    XUngrabKeyboard(dpy, CurrentTime);

    if (af3d::settings.fullscreen) {
        XF86VidModeSwitchToMode(dpy, DefaultScreen(dpy), &desktopMode);
        XF86VidModeSetViewPort(dpy, DefaultScreen(dpy), 0, 0);
    }

    XDestroyWindow(dpy, window);

    LOG4CPLUS_INFO(af3d::logger(), "Window destroyed");
}

static std::mutex cmMutex;
static std::condition_variable cmCond;
static bool cmDone = false;

static void renderThread()
{
    LOG4CPLUS_INFO(af3d::logger(), "Render thread started");

    makeCurrent(dpy, window, context);

    af3d::HardwareContext ctx;

    if (!game.renderReload(ctx)) {
        abort();
    }

    {
        af3d::ScopedLock lock(cmMutex);
        cmDone = true;
    }

    cmCond.notify_one();

    while (game.render(ctx)) {
        swapBuffers(dpy, window);
    }

    makeCurrent(dpy, 0, nullptr);

    LOG4CPLUS_INFO(af3d::logger(), "Render thread finished");
}

static bool OALInit()
{
    LOG4CPLUS_INFO(af3d::logger(), "Initializing OpenAL...");

    void* handle = ::dlopen("libopenal.so.1", RTLD_NOW | RTLD_GLOBAL);

    if (!handle) {
        LOG4CPLUS_ERROR(af3d::logger(), "Unable to load libopenal.so.1");
        return false;
    }

    AL_GET_PROC(cOpenDevice, alcOpenDevice);
    AL_GET_PROC(cCloseDevice, alcCloseDevice);
    AL_GET_PROC(cCreateContext, alcCreateContext);
    AL_GET_PROC(cMakeContextCurrent, alcMakeContextCurrent);
    AL_GET_PROC(cDestroyContext, alcDestroyContext);
    AL_GET_PROC(cGetIntegerv, alcGetIntegerv);
    AL_GET_PROC(cGetError, alcGetError);
    AL_GET_PROC(cGetString, alcGetString);
    AL_GET_PROC(cSuspendContext, alcSuspendContext);
    AL_GET_PROC(cProcessContext, alcProcessContext);

    AL_GET_PROC(DistanceModel, alDistanceModel);
    AL_GET_PROC(GetString, alGetString);
    AL_GET_PROC(BufferData, alBufferData);
    AL_GET_PROC(GenBuffers, alGenBuffers);
    AL_GET_PROC(DeleteBuffers, alDeleteBuffers);
    AL_GET_PROC(Sourcef, alSourcef);
    AL_GET_PROC(Source3f, alSource3f);
    AL_GET_PROC(GetSourceiv, alGetSourceiv);
    AL_GET_PROC(GetSourcefv, alGetSourcefv);
    AL_GET_PROC(Sourcei, alSourcei);
    AL_GET_PROC(SourcePlay, alSourcePlay);
    AL_GET_PROC(SourcePause, alSourcePause);
    AL_GET_PROC(SourceStop, alSourceStop);
    AL_GET_PROC(SourceQueueBuffers, alSourceQueueBuffers);
    AL_GET_PROC(GenSources, alGenSources);
    AL_GET_PROC(DeleteSources, alDeleteSources);
    AL_GET_PROC(GetError, alGetError);
    AL_GET_PROC(SourceRewind, alSourceRewind);
    AL_GET_PROC(Listenerf, alListenerf);
    AL_GET_PROC(GetListenerfv, alGetListenerfv);
    AL_GET_PROC(Listener3f, alListener3f);
    AL_GET_PROC(SourceUnqueueBuffers, alSourceUnqueueBuffers);

    LOG4CPLUS_INFO(af3d::logger(), "OpenAL initialized");

    return true;
}

boost::thread thr;

bool af3d::PlatformLinux::changeVideoMode(bool fullscreen, int videoMode, int msaaMode, bool vsync, bool trilinearFilter)
{
    std::uint32_t samples = platform->msaaModes()[msaaMode];
    VideoMode vm;

    if (fullscreen) {
        vm = platform->desktopVideoModes()[videoMode];
    } else {
        vm = platform->winVideoModes()[videoMode];
    }

    int n = 0;

    const int* attribs = nullptr;

    if (samples > 0) {
        msaaConfigAttribs[23] = samples;

        attribs = msaaConfigAttribs;
    } else {
        attribs = configAttribs;
    }

    GLXFBConfig* configs = chooseFBConfig(dpy,
        DefaultScreen(dpy),
        attribs,
        &n);

    if (n <= 0) {
        LOG4CPLUS_ERROR(logger(), "Unable to choose config");
        return false;
    }

    config = configs[0];

    XVisualInfo* vi = getVisualFromFBConfig(dpy, config);

    ::XFree(configs);

    if (window) {
        game.cancelRender();

        thr.join();

        makeCurrent(dpy, window, context);

        OGLShutdown();

        destroyWindow();
    }

    if (fullscreen) {
        int i;

        for (i = 0; i < numVidmodes; ++i) {
            if ((vidmodes[i]->hdisplay == vm.width) && (vidmodes[i]->vdisplay == vm.height)) {
                break;
            }
        }

        XF86VidModeSwitchToMode(dpy, DefaultScreen(dpy), vidmodes[i]);
        XF86VidModeSetViewPort(dpy, DefaultScreen(dpy), 0, 0);
    }

    if (!createWindow(vm.width, vm.height, vi, fullscreen)) {
        ::XFree(vi);

        return false;
    }

    ::XFree(vi);

    if (!OGLInit(vsync)) {
        return false;
    }

    ::XSelectInput(dpy, window, KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

    ::XkbSetDetectableAutoRepeat(dpy, True, nullptr);

    makeCurrent(dpy, 0, nullptr);

    settings.videoMode = videoMode;
    settings.msaaMode = msaaMode;
    settings.vsync = vsync;
    settings.fullscreen = fullscreen;
    settings.trilinearFilter = trilinearFilter;

    if (settings.viewAspect >= 1.0f) {
        settings.viewWidth = vm.width;
        settings.viewHeight = static_cast<float>(vm.width) / settings.viewAspect;

        if (settings.viewHeight <= vm.height) {
            settings.viewX = 0;
            settings.viewY = static_cast<float>(vm.height - settings.viewHeight) / 2.0f;
        } else {
            settings.viewHeight = vm.height;
            settings.viewWidth = static_cast<float>(settings.viewHeight) * settings.viewAspect;
            settings.viewX = static_cast<float>(vm.width - settings.viewWidth) / 2.0f;
            settings.viewY = 0;
        }
    } else {
        settings.viewHeight = vm.height;
        settings.viewWidth = static_cast<float>(settings.viewHeight) * settings.viewAspect;

        if (settings.viewWidth <= vm.width) {
            settings.viewX = static_cast<float>(vm.width - settings.viewWidth) / 2.0f;
            settings.viewY = 0;
        } else {
            settings.viewWidth = vm.width;
            settings.viewHeight = static_cast<float>(vm.width) / settings.viewAspect;
            settings.viewX = 0;
            settings.viewY = static_cast<float>(vm.height - settings.viewHeight) / 2.0f;
        }
    }

    {
        af3d::ScopedLockA lock(cmMutex);
        cmDone = false;
    }

    thr = boost::thread(&renderThread);

    {
        af3d::ScopedLockA lock(cmMutex);
        while (!cmDone) {
            cmCond.wait(lock);
        }
    }

    game.reload();

    return true;
}

/*
 * Gamepad stuff.
 * @{
 */

#define GAMEPAD_NUM 16

static struct
{
    struct
    {
        int fd;
        std::string path;
        uint16_t btnmap[KEY_MAX - BTN_MISC + 1];
        uint8_t axmap[ABS_MAX + 1];
        af3d::Vector2f stick[2];
        af3d::GamepadButton dpad[2];
    } inst[GAMEPAD_NUM];
    int inotify;
    int watch;
} gamepad;

static void gamepadOpen(const std::string& path)
{
    int idx;

    for (idx = 0; idx < GAMEPAD_NUM; ++idx) {
        if (gamepad.inst[idx].fd == -1) {
            continue;
        }

        if (gamepad.inst[idx].path == path) {
            return;
        }
    }

    for (idx = 0; idx < GAMEPAD_NUM; ++idx) {
        if (gamepad.inst[idx].fd == -1) {
            break;
        }
    }

    if (idx >= GAMEPAD_NUM) {
        return;
    }

    int fd = open(path.c_str(), O_RDONLY | O_NONBLOCK);

    if (fd == -1) {
        LOG4CPLUS_ERROR(af3d::logger(), "Cannot open " << path << ": " << strerror(errno));

        return;
    }

    int version = 0;

    ioctl(fd, JSIOCGVERSION, &version);
    if (version < 0x010000) {
        LOG4CPLUS_WARN(af3d::logger(), "Gamepad " << path << " uses outdated interface, skipping");
        close(fd);
        return;
    }

    char name[256];

    if (ioctl(fd, JSIOCGNAME(sizeof(name)), name) < 0) {
        strncpy(name, "Unknown gamepad", sizeof(name));
    }

    gamepad.inst[idx].fd = fd;
    gamepad.inst[idx].path = path;
    ioctl(fd, JSIOCGAXMAP, gamepad.inst[idx].axmap);
    ioctl(fd, JSIOCGBTNMAP, gamepad.inst[idx].btnmap);
    gamepad.inst[idx].stick[0] = af3d::Vector2f_zero;
    gamepad.inst[idx].stick[1] = af3d::Vector2f_zero;
    gamepad.inst[idx].dpad[0] = af3d::GamepadButton::Unknown;
    gamepad.inst[idx].dpad[1] = af3d::GamepadButton::Unknown;

    LOG4CPLUS_INFO(af3d::logger(), "gamepad " << idx << ": " << path << " - " << name);
}

static void gamepadInit()
{
    LOG4CPLUS_INFO(af3d::logger(), "Initializing gamepads...");

    for (int idx = 0; idx < GAMEPAD_NUM; ++idx) {
        gamepad.inst[idx].fd = -1;
    }

    gamepad.inotify = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
    gamepad.watch = -1;

    if (gamepad.inotify != -1) {
        gamepad.watch = inotify_add_watch(gamepad.inotify,
            "/dev/input", IN_CREATE | IN_ATTRIB);
        if (gamepad.watch == -1) {
            LOG4CPLUS_WARN(af3d::logger(), "cannot add inotify watch on /dev/input (" << strerror(errno) << "), disabling gamepad hotplug");
            close(gamepad.inotify);
            gamepad.inotify = -1;
        }
    } else {
        LOG4CPLUS_WARN(af3d::logger(), "cannot init inotify (" << strerror(errno) << "), disabling gamepad hotplug");
    }

    DIR* dir = opendir("/dev/input");
    if (dir) {
        struct dirent* entry;

        while ((entry = readdir(dir))) {
            std::string name = entry->d_name;

            if (name.compare(0, 2, "js") != 0) {
                continue;
            }

            name = name.substr(2);

            std::istringstream is(name);
            int idx = 0;

            if (!(is >> idx) || !is.eof() || (idx < 0)) {
                continue;
            }

            gamepadOpen(std::string("/dev/input/") + entry->d_name);
        }

        closedir(dir);
    } else {
        LOG4CPLUS_WARN(af3d::logger(), "cannot open /dev/input (" << strerror(errno) << "), no gamepads listed");
    }

    LOG4CPLUS_INFO(af3d::logger(), "gamepads initialized");
}

static void gamepadShutdown()
{
    LOG4CPLUS_INFO(af3d::logger(), "Shutting down gamepads...");

    for (int idx = 0; idx < GAMEPAD_NUM; ++idx) {
        if (gamepad.inst[idx].fd != -1) {
            close(gamepad.inst[idx].fd);
            gamepad.inst[idx].fd = -1;
        }
    }

    if (gamepad.inotify != -1) {
        if (gamepad.watch != -1) {
            inotify_rm_watch(gamepad.inotify, gamepad.watch);
        }
        close(gamepad.inotify);
    }

    LOG4CPLUS_INFO(af3d::logger(), "gamepads shut down");
}

static void gamepadUpdate()
{
    if (gamepad.inotify != -1) {
        ssize_t offset = 0;
        static char buffer[16384];

        ssize_t size = read(gamepad.inotify, buffer, sizeof(buffer));

        while (size > offset) {
            const struct inotify_event* e = (struct inotify_event*)(buffer + offset);

            std::string name = e->name;

            if (name.compare(0, 2, "js") == 0) {
                name = name.substr(2);

                std::istringstream is(name);
                int idx = 0;

                if ((is >> idx) && is.eof() && (idx >= 0)) {
                    gamepadOpen(std::string("/dev/input/") + e->name);
                }
            }

            offset += sizeof(struct inotify_event) + e->len;
        }
    }

    for (int i = 0; i < GAMEPAD_NUM; ++i) {
        if (gamepad.inst[i].fd == -1) {
            continue;
        }

        for (;;) {
            struct js_event e;

            errno = 0;

            if (read(gamepad.inst[i].fd, &e, sizeof(e)) < 0) {
                if (errno == ENODEV) {
                    LOG4CPLUS_INFO(af3d::logger(), "gamepad " << i << ": disconnected");
                    close(gamepad.inst[i].fd);
                    gamepad.inst[i].fd = -1;
                }

                break;
            }

            e.type &= ~JS_EVENT_INIT;

            switch (e.type) {
            case JS_EVENT_AXIS: {
                int axis = gamepad.inst[i].axmap[e.number];
                float value = static_cast<float>(e.value) / 32767.0f;
                switch (axis) {
                case ABS_X:
                    gamepad.inst[i].stick[0].setX(value);
                    game.gamepadMoveStick(true, gamepad.inst[i].stick[0]);
                    break;
                case ABS_Y:
                    gamepad.inst[i].stick[0].setY(-value);
                    game.gamepadMoveStick(true, gamepad.inst[i].stick[0]);
                    break;
                case ABS_Z:
                    game.gamepadMoveTrigger(true, (value + 1.0f) / 2.0f);
                    break;
                case ABS_RX:
                    gamepad.inst[i].stick[1].setX(value);
                    game.gamepadMoveStick(false, gamepad.inst[i].stick[1]);
                    break;
                case ABS_RY:
                    gamepad.inst[i].stick[1].setY(-value);
                    game.gamepadMoveStick(false, gamepad.inst[i].stick[1]);
                    break;
                case ABS_RZ:
                    game.gamepadMoveTrigger(false, (value + 1.0f) / 2.0f);
                    break;
                case ABS_HAT0X:
                    if (e.value == 32767) {
                        if (gamepad.inst[i].dpad[0] == af3d::GamepadButton::DPADLeft) {
                            game.gamepadRelease(gamepad.inst[i].dpad[0]);
                        }
                        gamepad.inst[i].dpad[0] = af3d::GamepadButton::DPADRight;
                        game.gamepadPress(af3d::GamepadButton::DPADRight);
                    } else if (e.value == -32767) {
                        if (gamepad.inst[i].dpad[0] == af3d::GamepadButton::DPADRight) {
                            game.gamepadRelease(gamepad.inst[i].dpad[0]);
                        }
                        gamepad.inst[i].dpad[0] = af3d::GamepadButton::DPADLeft;
                        game.gamepadPress(af3d::GamepadButton::DPADLeft);
                    } else if (gamepad.inst[i].dpad[0] != af3d::GamepadButton::Unknown) {
                        game.gamepadRelease(gamepad.inst[i].dpad[0]);
                        gamepad.inst[i].dpad[0] = af3d::GamepadButton::Unknown;
                    }
                    break;
                case ABS_HAT0Y:
                    if (e.value == 32767) {
                        if (gamepad.inst[i].dpad[1] == af3d::GamepadButton::DPADUp) {
                            game.gamepadRelease(gamepad.inst[i].dpad[1]);
                        }
                        gamepad.inst[i].dpad[1] = af3d::GamepadButton::DPADDown;
                        game.gamepadPress(af3d::GamepadButton::DPADDown);
                    } else if (e.value == -32767) {
                        if (gamepad.inst[i].dpad[1] == af3d::GamepadButton::DPADDown) {
                            game.gamepadRelease(gamepad.inst[i].dpad[1]);
                        }
                        gamepad.inst[i].dpad[1] = af3d::GamepadButton::DPADUp;
                        game.gamepadPress(af3d::GamepadButton::DPADUp);
                    } else if (gamepad.inst[i].dpad[1] != af3d::GamepadButton::Unknown) {
                        game.gamepadRelease(gamepad.inst[i].dpad[1]);
                        gamepad.inst[i].dpad[1] = af3d::GamepadButton::Unknown;
                    }
                    break;
                default:
                    break;
                }
                break;
            }
            case JS_EVENT_BUTTON: {
                int button = gamepad.inst[i].btnmap[e.number];
                af3d::GamepadButton gb = af3d::GamepadButton::Unknown;
                switch (button) {
                case BTN_A: gb = af3d::GamepadButton::A; break;
                case BTN_B: gb = af3d::GamepadButton::B; break;
                case BTN_X: gb = af3d::GamepadButton::X; break;
                case BTN_Y: gb = af3d::GamepadButton::Y; break;
                case BTN_TL: gb = af3d::GamepadButton::LeftBumper; break;
                case BTN_TR: gb = af3d::GamepadButton::RightBumper; break;
                case BTN_SELECT: gb = af3d::GamepadButton::Back; break;
                case BTN_START: gb = af3d::GamepadButton::Start; break;
                case BTN_THUMBL: gb = af3d::GamepadButton::LeftStick; break;
                case BTN_THUMBR: gb = af3d::GamepadButton::RightStick; break;
                default: break;
                }
                if (gb != af3d::GamepadButton::Unknown) {
                    if (e.value) {
                        game.gamepadPress(gb);
                    } else {
                        game.gamepadRelease(gb);
                    }
                }
                break;
            }
            default:
                break;
            }
        }
    }
}

/*
 * @}
 */

extern const char configIniStr[];

static void startupFailed()
{
    std::cerr << "Game startup failed, see log.txt for details" << std::endl;
}

static af3d::AppConfigPtr getNormalAppConfig(const af3d::AppConfigPtr& appConfig1)
{
    auto appConfig = std::make_shared<af3d::SequentialAppConfig>();

    appConfig->add(appConfig1);

    std::ifstream is("config.ini");

    if (is) {
        auto appConfig2 = std::make_shared<af3d::StreamAppConfig>();

        if (!appConfig2->load(is)) {
            std::cerr << "Cannot read config.ini" << std::endl;
            return af3d::AppConfigPtr();
        }

        is.close();

        appConfig->add(appConfig2);
    }

    return appConfig;
}

int main(int argc, char *argv[])
{
    srand(static_cast<unsigned int>(time(nullptr)));

    if (!platformLinux->init("./assets")) {
        std::cerr << "Cannot init linux platform" << std::endl;
        return 1;
    }

    std::istringstream is(configIniStr);

    auto appConfig1 = std::make_shared<af3d::StreamAppConfig>();

    if (!appConfig1->load(is)) {
        std::cerr << "Cannot read built-in config.ini" << std::endl;
        return 1;
    }

    af3d::AppConfigPtr appConfig = getNormalAppConfig(appConfig1);

    if (!appConfig) {
        return 1;
    }

    std::istringstream iss(appConfig->getLoggerConfig());

    log4cplus::spi::AppenderFactoryRegistry& reg
        = log4cplus::spi::getAppenderFactoryRegistry();
    LOG4CPLUS_REG_APPENDER(reg, GameLogAppender);

    log4cplus::PropertyConfigurator loggerConfigurator(iss);
    loggerConfigurator.configure();

    auto logLevel = log4cplus::Logger::getRoot().getLogLevel();
    if (logLevel <= log4cplus::DEBUG_LOG_LEVEL) {
        Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
    } else {
        Assimp::DefaultLogger::create("", Assimp::Logger::NORMAL);
    }

    Assimp::DefaultLogger::get()->attachStream(new af3d::AssimpLogStream(log4cplus::DEBUG_LOG_LEVEL), Assimp::Logger::Debugging);
    Assimp::DefaultLogger::get()->attachStream(new af3d::AssimpLogStream(log4cplus::INFO_LOG_LEVEL), Assimp::Logger::Info);
    Assimp::DefaultLogger::get()->attachStream(new af3d::AssimpLogStream(log4cplus::WARN_LOG_LEVEL), Assimp::Logger::Warn);
    Assimp::DefaultLogger::get()->attachStream(new af3d::AssimpLogStream(log4cplus::ERROR_LOG_LEVEL), Assimp::Logger::Err);

    af3d::settings.init(appConfig);

    LOG4CPLUS_INFO(af3d::logger(), "Starting...");

    af3d::gameShell.reset(new af3d::DummyShell());

    gamepadInit();

    InitKIMap();

    XInitThreads();

    dpy = ::XOpenDisplay(nullptr);

    if (!dpy) {
        LOG4CPLUS_ERROR(af3d::logger(), "Cannot open display");
        startupFailed();
        return 1;
    }

    if (!OGLPreInit()) {
        startupFailed();
        return 1;
    }

    PopulateVideoModes();

    if (!OALInit()) {
        startupFailed();
        return 1;
    }

    bool res = game.init(argc > 1 ? argv[1] : "startup.lua");

    if (!res) {
        startupFailed();
        return 1;
    }

    bool running = true;
    XEvent event;

    KeySym keysym;
    KeySym lowersym, uppersym;
    while (running) {
        while (::XPending(dpy) > 0) {
            ::XNextEvent(dpy, &event);
            switch (event.type) {
            case ClientMessage:
                if (event.xclient.data.l[0] == static_cast<long>(deleteMessage)) {
                    running = false;
                }
                break;
            case KeyPress:
                XLookupString(&event.xkey, nullptr, 0, &keysym, nullptr);
                XConvertCase(keysym, &lowersym, &uppersym);
                game.keyPress(kiMap[lowersym & 0xFF]);
                break;
            case KeyRelease:
                XLookupString(&event.xkey, nullptr, 0, &keysym, nullptr);
                XConvertCase(keysym, &lowersym, &uppersym);
                game.keyRelease(kiMap[lowersym & 0xFF]);
                break;
            case ButtonPress:
                if ((event.xbutton.button == Button1) || (event.xbutton.button == Button3)) {
                    game.mouseDown(event.xbutton.button == Button1);
                } else if (event.xbutton.button == Button4) {
                    game.mouseWheel(-1);
                } else if (event.xbutton.button == Button5) {
                    game.mouseWheel(1);
                }
                break;
            case ButtonRelease:
                if ((event.xbutton.button == Button1) || (event.xbutton.button == Button3)) {
                    game.mouseUp(event.xbutton.button == Button1);
                }
                break;
            case MotionNotify:
                game.mouseMove(af3d::Vector2f(event.xmotion.x, event.xmotion.y));
                break;
            default:
                break;
            }
        }

        gamepadUpdate();

        af3d::gameShell->update();

        running &= game.update();
    }

    game.cancelRender();

    thr.join();

    makeCurrent(dpy, window, context);

    game.shutdown();

    OGLShutdown();

    destroyWindow();

    XCloseDisplay(dpy);

    gamepadShutdown();

    Assimp::DefaultLogger::kill();

    platformLinux->shutdown();

    return 0;
}
