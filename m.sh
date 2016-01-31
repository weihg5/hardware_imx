#!/bin/sh

adb shell mount -o remount /system
adb push ../../out/target/product/imx6ms600/system/lib/hw/camera.imx6.so /system/lib/hw/

adb reboot


