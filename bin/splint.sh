#!/bin/bash -
echo splint +posixlib -paramuse -warnposixheaders -preproc +matchanyintegral $1
echo ================
splint -paramuse -warnposixheaders -preproc +matchanyintegral $1
#-weak
