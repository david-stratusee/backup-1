#!/bin/bash -

#astyle -q -A3 -S -w -f -p -H -U -c --indent=tab -T4 -xT4 --delete-empty-lines --align-pointer=name --align-reference=name -j -xC128 -xL --mode=c -Z --suffix=none $@
astyle -q -A3 -S -w -f -p -H -U -c --delete-empty-lines --align-pointer=name -j -xL --mode=c -Z --suffix=none $@
