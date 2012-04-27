#
# mpiperf top-level makefile.
#

topdir := $(PWD)
srcdir := $(topdir)/src

#USERLIB_FLAGS := -L $(topdir)/examples/usermpi -l usermpi
USERLIB_FLAGS :=

INCLUDES := -I $(srcdir)
CFLAGS := -Wall -std=c99 -O2 -g $(INCLUDES)
LDFLAGS := $(USERLIB_FLAGS) -lm

CC := gcc
LD := gcc
MPICC := mpicc


