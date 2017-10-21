LIB="-L/data/data/u.r/lib-armhf -L/system/lib -lpython3.5m -llog -landroid"
INC="-I. -I${UROOT}/usr/include/python3.5m"
rm interpreter
gcc -DINTERPRETER -fPIE -DANDROID -D__ANDROID__ ${INC} -o interpreter cpython35m.cpp ${LIB}
