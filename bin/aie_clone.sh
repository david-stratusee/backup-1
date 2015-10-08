#!/bin/bash -
#===============================================================================
#          FILE: get_aie_code.sh
#         USAGE: ./get_aie_code.sh
#   DESCRIPTION:
#        AUTHOR: dengwei
#       CREATED: 2014年08月25日 13:53
#      REVISION:  ---
#===============================================================================

set -o nounset                              # Treat unset variables as an error

git clone https://github.com/stratusee/AIE.git $@
