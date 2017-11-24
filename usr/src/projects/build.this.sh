#!/data/data/u.root/bin/bash

reset

. ../board.cfg


APPNAME=$(basename $(echo -n src/main/java/*.*.*))

if echo $APPNAME|grep -q \\*
then
    echo "

Error: java source folder not found in $(pwd)/src/main/java/
    "
    exit 1
fi

echo "
* Builing APK for [${APPNAME}]
"




#cd src/main/jni; /SDK/android/crystax/ndk-build ; cd ..;cd ..; cd ..

echo "
* Board Ping-Pong
"

$adb disconnect ${ADB_HOST}

$adb connect ${ADB_HOST}

$adb "wait-for-device"

echo uninstalling old app if any
$adb uninstall ${APPNAME}

if [ -d ./build ]
then
    echo "
* cleaning up (including ndk build !)
"
    rm -rf ./libs/ ./build/
fi

#LOCAL_SHARED_LIBRARIES="-L/data/data/u.root/lib-armhf -lcrystax -lpython3.5m"


echo "
* building
"

if ./gradlew assembleDebug "$@"
then
    apkfile=$(echo -n ./build/outputs/apk/*-debug.apk)
    ACTIVITY=$(${BIN}/aapt dump badging $apkfile |awk -F" " '/launchable-activity/ {print $2}'|awk -F"'" '/name=/ {print $2}')
    $adb install $apkfile

    $adb shell am start -n ${APPNAME}/${ACTIVITY}
else
    echo "
     !! build failed !!
"
fi

$adb disconnect ${ADB_HOST}
