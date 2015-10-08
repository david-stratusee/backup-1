#!/bin/bash -

. tools.sh

function clear_tag_files()
{
    time_echo "rm cscope and tag files"
    find . -name "cscope*" -type f -exec rm -f {} \;
    find . -name "cppcomplete.tags" -type f -exec rm -f {} \;
    find . -name "tags" -type f -exec rm -f {} \;
    find . -name "udtags" -type f -exec rm -f {} \;
}

except_name=""
include_dir=""
while getopts 'e:i:hc' opt; do
    case $opt in
        e) except_name=${except_name}" -e $OPTARG";;
        i) 
            if [ ! -d $OPTARG ]; then
                echo include_dir $OPTARG does not exist, exit ...
                exit 1
            fi
            include_dir=" -i "$OPTARG
            ;;
        c)  clear_tag_files; exit 0;;
        h|*) echo "`basename $0` [ -e except_name ] [ -i include_dir ]"; exit 1;;
    esac
done

clear_tag_files

execute_hint vcollect_files.sh ${except_name}$include_dir
grep -r -H -E -n "__attribute__" `cat cscope.files` | awk -F ":" '{print $3}' | egrep -o "define.*" | awk '{if ($3 ~ /__attribute__/ && $1 ~ /define/) print $2}' | awk -F"(" '{print $1}' >/tmp/tags_ignore
echo __THROW >>/tmp/tags_ignore
execute_hint ctags --sort=foldcase --c-kinds=+xp --c++-kinds=+xp --fields=+liaS --extra=+q -I/tmp/tags_ignore -L cscope.files
rm -f /tmp/tags_ignore
cp -af tags cppcomplete.tags 2>/dev/null
#execute_hint cscope -Rbq
execute_hint cscope -bq
execute_hint parse_tags.py

echo -n [`date +%H:%M:%S`]
colorecho "green" " [done]"
