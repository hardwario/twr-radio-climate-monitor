SDK_DIR ?= sdk
TAG ?= dev

CFLAGS += -D'TAG="${TAG}"'

-include sdk/Makefile.mk

.PHONY: all
all: sdk
	@$(MAKE) -s debug

.PHONY: sdk
sdk:
	@if [ ! -f $(SDK_DIR)/Makefile.mk ]; then echo "Initializing Git submodules..."; git submodule update --init; fi

.PHONY: update
update: sdk
	@echo "Updating Git submodules..."; git submodule update --remote --merge
