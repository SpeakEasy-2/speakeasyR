#include <R.h>
#include <Rinternals.h>
#include <R_ext/Visibility.h>
#include <Matrix.h>

#include "igraph.h"
#include "speak_easy_2.h"

#define R_MATRIX(mat, i, j, vcount) REAL((mat))[(i) + ((j) * (vcount))]

// From Matrix_stubs.c. It's not exported which causes AS_CHM_SP to fail.
CHM_SP M_as_cholmod_sparse(CHM_SP ans, SEXP x,
                           Rboolean check_Udiag, Rboolean sort_in_place)
{
  static CHM_SP(*fun)(CHM_SP, SEXP, Rboolean, Rboolean) = NULL;
  if (fun == NULL)
    fun = (CHM_SP(*)(CHM_SP, SEXP, Rboolean, Rboolean))
          R_GetCCallable("Matrix", "as_cholmod_sparse");
  return fun(ans, x, check_Udiag, sort_in_place);
}

static bool se2_is_sparse(SEXP mat)
{
  const char* Matrix_valid_Csparse[] = { MATRIX_VALID_Csparse, ""};
  return R_check_class_etc(mat, Matrix_valid_Csparse) >= 0;
}

// Convert a matrix to an igraph graph. Returns n_nodes.
static int se2_matrix_to_igraph(SEXP mat, igraph_t* graph,
                                igraph_vector_t* weights,
                                igraph_bool_t is_directed)
{
  mat = PROTECT(coerceVector(mat, REALSXP));

  igraph_vector_int_t edges;
  int n_nodes = nrows(mat);

  igraph_integer_t n_edges = 0;
  for (int i = 0; i < n_nodes; i++) {
    for (int j = 0; j < n_nodes; j++) {
      n_edges += R_MATRIX(mat, i, j, n_nodes) != 0;
    }
  }

  igraph_vector_int_init(&edges, n_edges * 2);
  igraph_vector_init(weights, n_edges);

  n_edges = 0;
  for (int i = 0; i < n_nodes; i++) {
    for (int j = 0; j < n_nodes; j++) {
      if (R_MATRIX(mat, i, j, n_nodes) != 0) {
        VECTOR(*weights)[n_edges / 2] = R_MATRIX(mat, i, j, n_nodes);
        VECTOR(edges)[n_edges++] = i;
        VECTOR(edges)[n_edges++] = j;
      }
    }
  }
  UNPROTECT(1);

  igraph_create(graph, &edges, n_nodes, is_directed);
  igraph_vector_int_destroy(&edges);

  return n_nodes;
}

/* Convert a sparse matrix to an igraph graph. Returns n_nodes.

Currently assumes:
    itype == int,
    xtype == real,
    dtype == double,
    packed == TRUE

SE2 only works on real graph so xtype should have to be real. The others
assumptions will need to be removed. */
static int se2_spmatrix_to_igraph(SEXP mat, igraph_t* graph,
                                  igraph_vector_t* weights,
                                  igraph_bool_t is_directed)
{
  CHM_SP mat_i = AS_CHM_SP(mat);

  igraph_vector_int_t edges;
  int n_nodes = mat_i->nrow;
  int n_edges = mat_i->nzmax;
  int* cols = (int*)mat_i->p;
  int* rows = (int*)mat_i->i;
  double* values = (double*)mat_i->x;

  igraph_vector_int_init(&edges, n_edges * 2);
  igraph_vector_init(weights, n_edges);

  int count = 0;
  for (int j = 0; j < mat_i->ncol; j++) {
    for (int i = cols[j]; i < cols[j + 1]; i++) {
      VECTOR(*weights)[count / 2] = values[i];
      VECTOR(edges)[count++] = rows[i];
      VECTOR(edges)[count++] = j;
    }
  }

  igraph_create(graph, &edges, n_nodes, is_directed);
  igraph_vector_int_destroy(&edges);

  return n_nodes;
}

SEXP call_speakeasy2(SEXP adj, SEXP discard_transient, SEXP independent_runs,
                     SEXP max_threads, SEXP seed, SEXP target_clusters,
                     SEXP target_partitions, SEXP subcluster, SEXP min_clust,
                     SEXP verbose, SEXP is_directed)
{
  igraph_t graph;
  igraph_vector_t weights;
  igraph_matrix_int_t membership_i;
  int n_nodes = 0;
  int n_levels = asInteger(subcluster);
  SEXP membership;

  se2_options opts = {
    .discard_transient = asInteger(discard_transient),
    .independent_runs = asInteger(independent_runs),
    .max_threads = asInteger(max_threads),
    .minclust = asInteger(min_clust),
    .subcluster = asInteger(subcluster),
    .random_seed = asInteger(seed),
    .target_clusters = asInteger(target_clusters),
    .target_partitions = asInteger(target_partitions),
    .verbose = asLogical(verbose)
  };

  if (se2_is_sparse(adj)) {
    n_nodes = se2_spmatrix_to_igraph(adj, &graph, &weights,
                                     asLogical(is_directed));
  } else {
    n_nodes = se2_matrix_to_igraph(adj, &graph, &weights,
                                   asLogical(is_directed));
  }

  speak_easy_2(&graph, NULL, &opts, &membership_i);

  PROTECT(membership = allocMatrix(INTSXP, n_levels, n_nodes));
  for (int i = 0; i < n_levels; i++) {
    for (int j = 0; j < n_nodes; j++) {
      INTEGER(membership)[i + (j * n_levels)] = MATRIX(membership_i, i, j);
    }
  }
  UNPROTECT(1);

  igraph_matrix_int_destroy(&membership_i);
  igraph_vector_destroy(&weights);
  igraph_destroy(&graph);

  return membership;
}

static const R_CallMethodDef callMethods[] = {
  {"speakeasy2", (DL_FUNC) &call_speakeasy2, 11},
  {NULL, NULL, 0}
};

void attribute_visible R_init_speakeasyr(DllInfo* info)
{
  R_registerRoutines(info, NULL, callMethods, NULL, NULL);
  R_useDynamicSymbols(info, FALSE);
  R_forceSymbols(info, TRUE);
}
