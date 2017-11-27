#!/bin/bash
export NDK=/data/data/u.root/android/ndk
export UR=/data/data/u.r
GCC="${NDK}/toolchains/llvm/prebuilt/linux-x86_64/bin/clang -target armv7-none-linux-androideabi -gcc-toolchain ${NDK}/toolchains/arm-linux-androideabi-4.9/prebuilt/linux-x86_64 --sysroot=${NDK}/platforms/android-19/arch-arm -march=armv7-a -Wl,--fix-cortex-a8  -pie -Xlinker -export-dynamic"
LIB="-L${UR}/lib-armhf -L/system/lib -lpython3.7dm -llog -landroid"
INC="-I. -I${UR}/usr/include/python3.7dm"
rm interpreter37dm
$GCC -DINTERPRETER -fPIE -DANDROID -D__ANDROID__ ${INC} -o interpreter37dm cpython37dm.cpp ${LIB}
echo ------------------- test ----------------------
PYTHONCOERCECLOCALE=1 ./interpreter37dm test.py
echo ----------------------------------------------
