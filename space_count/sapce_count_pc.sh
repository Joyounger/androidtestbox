#!/bin/bash

jsonpath="~/test/spacecount"

function FileCount(){
  for file in `ls $1`
  do
    if [ -d $1"/"$file ]; then
      echo "    {" >> ${jsonfile}
      local dirpath=$1"/"$file
      local dirsize=`stat -c %s $dirpath`
      let usedsize=usedsize+$dirsize
      dirpath=${dirpath:1}
      dirpath="$(echo $dirpath | tr '[:upper:]' '[:lower:]')"

      echo "      \"file_path\":\"$dirpath\"," >> ${jsonfile}
      echo "      \"file_size\":\"$dirsize\"," >> ${jsonfile}
      echo "      \"file_type\":\"dir\"" >> ${jsonfile}
      echo "    }," >> ${jsonfile}
      FileCount $1"/"$file
    else
      if [ ! -L $1"/"$file ]; then # don't stat link file
        echo "    {" >> ${jsonfile}
        local filepath=$1"/"$file
        local filesize=`ls -l $filepath | cut -d ' ' -f 5`
        let usedsize=usedsize+$filesize
        filepath=${filepath:1}
        filepath="$(echo $filepath | tr '[:upper:]' '[:lower:]')"

        echo "      \"file_path\":\"$filepath\"," >> ${jsonfile}
        echo "      \"file_size\":\"$filesize\"," >> ${jsonfile}
        echo "      \"file_type\":\"file\"" >> ${jsonfile}
        echo "    }," >> ${jsonfile}
      fi
    fi
  done
}


IFS=$'\n'
serverfile=$1


time_stamp=$(date +%Y.%m.%d-%H:%M)
outdir=$(cat $serverfile | grep "path_to_out" | cut -d ' ' -f 2)
device_name=$(cat $serverfile | grep "^target_product" | cut -d ' ' -f 2)
build_success=$(cat $serverfile | grep "success" | cut -d ' ' -f 2)
jsonfile="${jsonpath}/${device_name}-_${time_stamp}.json"
system_total=0
data_total=0
objdir=${outdir}/obj/PACKAGING/target_files_intermediates/${device_name}-target_files
system_total=$(cat ${objdir}/META/misc_info.txt | grep "system_size" | cut -d '=' -f 2)
data_total=$(cat ${objdir}/META/misc_info.txt | grep "userdata_size" | cut -d '=' -f 2)


if [ ! -d $jsonpath ]; then
    cd /home/work
    mkdir spacecount
fi




echo "{" >> ${jsonfile}
echo "  \"ts\":\"$time_stamp\"," >> ${jsonfile}
echo "  \"success\":\"$build_success\"," >> ${jsonfile}
echo "  \"name\":\"$device_name\"," >> ${jsonfile}
echo "  \"type\":\"$version_type\"," >> ${jsonfile}
echo "  \"g\":\"$is_global\"," >> ${jsonfile}


echo "  \"files\":[" >> ${jsonfile}



cd $objdir
usedsize=0
FileCount "./DATA" $usedsize
data_used=$usedsize
cd $outdir
usedsize=0
FileCount "./system" $usedsize
system_used=$usedsize


sed -i '$d' ${jsonfile}
echo "    }" >> ${jsonfile}
echo "  ]," >> ${jsonfile}


echo "  \"st\":\"$system_total\"," >> ${jsonfile}
echo "  \"su\":\"$system_used\"," >> ${jsonfile}
echo "  \"dt\":\"$data_total\"," >> ${jsonfile}
echo "  \"du\":\"$data_used\"," >> ${jsonfile}
echo "}" >> ${jsonfile}


curl -X POST -H "'Content-type':'application/json', 'charset':'utf-8'" --data-binary @${jsonfile} http://www.test.com/data/partitionUpload


# debug code
echo "time_stamp:               ${time_stamp}"
echo "outdir:                   ${outdir}"
echo "device_name:              ${device_name}"
echo "version_type:             ${version_type}"
echo "system_total:             ${system_total}"
echo "system_used:              ${system_used}"
echo "data_total:               ${data_total}"
echo "data_used:                ${data_used}"
echo "jsonfile:                 ${jsonfile}"
