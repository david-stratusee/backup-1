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

cgrep.sh -o "#include <(.*)>" | sort -u | grep -v "\-\-" | sed -e 's/#include </\/usr\/include\//g' | sed -e 's/>//g' >/tmp/csfile.list
cgrep.sh -o "#include <(.*)>" | sort -u | grep -v "\-\-" | sed -e 's/#include </\/usr\/local\/include\//g' | sed -e 's/>//g' >>/tmp/csfile.list
if [ -d /usr/include/glib-2.0/ ]; then
    find /usr/include/glib-2.0/ -type f -name "*.h" | grep -v "\/gio\/" >>/tmp/csfile.list
fi
while read line; do
    if [ -f $line ] && [ ! -h $line ]; then
        echo $line >>cscope.files
    fi
done </tmp/csfile.list
rm -f /tmp/csfile.list
