#
# User library makefile.
#

lib := libusertest.a
lib_objs := usertest.o

all: $(lib)

$(lib): $(lib_objs)
	$(AR) rcs $@ $^

%.o: %.c
	mpicc -Wall -c $< -o $@

usertest.o: usertest.c

clean:
	@rm -rf $(lib) $(lib_objs)
