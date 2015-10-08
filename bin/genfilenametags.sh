#!/bin/bash -
#===============================================================================
#          FILE: genfilenametags.sh
#         USAGE: ./genfilenametags.sh
#   DESCRIPTION: generate tag file for lookupfile plugin
#        AUTHOR: dengwei
#       CREATED: 12/14/13 23:10
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

except_name=''
include_dir=''

function fill_genfile()
{
    this_dir=$1
    lname=$2
    find $this_dir \( -name "*.c" -o -name "*.h" -o -name "*.cc" -o -name "*.cpp" -o -name "*.hpp" \) -type f -printf "%f\t%p\t1\n" | egrep -v "(demo|pclint|backup|\\ut\\|utcode|ut_${except_name})" | sort -f >> $lname
}

while getopts 'e:i:' opt; do
    case $opt in
        e) except_name='|'$OPTARG;;
        i) 
            include_dir=$OPTARG
            if [ ! -d $include_dir ]; then
                echo include_dir $include_dir does not exist, exit ...
                exit 1
            fi
            ;;
        *) echo "`basename $0` [ -e except_name ] [ -i include_dir ]"; exit 1;;
    esac
done


echo -e "!_TAG_FILE_SORTED\t2\t/2=foldcase/" > filenametags
fill_genfile "." "filenametags"

if [ "$include_dir" != "" ]; then
    fill_genfile $include_dir "filenametags"
fi
