TEST = test/raw_soc_test \
	   test/raw_test

OBJS = raw/soc.o \
	   raw.o \
	   util.o

CFLAGS := -g -W -Wall -Wno-unused-parameter -I .

.PHONY: all clean

all: $(TEST)

$(TEST): % : %.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

.c.o:
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(TEST) $(TEST:=.o) $(OBJS)
