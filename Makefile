#
# mpiperf makefile.
#

include Makefile.inc

SRC_DIR := src
TEST_DIR := test

.PHONY: all clean mpiperf test

all: mpiperf test

mpiperf:
	@$(MAKE) -C $(SRC_DIR)

test: mpiperf
	@cp -f $(SRC_DIR)/mpiperf $(TEST_DIR)

clean:
	@$(MAKE) -C $(SRC_DIR) clean
