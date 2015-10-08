#!/usr/bin/python
#coding=utf-8

import commands,sys,time
import threading
import re
import socket

def valid_ip(address):
    try: 
        socket.inet_aton(address)
        return True
    except:
        return False


class myThread (threading.Thread):
    def __init__(self, threadID, result, mydomain):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.domain = mydomain.strip()
        self.result = result

    def run(self):
        digcmd="dig +noquestion +noqr +nocomment +time=30 " + self.domain
        status, msg = commands.getstatusoutput(digcmd)
        self.result[self.threadID] = (self.threadID, self.domain, status, msg)
        #print digcmd, status


if __name__ == '__main__':
    filename = 'domain.list'
    if len(sys.argv) > 1:
        filename = sys.argv[1]

    fp = open(filename)
    lines = fp.readlines();
    fp.close()

    threadID = 1
    globalid = 0
    thread_hdl = []
    thread_result = []

    print "total lines: %d" % len(lines)

    for mydomain in lines:
        thread = myThread(globalid, thread_result, mydomain)
        thread_hdl.append(thread)
        thread_result.insert(globalid, (globalid, mydomain, 0, "Initial"))

        threadID += 1
        globalid += 1

        if threadID == 300:
            for t in thread_hdl:
                t.start()

            for t in thread_hdl:
                t.join()
                thread_hdl.remove(t)
                del t

            thread_hdl = []
            threadID = 1
            print "finish: %d" % globalid

    if threadID > 1:
        for t in thread_hdl:
            t.start()

        for t in thread_hdl:
            t.join()
            thread_hdl.remove(t)

    print "-------------"

    domainset={}
    faileddomain=set()
    noresultdomain=set()
    usefuldomain=set()
    re_hdl = re.compile("^.*IN[ \t]+A[\t ]+(.*)$", re.MULTILINE)

    if thread_result is not None and len(thread_result) > 0:
        for id,domain,status,msg in thread_result:
            if status != 0:
                print "ERR",domain,status,msg
                faileddomain.add(domain)
            else:
                m = re_hdl.findall(msg)
                if m is not None and len(m) > 0:
                    for dstip in m:
                        if domainset.has_key(dstip):
                            domainset[dstip] = domainset[dstip] + " " + domain
                        else:
                            domainset[dstip] = domain
                    #print domain + ": " + str(m)
                    usefuldomain.add(domain)
                else:
                    print domain,msg
                    noresultdomain.add(domain)

    if len(domainset) > 0:
        resultfile="hosts.all"
        print "RESULT:[%d] %s" % (len(domainset), resultfile)
        fp = open(resultfile, "w")
        for dstip in domainset.keys():
            fp.write(dstip + "\t" + domainset[dstip] + "\n")
        fp.close()

    print "-------------"
    if len(faileddomain) > 0:
        print "FAILED DNS:[%d]" % len(faileddomain)
        fp = open('fail.txt', "w")
        for domain in faileddomain:
            fp.write(domain + "\n")
        fp.close()

    print "-------------"
    if len(noresultdomain) > 0:
        print "NO RESULT DNS:[%d]" % len(noresultdomain)
        fp = open('noresult.txt', "w")
        for domain in noresultdomain:
            fp.write(domain + "\n")
        fp.close()

    print "-------------"
    if len(usefuldomain) > 0:
        print "VALID DNS:[%d]" % len(usefuldomain)
        fp = open('valid.txt', "w")
        for domain in usefuldomain:
            fp.write(domain + "\n")
        fp.close();

    print "-------------"

    print "Exiting Main Thread"

