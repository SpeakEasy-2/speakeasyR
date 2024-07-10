export ROOT_DIR := $(PWD)
export SRC_DIR := $(ROOT_DIR)/src

CFLAGS := -Wall -pedantic
FFLAGS := -Wall -pedantic

R_FILES := $(wildcard $(ROOT_DIR)/R/*.R)
C_FILES := $(wildcard $(ROOT_DIR)/src/*.c)
HEADERS := $(wildcard $(ROOT_DIR)/src/include/*.h)
ARPACK := $(wildcard $(ROOT_DIR)/src/arpack/*)
BUILD_FLAGS :=
VERSION := $(shell \
	grep -o "Version: [[:digit:]]\+\.[[:digit:]]\+\.[[:digit:]]\+" < DESCRIPTION | \
	sed 's/Version: //')
MAJOR_VERSION := $(shell echo "$(VERSION)" | sed -n 's/^\([0-9]\+\)\..*$$/\1/p')
MINOR_VERSION := $(shell \
	echo "$(VERSION)" | sed -n 's/^[0-9]\+\.\([0-9]\+\)\..*$$/\1/p')
PATCH_VERSION := $(shell \
	echo "$(VERSION)" | sed -n 's/^.*\.\([0-9]\+\)$$/\1/p')

.PHONY: all
all: build

.PHONY: build
build: speakeasyR_$(VERSION).tar.gz

speakeasyR_$(VERSION).tar.gz: $(SRC_DIR)/speakeasyR.c $(R_FILES) $(C_FILES)
speakeasyR_$(VERSION).tar.gz: $(HEADERS) configure $(SRC_DIR)/Makevars.in
speakeasyR_$(VERSION).tar.gz: $(SRC_DIR)/include/igraph_version.h $(ARPACK)
	CC="$(CC)" CFLAGS="$(CFLAGS)" FFLAGS="$(FFLAGS)" \
	  R CMD build $(BUILD_FLAGS) $(ROOT_DIR)

configure: configure.ac tools/config.guess tools/config.sub $(SRC_DIR)/include/config.h.in
	autoconf

$(SRC_DIR)/include/config.h.in tools/config.sub tools/config.guess:
	autoreconf -i

$(SRC_DIR)/include/igraph_version.h: $(SRC_DIR)/se2/vendor/igraph/include/igraph_version.h.in
	sed -e "s/\@PACKAGE_VERSION\@/$(VERSION)/g" \
	  -e "s/\@PACKAGE_VERSION_MAJOR\@/$(MAJOR_VERSION)/g" \
	  -e "s/\@PACKAGE_VERSION_MINOR\@/$(MINOR_VERSION)/g" \
	  -e "s/\@PACKAGE_VERSION_PATCH\@/$(PATCH_VERSION)/g" <$< >$@

.PHONY: check
check: build
	CC="$(CC)" CFLAGS="$(CFLAGS)" FFLAGS="$(FFLAGS)" \
	  R CMD check --as-cran speakeasyR_$(VERSION).tar.gz

.PHONY: check-deps
check-deps: build
	_R_CHECK_DEPENDS_ONLY_=true \
          R CMD check --as-cran speakeasyR_$(VERSION).tar.gz

.PHONY: check-quick
check-quick: BUILD_FLAGS += --no-build-vignettes
check-quick: build
	R CMD check --no-build-vignettes \
		--no-examples \
		--no-tests speakeasyR_$(VERSION).tar.gz

.PHONY: clean
clean:
	@find src -name '*.o' -not -path "*/tests/*" -exec rm {} \;
	rm -f configure~
	rm -f src/include/config.h.in~
	rm -f config.*

.PHONY: clean-dist
clean-dist: clean
	rm -f speakeasyR_*.tar.gz
	rm -rf speakeasyR.Rcheck
	rm -f $(SRC_DIR)/speakeasyR.so
	rm -f $(SRC_DIR)/speakeasyR.dll
	rm -rf vignettes/speakeasyr_files
	rm -f vignettes/.build.timestamp
	rm -f vignettes/speakeasyr.R
	rm -f src/include/config.h
	rm -f src/include/igraph_version.h
	rm -f src/Makevars
	rm -f configure
	rm -f tools/config.*
	rm -f src/include/config.h.in
