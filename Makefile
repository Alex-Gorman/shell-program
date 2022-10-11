PLATFORM = $(shell uname -s)
ARCH = $(shell uname -m)
CC = gcc
AR = ar
ARFLAGS = -rcs
CPPFLAGS = -std=gnu90 -Wall -Wextra -pedantic -g -D$(ARCH)
CFLAGS = -g

ifeq ($(PLATFORM), Linux)
	TARGETS = shell
endif

all: $(TARGETS)

shell: shell_program.o
	$(CC) -o shell shell_program.o

shell_program.o: shell.c
	$(CC) -o shell_program.o -c $(CFLAGS) $(CPPFLAGS) shell.c

clean:
	@echo "Cleaning Up..."
	@rm -f *.o
	@rm -f shell

