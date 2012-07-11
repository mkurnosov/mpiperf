#
# mpiperf top-level makefile.
#

topdir := $(PWD)
srcdir := $(topdir)/src

USERLIB_FLAGS :=
#USERLIB_FLAGS := -L $(topdir)/doc/usertest -l usertest

INCLUDES := -I $(srcdir)
CFLAGS := -Wall -std=c99 -O2 -g $(INCLUDES)
LDFLAGS := $(USERLIB_FLAGS) -lm

CC := gcc
LD := gcc
MPICC := mpicc


