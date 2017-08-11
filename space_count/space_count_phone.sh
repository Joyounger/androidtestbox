#!/system/bin/sh


#  
#   purpose:    this shell script can acquire some partitions's totoal size and free size in a Android's shell environment,now support form KK to N.
#   param:      none
#   date:       16.12.5 


time_stamp=$(date +%Y.%m.%d-%H:%M)                          # record time
device_name=$(getprop "ro.product.device")                  # get device_name
android_version=$(getprop "ro.build.version.release")
android_version_prefix=${android_version%%.*}               # android's big version munber

# in Android KK and L's shell environment, 'sed' and 'cut' command 
# were integration into busybox
if [ ${android_version_prefix} -lt 6 ]; then
    with_busybox="busybox"
fi

# in df command's output, the second column stands for the partition's total size,
# the fotth column stands for the partition's free size.
total=2
free=4

get_size()
{
    df $1 | $with_busybox sed -n '2,$p' | $with_busybox sed 's/  */ /g' | $with_busybox cut -d ' ' -f $2;
}

# $1's unit is KB, convert its unit to GB here we use dc command, 
# which is a reverse-polish desk calculator which supports unlimited precision arithmetic.
# In android's shell environemt, type 'busybox dc --help' to learn how to use it.
KBtoGB()
{
    size_MB=$(busybox dc $1 1024 / p)
    size_GB=$(busybox dc $size_MB 1024 / p)
    echo $size_GB
}

# $1's unit is KB, convert its unit to MB
KBtoMB()
{
    size_MB=$(busybox dc $1 1024 / p)
    echo $size_MB
}

system_total=$(get_size /system $total)
system_free=$(get_size /system $free)



if [ ${android_version_prefix} -ge 7 ]; then
    system_total_float=$(KBtoGB $system_total)
    system_total_GB=${system_total_float}G
    system_free_float=$(KBtoGB $system_free)
    system_free_GB=${system_free_float}G

fi



# debug case 
echo $time_stamp
echo $device_name
echo $android_version
echo $android_version_prefix
echo $system_total
echo $system_free
echo $system_total_GB
echo $system_free_GB




