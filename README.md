# speakeasyR Community Detection
  [![R-CMD-check](https://github.com/SpeakEasy-2/speakeasyR/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/SpeakEasy-2/speakeasyR/actions/workflows/R-CMD-check.yaml)

This packages provides R functions for running the SpeakEasy 2 community detection algorithm using the [SpeakEasy2 C library](https://github.com/speakeasy-2/libspeakeasy2). See the [Genome Biology article](https://genomebiology.biomedcentral.com/articles/10.1186/s13059-023-03062-0).

SpeakEasy 2 (SE2) is a graph community detection algorithm that aims to be performant on large graphs and robust, returning consistent results across runs. SE2 does not require precognition about the number of communities in the network. Additionally, while the user can provide parameters to alter how the algorithm is run, the default option work well on a wide arrange of graphs and tweaking options generally has little affect on the results, reducing the risk of influencing the algorithm.

The core algorithm is written in C, providing speed and keeping the memory requirements low. This implementation can take advantage of multiple computing cores without increasing memory usage. SE2 can detect community structure across scales, making it a good choice for biological data, which is often organized hierarchical structure.

Graphs can be passed to the algorithm as adjacency matrices using the `Matrix` library, `igraph` graphs, or any data that can coerced into a matrix.

## Installation

For most users, this package should be installed from CRAN using:

``` r
install.packages("speakeasyR")
```

It can also be installed using `devtools`:

``` r
devtools::install_github("speakeasy-2/speakeasyR")
```

### Linux

Installation with `devtools::install_github` has been tested in clean VMs running Ubuntu and Fedora.

### Windows

To set up the development environment on Windows, install the appropriate version of [Rtools](https://cran.r-project.org/bin/windows/Rtools/) for your R install. Using Rtools' MSYS2, install the required build tools. This has been tested with ucrt64 environment but likely works in other environments.

```bash
pacman -S mingw-w64-ucrt-x86_64-toolchain \
	mingw-ucrt-w64-x86_64-libxml2 git
```

## Building from source

For development, clone this repository and use:

```bash
git submodule update --init --recursive
```

To set up the vendored dependencies.

For development `astyle` is recommended for formatting C code while `texlive`/`latex`, `qpdf`, and `checkbashims` are expected by `R` for building the documentation and checking shell scripts during the `R CMD build` process.

It should now be possible to run `devtools::load_all()` in `R`.

## Development

GNU autotools is used to generate the configuration script and files needed to run the configuration script. `R`'s build commands do not run `autoconf` instead, if changes are made to the `configuration.ac` file, `autoconf` (and possibly `autoreconf -i`) needs to be run and manually and the resulting files should be committed along with the source `configuration.ac` file.
The `Makefile` can determine when the `autoconf` programs need to be run by either directly calling the configure target (i.e. `make configure`) or running a build target (i.e. `make build` or `make check` or similar).
