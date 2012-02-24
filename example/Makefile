CFLAGS=-ansi -Wall -Werror -I .. -g -O0
DEPS=../pb_decode.c ../pb_decode.h ../pb_encode.c ../pb_field.c ../pb_encode.h ../pb.h ../pb_field.h

CC_VER := $(shell gcc --version | grep gcc)
ifneq "$(CC_VER)" ""
CFLAGS += -m32
endif
ifndef PB_PATH
PBPATHOPT=-I/usr/include -I/usr/local/include
else
PBPATHOPT=-I$(PB_PATH)
endif

all: server client

clean:
	rm -f server client fileproto.pb.c fileproto.pb.h fileproto.pb

%: %.c $(DEPS) fileproto.pb.h fileproto.pb.c
	$(CC) $(CFLAGS) -o $@ $< ../pb_decode.c ../pb_encode.c ../pb_field.c fileproto.pb.c common.c

fileproto.pb.c fileproto.pb.h: fileproto.proto ../generator/nanopb_generator.py
	protoc -I. -I../generator $(PBPATHOPT) -ofileproto.pb $<
	python ../generator/nanopb_generator.py fileproto.pb
