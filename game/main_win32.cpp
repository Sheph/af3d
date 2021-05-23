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
#include "PlatformWin32.h"
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
#include <log4cplus/consoleappender.h>
#include <log4cplus/hierarchy.h>
#include <iostream>
#include <fstream>
#include <GL/wglext.h>
#include <fcntl.h>
#include <io.h>
#include <process.h>
#include <conio.h>
#include <xinput.h>

#define WGL_GET_PROC(func, sym) \
    do { \
        *(void**)(&func) = GetProcAddress(gHandle, #sym); \
        if (!func) { \
            LOG4CPLUS_ERROR(af3d::logger(), "Unable to load WGL symbol " #sym); \
            return false; \
        } \
    } while (0)

#define WGL_GET_EXT_PROC(extName, func, sym) \
    do { \
        if ((strstr(extStr, #extName " ") == nullptr)) { \
            LOG4CPLUS_ERROR(af3d::logger(), #extName " is not supported"); \
            return false; \
        } \
        *(void**)(&func) = gGetProcAddress((LPCSTR)#sym); \
        if (!func) { \
            LOG4CPLUS_ERROR(af3d::logger(), "Unable to load WGL symbol " #sym); \
            return false; \
        } \
    } while (0)

#define WGL_GET_EXT_PROC_OPT(extName, func, sym) \
    do { \
        if ((strstr(extStr, #extName " ") != nullptr)) { \
            *(void**)(&func) = gGetProcAddress((LPCSTR)#sym); \
            if (!func) { \
                LOG4CPLUS_WARN(af3d::logger(), "Unable to load WGL symbol " #sym); \
            } \
        } else { \
            LOG4CPLUS_WARN(af3d::logger(), #extName " is not supported"); \
        } \
    } while (0)

#define GL_GET_PROC(func, sym) \
    do { \
        *(void**)(&af3d::ogl.func) = gGetProcAddress((LPCSTR)#sym); \
        if (!af3d::ogl.func) { \
            *(void**)(&af3d::ogl.func) = GetProcAddress(gHandle, #sym); \
            if (!af3d::ogl.func) { \
                LOG4CPLUS_ERROR(af3d::logger(), "Unable to load GL symbol " #sym); \
                return false; \
            } \
        } \
    } while (0)

#define AL_GET_PROC(func, sym) \
    do { \
        *(void**)(&af3d::oal.func) = GetProcAddress(handle, #sym); \
        if (!af3d::oal.func) { \
            LOG4CPLUS_ERROR(af3d::logger(), "Unable to load OpenAL symbol " #sym); \
            return false; \
        } \
    } while (0)

typedef HGLRC(WINAPI *PFNWGLCREATECONTEXTPROC)(HDC hdl);
typedef BOOL(WINAPI *PFNWGLDELETECONTEXTPROC)(HGLRC hdl);
typedef PROC(WINAPI *PFNWGLGETPROCADDRESSPROC)(LPCSTR sym);
typedef BOOL(WINAPI *PFNWGLMAKECURRENTPROC)(HDC dev_ctx, HGLRC rend_ctx);

static HINSTANCE gHandle;
static PFNWGLCREATECONTEXTPROC gCreateContext;
static PFNWGLDELETECONTEXTPROC gDeleteContext;
static PFNWGLGETPROCADDRESSPROC gGetProcAddress;
static PFNWGLMAKECURRENTPROC gMakeCurrent;
/* WGL extensions */
static PFNWGLGETEXTENSIONSSTRINGEXTPROC gGetExtensionsStringEXT;
static PFNWGLGETEXTENSIONSSTRINGARBPROC gGetExtensionsStringARB;
static PFNWGLCHOOSEPIXELFORMATARBPROC gChoosePixelFormatARB;
static PFNWGLGETPIXELFORMATATTRIBIVARBPROC gGetPixelFormatAttribivARB;
/* WGL_ARB_create_context */
static PFNWGLCREATECONTEXTATTRIBSARBPROC gCreateContextAttribsARB = nullptr;
/* WGL_EXT_swap_control */
static PFNWGLSWAPINTERVALEXTPROC gSwapIntervalEXT = nullptr;

static DEVMODEA desktopMode;

static HWND gWnd = nullptr;
static HDC gDC;
static HGLRC gCtx;
static std::unique_ptr<af3d::HardwareContext> gHwCtx;
static HCURSOR gCursor;

static af3d::Game game;

static bool gRunning = true;

static bool gTrueFullscreen = false;

struct InputEvent
{
    InputEvent(af3d::KeyIdentifier ki, bool up, std::uint32_t modifiersState)
    : type(up ? 1 : 0),
      ki(ki),
      misc(modifiersState)
    {
    }

    InputEvent(bool left, const af3d::Vector2f& point)
    : type(2),
      left(left),
      point(point)
    {
    }

    explicit InputEvent(bool left)
    : type(3),
      left(left)
    {
    }

    explicit InputEvent(const af3d::Vector2f& point)
    : type(4),
      point(point)
    {
    }

    InputEvent(int delta, int dummy1, int dummy2)
    : type(5),
      misc(delta)
    {
    }

    InputEvent(std::uint16_t ch)
    : type(6),
      misc(ch)
    {
    }

    int type;
    af3d::KeyIdentifier ki;
    bool left;
    af3d::Vector2f point;
    std::uint32_t misc;
};

static std::mutex gInputMtx;
static std::vector<InputEvent> gInputEvents;

static const int gCtxAttribs[] =
{
    WGL_CONTEXT_MAJOR_VERSION_ARB, 4,
    WGL_CONTEXT_MINOR_VERSION_ARB, 3,
    WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
    0
};

static int gMsaaConfigAttribs[] = {
    WGL_SUPPORT_OPENGL_ARB, TRUE,
    WGL_DOUBLE_BUFFER_ARB, TRUE,
    WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
    WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
    WGL_RED_BITS_ARB, 8,
    WGL_GREEN_BITS_ARB, 8,
    WGL_BLUE_BITS_ARB, 8,
    WGL_ALPHA_BITS_ARB, 8,
    WGL_COLOR_BITS_ARB, 32,
    WGL_DEPTH_BITS_ARB, 24,
    WGL_SAMPLES_ARB, 1,
    WGL_SAMPLE_BUFFERS_ARB, 1,
    0,
};

static const int gConfigAttribs[] = {
    WGL_SUPPORT_OPENGL_ARB, TRUE,
    WGL_DOUBLE_BUFFER_ARB, TRUE,
    WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
    WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
    WGL_RED_BITS_ARB, 8,
    WGL_GREEN_BITS_ARB, 8,
    WGL_BLUE_BITS_ARB, 8,
    WGL_ALPHA_BITS_ARB, 8,
    WGL_COLOR_BITS_ARB, 32,
    WGL_DEPTH_BITS_ARB, 24,
    0,
};

static bool OGLPreInit()
{
    PIXELFORMATDESCRIPTOR pixfmt;
    WNDCLASSEXA wcex;
    HWND hWnd = nullptr;
    HDC hDC = nullptr;
    HGLRC ctx = nullptr;
    int configId = 0;
    const char *extStr = nullptr;

    LOG4CPLUS_INFO(af3d::logger(), "Initializing OpenGL...");

    memset(&pixfmt, 0, sizeof(pixfmt));

    pixfmt.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pixfmt.nVersion = 1;
    pixfmt.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pixfmt.iPixelType = PFD_TYPE_RGBA;
    pixfmt.cColorBits = 32;
    pixfmt.cDepthBits = 24;
    pixfmt.iLayerType = PFD_MAIN_PLANE;

    memset(&wcex, 0, sizeof(wcex));

    wcex.cbSize = sizeof(wcex);
    wcex.lpfnWndProc = &DefWindowProcA;
    wcex.hInstance = GetModuleHandle(nullptr);
    wcex.lpszClassName = "AirForce3DDummyWinClass";

    if (!RegisterClassExA(&wcex)) {
        LOG4CPLUS_ERROR(af3d::logger(), "Unable to register dummy win class");
        return false;
    }

    gHandle = LoadLibraryA("opengl32");

    if (!gHandle) {
        LOG4CPLUS_ERROR(af3d::logger(), "Unable to load opengl32.dll");
        return false;
    }

    WGL_GET_PROC(gCreateContext, wglCreateContext);
    WGL_GET_PROC(gDeleteContext, wglDeleteContext);
    WGL_GET_PROC(gGetProcAddress, wglGetProcAddress);
    WGL_GET_PROC(gMakeCurrent, wglMakeCurrent);

    hWnd = CreateWindowA("AirForce3DDummyWinClass", "AirForce3DDummyWin",
        WS_DISABLED | WS_POPUP,
        0, 0, 1, 1, nullptr, nullptr, 0, 0);

    if (!hWnd) {
        LOG4CPLUS_ERROR(af3d::logger(), "Unable to create dummy win");
        return false;
    }

    hDC = GetDC(hWnd);

    if (!hDC) {
        LOG4CPLUS_ERROR(af3d::logger(), "Unable to get dummy win DC");
        return false;
    }

    configId = ChoosePixelFormat(hDC, &pixfmt);

    if (!configId) {
        LOG4CPLUS_ERROR(af3d::logger(), "ChoosePixelFormat failed");
        return false;
    }

    if (!SetPixelFormat(hDC, configId, &pixfmt)) {
        LOG4CPLUS_ERROR(af3d::logger(), "SetPixelFormat failed");
        return false;
    }

    ctx = gCreateContext(hDC);
    if (!ctx) {
        LOG4CPLUS_ERROR(af3d::logger(), "wglCreateContext failed");
        return false;
    }

    if (!gMakeCurrent(hDC, ctx)) {
        LOG4CPLUS_ERROR(af3d::logger(), "wglMakeCurrent failed");
        return false;
    }

    /*
     * WGL extensions couldn't be queried by glGetString(), we need to use
     * wglGetExtensionsStringARB or wglGetExtensionsStringEXT for this, which
     * themselves are extensions
     */
    gGetExtensionsStringARB = (PFNWGLGETEXTENSIONSSTRINGARBPROC)gGetProcAddress((LPCSTR)"wglGetExtensionsStringARB");
    gGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)gGetProcAddress((LPCSTR)"wglGetExtensionsStringEXT");

    if (gGetExtensionsStringARB) {
        extStr = gGetExtensionsStringARB(hDC);
    } else if (gGetExtensionsStringEXT) {
        extStr = gGetExtensionsStringEXT();
    }

    if (!extStr) {
        LOG4CPLUS_ERROR(af3d::logger(), "Unable to obtain WGL extension string");
        return false;
    }

    WGL_GET_EXT_PROC(WGL_ARB_pixel_format, gChoosePixelFormatARB, wglChoosePixelFormatARB);
    WGL_GET_EXT_PROC(WGL_ARB_pixel_format, gGetPixelFormatAttribivARB, wglGetPixelFormatAttribivARB);
    WGL_GET_EXT_PROC_OPT(WGL_ARB_create_context, gCreateContextAttribsARB, wglCreateContextAttribsARB);
    WGL_GET_EXT_PROC_OPT(WGL_EXT_swap_control, gSwapIntervalEXT, wglSwapIntervalEXT);

    GL_GET_PROC(DrawBuffers, glDrawBuffers);
    GL_GET_PROC(GetTexImage, glGetTexImage);
    GL_GET_PROC(GenRenderbuffers, glGenRenderbuffers);
    GL_GET_PROC(DeleteRenderbuffers, glDeleteRenderbuffers);
    GL_GET_PROC(BindRenderbuffer, glBindRenderbuffer);
    GL_GET_PROC(RenderbufferStorage, glRenderbufferStorage);
    GL_GET_PROC(FramebufferRenderbuffer, glFramebufferRenderbuffer);
    GL_GET_PROC(PixelStorei, glPixelStorei);
    GL_GET_PROC(GenSamplers, glGenSamplers);
    GL_GET_PROC(DeleteSamplers, glDeleteSamplers);
    GL_GET_PROC(BindSampler, glBindSampler);
    GL_GET_PROC(SamplerParameterf, glSamplerParameterf);
    GL_GET_PROC(SamplerParameteri, glSamplerParameteri);
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
    GL_GET_PROC(TexImage3D, glTexImage3D);
    GL_GET_PROC(TexSubImage3D, glTexSubImage3D);
    GL_GET_PROC(CompressedTexImage2D, glCompressedTexImage2D);
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
    GL_GET_PROC(Uniform1iv, glUniform1iv);
    GL_GET_PROC(Uniform2fv, glUniform2fv);
    GL_GET_PROC(Uniform3fv, glUniform3fv);
    GL_GET_PROC(Uniform4fv, glUniform4fv);
    GL_GET_PROC(Enable, glEnable);
    GL_GET_PROC(Disable, glDisable);
    GL_GET_PROC(BlendFunc, glBlendFunc);
    GL_GET_PROC(BlendFuncSeparate, glBlendFuncSeparate);
    GL_GET_PROC(UniformMatrix3fv, glUniformMatrix3fv);
    GL_GET_PROC(UniformMatrix4fv, glUniformMatrix4fv);
    GL_GET_PROC(Uniform1i, glUniform1i);
    GL_GET_PROC(Uniform2i, glUniform2i);
    GL_GET_PROC(GetIntegerv, glGetIntegerv);
    GL_GET_PROC(GenerateMipmap, glGenerateMipmapEXT);
    GL_GET_PROC(PointSize, glPointSize);
    GL_GET_PROC(LineWidth, glLineWidth);
    GL_GET_PROC(GenFramebuffers, glGenFramebuffers);
    GL_GET_PROC(DeleteFramebuffers, glDeleteFramebuffers);
    GL_GET_PROC(BindFramebuffer, glBindFramebuffer);
    GL_GET_PROC(FramebufferTexture2D, glFramebufferTexture2D);
    GL_GET_PROC(FramebufferTextureLayer, glFramebufferTextureLayer);
    GL_GET_PROC(CheckFramebufferStatus, glCheckFramebufferStatus);
    GL_GET_PROC(Uniform1f, glUniform1f);
    GL_GET_PROC(Uniform2f, glUniform2f);
    GL_GET_PROC(ColorMask, glColorMask);
    GL_GET_PROC(StencilFunc, glStencilFunc);
    GL_GET_PROC(StencilOp, glStencilOp);
    GL_GET_PROC(GetString, glGetString);
    GL_GET_PROC(GetStringi, glGetStringi);
    GL_GET_PROC(Scissor, glScissor);
    GL_GET_PROC(GetProgramInterfaceiv, glGetProgramInterfaceiv);
    GL_GET_PROC(GetProgramResourceiv, glGetProgramResourceiv);
    GL_GET_PROC(GetProgramResourceName, glGetProgramResourceName);
    GL_GET_PROC(BindBufferBase, glBindBufferBase);
    GL_GET_PROC(DispatchCompute, glDispatchCompute);
    GL_GET_PROC(MemoryBarrier, glMemoryBarrier);

    const int numPixelFormatsQuery = WGL_NUMBER_PIXEL_FORMATS_ARB;
    int numFormats = 0;

    if (!gGetPixelFormatAttribivARB(hDC, 0, 0, 1, &numPixelFormatsQuery, &numFormats) || !numFormats) {
        LOG4CPLUS_ERROR(af3d::logger(), "wglGetPixelFormatAttribivARB failed to query number of formats");
        return false;
    }

    std::vector<int> configIds(numFormats, 0);

    UINT n = 0;

    if (!gChoosePixelFormatARB(hDC,
        gConfigAttribs,
        nullptr,
        numFormats,
        &configIds[0],
        &n) || (n == 0)) {
        LOG4CPLUS_ERROR(af3d::logger(), "wglChoosePixelFormatARB failed to enumerate formats");
        return false;
    }

    const int queryList[6] = {
        WGL_RED_BITS_ARB,
        WGL_GREEN_BITS_ARB,
        WGL_BLUE_BITS_ARB,
        WGL_ALPHA_BITS_ARB,
        WGL_DEPTH_BITS_ARB,
        WGL_SAMPLES_ARB,
    };

    int attrVals[6];

    std::set<std::uint32_t> ss;

    ss.insert(0);

    PIXELFORMATDESCRIPTOR pfd;

    for (UINT i = 0; i < n; ++i) {
        if (!DescribePixelFormat(hDC, configIds[i], sizeof(pfd), &pfd)) {
            continue;
        }

        if (!gGetPixelFormatAttribivARB(hDC, configIds[i], 0, 6, queryList, attrVals)) {
            continue;
        }

        if ((attrVals[0] == 8) && (attrVals[1] == 8) && (attrVals[2] == 8) &&
            (attrVals[3] == 8) && (attrVals[4] == 24) && (attrVals[5] >= 0)) {
            ss.insert(attrVals[5]);
        }
    }

    gMakeCurrent(nullptr, nullptr);
    gDeleteContext(ctx);
    ReleaseDC(hWnd, hDC);
    DestroyWindow(hWnd);

    std::vector<std::uint32_t> msaaModes;

    std::copy(ss.begin(), ss.end(), std::back_inserter(msaaModes));

    af3d::platform->setMsaaModes(msaaModes);

    if (gSwapIntervalEXT) {
        af3d::platform->setVSyncSupported(true);
    }

    return true;
}

static void PopulateVideoModes()
{
    std::uint32_t maxWinWidth = af3d::settings.viewWidth;
    std::uint32_t maxWinHeight = af3d::settings.viewHeight;

    ZeroMemory(&desktopMode, sizeof(DEVMODEA));
    desktopMode.dmSize = sizeof(DEVMODEA);

    if (EnumDisplaySettingsA(nullptr, ENUM_CURRENT_SETTINGS, &desktopMode)) {
        int modeIndex = 0;

        std::set<af3d::VideoMode> tmp;

        for (;;) {
            DEVMODEA dm;

            ZeroMemory(&dm, sizeof(DEVMODEA));
            dm.dmSize = sizeof(DEVMODEA);

            if (!EnumDisplaySettingsA(nullptr, modeIndex, &dm)) {
                break;
            }

            modeIndex++;

            if (dm.dmBitsPerPel != 32) {
                continue;
            }

            if ((dm.dmPelsWidth <= desktopMode.dmPelsWidth) && (dm.dmPelsHeight <= desktopMode.dmPelsHeight) && (dm.dmDisplayFrequency == desktopMode.dmDisplayFrequency)) {
                tmp.insert(af3d::VideoMode(dm.dmPelsWidth, dm.dmPelsHeight));
            }
        }

        tmp.insert(af3d::VideoMode(desktopMode.dmPelsWidth, desktopMode.dmPelsHeight));

        if (desktopMode.dmPelsWidth > maxWinWidth) {
            maxWinWidth = desktopMode.dmPelsWidth;
        }

        if (desktopMode.dmPelsHeight > maxWinHeight) {
            maxWinHeight = desktopMode.dmPelsHeight;
        }

        int i = 0;

        for (std::set<af3d::VideoMode>::const_iterator it = tmp.begin(); it != tmp.end(); ++it) {
            if ((it->width == desktopMode.dmPelsWidth) && (it->height == desktopMode.dmPelsHeight)) {
                af3d::platform->setDesktopVideoMode(i);
                break;
            }
            ++i;
        }

        std::vector<af3d::VideoMode> videoModes;

        std::copy(tmp.begin(), tmp.end(), std::back_inserter(videoModes));

        af3d::platform->setDesktopVideoModes(videoModes);
    } else {
        maxWinWidth *= 2;
        maxWinHeight *= 2;
    }

    std::set<af3d::VideoMode> tmp = af3d::settings.winVideoModes;

    tmp.insert(af3d::VideoMode(af3d::settings.viewWidth, af3d::settings.viewHeight));

    int i = 0;

    std::vector<af3d::VideoMode> videoModes;

    for (auto it = tmp.begin(); it != tmp.end(); ++it) {
        if ((it->width > maxWinWidth) || (it->height > maxWinHeight)) {
            continue;
        }

        if ((it->width == af3d::settings.viewWidth) && (it->height == af3d::settings.viewHeight)) {
            af3d::platform->setDefaultVideoMode(i);
        }

        videoModes.push_back(*it);

        ++i;
    }

    af3d::platform->setWinVideoModes(videoModes);
}

static af3d::KeyIdentifier kiMap[256];

static void InitKIMap()
{
    memset(kiMap, 0, sizeof(kiMap));

    kiMap['A'] = Rocket::Core::Input::KI_A;
    kiMap['B'] = Rocket::Core::Input::KI_B;
    kiMap['C'] = Rocket::Core::Input::KI_C;
    kiMap['D'] = Rocket::Core::Input::KI_D;
    kiMap['E'] = Rocket::Core::Input::KI_E;
    kiMap['F'] = Rocket::Core::Input::KI_F;
    kiMap['G'] = Rocket::Core::Input::KI_G;
    kiMap['H'] = Rocket::Core::Input::KI_H;
    kiMap['I'] = Rocket::Core::Input::KI_I;
    kiMap['J'] = Rocket::Core::Input::KI_J;
    kiMap['K'] = Rocket::Core::Input::KI_K;
    kiMap['L'] = Rocket::Core::Input::KI_L;
    kiMap['M'] = Rocket::Core::Input::KI_M;
    kiMap['N'] = Rocket::Core::Input::KI_N;
    kiMap['O'] = Rocket::Core::Input::KI_O;
    kiMap['P'] = Rocket::Core::Input::KI_P;
    kiMap['Q'] = Rocket::Core::Input::KI_Q;
    kiMap['R'] = Rocket::Core::Input::KI_R;
    kiMap['S'] = Rocket::Core::Input::KI_S;
    kiMap['T'] = Rocket::Core::Input::KI_T;
    kiMap['U'] = Rocket::Core::Input::KI_U;
    kiMap['V'] = Rocket::Core::Input::KI_V;
    kiMap['W'] = Rocket::Core::Input::KI_W;
    kiMap['X'] = Rocket::Core::Input::KI_X;
    kiMap['Y'] = Rocket::Core::Input::KI_Y;
    kiMap['Z'] = Rocket::Core::Input::KI_Z;
    kiMap['0'] = Rocket::Core::Input::KI_0;
    kiMap['1'] = Rocket::Core::Input::KI_1;
    kiMap['2'] = Rocket::Core::Input::KI_2;
    kiMap['3'] = Rocket::Core::Input::KI_3;
    kiMap['4'] = Rocket::Core::Input::KI_4;
    kiMap['5'] = Rocket::Core::Input::KI_5;
    kiMap['6'] = Rocket::Core::Input::KI_6;
    kiMap['7'] = Rocket::Core::Input::KI_7;
    kiMap['8'] = Rocket::Core::Input::KI_8;
    kiMap['9'] = Rocket::Core::Input::KI_9;
    kiMap[VK_BACK] = Rocket::Core::Input::KI_BACK;
    kiMap[VK_TAB] = Rocket::Core::Input::KI_TAB;
    kiMap[VK_CLEAR] = Rocket::Core::Input::KI_CLEAR;
    kiMap[VK_RETURN] = Rocket::Core::Input::KI_RETURN;
    kiMap[VK_PAUSE] = Rocket::Core::Input::KI_PAUSE;
    kiMap[VK_CAPITAL] = Rocket::Core::Input::KI_CAPITAL;
    kiMap[VK_KANA] = Rocket::Core::Input::KI_KANA;
    kiMap[VK_HANGUL] = Rocket::Core::Input::KI_HANGUL;
    kiMap[VK_JUNJA] = Rocket::Core::Input::KI_JUNJA;
    kiMap[VK_FINAL] = Rocket::Core::Input::KI_FINAL;
    kiMap[VK_HANJA] = Rocket::Core::Input::KI_HANJA;
    kiMap[VK_KANJI] = Rocket::Core::Input::KI_KANJI;
    kiMap[VK_ESCAPE] = Rocket::Core::Input::KI_ESCAPE;
    kiMap[VK_CONVERT] = Rocket::Core::Input::KI_CONVERT;
    kiMap[VK_NONCONVERT] = Rocket::Core::Input::KI_NONCONVERT;
    kiMap[VK_ACCEPT] = Rocket::Core::Input::KI_ACCEPT;
    kiMap[VK_MODECHANGE] = Rocket::Core::Input::KI_MODECHANGE;
    kiMap[VK_SPACE] = Rocket::Core::Input::KI_SPACE;
    kiMap[VK_PRIOR] = Rocket::Core::Input::KI_PRIOR;
    kiMap[VK_NEXT] = Rocket::Core::Input::KI_NEXT;
    kiMap[VK_END] = Rocket::Core::Input::KI_END;
    kiMap[VK_HOME] = Rocket::Core::Input::KI_HOME;
    kiMap[VK_LEFT] = Rocket::Core::Input::KI_LEFT;
    kiMap[VK_UP] = Rocket::Core::Input::KI_UP;
    kiMap[VK_RIGHT] = Rocket::Core::Input::KI_RIGHT;
    kiMap[VK_DOWN] = Rocket::Core::Input::KI_DOWN;
    kiMap[VK_SELECT] = Rocket::Core::Input::KI_SELECT;
    kiMap[VK_PRINT] = Rocket::Core::Input::KI_PRINT;
    kiMap[VK_EXECUTE] = Rocket::Core::Input::KI_EXECUTE;
    kiMap[VK_SNAPSHOT] = Rocket::Core::Input::KI_SNAPSHOT;
    kiMap[VK_INSERT] = Rocket::Core::Input::KI_INSERT;
    kiMap[VK_DELETE] = Rocket::Core::Input::KI_DELETE;
    kiMap[VK_HELP] = Rocket::Core::Input::KI_HELP;
    kiMap[VK_LWIN] = Rocket::Core::Input::KI_LWIN;
    kiMap[VK_RWIN] = Rocket::Core::Input::KI_RWIN;
    kiMap[VK_APPS] = Rocket::Core::Input::KI_APPS;
    kiMap[VK_SLEEP] = Rocket::Core::Input::KI_SLEEP;
    kiMap[VK_NUMPAD0] = Rocket::Core::Input::KI_NUMPAD0;
    kiMap[VK_NUMPAD1] = Rocket::Core::Input::KI_NUMPAD1;
    kiMap[VK_NUMPAD2] = Rocket::Core::Input::KI_NUMPAD2;
    kiMap[VK_NUMPAD3] = Rocket::Core::Input::KI_NUMPAD3;
    kiMap[VK_NUMPAD4] = Rocket::Core::Input::KI_NUMPAD4;
    kiMap[VK_NUMPAD5] = Rocket::Core::Input::KI_NUMPAD5;
    kiMap[VK_NUMPAD6] = Rocket::Core::Input::KI_NUMPAD6;
    kiMap[VK_NUMPAD7] = Rocket::Core::Input::KI_NUMPAD7;
    kiMap[VK_NUMPAD8] = Rocket::Core::Input::KI_NUMPAD8;
    kiMap[VK_NUMPAD9] = Rocket::Core::Input::KI_NUMPAD9;
    kiMap[VK_MULTIPLY] = Rocket::Core::Input::KI_MULTIPLY;
    kiMap[VK_ADD] = Rocket::Core::Input::KI_ADD;
    kiMap[VK_SEPARATOR] = Rocket::Core::Input::KI_SEPARATOR;
    kiMap[VK_SUBTRACT] = Rocket::Core::Input::KI_SUBTRACT;
    kiMap[VK_DECIMAL] = Rocket::Core::Input::KI_DECIMAL;
    kiMap[VK_DIVIDE] = Rocket::Core::Input::KI_DIVIDE;
    kiMap[VK_F1] = Rocket::Core::Input::KI_F1;
    kiMap[VK_F2] = Rocket::Core::Input::KI_F2;
    kiMap[VK_F3] = Rocket::Core::Input::KI_F3;
    kiMap[VK_F4] = Rocket::Core::Input::KI_F4;
    kiMap[VK_F5] = Rocket::Core::Input::KI_F5;
    kiMap[VK_F6] = Rocket::Core::Input::KI_F6;
    kiMap[VK_F7] = Rocket::Core::Input::KI_F7;
    kiMap[VK_F8] = Rocket::Core::Input::KI_F8;
    kiMap[VK_F9] = Rocket::Core::Input::KI_F9;
    kiMap[VK_F10] = Rocket::Core::Input::KI_F10;
    kiMap[VK_F11] = Rocket::Core::Input::KI_F11;
    kiMap[VK_F12] = Rocket::Core::Input::KI_F12;
    kiMap[VK_F13] = Rocket::Core::Input::KI_F13;
    kiMap[VK_F14] = Rocket::Core::Input::KI_F14;
    kiMap[VK_F15] = Rocket::Core::Input::KI_F15;
    kiMap[VK_F16] = Rocket::Core::Input::KI_F16;
    kiMap[VK_F17] = Rocket::Core::Input::KI_F17;
    kiMap[VK_F18] = Rocket::Core::Input::KI_F18;
    kiMap[VK_F19] = Rocket::Core::Input::KI_F19;
    kiMap[VK_F20] = Rocket::Core::Input::KI_F20;
    kiMap[VK_F21] = Rocket::Core::Input::KI_F21;
    kiMap[VK_F22] = Rocket::Core::Input::KI_F22;
    kiMap[VK_F23] = Rocket::Core::Input::KI_F23;
    kiMap[VK_F24] = Rocket::Core::Input::KI_F24;
    kiMap[VK_NUMLOCK] = Rocket::Core::Input::KI_NUMLOCK;
    kiMap[VK_SCROLL] = Rocket::Core::Input::KI_SCROLL;
    kiMap[VK_OEM_NEC_EQUAL] = Rocket::Core::Input::KI_OEM_NEC_EQUAL;
    kiMap[VK_OEM_FJ_JISHO] = Rocket::Core::Input::KI_OEM_FJ_JISHO;
    kiMap[VK_OEM_FJ_MASSHOU] = Rocket::Core::Input::KI_OEM_FJ_MASSHOU;
    kiMap[VK_OEM_FJ_TOUROKU] = Rocket::Core::Input::KI_OEM_FJ_TOUROKU;
    kiMap[VK_OEM_FJ_LOYA] = Rocket::Core::Input::KI_OEM_FJ_LOYA;
    kiMap[VK_OEM_FJ_ROYA] = Rocket::Core::Input::KI_OEM_FJ_ROYA;
    kiMap[VK_SHIFT] = Rocket::Core::Input::KI_LSHIFT;
    kiMap[VK_CONTROL] = Rocket::Core::Input::KI_LCONTROL;
    kiMap[VK_MENU] = Rocket::Core::Input::KI_LMENU;
    kiMap[VK_BROWSER_BACK] = Rocket::Core::Input::KI_BROWSER_BACK;
    kiMap[VK_BROWSER_FORWARD] = Rocket::Core::Input::KI_BROWSER_FORWARD;
    kiMap[VK_BROWSER_REFRESH] = Rocket::Core::Input::KI_BROWSER_REFRESH;
    kiMap[VK_BROWSER_STOP] = Rocket::Core::Input::KI_BROWSER_STOP;
    kiMap[VK_BROWSER_SEARCH] = Rocket::Core::Input::KI_BROWSER_SEARCH;
    kiMap[VK_BROWSER_FAVORITES] = Rocket::Core::Input::KI_BROWSER_FAVORITES;
    kiMap[VK_BROWSER_HOME] = Rocket::Core::Input::KI_BROWSER_HOME;
    kiMap[VK_VOLUME_MUTE] = Rocket::Core::Input::KI_VOLUME_MUTE;
    kiMap[VK_VOLUME_DOWN] = Rocket::Core::Input::KI_VOLUME_DOWN;
    kiMap[VK_VOLUME_UP] = Rocket::Core::Input::KI_VOLUME_UP;
    kiMap[VK_MEDIA_NEXT_TRACK] = Rocket::Core::Input::KI_MEDIA_NEXT_TRACK;
    kiMap[VK_MEDIA_PREV_TRACK] = Rocket::Core::Input::KI_MEDIA_PREV_TRACK;
    kiMap[VK_MEDIA_STOP] = Rocket::Core::Input::KI_MEDIA_STOP;
    kiMap[VK_MEDIA_PLAY_PAUSE] = Rocket::Core::Input::KI_MEDIA_PLAY_PAUSE;
    kiMap[VK_LAUNCH_MAIL] = Rocket::Core::Input::KI_LAUNCH_MAIL;
    kiMap[VK_LAUNCH_MEDIA_SELECT] = Rocket::Core::Input::KI_LAUNCH_MEDIA_SELECT;
    kiMap[VK_LAUNCH_APP1] = Rocket::Core::Input::KI_LAUNCH_APP1;
    kiMap[VK_LAUNCH_APP2] = Rocket::Core::Input::KI_LAUNCH_APP2;
    kiMap[VK_OEM_1] = Rocket::Core::Input::KI_OEM_1;
    kiMap[VK_OEM_PLUS] = Rocket::Core::Input::KI_OEM_PLUS;
    kiMap[VK_OEM_COMMA] = Rocket::Core::Input::KI_OEM_COMMA;
    kiMap[VK_OEM_MINUS] = Rocket::Core::Input::KI_OEM_MINUS;
    kiMap[VK_OEM_PERIOD] = Rocket::Core::Input::KI_OEM_PERIOD;
    kiMap[VK_OEM_2] = Rocket::Core::Input::KI_OEM_2;
    kiMap[VK_OEM_3] = Rocket::Core::Input::KI_OEM_3;
    kiMap[VK_OEM_4] = Rocket::Core::Input::KI_OEM_4;
    kiMap[VK_OEM_5] = Rocket::Core::Input::KI_OEM_5;
    kiMap[VK_OEM_6] = Rocket::Core::Input::KI_OEM_6;
    kiMap[VK_OEM_7] = Rocket::Core::Input::KI_OEM_7;
    kiMap[VK_OEM_8] = Rocket::Core::Input::KI_OEM_8;
    kiMap[VK_OEM_AX] = Rocket::Core::Input::KI_OEM_AX;
    kiMap[VK_OEM_102] = Rocket::Core::Input::KI_OEM_102;
    kiMap[VK_ICO_HELP] = Rocket::Core::Input::KI_ICO_HELP;
    kiMap[VK_ICO_00] = Rocket::Core::Input::KI_ICO_00;
    kiMap[VK_PROCESSKEY] = Rocket::Core::Input::KI_PROCESSKEY;
    kiMap[VK_ICO_CLEAR] = Rocket::Core::Input::KI_ICO_CLEAR;
    kiMap[VK_ATTN] = Rocket::Core::Input::KI_ATTN;
    kiMap[VK_CRSEL] = Rocket::Core::Input::KI_CRSEL;
    kiMap[VK_EXSEL] = Rocket::Core::Input::KI_EXSEL;
    kiMap[VK_EREOF] = Rocket::Core::Input::KI_EREOF;
    kiMap[VK_PLAY] = Rocket::Core::Input::KI_PLAY;
    kiMap[VK_ZOOM] = Rocket::Core::Input::KI_ZOOM;
    kiMap[VK_PA1] = Rocket::Core::Input::KI_PA1;
    kiMap[VK_OEM_CLEAR] = Rocket::Core::Input::KI_OEM_CLEAR;
}

static std::uint32_t getModifiersState()
{
    std::uint32_t mods = 0;

    if ((::GetKeyState(VK_SHIFT) & 0x8000) != 0) {
        mods |= Rocket::Core::Input::KM_SHIFT;
    }
    if ((::GetKeyState(VK_CONTROL) & 0x8000) != 0) {
        mods |= Rocket::Core::Input::KM_CTRL;
    }
    if ((::GetKeyState(VK_MENU) & 0x8000) != 0) {
        mods |= Rocket::Core::Input::KM_ALT;
    }
    if (((::GetKeyState(VK_RWIN) & 0x8000) != 0) || (::GetKeyState(VK_LWIN) & 0x8000) != 0) {
        mods |= Rocket::Core::Input::KM_META;
    }

    return mods;
}

static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) {
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    case WM_KEYDOWN: {
        af3d::ScopedLock lock(gInputMtx);
        gInputEvents.push_back(InputEvent(kiMap[wParam], false, getModifiersState()));
        break;
    }
    case WM_KEYUP: {
        af3d::ScopedLock lock(gInputMtx);
        gInputEvents.push_back(InputEvent(kiMap[wParam], true, getModifiersState()));
        break;
    }
    case WM_LBUTTONDOWN:
    case WM_RBUTTONDOWN: {
        SetCapture(hWnd);
        af3d::ScopedLock lock(gInputMtx);
        gInputEvents.push_back(InputEvent(message == WM_LBUTTONDOWN, af3d::Vector2f(LOWORD(lParam), HIWORD(lParam))));
        break;
    }
    case WM_LBUTTONUP:
    case WM_RBUTTONUP: {
        ReleaseCapture();
        af3d::ScopedLock lock(gInputMtx);
        gInputEvents.push_back(InputEvent(message == WM_LBUTTONUP));
        break;
    }
    case WM_MOUSEMOVE: {
        int mx = LOWORD(lParam);
        int my = HIWORD(lParam);
        if (mx & 1 << 15) mx -= (1 << 16);
        if (my & 1 << 15) my -= (1 << 16);
        af3d::ScopedLock lock(gInputMtx);
        gInputEvents.push_back(InputEvent(af3d::Vector2f(mx, my)));
        break;
    }
    case WM_MOUSEWHEEL: {
        af3d::ScopedLock lock(gInputMtx);
        gInputEvents.push_back(InputEvent(static_cast<short>(HIWORD(wParam)) / -WHEEL_DELTA, 0, 0));
        break;
    }
    case WM_SETCURSOR: {
        SetCursor(gCursor);
        return TRUE;
        break;
    }
    case WM_CHAR:
        if ((wParam > 0) && (wParam < 0x10000)) {
            af3d::ScopedLock lock(gInputMtx);
            gInputEvents.push_back(InputEvent((std::uint16_t)wParam));
        }
        break;
    default:
        return DefWindowProcA(hWnd, message, wParam, lParam);
    }

    return 0;
}

static bool createWindow(std::uint32_t width, std::uint32_t height, bool fullscreen, std::uint32_t samples)
{
    WNDCLASSEXA wcex;
    int configId = 0;
    UINT n = 0;
    PIXELFORMATDESCRIPTOR pixfmt;

    memset(&wcex, 0, sizeof(wcex));

    wcex.cbSize = sizeof(wcex);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = wndProc;
    wcex.hInstance = GetModuleHandle(nullptr);
    wcex.hIcon = LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(107));
    wcex.hCursor = nullptr;
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszClassName = "AirForce3DWinClass";

    if (gWnd == nullptr) {
        gCursor = LoadCursor(nullptr, IDC_ARROW);

        if (!RegisterClassExA(&wcex)) {
            LOG4CPLUS_ERROR(af3d::logger(), "Unable to register win class");
            return false;
        }
    }

    RECT rect;

    rect.left = 0;
    rect.top = 0;
    rect.right = width;
    rect.bottom = height;

    DWORD dwExStyle;
    DWORD dwStyle;

    if (fullscreen) {
        dwExStyle = WS_EX_APPWINDOW;
        dwStyle = WS_POPUP;

        AdjustWindowRectEx(&rect,
            dwStyle,
            FALSE,
            dwExStyle);
    } else {
        dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU;

        AdjustWindowRectEx(&rect,
            WS_OVERLAPPEDWINDOW,
            FALSE,
            dwExStyle);
    }

    gWnd = CreateWindowExA(dwExStyle, "AirForce3DWinClass",
        "AirForce3D", dwStyle | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        0, 0, rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, GetModuleHandle(nullptr), nullptr);

    if (!gWnd) {
        LOG4CPLUS_ERROR(af3d::logger(), "Unable to create win");
        return false;
    }

    gDC = GetDC(gWnd);

    if (!gDC) {
        LOG4CPLUS_ERROR(af3d::logger(), "Unable to get win DC");
        return false;
    }

    const int* attribs = nullptr;

    // Use own AA.
    if (false/*samples > 0*/) {
        gMsaaConfigAttribs[21] = samples;

        attribs = gMsaaConfigAttribs;
    } else {
        attribs = gConfigAttribs;
    }

    if (!gChoosePixelFormatARB(gDC,
        attribs,
        nullptr,
        1,
        &configId,
        &n) || (n == 0)) {
        LOG4CPLUS_ERROR(af3d::logger(), "wglChoosePixelFormatARB failed");
        return false;
    }

    if (!DescribePixelFormat(gDC,
        configId,
        sizeof(PIXELFORMATDESCRIPTOR),
        &pixfmt)) {
        LOG4CPLUS_ERROR(af3d::logger(), "DescribePixelFormat failed");
        return false;
    }

    if (!SetPixelFormat(gDC, configId, &pixfmt)) {
        LOG4CPLUS_ERROR(af3d::logger(), "SetPixelFormat failed");
        return false;
    }

    return true;
}

static bool OGLInit(bool vsync)
{
    if (gCreateContextAttribsARB) {
        gCtx = gCreateContextAttribsARB(gDC, nullptr, gCtxAttribs);
    } else {
        gCtx = gCreateContext(gDC);
    }

    if (!gCtx) {
        LOG4CPLUS_ERROR(af3d::logger(), "Unable to create OpenGL context");
        return false;
    }

    if (!gMakeCurrent(gDC, gCtx)) {
        LOG4CPLUS_ERROR(af3d::logger(), "Unable to make OpenGL context current");
        return false;
    }

    if (gSwapIntervalEXT) {
        gSwapIntervalEXT(vsync ? 1 : 0);
    }

    LOG4CPLUS_INFO(af3d::logger(), "OpenGL initialized");

    gHwCtx.reset(new af3d::HardwareContext);

    return true;
}

static void destroyWindow()
{
    LOG4CPLUS_INFO(af3d::logger(), "Destroying window...");

    if (!gMakeCurrent(nullptr, nullptr)) {
        LOG4CPLUS_WARN(af3d::logger(), "Unable to release current context");
    }

    gHwCtx.reset();
    gDeleteContext(gCtx);
    ReleaseDC(gWnd, gDC);
    DestroyWindow(gWnd);

    if (af3d::settings.fullscreen && gTrueFullscreen) {
        ChangeDisplaySettingsA(nullptr, 0);
        gTrueFullscreen = false;
    }

    LOG4CPLUS_INFO(af3d::logger(), "Window destroyed");
}

/*
 * Gamepad stuff.
 * @{
 */

typedef DWORD(WINAPI *PFNXINPUTGETSTATE)(DWORD, XINPUT_STATE*);

static HINSTANCE gXInputHandle;
static PFNXINPUTGETSTATE gXInputGetState;

static struct
{
    bool present;
    XINPUT_GAMEPAD prev;
} gamepad[XUSER_MAX_COUNT];

static void gamepadInit()
{
    LOG4CPLUS_INFO(af3d::logger(), "Initializing gamepads...");

    gXInputHandle = LoadLibraryA("XInput1_4.dll");  /* 1.4 Ships with Windows 8. */

    if (!gXInputHandle) {
        gXInputHandle = LoadLibraryA("XInput1_3.dll");  /* 1.3 can be installed as a redistributable component. */
    }

    if (!gXInputHandle) {
        /* "9.1.0" Ships with Vista and Win7, and is more limited than 1.3+ (e.g. XInputGetStateEx is not available.)  */
        gXInputHandle = LoadLibraryA("XInput9_1_0.dll");
    }

    if (gXInputHandle) {
        gXInputGetState = (PFNXINPUTGETSTATE)GetProcAddress(gXInputHandle, "XInputGetState");

        if (!gXInputGetState) {
            LOG4CPLUS_ERROR(af3d::logger(), "Unable to locale XInputGetState symbol, gamepad support not available");
            gXInputHandle = nullptr;
        }
    } else {
        LOG4CPLUS_ERROR(af3d::logger(), "Unable to load xinput dll, gamepad support not available");
    }

    LOG4CPLUS_INFO(af3d::logger(), "gamepads initialized");
}

static void gamepadShutdown()
{
    LOG4CPLUS_INFO(af3d::logger(), "Shutting down gamepads...");
    LOG4CPLUS_INFO(af3d::logger(), "gamepads shut down");
}

static inline void gamepadUpdateButton(WORD buttons, WORD newButtons, WORD bit, af3d::GamepadButton gb)
{
    if ((buttons & bit) != 0) {
        if ((newButtons & bit) != 0) {
            game.gamepadPress(gb);
        } else {
            game.gamepadRelease(gb);
        }
    }
}

static void gamepadUpdate()
{
    if (!gXInputGetState) {
        return;
    }

    for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i) {
        XINPUT_STATE xs;

        if (gXInputGetState(i, &xs) == 0) {
            if (!gamepad[i].present) {
                LOG4CPLUS_INFO(af3d::logger(), "gamepad " << i << ": connected");
                gamepad[i].present = true;
            }

            WORD buttons = gamepad[i].prev.wButtons ^ xs.Gamepad.wButtons;

            gamepadUpdateButton(buttons, xs.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_UP, af3d::GamepadButton::DPADUp);
            gamepadUpdateButton(buttons, xs.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_DOWN, af3d::GamepadButton::DPADDown);
            gamepadUpdateButton(buttons, xs.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_LEFT, af3d::GamepadButton::DPADLeft);
            gamepadUpdateButton(buttons, xs.Gamepad.wButtons, XINPUT_GAMEPAD_DPAD_RIGHT, af3d::GamepadButton::DPADRight);
            gamepadUpdateButton(buttons, xs.Gamepad.wButtons, XINPUT_GAMEPAD_START, af3d::GamepadButton::Start);
            gamepadUpdateButton(buttons, xs.Gamepad.wButtons, XINPUT_GAMEPAD_BACK, af3d::GamepadButton::Back);
            gamepadUpdateButton(buttons, xs.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_THUMB, af3d::GamepadButton::LeftStick);
            gamepadUpdateButton(buttons, xs.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_THUMB, af3d::GamepadButton::RightStick);
            gamepadUpdateButton(buttons, xs.Gamepad.wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER, af3d::GamepadButton::LeftBumper);
            gamepadUpdateButton(buttons, xs.Gamepad.wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER, af3d::GamepadButton::RightBumper);
            gamepadUpdateButton(buttons, xs.Gamepad.wButtons, XINPUT_GAMEPAD_A, af3d::GamepadButton::A);
            gamepadUpdateButton(buttons, xs.Gamepad.wButtons, XINPUT_GAMEPAD_B, af3d::GamepadButton::B);
            gamepadUpdateButton(buttons, xs.Gamepad.wButtons, XINPUT_GAMEPAD_X, af3d::GamepadButton::X);
            gamepadUpdateButton(buttons, xs.Gamepad.wButtons, XINPUT_GAMEPAD_Y, af3d::GamepadButton::Y);

            if (gamepad[i].prev.bLeftTrigger != xs.Gamepad.bLeftTrigger) {
                game.gamepadMoveTrigger(true, static_cast<float>(xs.Gamepad.bLeftTrigger) / 255.0f);
            }

            if (gamepad[i].prev.bRightTrigger != xs.Gamepad.bRightTrigger) {
                game.gamepadMoveTrigger(false, static_cast<float>(xs.Gamepad.bRightTrigger) / 255.0f);
            }

            if ((gamepad[i].prev.sThumbLX != xs.Gamepad.sThumbLX) || (gamepad[i].prev.sThumbLY != xs.Gamepad.sThumbLY)) {
                SHORT tmpX = xs.Gamepad.sThumbLX;
                if (tmpX < -32767) {
                    tmpX = -32767;
                }
                SHORT tmpY = xs.Gamepad.sThumbLY;
                if (tmpY < -32767) {
                    tmpY = -32767;
                }
                game.gamepadMoveStick(true, af3d::Vector2f(static_cast<float>(tmpX) / 32767.0f, static_cast<float>(tmpY) / 32767.0f));
            }

            if ((gamepad[i].prev.sThumbRX != xs.Gamepad.sThumbRX) || (gamepad[i].prev.sThumbRY != xs.Gamepad.sThumbRY)) {
                SHORT tmpX = xs.Gamepad.sThumbRX;
                if (tmpX < -32767) {
                    tmpX = -32767;
                }
                SHORT tmpY = xs.Gamepad.sThumbRY;
                if (tmpY < -32767) {
                    tmpY = -32767;
                }
                game.gamepadMoveStick(false, af3d::Vector2f(static_cast<float>(tmpX) / 32767.0f, static_cast<float>(tmpY) / 32767.0f));
            }

            gamepad[i].prev = xs.Gamepad;
        } else if (gamepad[i].present) {
            LOG4CPLUS_INFO(af3d::logger(), "gamepad " << i << ": disconnected");
            memset(&gamepad[i].prev, 0, sizeof(gamepad[i].prev));
            gamepad[i].present = false;
        }
    }
}

/*
 * @}
 */

static void gameThread()
{
    LOG4CPLUS_INFO(af3d::logger(), "Game thread started");

    std::vector<InputEvent> tmp;

    while (gRunning && game.update()) {
        {
            af3d::ScopedLock lock(gInputMtx);
            tmp.swap(gInputEvents);
        }

        for (std::vector<InputEvent>::const_iterator it = tmp.begin();
             it != tmp.end(); ++it) {
            switch (it->type) {
            case 0:
                game.keyPress(it->ki, it->misc);
                break;
            case 1:
                game.keyRelease(it->ki, it->misc);
                break;
            case 2:
                game.mouseDown(it->left);
                break;
            case 3:
                game.mouseUp(it->left);
                break;
            case 4:
                game.mouseMove(it->point);
                break;
            case 5:
                game.mouseWheel(it->misc);
                break;
            case 6:
                game.textInputUCS2(it->misc);
                break;
            default:
                assert(false);
                break;
            }
        }

        gamepadUpdate();

        af3d::gameShell->update();

        tmp.clear();
    }

    gRunning = false;

    game.cancelRender();

    LOG4CPLUS_INFO(af3d::logger(), "Game thread finished");
}

static bool OALInit()
{
    LOG4CPLUS_INFO(af3d::logger(), "Initializing OpenAL...");

    HINSTANCE handle = LoadLibraryA("openal32");

    if (!handle) {
        LOG4CPLUS_ERROR(af3d::logger(), "Unable to load openal32.dll");
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

static bool cmFullscreen;
static int cmVideoMode;
static int cmMsaaMode;
static bool cmVsync;
static bool cmTrilinearFilter;
static bool cmRes;

static std::mutex cmMutex;
static std::condition_variable cmCond;
static bool cmDone = false;

void changeVideoModeInternal()
{
    std::uint32_t samples = af3d::platform->msaaModes()[cmMsaaMode];
    af3d::VideoMode vm;

    if (cmFullscreen) {
        vm = af3d::platform->desktopVideoModes()[cmVideoMode];
    } else {
        vm = af3d::platform->winVideoModes()[cmVideoMode];
    }

    if (gWnd) {
        destroyWindow();

        MSG msg;

        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        }
    }

    if (cmFullscreen) {
        if ((vm.width == desktopMode.dmPelsWidth) && (vm.height == desktopMode.dmPelsHeight)) {
            gTrueFullscreen = false;
        } else {
            gTrueFullscreen = true;

            DEVMODEA dm;

            memset(&dm, 0, sizeof(dm));
            dm.dmSize = sizeof(dm);

            dm.dmPelsWidth = vm.width;
            dm.dmPelsHeight = vm.height;
            dm.dmBitsPerPel = 32;
            dm.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

            if (desktopMode.dmDisplayFrequency != 0) {
                dm.dmDisplayFrequency = desktopMode.dmDisplayFrequency;
                dm.dmFields |= DM_DISPLAYFREQUENCY;
            }

            LONG res = ChangeDisplaySettingsA(&dm, CDS_FULLSCREEN);

            if (res != DISP_CHANGE_SUCCESSFUL) {
                LOG4CPLUS_ERROR(af3d::logger(), "ChangeDisplaySettingsA(" << vm.width << ", " << vm.height << ", " << desktopMode.dmDisplayFrequency << ") failed: " << res);
                return;
            }
        }
    }

    if (!createWindow(vm.width, vm.height, cmFullscreen, samples)) {
        return;
    }

    af3d::settings.videoMode = cmVideoMode;
    af3d::settings.msaaMode = cmMsaaMode;
    af3d::settings.vsync = cmVsync;
    af3d::settings.fullscreen = cmFullscreen;
    af3d::settings.trilinearFilter = cmTrilinearFilter;

    if (af3d::settings.viewAspect >= 1.0f) {
        af3d::settings.viewWidth = vm.width;
        af3d::settings.viewHeight = static_cast<float>(vm.width) / af3d::settings.viewAspect;

        if (af3d::settings.viewHeight <= vm.height) {
            af3d::settings.viewX = 0;
            af3d::settings.viewY = static_cast<float>(vm.height - af3d::settings.viewHeight) / 2.0f;
        }
        else {
            af3d::settings.viewHeight = vm.height;
            af3d::settings.viewWidth = static_cast<float>(af3d::settings.viewHeight) * af3d::settings.viewAspect;
            af3d::settings.viewX = static_cast<float>(vm.width - af3d::settings.viewWidth) / 2.0f;
            af3d::settings.viewY = 0;
        }
    }
    else {
        af3d::settings.viewHeight = vm.height;
        af3d::settings.viewWidth = static_cast<float>(af3d::settings.viewHeight) * af3d::settings.viewAspect;

        if (af3d::settings.viewWidth <= vm.width) {
            af3d::settings.viewX = static_cast<float>(vm.width - af3d::settings.viewWidth) / 2.0f;
            af3d::settings.viewY = 0;
        } else {
            af3d::settings.viewWidth = vm.width;
            af3d::settings.viewHeight = static_cast<float>(vm.width) / af3d::settings.viewAspect;
            af3d::settings.viewX = 0;
            af3d::settings.viewY = static_cast<float>(vm.height - af3d::settings.viewHeight) / 2.0f;
        }
    }

    if (!OGLInit(cmVsync)) {
        return;
    }

    ShowWindow(gWnd, SW_SHOW);
    SetForegroundWindow(gWnd);
    SetFocus(gWnd);

    game.renderReload(*gHwCtx);

    cmRes = true;
}

bool af3d::PlatformWin32::changeVideoMode(bool fullscreen, int videoMode, int msaaMode, bool vsync, bool trilinearFilter)
{
    af3d::VideoMode vm;

    if (fullscreen) {
        vm = platform->desktopVideoModes()[videoMode];
    } else {
        vm = platform->winVideoModes()[videoMode];
    }

    cmFullscreen = fullscreen;
    cmVideoMode = videoMode;
    cmMsaaMode = msaaMode;
    cmVsync = vsync;
    cmTrilinearFilter = trilinearFilter;
    cmRes = false;

    bool notify = false;

    if (gWnd == nullptr) {
        changeVideoModeInternal();
    } else {
        notify = true;

        game.cancelRender();

        af3d::ScopedLockA lock(cmMutex);

        while (!cmDone) {
            cmCond.wait(lock);
        }
    }

    if (!cmRes) {
        if (notify) {
            {
                af3d::ScopedLock lock(cmMutex);
                cmDone = false;
            }

            cmCond.notify_one();
        }

        return false;
    }

    game.reload();

    {
        af3d::ScopedLock lock(cmMutex);
        cmDone = false;
    }

    cmCond.notify_one();

    return true;
}

extern const char configIniStr[];

static void DoAllocConsole()
{
    static const WORD MAX_CONSOLE_LINES = 2048;
    int hConHandle;
    long lStdHandle;
    CONSOLE_SCREEN_BUFFER_INFO coninfo;
    FILE *fp;

    AllocConsole();

    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
    coninfo.dwSize.Y = MAX_CONSOLE_LINES;
    SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);

    lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen(hConHandle, "w");

    *stdout = *fp;
    setvbuf(stdout, nullptr, _IONBF, 0);

    lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen(hConHandle, "r");

    *stdin = *fp;
    setvbuf(stdin, nullptr, _IONBF, 0);

    lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);
    hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);
    fp = _fdopen(hConHandle, "w");
    *stderr = *fp;

    setvbuf(stderr, nullptr, _IONBF, 0);

    HMENU hMenu = ::GetSystemMenu(GetConsoleWindow(), FALSE);
    if (hMenu != nullptr) {
        DeleteMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
    }

    ShowWindow(GetConsoleWindow(), SW_SHOW);
}

static void startupFailed()
{
    MessageBox(nullptr, "Game startup failed, see log.txt for details", "Error", MB_OK | MB_ICONSTOP);
}

static af3d::AppConfigPtr getNormalAppConfig(const af3d::AppConfigPtr& appConfig1)
{
    std::shared_ptr<af3d::SequentialAppConfig> appConfig =
        std::make_shared<af3d::SequentialAppConfig>();

    appConfig->add(appConfig1);

    std::ifstream is("config.ini");

    if (is) {
        std::shared_ptr<af3d::StreamAppConfig> appConfig2 =
            std::make_shared<af3d::StreamAppConfig>();

        if (!appConfig2->load(is)) {
            MessageBox(nullptr, "Cannot read config.ini", "Error", MB_OK | MB_ICONSTOP);
            return af3d::AppConfigPtr();
        }

        is.close();

        appConfig->add(appConfig2);
    }

    return appConfig;
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);

    int argc;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    srand(static_cast<unsigned int>(time(nullptr)));

    std::istringstream is1(configIniStr);

    std::shared_ptr<af3d::StreamAppConfig> appConfig1 =
        std::make_shared<af3d::StreamAppConfig>();

    if (!appConfig1->load(is1)) {
        MessageBox(nullptr, "Cannot read built-in config.ini", "Error", MB_OK | MB_ICONSTOP);
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
    if (logLevel <= log4cplus::TRACE_LOG_LEVEL) {
        Assimp::DefaultLogger::create("", Assimp::Logger::VERBOSE);
    }
    else {
        Assimp::DefaultLogger::create("", Assimp::Logger::NORMAL);
    }

    Assimp::DefaultLogger::get()->attachStream(new af3d::AssimpLogStream(log4cplus::TRACE_LOG_LEVEL), Assimp::Logger::Debugging);
    Assimp::DefaultLogger::get()->attachStream(new af3d::AssimpLogStream(log4cplus::TRACE_LOG_LEVEL), Assimp::Logger::Info);
    Assimp::DefaultLogger::get()->attachStream(new af3d::AssimpLogStream(log4cplus::WARN_LOG_LEVEL), Assimp::Logger::Warn);
    Assimp::DefaultLogger::get()->attachStream(new af3d::AssimpLogStream(log4cplus::ERROR_LOG_LEVEL), Assimp::Logger::Err);

    af3d::settings.init(appConfig);

    if (!platformWin32->init(af3d::settings.assets)) {
        MessageBox(nullptr, "Cannot init win32 platform", "Error", MB_OK | MB_ICONSTOP);
        return 1;
    }

    bool withConsole = false;

    log4cplus::SharedAppenderPtr consoleAppender = af3d::logger().getDefaultHierarchy().getRoot().getAppender("console");
    if (consoleAppender && dynamic_cast<log4cplus::ConsoleAppender*>(consoleAppender.get())) {
        DoAllocConsole();
        withConsole = true;
    }

    LOG4CPLUS_INFO(af3d::logger(), "Starting...");

    af3d::gameShell.reset(new af3d::DummyShell());

    gamepadInit();

    InitKIMap();

    if (!OGLPreInit()) {
        startupFailed();
        return 1;
    }

    PopulateVideoModes();

    if (!OALInit()) {
        startupFailed();
        return 1;
    }

    if (argc > 2) {
        std::wstring argv2w = argv[2];
        std::string argv2(argv2w.begin(), argv2w.end());
        af3d::settings.editor.enabled = atoi(argv2.c_str()) > 0;
    }

    std::string argv1 = "intro.af3";

    if (argc > 1) {
        std::wstring argv1w = argv[1];
        argv1 = std::string(argv1w.begin(), argv1w.end());
    }

    bool res = game.init(argv1);

    if (!res) {
        startupFailed();
        return 1;
    }

    std::thread thr(&gameThread);

    MSG msg;

    while (gRunning) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                gRunning = false;
            } else {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        if (game.render(*gHwCtx)) {
            SwapBuffers(gDC);
        } else if (gRunning) {
            changeVideoModeInternal();

            {
                af3d::ScopedLock lock(cmMutex);
                cmDone = true;
            }

            cmCond.notify_one();

            {
                af3d::ScopedLockA lock(cmMutex);

                while (cmDone) {
                    cmCond.wait(lock);
                }
            }
        }
    }

    {
        {
            af3d::ScopedLock lock(cmMutex);
            cmDone = true;
        }

        cmCond.notify_one();
    }

    game.cancelUpdate(*gHwCtx);

    thr.join();

    gHwCtx.reset();

    game.shutdown();

    destroyWindow();

    gamepadShutdown();

    Assimp::DefaultLogger::kill();

    platformWin32->shutdown();

    runtime_assert(af3d::AObject::getCount() == 0);

    if (withConsole) {
        std::cout << "Press any key..." << std::endl;
        _getch();
    }

    return 0;
}
