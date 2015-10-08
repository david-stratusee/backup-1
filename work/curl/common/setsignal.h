#ifndef _SETSIGNAL_H
#define _SETSIGNAL_H

#include <signal.h>

void (*setsignal(int signum, void (*sighandler) (int, siginfo_t *, void *))) (int);
const char *signal_id2name(int id);
void pthread_set_ignore_sig(int *sig_array, int sig_num);

#endif    // _SETSIGNAL_H

