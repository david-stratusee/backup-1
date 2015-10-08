AUTOMAKE_OPTIONS = foreign
#bin_PROGRAMS = http_server http_client
bin_PROGRAMS = http_client http_client_ori
DEFS = @DEFS@

http_client_ori_SOURCES = http_client_ori.c
http_client_ori_CFLAGS = -std=c11 $(LIBCURL_CFLAGS)
http_client_ori_LDADD = $(LIBCURL_LIBS)

http_client_SOURCES = http_client.c
http_client_CFLAGS = -std=c11 $(LIBCURL_CFLAGS)
http_client_LDADD = $(LIBCURL_LIBS)

MAINTAINERCLEANFILES = $(srcdir)/*~ $(srcdir)/config.h.in $(srcdir)/Makefile.in $(srcdir)/aclocal.m4 $(srcdir)/configure $(srcdir)/depcomp $(srcdir)/install-sh $(srcdir)/missing
