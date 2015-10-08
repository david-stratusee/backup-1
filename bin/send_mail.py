#!/usr/bin/python

from email.header import Header
from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart
import smtplib, sys, os, urllib, commands, time

def send_func(fullpath, local_convert):
    subfix=os.path.splitext(fullpath)[-1]
    need_delete = False

    if subfix == '.epub' or subfix != ".mobi" and local_convert:
        conv_command='/Applications/calibre.app/Contents/MacOS/ebook-convert \"%s\" \".mobi\"' % fullpath
        conv_command += ' --output-profile kindle_pw3 --mobi-keep-original-images --max-toc-links 500'
        #conv_command += ' -v'
        print "[%s] start: %s" % (time.asctime(time.localtime(time.time())), conv_command)
        try:
            status,result = commands.getstatusoutput(conv_command)
            print "[%s] finish: %s" % (time.asctime(time.localtime(time.time())), conv_command)

            if status != 0:
                print result
                return 1
        except KeyboardInterrupt, e1:
            print '\nbreak when execute command: ' + str(e1)
            print "[%s] finish: %s" % (time.asctime(time.localtime(time.time())), conv_command)
            return 1
        except Exception, e:
            print 'error when execute command: ' + str(e)
            print "[%s] finish: %s" % (time.asctime(time.localtime(time.time())), conv_command)
            return 1

        fullpath = fullpath.replace(subfix, '.mobi')
        subfix = '.mobi'
        need_delete = True

    basefile=os.path.basename(fullpath)
    msg = MIMEMultipart()

    try:
        fp = open(fullpath, 'rb')
        att1 = MIMEText(fp.read(), 'base64', 'utf-8')
        fp.close()
        if need_delete:
            os.remove(fullpath)
    except Exception, e:
        if need_delete:
            os.remove(fullpath)
        print 'error when create attachment: ' + str(e)
        return 1

    att1["Content-Type"] = 'application/octet-stream'
    att1["Content-Disposition"] = 'attachment; filename=\"%s\"' % urllib.quote(basefile, ' \t')
    msg.attach(att1)

    print att1["Content-Disposition"]

    msg["Accept-Language"]="zh-CN"
    msg["Accept-Charset"]="ISO-8859-1,utf-8"
    msg['to'] = 'crazyman80@kindle.cn'
    msg['from'] = 'dengwei98406@163.com'
    if subfix == ".mobi":
        msg['subject'] = "send %s" % urllib.quote(basefile, ' \t')
    else:
        msg['subject'] = "convert"

    print "from %s to %s" % (msg['from'], msg['to'])
    print "subject: %s, filename: %s" % (msg['subject'], basefile)
    print "[%s] begin to send file %s" % (time.asctime(time.localtime(time.time())), fullpath)

    try:
        server = smtplib.SMTP('smtp.163.com')
        #server.ehlo()
        #server.starttls()
        #server.ehlo()
        server.login('dengwei98406@163.com','torrent')
        server.sendmail(msg['from'], msg['to'], msg.as_string())
        #server.quit()
        server.close()
        print "[%s] OK" % time.asctime(time.localtime(time.time()))

        return 0
    except Exception, e:
        print 'error when send file: ' + str(e)
        return 1

def send_dir_func(fullpath, local_convert):
    status,result= commands.getstatusoutput('ls %s' % fullpath)
    if status != 0:
        print 'list directory error, %s' % fullpath
        return 1

    for f in result.split('\n'):
        fpath = fullpath + '/' + f
        if os.path.isfile(fpath):
            deal_ret = send_func(fpath, local_convert)
            print "deal with %s return %d" % (fpath, deal_ret)

    return 0


def main(argv=sys.argv):
    local_convert = True
    argc = len(argv)

    if len(argv) <= 1:
        print "need one filename as attachment, -c for remote convert"
        return 0

    arg_index = 1
    if argv[arg_index] == '-c':
        local_convert = False
        arg_index += 1;

    if argc == arg_index:
        print 'need file argument'
        return 1

    while arg_index < argc:
        if os.path.isfile(argv[arg_index]):
            deal_ret = send_func(argv[arg_index], local_convert)
            print "deal with %s return %d" % (argv[arg_index], deal_ret)
        elif os.path.isdir(argv[arg_index]):
            deal_ret = send_dir_func(argv[arg_index], local_convert)
        else:
            print '%s is error path' % argv[arg_index]

        arg_index += 1

    return 0

if __name__ == "__main__":
    exit(main())
