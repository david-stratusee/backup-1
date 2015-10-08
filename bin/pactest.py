#!/usr/bin/env python

import pacparser,sys

print sys.argv[1]

if len(sys.argv) >= 3:
    pacfile=sys.argv[2]
    print "use: ", pacfile
else:
    pacfile="proxy.pac"

pacparser.init()
pacparser.parse_pac(pacfile)
proxy = pacparser.find_proxy(sys.argv[1])
print proxy
pacparser.cleanup()

# Or simply,
print pacparser.just_find_proxy(pacfile, sys.argv[1])
