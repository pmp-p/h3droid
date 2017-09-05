#!/system/bin/sh
if dmesg |grep 'ddc read timeout'|wc -l|grep -q 128
then
    input keyevent KEYCODE_POWER
    sleep 1
    input keyevent KEYCODE_POWER
fi

