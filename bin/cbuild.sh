#!/bin/bash -
#===============================================================================
#          FILE: cbuild.sh
#         USAGE: ./cbuild.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 07/31/2014 13:38
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

BUILD_DIR=cbuild
CMAKE_LISTFILE="CMakeLists.txt"
BACKUP_CMAKELIST=".CMakeLists.txt"
PRO_DIR=.
CLEAN_FLAG=1
UNINSTALL_FLAG=2
INSTALL_FLAG=3
GET_ACTION=0
CUR_DIR=`pwd`
CMAKE_ARG=""
QUIET_FLAG=0

function cexit()
{
    cd $CUR_DIR
    exit $1
}

function check_result()
{
    result=$1
    shift
    if [ $result -ne 0 ]; then
        echo -e "get result $result when $@, exit ..."
        cexit $result
    fi
}

function cbuild_help()
{
    echo "Usage: $1 [-a (clean|install|uninstall)] [-d project_dir] [-q] [-g argument-for-cmake]"
    echo "       default action is only make"

    exit 0
}

while getopts 'd:a:g:hq' opt; do
    case $opt in
        a) 
            case $OPTARG in
                "clean") GET_ACTION=$CLEAN_FLAG;;
                "uninstall") GET_ACTION=$UNINSTALL_FLAG;;
                "install") GET_ACTION=$INSTALL_FLAG;;
                "compile") GET_ACTION=0;;
                ?|*) echo "action $OPTARG is invalid"; cbuild_help `basename $0`; exit 1;;
            esac
            ;;
        d) 
            if [ -d $OPTARG ]; then
                PRO_DIR=$OPTARG
            else
                echo "the directory $OPTARG does not exist, exit ..."
                exit 1
            fi;;
        g)
            CMAKE_ARG=${CMAKE_ARG}" -D"$OPTARG
            ;;
        q)
            QUIET_FLAG=1
            CMAKE_ARG=${CMAKE_ARG}" -DVERBOSE=quiet"
            ;;
        h) cbuild_help `basename $0`; exit 0;;
        ?) cbuild_help `basename $0`; exit 1;;
    esac
done

cd $PRO_DIR
if [ $GET_ACTION -gt 0 ]; then
    if [ ! -d $BUILD_DIR ]; then
        echo "$BUILD_DIR does not exist, exit ..."
        cexit 1
    fi

    case $GET_ACTION in
        $CLEAN_FLAG)
            echo "remove temporary files for $PRO_DIR ..."
            cd $BUILD_DIR
            make clean
            check_result $? make clean
            cd ..
            rm -rf $BUILD_DIR
            ;;
        $INSTALL_FLAG)
            echo "install action for $PRO_DIR ..."

            cd $BUILD_DIR
            sudo make install
            check_result $? make install
            cd ..
            ;;
        $UNINSTALL_FLAG)
            echo "uninstall action for $PRO_DIR ..."

            cd $BUILD_DIR
            if [ ! -f "install_manifest.txt" ]; then
                echo "install_manifest.txt does not exist, exit ..."
                cd ..
                cexit 1
            fi
            sudo xargs rm < install_manifest.txt
            cd ..
            ;;
    esac
    cexit 0
fi

####################
# only make below
####################
if [ ! -f $CMAKE_LISTFILE ]; then
    echo "$CMAKE_LISTFILE does not exist, exit ..."
    exit 1
fi

if [ ! -d $BUILD_DIR ]; then
    mkdir $BUILD_DIR
fi

cd $BUILD_DIR
if [ -f $BACKUP_CMAKELIST ]; then
    diff_count=`diff ../${CMAKE_LISTFILE} $BACKUP_CMAKELIST | wc -l`
else
    diff_count=1
fi

if [ $diff_count -gt 0 ]; then
    echo cmake${CMAKE_ARG} ..
    cmake${CMAKE_ARG} ..
    check_result $? cmake

    cp ../${CMAKE_LISTFILE} $BACKUP_CMAKELIST
fi

make
check_result $? make
cd ..

cexit 0

