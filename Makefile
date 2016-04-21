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
	$(CC) -o testcases/own testcases/own.c

clean:
	$(RM) $(LIBRARY) $(LIB_OBJ)
	$(RM) ./testcases/own
	$(RM) ./testcases/rvm_dir/*
	$(CP) ./testcases/new1.txt testcases/rvm_dir/.
	$(CP) ./testcases/new2.txt testcases/rvm_dir/.
