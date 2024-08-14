# speakeasyR 0.1.3

## Add

- Ability to interrupt speakeasy 2.

## Changed

- Update SE2 C library providing performance improvements.

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
