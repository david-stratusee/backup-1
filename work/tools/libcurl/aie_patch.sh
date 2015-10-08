#!/bin/bash -
#===============================================================================
#          FILE: aie_patch.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 12/11/2014 10:05
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

#vername="squid-3.4.9"
#tgz_url="http://www.squid-cache.org/Versions/v3/3.4/${vername}.tar.gz"
vername=""
tgz_url=""

function check_result()
{
    result=$1
    msg=$2
    shift
    shift
    if [ $result -ne 0 ]; then
        echo -e "get result $result when $msg, exit..."
        if [ $# -gt 0 ]; then
            echo $@
            $@
        fi
        exit $result
    else
        echo -e " $msg -- [OK]"
    fi
}

WGET_CMD="wget -nv -c"

function prepare_target_dir()
{
    if [ -d ${vername} ]; then
        rm -rf ${vername}
    fi
    if [ ! -f ${vername}.tar.gz ]; then
        ${WGET_CMD} -O ${vername}.tar.gz ${tgz_url}
        check_result $? "get ${vername}" rm -f ${vername}.tar.gz
    fi

    tar zxf ${vername}.tar.gz
    check_result $? "extract ${vername}" rm -f ${vername}.tar.gz
}

function create_patch()
{
    local_patch_file=$1
    local_updated_proj=$2

    prepare_target_dir

    echo diff -E -b -w -B -uNr ${vername} ${local_updated_proj}
    diff -E -b -w -B -uNr ${vername} ${local_updated_proj} > ${local_patch_file}

    echo "create patch ${local_patch_file}, updated_proj ${local_updated_proj} can be deleted now"
}

function use_patch()
{
    local_patch_file=$1

    prepare_target_dir
    echo "use patch for ${vername} with patch file ${local_patch_file}"
    for file in ${local_patch_file}; do
        patch -r - -lNp0 -i ${file}
        check_result $? "patch ${file}"
    done
}

function patch_help()
{
    echo "Usage: $1 [-m][-h][-f patch_file][-d updated_proj][-u download_url][-t target_directory]"
    echo "       -m means create patch, default action is use patch"
}

patch_file=""
updated_proj=""
CREATE_PATCH=1
USE_PATCH=2
PATCH_ACTION=${USE_PATCH}

while getopts 't:u:d:f:mh' opt; do
    case $opt in
        u) 
            if [ -f $OPTARG ]; then
                tgz_url=`cat $OPTARG`
            else
                tgz_url=$OPTARG
            fi
            ;;
        t) vername=$OPTARG;;
        d) updated_proj=$OPTARG;;
        f) patch_file=$OPTARG;;
        m) PATCH_ACTION=${CREATE_PATCH};;
        h) patch_help `basename $0`; exit 0;;
        *) patch_help `basename $0`; exit 1;;
    esac
done

if [ "${vername}" == "" ] || [ "${tgz_url}" == "" ]; then
    echo "target and download url must be set"
    exit 3
fi

case ${PATCH_ACTION} in
    ${CREATE_PATCH})
        if [ ! -d ${updated_proj} ]; then
            echo "need use -d to add updated_proj directory for create patch"
            exit 1
        fi

        if [ "${patch_file}" == "" ]; then
            echo "need use -f to set patch_file name for create patch"
            exit 2
        fi

        for file in ${patch_file}; do
            break;
        done

        echo "create patch for ${updated_proj} with patch file ${file}"
        create_patch ${file} ${updated_proj}
        ;;
    ${USE_PATCH})
        if [ "${patch_file}" == "" ]; then
            echo "need use -f to set patch_file name for using patch"
            exit 1
        fi

        for file in ${patch_file}; do
            if [ ! -f ${file} ]; then
                echo "file ${file} does not exist, exit..."
                exit 2
            fi
        done
        use_patch "${patch_file}"
        ;;
    *)
        ;;
esac

