#!/bin/bash -

except_name=''
include_dir=''

function fill_cscope_file()
{
    this_dir=$1
    lname=$2
    find ${this_dir} \( -name "*.c" -o -name "*.h" -o -name "*.cc" -o -name "*.cpp" -o -name "*.hpp" \) -type f | egrep -v "(demo|pclint|\/temp\/|\/tmp\/|\/ut\/|utcode|ut_${except_name})" >>$lname
}

while getopts 'e:i:' opt; do
    case $opt in
        e) except_name=${except_name}'|'$OPTARG;;
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

>cscope.files
fill_cscope_file "." "cscope.files"

if [ "$include_dir" != "" ]; then
    fill_cscope_file ${include_dir} "cscope.files"
fi

cgrep.sh -o "^[ ]*#include *<([^>]*)>" | grep -E -o "#include *<([^>]*)>" | sort -u | grep -v "\-\-" | sed -e 's/#include *<//g' | sed -e 's/>//g' >/tmp/csfile.list

glib_file_path=$(find /usr -name glibconfig.h 2>/dev/null)
if [ "$glib_file_path" != "" ]; then
    glibver=$(grep GLIB_MAJOR_VERSION ${glib_file_path} | awk '{print $3}')
    if [ -d "/usr/include/glib-${glibver}.0/" ]; then
        find /usr/include/glib-${glibver}.0/ -type f -name "*.h" | grep -v "\/gio\/" >>/tmp/csfile.list
    fi
fi
gcc_version=$(gcc --version | grep gcc | awk '{print $3}')
while read line; do
    filepath=$line
    if [ -f $filepath ] && [ ! -h $filepath ]; then
        echo $filepath >>cscope.files
        continue
    fi

    filepath="/usr/include/"$line
    if [ -f $filepath ] && [ ! -h $filepath ]; then
        echo $filepath >>cscope.files
        continue
    fi

    filepath="/usr/local/include/"$line
    if [ -f $filepath ] && [ ! -h $filepath ]; then
        echo $filepath >>cscope.files
        continue
    fi

    filepath="/usr/lib/gcc/x86_64-redhat-linux/${gcc_version}/include/"$line
    if [ -f $filepath ] && [ ! -h $filepath ]; then
        echo $filepath >>cscope.files
        continue
    fi

    #echo "ignore file: $line"
done </tmp/csfile.list
rm -f /tmp/csfile.list
