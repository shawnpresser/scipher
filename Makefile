PREFIX = /usr/local
MAKE = gcc

scipher_DRIVER = lib/crypto/crypto_scrypt-sse.c
#
# Uncomment the following line to disable SSE.
#
#scipher_DRIVER = lib/crypto/crypto_scrypt-nosse.c

scipher_SOURCES = main.c				\
			lib/b64/b64.c		\
			lib/util/memlimit.c		\
			lib/util/readpass.c		\
			lib/util/warn.c			\
			lib/scryptenc/scryptenc.c	\
			lib/scryptenc/scryptenc_cpuperf.c	\
			lib/crypto/crypto_aesctr.c	\
			lib/crypto/sha256.c		\
			$(scipher_DRIVER)

scipher_CFLAGS = -g -O2 -I . -I lib/b64 -I lib/util -I lib/scryptenc -I lib/crypto
scipher_LDADD = -lcrypto -lrt
scipher:
	$(MAKE) $(scipher_CFLAGS) $(scipher_LDADD) $(scipher_SOURCES) -DHAVE_CONFIG_H -o scipher

all: scipher

clean:
	rm -f ./scipher

install: scipher
	install -m 0755 ./scipher $(PREFIX)/bin

uninstall:
	rm -f $(PREFIX)/bin/scipher

.PHONY: all clean install uninstall

