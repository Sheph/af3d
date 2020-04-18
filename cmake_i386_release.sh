DIR_NAME=`basename ${PWD}`
mkdir -p ../${DIR_NAME}-release
cd ../${DIR_NAME}-release
cmake -G "Eclipse CDT4 - Unix Makefiles" \
-DCMAKE_ECLIPSE_VERSION=4.14 \
-DCMAKE_BUILD_TYPE="Release" \
-DUSE_LUAJIT=1 -DUSE_LUAJIT_VALGRIND=0 \
../${DIR_NAME}
