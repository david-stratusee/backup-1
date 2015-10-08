#!/usr/bin/env python

"""
kind        Kind of tag.  The value depends on the language.  For C and C++ these kinds are recommended:
    c   class name
    d   define (from #define XXX)
    e   enumerator
    f   function or method name
    F   file name
    g   enumeration name
    m   member (of structure or class data)
    p   function prototype
    s   structure name
    t   typedef
    u   union name
    v   variable
"""

match_keyword  = set(['contains', 'contained', 'oneline', 'fold', 'display', 'extend', 'concealends', 'transparent', 'matchgroup', 'replace'])
sys_keyword    = set(['bool', 'true', 'false', 'null'])
undo_nameset   = match_keyword | sys_keyword

cUserTypes     = set([])
cUserDefines   = set(['__FUNCTION__'])
cUserFunctions = set([])
cUserNote      = set(['NOTE'])

def get_set(l_flag):
    if l_flag == 't' or l_flag == 'u' or l_flag == 's' or l_flag == 'g':
        return cUserTypes
    elif l_flag == 'd' or l_flag == 'e':
        return cUserDefines
    elif l_flag == 'f' or l_flag == 'p':
        return cUserFunctions
    else:
        return None

fp=open('./tags', 'r')
lines=fp.readlines()
fp.close()

for line in lines:
    if line[0] == '!' or line[0] == '~':
        continue

    #print line.strip()

    index = line.find(';"\t')
    if index == -1:
        continue

    flag=line[index+3:index+4]

    this_set = get_set(flag)
    if this_set is None:
        continue

    end_idx=line.find('\t', 0, index)
    if end_idx == -1:
        continue

    start_idx = line.rfind('::', 0, end_idx)
    if start_idx == -1:
        start_idx = 0
    else:
        start_idx += 2

    name=line[start_idx : end_idx]
    if name[0] == '~' or name.find(' ') != -1 or name.lower() in undo_nameset:
        continue

    #print flag, name
    this_set.add(name)

fp=open('./udtags', 'w')
for node in cUserTypes:
    fp.write(' ' + node);
fp.write('\n')

for node in cUserDefines:
    fp.write(' ' + node);
fp.write('\n')

for node in cUserFunctions:
    fp.write(' ' + node);
fp.write('\n')

for node in cUserNote:
    fp.write(' ' + node);
fp.write('\n')

fp.close()

