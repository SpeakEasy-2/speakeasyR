# speakeasyR 0.1.4

## Add

- A high level function for clustering genes based on gene expression.

## Changed

- Update libSE2 to 0.1.8.

## Fixed

- Replace deprecated use of `CHARACTER*len` in arpack.

# speakeasyR 0.1.3

## Add

- Ability to interrupt speakeasy 2.
- Release built tarball for manual installation.
- Add explicit test for reproducible results.

## Changed

- Update SE2 C library providing performance improvements.
- Tests to be based on quality of clustering instead of exact match to previous run. Since SE2 is stochastic, tweaking the underlying algorithm can change the resulting membership vector even when using a random seed. Even if the membership vector is not identical it should be similar based on normalized mutual information.

## Fixed

- Clean up generated module files when compiling using intel's fortran compiler.

# speakeasyR 0.1.2

## Add

- Contributors of vendored software to DESCRIPTION.
- Support for external ARPACK

## Changed

- Use autoconf to generate configure file in order to detect system packages and support for pthreads.
- Remove support for compiling internal linear algebra packages and no longer ship source code for those packages.

## Fixed

- Prevent unused igraph vendored packages from being shipped in the source tarball.
- Update SE2 C library to fix address sanitizer errors.

# speakeasyR 0.1.1

## Fixed

- Guard against missing suggested packages in vignettes, tests, and examples.

# speakeasyR 0.1.0

## Add

- Set up as R package
- Cluster
- Order nodes
- Knn graph
