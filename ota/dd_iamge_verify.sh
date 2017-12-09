#! /system/bin/sh


#usage:
#adb push dd_image_vefify.sh /data/media/0/
#adb shell sh /data/media/0/dd_image_vefify.sh 
#1 必须将此脚本push到手机中运行,不能用adb shell
#2 计算出来的sha1,与升级包中对应文件的sha1对比,对应关系为
# package_extract_file("boot.img", "/dev/block/bootdevice/by-name/boot");
# package_extract_file("firmware-update/NON-HLOS.bin", "/dev/block/bootdevice/by-name/modem");

# package_extract_file("firmware-update/cmnlib64.mbn", "/dev/block/bootdevice/by-name/cmnlib64");
# package_extract_file("firmware-update/cmnlib.mbn", "/dev/block/bootdevice/by-name/cmnlib");
# package_extract_file("firmware-update/adspso.bin", "/dev/block/bootdevice/by-name/dsp");
# package_extract_file("firmware-update/rpm.mbn", "/dev/block/bootdevice/by-name/rpm");
# package_extract_file("firmware-update/tz.mbn", "/dev/block/bootdevice/by-name/tz");
# package_extract_file("firmware-update/emmc_appsboot.mbn", "/dev/block/bootdevice/by-name/aboot");
# package_extract_file("firmware-update/lksecapp.mbn", "/dev/block/bootdevice/by-name/lksecapp");
# package_extract_file("firmware-update/sbl1.mbn", "/dev/block/bootdevice/by-name/sbl1");
# package_extract_file("firmware-update/devcfg.mbn", "/dev/block/bootdevice/by-name/devcfg");
# package_extract_file("firmware-update/keymaster.mbn", "/dev/block/bootdevice/by-name/keymaster");
# package_extract_file("firmware-update/hyp.mbn", "/dev/block/bootdevice/by-name/hyp");

# package_extract_file("firmware-update/cmnlib64.mbn", "/dev/block/bootdevice/by-name/cmnlib64bak");
# package_extract_file("firmware-update/cmnlib.mbn", "/dev/block/bootdevice/by-name/cmnlibbak");
# package_extract_file("firmware-update/adspso.bin", "/dev/block/bootdevice/by-name/dspbak");
# package_extract_file("firmware-update/rpm.mbn", "/dev/block/bootdevice/by-name/rpmbak");
# package_extract_file("firmware-update/tz.mbn", "/dev/block/bootdevice/by-name/tzbak");
# package_extract_file("firmware-update/emmc_appsboot.mbn", "/dev/block/bootdevice/by-name/abootbak");
# package_extract_file("firmware-update/lksecapp.mbn", "/dev/block/bootdevice/by-name/lksecappbak");
# package_extract_file("firmware-update/sbl1.mbn", "/dev/block/bootdevice/by-name/sbl1bak");
# package_extract_file("firmware-update/devcfg.mbn", "/dev/block/bootdevice/by-name/devcfgbak");
# package_extract_file("firmware-update/keymaster.mbn", "/dev/block/bootdevice/by-name/keymasterbak");
# package_extract_file("firmware-update/hyp.mbn", "/dev/block/bootdevice/by-name/hypbak");

#bs的值 根据updater-scipt脚本中的命令参数确定 eg:
#abort("E3008: Failed to apply patch to EMMC:/dev/block/bootdevice/by-name/boot:16467238:f3dfdb458f219c1b0b07bb380d195cafce4a7dd2:16467238:b8ae9acbd110eb04c69dc1326fcd471b6cb91838");
# bs=16467238


function verify_boot()
{
    echo "input boot.img size"
    read size
    
    if [ "$size" -gt 0 ]; then
        dd if=/dev/block/bootdevice/by-name/boot bs=$size count=1 | sha1sum
        echo "boot partition sha1 is above"
    fi
}


function verify_modem()
{
    echo "input NON-HLOS.bin size"
    read size

    if [ "$size" -gt 0 ]; then
        dd if=/dev/block/bootdevice/by-name/modem bs=$size count=1 | sha1sum
        echo "modem partition(NON-HLOS.bin) sha1 is above"
    fi
}

function verify_dsp()
{
    echo "input adspso.bin size"
    read size

    if [ "$size" -gt 0 ]; then
        dd if=/dev/block/bootdevice/by-name/dsp bs=$size count=1 | sha1sum
        echo "dsp partition(adspso.bin) sha1 is above"
    fi
}

function verify_cmnlib64()
{
    echo "input cmnlib64.mbn size"
    read size

    if [ "$size" -gt 0 ]; then
        dd if=/dev/block/bootdevice/by-name/cmnlib64 bs=$size count=1 | sha1sum
        echo "cmnlib64 partition sha1 is above"
        dd if=/dev/block/bootdevice/by-name/cmnlib64bak bs=$size count=1 | sha1sum
        echo "cmnlib64bak partition sha1 is above"
    fi
}

function verify_cmnlib()
{
    echo "input cmnlib.mbn size"
    read size

    if [ "$size" -gt 0 ]; then
        dd if=/dev/block/bootdevice/by-name/cmnlib bs=$size count=1 | sha1sum
        echo "cmnlib partition sha1 is above"
        dd if=/dev/block/bootdevice/by-name/cmnlibbak bs=$size count=1 | sha1sum
        echo "cmnlibbak partition sha1 is above"
    fi
}

function verify_rpm()
{
    echo "input rpm.mbn size"
    read size

    if [ "$size" -gt 0 ]; then
        dd if=/dev/block/bootdevice/by-name/rpm bs=$size count=1 | sha1sum
        echo "rpm partition sha1 is above"
        dd if=/dev/block/bootdevice/by-name/rpmbak bs=$size count=1 | sha1sum
         echo "rpmbak partition sha1 is above"
    fi
}

function verify_tz()
{
    echo "input tz.mbn size"
    read size

    if [ "$size" -gt 0 ]; then
        dd if=/dev/block/bootdevice/by-name/tz bs=$size count=1 | sha1sum
        echo "tz partition sha1 is above"
        dd if=/dev/block/bootdevice/by-name/tzbak bs=$size count=1 | sha1sum
        echo "tzbak partition sha1 is above"
    fi
}

function verify_aboot()
{
    echo "input emmc_appsboot.mbn size"
    read size

    if [ "$size" -gt 0 ]; then
        dd if=/dev/block/bootdevice/by-name/aboot bs=$size count=1 | sha1sum
        echo "aboot partition sha1 is above"
        dd if=/dev/block/bootdevice/by-name/abootbak bs=$size count=1 | sha1sum
        echo "abootbak partition sha1 is above"
    fi
}

function verify_lksecapp()
{
    echo "input lksecapp.mbn size"
    read size

    if [ "$size" -gt 0 ]; then
        dd if=/dev/block/bootdevice/by-name/lksecapp bs=$size count=1 | sha1sum
        echo "lksecapp partition sha1 is above"
        dd if=/dev/block/bootdevice/by-name/lksecappbak bs=$size count=1 | sha1sum
        echo "lksecappbak partition sha1 is above"
    fi
}

function verify_sbl1()
{
    echo "input sbl1.mbn size"
    read size

    if [ "$size" -gt 0 ]; then
        dd if=/dev/block/bootdevice/by-name/sbl1 bs=$size count=1 | sha1sum
        echo "sbl1 partition sha1 is above"
        dd if=/dev/block/bootdevice/by-name/sbl1bak bs=$size count=1 | sha1sum
        echo "sbl1bak partition sha1 is above"
    fi
}

function verify_devcfg()
{
    echo "input devcfg.mbn size"
    read size

    if [ "$size" -gt 0 ]; then
        dd if=/dev/block/bootdevice/by-name/devcfg bs=$size count=1 | sha1sum
        echo "devcfg partition sha1 is above"
        dd if=/dev/block/bootdevice/by-name/devcfgbak bs=$size count=1 | sha1sum
        echo "devcfgbak partition sha1 is above"
    fi
}

function verify_keymaster()
{
    echo "input keymaster.mbn size"
    read size

    if [ "$size" -gt 0 ]; then
        dd if=/dev/block/bootdevice/by-name/keymaster bs=$size count=1 | sha1sum
        echo "keymaster partition sha1 is above"
        dd if=/dev/block/bootdevice/by-name/keymasterbak bs=$size count=1 | sha1sum
        echo "keymasterbak partition sha1 is above"
    fi
}

function verify_hyp()
{
    echo "input hyp.mbn size"
    read size

    if [ "$size" -gt 0 ]; then
        dd if=/dev/block/bootdevice/by-name/hyp bs=$size count=1 | sha1sum
        echo "keymaster partition sha1 is above"
        dd if=/dev/block/bootdevice/by-name/hypbak bs=$size count=1 | sha1sum
        echo "keymasterbak partition sha1 is above"
    fi
}

verify_boot
verify_modem

verify_cmnlib64
verify_cmnlib
verify_dsp
verify_rpm
verify_tz
verify_aboot
verify_lksecapp
verify_sbl1
verify_devcfg
verify_keymaster
verify_hyp


