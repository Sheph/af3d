DIR_NAME=`basename ${PWD}`
mkdir -p ../${DIR_NAME}-debug
cd ../${DIR_NAME}-debug
cmake -G "Eclipse CDT4 - Unix Makefiles" \
-DCMAKE_ECLIPSE_VERSION=4.14 \
-DCMAKE_BUILD_TYPE="Debug" \
-DUSE_LUAJIT=1 -DUSE_LUAJIT_VALGRIND=0 \
../${DIR_NAME}
