@mkdir build_Win32_VS2017 2>nul
@cd build_Win32_VS2017
@cmake -DUSE_LUAJIT=1 -DUSE_BT_SSE=0 -DCMAKE_INSTALL_PREFIX="%CD%\..\install_Win32_VS2017" -G "Visual Studio 15 2017" ..
