#!/usr/bin/env python

import pexpect
import sys
import os

git_username='david-stratusee'
git_password='dengwei98406'

cmd='git'

for idx in range(1, len(sys.argv)):
    if idx >= 2 and not sys.argv[idx][0] == '-':
        cmd = cmd + ' "' + sys.argv[idx] + '"'
    else:
        cmd = cmd + ' ' + sys.argv[idx]

print cmd

child = pexpect.spawn(cmd)
try:
    child.expect('Username *')
except (pexpect.EOF, pexpect.TIMEOUT):
    exit(1)

child.sendline(git_username)
try:
    child.expect('Password *')
except (pexpect.EOF, pexpect.TIMEOUT):
    exit(1)

child.sendline(git_password)
print 'successfully login'
child.logfile = sys.stdout
try:
    i = child.expect([pexpect.EOF, pexpect.TIMEOUT], timeout=120)
except (pexpect.EOF, pexpect.TIMEOUT):
    exit(1)

