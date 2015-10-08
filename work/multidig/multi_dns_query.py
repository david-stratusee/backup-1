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
    def __init__(self, threadID, result, myip, host):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.ip = myip.strip()
        self.host = host
        self.result = result
        if valid_ip(host):
            self.reverse = True
        else:
            self.reverse = False

    def run(self):
        #print str(self.threadID) + ":" + self.ip + "," + self.host
        if not self.reverse:
            digcmd="dig @" + self.ip + " +time=10 " + host
        else:
            digcmd="dig @" + self.ip + " +time=10 -x " + host
        status, msg = commands.getstatusoutput(digcmd)
        self.result[self.threadID] = (self.threadID, self.ip, status, msg)
        #print digcmd, status


if __name__ == '__main__':
    fp = open('use_dns.txt')
    lines = fp.readlines();
    fp.close()

    if len(sys.argv) == 0:
        print "need one argument"
        exit(0)

    host=sys.argv[1]
    #print host

    threadID = 1
    globalid = 0
    thread_hdl = []
    thread_result = []

    for myip in lines:
        thread = myThread(globalid, thread_result, myip, host)
        thread_hdl.append(thread)
        thread_result.insert(globalid, (globalid, myip, 0, "Initial"))

        threadID += 1
        globalid += 1

        if threadID == 200:
            for t in thread_hdl:
                t.start()

            for t in thread_hdl:
                t.join()
                thread_hdl.remove(t)
                #del t

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

    ipset=set()
    failedip=set()
    noresultip=set()
    usefulip=set()
    if valid_ip(host):
        re_hdl = re.compile("^.*IN[ \t]+PTR[\t ]+(.*)$", re.MULTILINE)
    else:
        re_hdl = re.compile("^.*IN[ \t]+A[\t ]+(.*)$", re.MULTILINE)

    if thread_result is not None and len(thread_result) > 0:
        for id,ip,status,msg in thread_result:
            if status != 0:
                print ip,status
                failedip.add(ip)
            else:
                m = re_hdl.findall(msg)
                if m is not None and len(m) > 0:
                    for dstip in m:
                        ipset.add(dstip)
                    #print ip + ": " + str(m)
                    usefulip.add(ip)
                else:
                    #print ip,msg
                    noresultip.add(ip)

    if len(ipset) > 0:
        resultfile=host + "_result.log"
        print "RESULT:[%d] %s" % (len(ipset), resultfile)
        fp = open(resultfile, "w")
        for ip in ipset:
            fp.write(ip + "\n")
            #print ip
        fp.close()

    print "-------------"
    if len(failedip) > 0:
        print "FAILED DNS:[%d]" % len(failedip)
        fp = open("fail_dns.txt", "w")
        for ip in failedip:
            fp.write(ip + "\n")
            #print ip
        fp.close()

    print "-------------"
    if len(noresultip) > 0:
        print "NO RESULT DNS:[%d]" % len(noresultip)
        fp = open("noresult_dns.txt", "w")
        for ip in noresultip:
            fp.write(ip + "\n")
            #print ip
        fp.close()

    print "-------------"
    if len(usefulip) > 0:
        print "VALID DNS:[%d]" % len(usefulip)
        fp = open("valid_dns.txt", "w")
        for ip in usefulip:
            fp.write(ip + "\n")
            #print ip
        fp.close()

    print "-------------"

    print "Exiting Main Thread"

