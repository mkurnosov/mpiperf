#
# User library makefile.
#

lib := libusermpi.a
lib_objs := usermpi.o

all: $(lib)

$(lib): $(lib_objs)
	$(AR) rcs $@ $^

%.o: %.c
	mpicc -Wall -c $< -o $@

usermpi.o: usermpi.c

clean:
	@rm -rf $(lib) $(lib_objs)
