DIR_NAME=`basename ${PWD}`
mkdir -p ../${DIR_NAME}-debug
cd ../${DIR_NAME}-debug
cmake -G "Eclipse CDT4 - Unix Makefiles" \
-DCMAKE_ECLIPSE_VERSION=4.14 \
-DCMAKE_BUILD_TYPE="Debug" \
../${DIR_NAME}
