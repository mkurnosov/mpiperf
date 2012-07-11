#
# mpiperf makefile.
#
include Makefile.inc

package = mpiperf
version = $(shell cat ./VERSION)
distdir = $(package)-$(version)

SRC_DIR := src
TEST_DIR := test

.PHONY: all clean mpiperf test dist

all: mpiperf test

mpiperf:
	$(MAKE) -C $(SRC_DIR)

test: mpiperf
	cp -f $(SRC_DIR)/mpiperf $(TEST_DIR)

clean:
	$(MAKE) -C $(SRC_DIR) clean
	rm -f $(TEST_DIR)/mpiperf

dist: clean $(distdir).tar.gz

$(distdir).tar.gz: $(distdir)
	tar -chof - $(distdir) | gzip -9 -c > $@
	rm -rf $(distdir)

$(distdir):
	mkdir -p $@
	cp -R config $@
	cp -R doc $@
	cp -R examples $@
	cp -R maint $@
	cp -R src $@
	cp -R test $@
	cp -R AUTHORS COPYING Makefile Makefile.inc README INSTALL VERSION $@
    