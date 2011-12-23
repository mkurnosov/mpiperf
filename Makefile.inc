#
# mpiperf top-level makefile.
#

topdir := $(PWD)
srcdir := $(topdir)/src

INCLUDES := -I $(srcdir)
CFLAGS := -Wall -std=c99 -O2 -g $(INCLUDES)
LDFLAGS := -lm

CC := gcc
LD := gcc
MPICC := mpicc


