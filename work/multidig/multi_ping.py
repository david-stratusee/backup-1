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
    def __init__(self, threadID, result, myip):
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.ip = myip.strip()
        self.result = result

    def run(self):
        digcmd="ping -t 3 %s" % self.ip
        status,msg = commands.getstatusoutput(digcmd)
        value = 0.0
        num = 0
        for line in msg.split('\n'):
            if line.find("time=") >= 0:
                arr = line.split()
                value += float(arr[6][5:])
                num += 1
        if num != 0:
            value /= num
        self.result[self.threadID] = (self.threadID, self.ip, status, value)
        #print digcmd, status


if __name__ == '__main__':

    if len(sys.argv) == 0:
        print "need one argument"
        exit(0)

    fp = open(sys.argv[1])
    lines = fp.readlines();
    fp.close()

    threadID = 1
    globalid = 0
    thread_hdl = []
    thread_result = []

    for myip in lines:
        thread = myThread(globalid, thread_result, myip)
        thread_hdl.append(thread)
        thread_result.insert(globalid, (globalid, myip, 0, 0.0))

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

    dic = {}
    if thread_result is not None and len(thread_result) > 0:
        for id,ip,status,value in thread_result:
            if value >= 1:
                dic[ip] = value

    for ip,value in sorted(dic.iteritems(), key=lambda d:d[1]):
        print "%s\t%.2f" % (ip, value)

    print "Exiting Main Thread"

