#!/system/bin/sh

dumpsys input_method | grep  "mSystemReady="

while true
do
    if dumpsys power 2>/dev/null|grep -q "mSystemReady=true"
    then
        break
    else
        echo "Waiting user interface ..."
        date
        sleep 2
    fi
done

sleep 5

if dmesg |grep 'ddc read timeout'|wc -l|grep -q 128
then
    echo "    DDC error detected"
    if dumpsys power|grep -q "mScreenOn=true"
    then
        echo "   1/2 - sending screen to sleep"
        input keyevent --shortpress KEYCODE_POWER
        sleep 0.8
    fi
    echo "    2/2 - waking up screen"
    input keyevent --shortpress KEYCODE_POWER
fi

#   adb shell input keyevent 82 # Unlock screen
