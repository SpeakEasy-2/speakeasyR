export ROOT_DIR := $(PWD)
export SRC_DIR := $(ROOT_DIR)/src

R_FILES := $(wildcard $(ROOT_DIR)/R/*.R)
HEADERS := $(wildcard $(ROOT_DIR)/src/include/*.h)
BUILD_FLAGS :=
VERSION := $(shell \
	grep -o "Version: [[:digit:]]\+\.[[:digit:]]\+\.[[:digit:]]\+" < DESCRIPTION | \
	sed 's/Version: //')

.PHONY: all
all: build

.PHONY: build
build: speakeasyR_$(VERSION).tar.gz

speakeasyR_$(VERSION).tar.gz: $(SRC_DIR)/speakeasyR.c $(R_FILES) $(HEADERS) configure
	R CMD build $(BUILD_FLAGS) $(ROOT_DIR)

.PHONY: check
check: build
	R CMD check --as-cran speakeasyR_$(VERSION).tar.gz

.PHONY: check-quick
check-quick: BUILD_FLAGS += --no-build-vignettes
check-quick: build
	R CMD check --no-build-vignettes \
		--no-examples \
		--no-tests speakeasyR_$(VERSION).tar.gz

.PHONY: clean
clean:
	@find src -name '*.o' -exec rm {} \;

.PHONY: clean-dist
clean-dist: clean
	rm -f speakeasyR_$(VERSION).tar.gz
	rm -rf speakeasyR.Rcheck
	rm -f $(SRC_DIR)/speakeasyR.so
	rm -f $(SRC_DIR)/speakeasyR.dll
	rm -rf vignettes/speakeasyr_files
	rm -f vignettes/.build.timestamp
	rm -f vignettes/speakeasyr.R
	rm -f $(SRC_DIR)/include/arith.h
