CFLAGS=-ansi -Wall -Werror -I .. -g -O0 --coverage
LDFLAGS=--coverage
DEPS=../pb_decode.h ../pb_encode.h ../pb.h ../pb_field.h person.pb.h callbacks.pb.h unittests.h unittestproto.pb.h alltypes.pb.h aligntype.pb.h
TESTS=test_decode1 test_encode1 test_encode2 test_decode2 test_encode3 test_decode3 test_encode4 test_decode4 decode_unittests encode_unittests test_encode_callbacks test_decode_callbacks

CC_VER := $(shell gcc --version | grep gcc)
ifneq "$(CC_VER)" ""
CFLAGS += -m32
LDFLAGS += -m32
endif
ifndef PB_PATH
PBPATHOPT=-I/usr/include -I/usr/local/include
else
PBPATHOPT=-I$(PB_PATH)
endif

all: breakpoints $(TESTS) run_unittests

clean:
	rm -f breakpoints $(TESTS) *.pb *.pb.c *.pb.h *.o *.gcda *.gcno

%.o: %.c
%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

pb_encode.o: ../pb_encode.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<
pb_decode.o: ../pb_decode.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<
pb_field.o: ../pb_field.c $(DEPS)
	$(CC) $(CFLAGS) -c -o $@ $<

test_decode1: test_decode1.o pb_decode.o pb_field.o person.pb.o
test_decode2: test_decode2.o pb_decode.o pb_field.o person.pb.o
test_decode3: test_decode3.o pb_decode.o pb_field.o alltypes.pb.o
test_decode4: test_decode4.o pb_decode.o pb_field.o aligntype.pb.o
test_encode1: test_encode1.o pb_encode.o pb_field.o person.pb.o
test_encode2: test_encode2.o pb_encode.o pb_field.o person.pb.o
test_encode3: test_encode3.o pb_encode.o pb_field.o alltypes.pb.o
test_encode4: test_encode4.o pb_encode.o pb_field.o aligntype.pb.o
test_decode_callbacks: test_decode_callbacks.o pb_decode.o pb_field.o callbacks.pb.o
test_encode_callbacks: test_encode_callbacks.o pb_encode.o pb_field.o callbacks.pb.o
decode_unittests: decode_unittests.o pb_decode.o pb_field.o unittestproto.pb.o
encode_unittests: encode_unittests.o pb_encode.o pb_field.o unittestproto.pb.o

%.pb: %.proto
	protoc -I. -I../generator $(PBPATHOPT) -o$@ $<

%.pb.c %.pb.h: %.pb ../generator/nanopb_generator.py
	python ../generator/nanopb_generator.py $<

breakpoints: ../*.c *.c
	grep -n 'return false;' $^ | cut -d: -f-2 | xargs -n 1 echo b > $@

coverage: run_unittests
	gcov pb_encode.gcda
	gcov pb_decode.gcda

run_unittests: $(TESTS)
	rm -f *.gcda
	
	./decode_unittests > /dev/null
	./encode_unittests > /dev/null
	
	[ "`./test_encode1 | ./test_decode1`" = \
	"`./test_encode1 | protoc --decode=Person -I. -I../generator $(PBPATHOPT) person.proto`" ]

	[ "`./test_encode2 | ./test_decode1`" = \
	"`./test_encode2 | protoc --decode=Person -I. -I../generator $(PBPATHOPT) person.proto`" ]

	[ "`./test_encode2 | ./test_decode2`" = \
	"`./test_encode2 | protoc --decode=Person -I. -I../generator $(PBPATHOPT) person.proto`" ]
	
	[ "`./test_encode_callbacks | ./test_decode_callbacks`" = \
	"`./test_encode_callbacks | protoc --decode=TestMessage callbacks.proto`" ]

	./test_encode3 | ./test_decode3
	./test_encode3 | protoc --decode=AllTypes -I. -I../generator $(PBPATHOPT) alltypes.proto >/dev/null

	./test_encode4 | ./test_decode4
	./test_encode4 | protoc --decode=AlignTypes -I. -I../generator $(PBPATHOPT) aligntype.proto >/dev/null

run_fuzztest: test_decode2
	bash -c 'I=1; while true; do cat /dev/urandom | ./test_decode2 > /dev/null; I=$$(($$I+1)); echo -en "\r$$I"; done'
