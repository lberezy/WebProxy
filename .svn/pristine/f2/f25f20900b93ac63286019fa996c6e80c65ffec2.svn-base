CC = gcc
CFLAGS = -Wall -std=c99 -g -O0 -Isrc
LDFLAGS = -lpthread -D_POSIX_C_SOURCE -D_POSIX_THREADS


UNAME_S = $(shell uname)
ifeq ($(UNAME_S),SunOS)
		LDFLAGS += -lsocket -lnsl
endif

TARGETS = bin/proxy
TESTS = tests/test_log tests/test_proxy tests/test_thread
TARGETS += $(TESTS)

all: $(TARGETS)
tests: $(TESTS)

bin/proxy: src/proxy.o src/log.o src/http.o src/thread_pool.o src/main.o
	$(CC) $(CFLAGS) $(LDFLAGS) src/thread_pool.o src/proxy.o src/log.o src/http.o src/main.o -o bin/proxy

src/%: src/%.c
src/main: src/main.c
src/log: src/log.c
src/http: src/http.c
src/proxy: src/proxy.c src/log.o src/http.o
src/thread_pool: src/thread_pool.c

tests/test_log: tests/test_log.o src/log.o
tests/test_proxy: tests/test_proxy.o src/proxy.o src/http.o src/log.o
tests/test_thread: src/thread_pool.o

clean:
	rm -f $(TARGETS) *~ */*~ */*.o */*.log
	rm -rf */*.dSYM