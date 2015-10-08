#!/usr/bin/env python

from sys import argv, stdout, stderr
from termcolor import colored, cprint

def color_print_usage(proc_name):
    print 'Usage:\n    %s [-n] color string' % proc_name
    print '    color can be: red, blue, cyan, green, yellow, magenta, white'
    print '    -n means newline'
    return

def main(argv=argv):
    # argv[1]: color -- red, blue, cyan, green, yellow, magenta, white
    # argv[2]: print string
    # argv[3]: stdout or stderr

    # colored(text, color)
    # cprint(text, color)

    #cprint(argv[2], argv[1])

    arg_idx = 1
    argc = len(argv)

    if argc < 3:
        color_print_usage(argv[0])
        return 1

    if argv[arg_idx] == '-h':
        color_print_usage(argv[0])
        return 0

    newline = False
    if argv[arg_idx] == '-n':
        newline = True
        if argc < 4:
            color_print_usage(argv[0])
            return 1
        arg_idx += 1

    color = argv[arg_idx]
    arg_idx += 1

    show_str = argv[arg_idx]
    arg_idx += 1

    show_error = False
    if arg_idx < argc and argv[arg_idx] == 'error':
        show_error = True

    text = colored(show_str, color)

    if not show_error:
        stdout.write(text)
        if newline:
            stdout.write('\n')
        stdout.flush()
    else:
        stderr.write(text)
        if newline:
            stderr.write('\n')
        stderr.flush()
    return 0

if __name__ == "__main__":
    exit(main())
