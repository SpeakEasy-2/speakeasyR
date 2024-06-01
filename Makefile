export ROOT_DIR := $(PWD)
export SRC_DIR := $(ROOT_DIR)/src

R_FILES := $(wildcard $(ROOT_DIR)/R/*.R)
HEADERS := $(wildcard $(ROOT_DIR)/src/include/*.h)
VERSION := $(shell \
	grep -o "Version: [[:digit:]]\+\.[[:digit:]]\+\.[[:digit:]]\+" < DESCRIPTION | \
	sed 's/Version: //')

.PHONY: all
all: build

.PHONY: build
build: speakeasyR_$(VERSION).tar.gz

speakeasyR_$(VERSION).tar.gz: $(SRC_DIR)/speakeasyR.c $(R_FILES) $(HEADERS) configure
	R CMD build $(ROOT_DIR)

.PHONY: check
check: build
	R CMD check --as-cran speakeasyR_$(VERSION).tar.gz

.PHONY: check-quick
check-quick: build
	R CMD check --no-build-vignettes \
		--no-examples \
		--no-tests speakeasyR_$(VERSION).tar.gz

.PHONY: clean
clean:
	find src -name '*.o' -exec rm {} \;

.PHONY: clean-dist
clean-dist: clean
	[ -f speakeasyR_$(VERSION).tar.gz ] && rm speakeasyR_$(VERSION).tar.gz
	[ -d speakeasyR.Rcheck ] && rm -rf speakeasyR.Rcheck
	[ -f $(SRC_DIR)/speakeasyR.so ] && rm $(SRC_DIR)/speakeasyR.so
	[ -f $(SRC_DIR)/speakeasyR.dll ] && rm $(SRC_DIR)/speakeasyR.dll
