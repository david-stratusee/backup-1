#FLAGS=-Wall -Wextra -I. -O2
FLAGS=-I. -I./common -I./adt -O2 -DUSE_THREAD_LOCK -DDEBUG
all: dns_proxy
.PHONY : clean

dns_proxy: dns_proxy.c adt/ac_bm.c common/memstr_func.c common/misc.c common/strcasestr_func.c common/string_s.c common/timestamp.c adt/hash.c
	gcc $(FLAGS) -o dns_proxy \
		dns_proxy.c	             \
		adt/ac_bm.c              \
		adt/hash.c               \
		common/memstr_func.c     \
		common/misc.c            \
		common/strcasestr_func.c \
		common/string_s.c        \
		common/timestamp.c

clean :
	@rm -f dns_proxy

install:
	@make
	@cp -f dns_proxy dns_proxy.sh ~/bin/
	@cp -f dns_proxy.conf ~/.dns_proxy.conf
	@sudo cp -f resolv_new.conf /usr/local/etc/dns_proxy_resolv.conf
	@echo install OK

uninstall:
	@sudo rm -f ~/bin/dns_proxy ~/bin/dns_proxy.sh ~/.dns_proxy.conf /usr/local/etc/dns_proxy_resolv.conf
	@echo uninstall OK
