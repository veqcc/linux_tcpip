TEST = test/raw_soc_test \
	   test/raw_test \
	   test/ethernet_test

OBJS = raw/soc.o \
	   raw.o \
	   util.o \
	   ethernet.o \
	   net.o

CFLAGS := -g -W -Wall -Wno-unused-parameter -I . -pthread -DHAVE_PF_PACKET

.PHONY: all clean

all: $(TEST)

$(TEST): % : %.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(TEST) $(TEST:=.o) $(OBJS)
