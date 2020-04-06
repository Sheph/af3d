DIR_NAME=`basename ${PWD}`
mkdir -p ../${DIR_NAME}-release
cd ../${DIR_NAME}-release
cmake -G "Eclipse CDT4 - Unix Makefiles" \
-DCMAKE_ECLIPSE_VERSION=4.14 \
-DCMAKE_BUILD_TYPE="Release" \
../${DIR_NAME}
