CC = gcc
CFLAGS = -g3 -c -m32 -lm
AR = ar -rc
RANLIB = ranlib

all: my_vm.a

my_vm.a: my_vm.o
	$(AR) libmy_vm.a *.o
	$(RANLIB) libmy_vm.a

my_vm.o: my_vm.c my_vm.h vm_utils.o scv.o
	$(CC) $(CFLAGS) my_vm.c

vm_utils.o: vm_utils.c vm_utils.h scv.o
	$(CC) $(CFLAGS) vm_utils.c

scv.o: scv.c scv.h
	$(CC) $(CFLAGS) scv.c

clean:
	rm -rf *.o *.a test
