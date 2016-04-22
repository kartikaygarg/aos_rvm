#### RVM Library Makefile

CFLAGS  = -Wall -g -I.
LFLAGS  =
CC      = g++
RM      = /bin/rm -rf
CP 	= /bin/cp
AR      = ar rc
RANLIB  = ranlib

LIBRARY = librvm.a

LIB_SRC = rvm.cpp

LIB_OBJ = $(patsubst %.cpp,%.o,$(LIB_SRC))

%.o: %.cpp
	$(CC) -c $(CFLAGS) $< -o $@

$(LIBRARY): $(LIB_OBJ)
	$(AR) $(LIBRARY) $(LIB_OBJ)
	$(RANLIB) $(LIBRARY)
	#$(CC) -g -o testcases/own testcases/own.c
	$(CC) -g -o testcases/basic testcases/basic.c
	$(CC) -g -o testcases/abort testcases/abort.c
	$(CC) -g -o testcases/multi testcases/multi.c
	$(CC) -g -o testcases/multi-abort testcases/multi-abort.c
	$(CC) -g -o testcases/truncate testcases/truncate.c

clean:
	$(RM) $(LIBRARY) $(LIB_OBJ)
	#$(RM) ./testcases/own
	$(RM) ./testcases/basic
	$(RM) ./testcases/abort
	$(RM) ./testcases/multi
	$(RM) ./testcases/multi-abort
	$(RM) ./testcases/truncate
	#$(RM) ./testcases/rvm_dir/*
	$(RM) ./testcases/rvm_segments/*
	#$(CP) ./testcases/new1.txt testcases/rvm_dir/.
	#$(CP) ./testcases/new2.txt testcases/rvm_dir/.
