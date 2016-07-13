EXEC=hashish
EXECFLAGS=-Wall -std=c11
SOURCES=exec_hash.c
prefix=/usr/local

all: hashish

hashish:
	$(CC) $(EXECFLAGS) $(CFLAGS) -o $(EXEC) $(SOURCES) -lcrypto -O0

install: hashish
	install -m755 $(EXEC) $(DESTDIR)$(prefix)/bin/$(EXEC)

clean:
	rm -f hashish
