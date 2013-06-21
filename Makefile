PREFIX = /usr/local
MAKE = gcc

scipher_SOURCES = main.c				\
			lib/b64/b64.c		\
			lib/util/memlimit.c		\
			lib/util/readpass.c		\
			lib/util/warn.c			\
			lib/scryptenc/scryptenc.c	\
			lib/scryptenc/scryptenc_cpuperf.c	\
			lib/crypto/crypto_aesctr.c	\
			lib/crypto/sha256.c		\
			lib/crypto/crypto_scrypt-sse.c

scipher_CFLAGS = -I . -I lib/b64 -I lib/util -I lib/scryptenc -I lib/crypto -g
scipher_LDADD = -lcrypto
scipher:
	$(MAKE) $(scipher_CFLAGS) $(scipher_LDADD) $(scipher_SOURCES) -DHAVE_CONFIG_H -o scipher

all: scipher

clean:
	rm -f ./scipher

install: scipher
	install -m 0755 ./scipher $(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)/bin/scipher

.PHONY: all install uninstall

