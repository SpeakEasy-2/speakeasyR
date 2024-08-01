AC_INIT([speakeasyR],[0.1.2])

AC_LANG(C)
AC_CONFIG_AUX_DIR([tools])

AC_CONFIG_HEADERS([src/include/config.h])
AC_CONFIG_FILES([src/Makevars])

AH_TOP([#ifndef IGRAPH_PRIVATE_CONFIG_H
#define IGRAPH_PRIVATE_CONFIG_H

#include "igraph_config.h"

#define HAVE_STRCASECMP 1
#define HAVE_STRNCASECMP 1

#define HAVE_BUILTIN_OVERFLOW 1

#if defined(__GNUC__) && defined(__SIZEOF_INT128__)
#  define HAVE___UINT128_T 1
#endif])

AH_BOTTOM([#endif])

m4_include([tools/ax_pthread.m4])

: ${R_HOME="`R RHOME`"}
if test -z "${R_HOME}"; then
  AC_MSG_ERROR([cannot determine R_HOME. Make sure you use R CMD INSTALL!])
fi
RBIN="${R_HOME}/bin/R"
: ${CC=$("${RBIN}" CMD config CC)}

ROOT=$(
    cd "$(dirname "$0")" || exit
    pwd
)
SRC="${ROOT}/src"
SE2="${SRC}/se2"
IGRAPH="${SE2}/vendor/igraph"
ARPACK="${SRC}/arpack"
REL_IGRAPH=".${IGRAPH#"$SRC"}"

# Collects C and fortran files. If C++ needed, extend patterns.
collect_objs() {
    cd "$SRC" || exit
    obj_path="${1#"$SRC"}"
    find ".$obj_path" -name '*.c' -or -name '*.f' | sed -n 's/.$/o/p' | tr '\n' ' '
}

SE2_CPPFLAGS="-DUSING_R"
SE2_LIBS="-lm"

dnl speakeasyR object
SE2_OBJECTS="speakeasyR.o"

IGRAPH_OBJS="${REL_IGRAPH}/src/core/interruption.o \
    ${REL_IGRAPH}/src/core/vector.o \
    ${REL_IGRAPH}/src/core/memory.o \
    ${REL_IGRAPH}/src/core/indheap.o \
    ${REL_IGRAPH}/src/core/matrix.o \
    ${REL_IGRAPH}/src/core/vector_ptr.o \
    ${REL_IGRAPH}/src/core/vector_list.o \
    ${REL_IGRAPH}/src/core/printing.o \
    ${REL_IGRAPH}/src/core/error.o \
    ${REL_IGRAPH}/src/core/sparsemat.o \
    ${REL_IGRAPH}/src/graph/type_indexededgelist.o \
    ${REL_IGRAPH}/src/graph/iterators.o \
    ${REL_IGRAPH}/src/graph/type_common.o \
    ${REL_IGRAPH}/src/graph/caching.o \
    ${REL_IGRAPH}/src/graph/attributes.o \
    ${REL_IGRAPH}/src/internal/hacks.o \
    ${REL_IGRAPH}/src/internal/qsort.o \
    ${REL_IGRAPH}/src/internal/qsort_r.o \
    ${REL_IGRAPH}/src/math/complex.o \
    ${REL_IGRAPH}/src/math/utils.o \
    ${REL_IGRAPH}/src/properties/degrees.o \
    ${REL_IGRAPH}/src/constructors/basic_constructors.o \
    ${REL_IGRAPH}/src/community/community_misc.o \
    ${REL_IGRAPH}/src/linalg/arpack.o"

dnl collect SE2 objects
SE2_OBJECTS="${SE2_OBJECTS} $(collect_objs "${SE2}/src") ${IGRAPH_OBJS} $(collect_objs ${IGRAPH}/src/random) $(collect_objs ${IGRAPH}/vendor/cs) $(collect_objs ${IGRAPH}/vendor/pcg)"

AC_ARG_ENABLE([parallel], AS_HELP_STRING([--disable-parallel], [disable running with parallel threads]))
AS_IF([test "x$enable_parallel" != "xno"], [
  AX_PTHREAD([SE2_CPPFLAGS="${SE2_CPPFLAGS} -DSE2PAR"
    AC_DEFINE([IGRAPH_F77_SAVE], [static __thread], Thread local storage.)
    AC_DEFINE([IGRAPH_THREAD_LOCAL], [__thread], Thread local storage.)
  ], AC_MSG_NOTICE([POSIX threads not supported. Not configuring for multithreading.]))
])
SE2_CPPFLAGS="$PTHREAD_CFLAGS ${SE2_CPPFLAGS}"
SE2_LIBS="$PTHREAD_LIBS ${SE2_LIBS}"

SE2_CPPFLAGS="${SE2_CPPFLAGS}  -I\"${SRC}/include\" -I\"${SE2}/include\" -I\"${IGRAPH}/include\" -I\"${IGRAPH}/src\" -I\"${IGRAPH}/vendor\""

ARPACK_OBJECTS="$(collect_objs "$ARPACK")"

# Always false now.
AH_TEMPLATE([INTERNAL_BLAS], [Use internal blas library.])
AH_TEMPLATE([INTERNAL_LAPACK], [Use internal lapack library.])
AH_TEMPLATE([INTERNAL_ARPACK], [Use internal arpack library.])

SE2_LIBS="${SE2_LIBS} \$(LAPACK_LIBS) \$(BLAS_LIBS) \$(FLIBS)"
AC_ARG_ENABLE([system_arpack], AS_HELP_STRING([--disable-system-arpack],
  [compile and use the internal arpack even if an external version is found.]))

AS_IF([test "x$enable_system_arpack" == "xno"], [
    AC_MSG_NOTICE([using shipped arpack])
    SE2_OBJECTS="${SE2_OBJECTS} ${ARPACK_OBJECTS}"
  ], [
    PKG_CHECK_MODULES([ARPACK], [arpack], [
    SE2_CPPFLAGS="${SE2_CPPFLAGS} ${ARPACK_CFLAGS}"
    SE2_LIBS="${SE2_LIBS} ${ARPACK_LIBS}"], [
    AC_MSG_NOTICE([arpack not found; using shipped arpack])
    SE2_OBJECTS="${SE2_OBJECTS} ${ARPACK_OBJECTS}"])
  ])

dnl Respect user specified options.
SE2_CPPFLAGS="${SE2_CPPFLAGS} ${CPPFLAGS}"
SE2_LIBS="${SE2_LIBS} ${LDFLAGS}"
SE2_CFLAGS="${SE2_CFLAGS} ${CFLAGS}"
SE2_FFLAGS="${SE2_FFLAGS} ${FFLAGS}"

AC_SUBST(SE2_CPPFLAGS)
AC_SUBST(SE2_LIBS)
AC_SUBST(SE2_CFLAGS)
AC_SUBST(SE2_FFLAGS)
AC_SUBST(SE2_OBJECTS)

AC_OUTPUT
