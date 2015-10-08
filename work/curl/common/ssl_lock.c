#ifndef MACOS
#define USE_OPENSSL
#endif

#include <stdio.h>
#include <pthread.h>

/* we have this global to let the callback get easy access to it */
#if defined(USE_OPENSSL)

static pthread_mutex_t *lockarray;
#include <openssl/crypto.h>
static void lock_callback(int mode, int type, char *file, int line)
{
    (void)file;
    (void)line;

    if (mode & CRYPTO_LOCK) {
        pthread_mutex_lock(&(lockarray[type]));
    } else {
        pthread_mutex_unlock(&(lockarray[type]));
    }
}

static unsigned long thread_id(void)
{
    unsigned long ret;
    ret = (unsigned long)pthread_self();
    return (ret);
}

void init_locks(void)
{
    int i;
    lockarray = (pthread_mutex_t *)OPENSSL_malloc(CRYPTO_num_locks() *
                sizeof(pthread_mutex_t));

    for (i = 0; i < CRYPTO_num_locks(); i++) {
        pthread_mutex_init(&(lockarray[i]), NULL);
    }

    CRYPTO_set_id_callback((unsigned long(*)())thread_id);
    CRYPTO_set_locking_callback((void (*)())lock_callback);
}

void kill_locks(void)
{
    int i;
    CRYPTO_set_locking_callback(NULL);

    for (i = 0; i < CRYPTO_num_locks(); i++) {
        pthread_mutex_destroy(&(lockarray[i]));
    }

    OPENSSL_free(lockarray);
}
#elif defined(USE_GNUTLS)
#include <gcrypt.h>
#include <errno.h>

GCRY_THREAD_OPTION_PTHREAD_IMPL;

void init_locks(void)
{
    gcry_control(GCRYCTL_SET_THREAD_CBS);
}

void kill_locks(void)
{
    return;
}
#else
void init_locks(void)
{
    return;
}

void kill_locks(void)
{
    return;
}
#endif

