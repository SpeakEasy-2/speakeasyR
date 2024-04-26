#include <R.h>
#include <Rinternals.h>
#include <R_ext/Visibility.h>
#include <Matrix/Matrix.h>

#include "igraph.h"
#include "speak_easy_2.h"

#define R_MATRIX(mat, i, j, vcount) REAL((mat))[(i) + ((j) * (vcount))]

R_MATRIX_INLINE CHM_SP M_sexp_as_cholmod_sparse(CHM_SP A, SEXP from,
    Rboolean checkUnit, Rboolean sortInPlace)
{
  static CHM_SP(*fn)(CHM_SP, SEXP, Rboolean, Rboolean) = NULL;
  if (!fn)
    fn = (CHM_SP(*)(CHM_SP, SEXP, Rboolean, Rboolean))
         R_GetCCallable("Matrix", "sexp_as_cholmod_sparse");
  return fn(A, from, checkUnit, sortInPlace);
}

static bool se2_is_sparse(SEXP mat)
{
  const char* Matrix_valid_Csparse[] = {"dgCMatrix", "ngCMatrix", ""};
  return R_check_class_etc(mat, Matrix_valid_Csparse) >= 0;
}

// Convert a matrix to an igraph graph.
static void se2_matrix_to_igraph(SEXP mat, igraph_t* graph,
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
}

/* Convert a sparse matrix to an igraph graph. Returns n_nodes.
   Sparse matrix is defined as a cholmod_sparse matrix (see cholmod_core.h in
   Matrix package), a sparse matrix in a non-compressed form is not accepted.

Currently assumes:
    itype == int,
    xtype == real,
    dtype == double,
    packed == TRUE

SE2 only works on real graph so xtype should have to be real. May need to
revist itype and dtype assumptions. */
static void se2_spmatrix_to_igraph(SEXP mat, igraph_t* graph,
                                   igraph_vector_t* weights,
                                   igraph_bool_t is_directed)
{
  CHM_SP mat_i = AS_CHM_SP(mat);

  igraph_vector_int_t edges;
  int n_nodes = mat_i->nrow;
  int n_edges = ((int*)mat_i->p)[mat_i->ncol];
  int* cols = (int*)mat_i->p;
  int* rows = (int*)mat_i->i;
  double* values = (double*)mat_i->x;
  bool has_x = mat_i->xtype != CHOLMOD_PATTERN;

  igraph_vector_int_init(&edges, n_edges * 2);
  igraph_vector_init(weights, n_edges);

  int count = 0;
  for (int j = 0; j < mat_i->ncol; j++) {
    for (int i = cols[j]; i < cols[j + 1]; i++) {
      VECTOR(*weights)[count / 2] = has_x ? values[i] : 1;
      VECTOR(edges)[count++] = rows[i];
      VECTOR(edges)[count++] = j;
    }
  }

  igraph_create(graph, &edges, n_nodes, is_directed);
  igraph_vector_int_destroy(&edges);
}

static void se2_R_adj_to_igraph(SEXP adj, igraph_t* graph,
                                igraph_vector_t* weights, SEXP is_directed)
{
  if (se2_is_sparse(adj)) {
    se2_spmatrix_to_igraph(adj, graph, weights,
                           (igraph_bool_t)asLogical(is_directed));
  } else {
    se2_matrix_to_igraph(adj, graph, weights,
                         (igraph_bool_t)asLogical(is_directed));
  }
}

static void se2_R_mat_int_to_igraph(SEXP mat_R,
                                    igraph_matrix_int_t* mat_igraph, bool shift_idx)
{
  int n_levels = nrows(mat_R);
  int n_nodes = ncols(mat_R);

  mat_R = PROTECT(coerceVector(mat_R, INTSXP));

  igraph_matrix_int_init(mat_igraph, n_levels, n_nodes);
  for (int i = 0; i < n_levels; i++) {
    for (int j = 0; j < n_nodes; j++) {
      MATRIX(*mat_igraph, i, j) = INTEGER(mat_R)[i + (j * n_levels)] -
                                  (int)shift_idx;
    }
  }

  UNPROTECT(1);
}

static void se2_igraph_int_to_R_mat(igraph_matrix_int_t* mat_igraph,
                                    SEXP* mat_R, bool shift_idx)
{
  igraph_integer_t n_levels = igraph_matrix_int_nrow(mat_igraph);
  igraph_integer_t n_nodes = igraph_matrix_int_ncol(mat_igraph);

  PROTECT(*mat_R = allocMatrix(INTSXP, n_levels, n_nodes));
  for (int i = 0; i < n_levels; i++) {
    for (int j = 0; j < n_nodes; j++) {
      INTEGER(*mat_R)[i + (j * n_levels)] = MATRIX(*mat_igraph, i, j) + shift_idx;
    }
  }
  UNPROTECT(1);
}

SEXP call_speakeasy2(SEXP adj, SEXP discard_transient, SEXP independent_runs,
                     SEXP max_threads, SEXP seed, SEXP target_clusters,
                     SEXP target_partitions, SEXP subcluster, SEXP min_clust,
                     SEXP verbose, SEXP is_directed)
{
  igraph_t graph;
  igraph_vector_t weights;
  igraph_matrix_int_t membership_i;
  SEXP membership = NULL;

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

  se2_R_adj_to_igraph(adj, &graph, &weights, is_directed);
  speak_easy_2(&graph, &weights, &opts, &membership_i);
  se2_igraph_int_to_R_mat(&membership_i, &membership, /* inc index */ true);

  igraph_matrix_int_destroy(&membership_i);
  igraph_vector_destroy(&weights);
  igraph_destroy(&graph);

  return membership;
}
SEXP call_order_nodes(SEXP adj, SEXP membership, SEXP is_directed)
{
  igraph_t graph;
  igraph_vector_t weights;
  igraph_matrix_int_t membership_i;
  igraph_matrix_int_t ordering_i;
  SEXP ordering = NULL;

  se2_R_mat_int_to_igraph(membership, &membership_i, /* dec idx */ true);
  se2_R_adj_to_igraph(adj, &graph, &weights, is_directed);
  se2_order_nodes(&graph, &weights, &membership_i, &ordering_i);
  se2_igraph_int_to_R_mat(&ordering_i, &ordering, /* ind idx */ true);

  igraph_matrix_int_destroy(&membership_i);
  igraph_matrix_int_destroy(&ordering_i);
  igraph_vector_destroy(&weights);
  igraph_destroy(&graph);

  return ordering;
}

static R_INLINE double se2_euclidean_dist(int const i, int const j,
    double* mat, int const n_rows)
{
  double out = 0;
  double* col_i = mat + (i * n_rows);
  double* col_j = mat + (j * n_rows);
  for (int k = 0; k < n_rows; k++) {
    double el = col_i[k] - col_j[k];
    out += (el * el);
  }

  return out;
}

static R_INLINE void se2_insert_dist(double const d, double* distances,
                                     int const idx, int* rows, int const k)
{
  if (k == 1) {
    distances[0] = d;
    rows[0] = idx;
    return;
  }

  int bounds[2] = {0, k};
  int pos = (k - 1) / 2;
  while (!((pos == (k - 1)) ||
           ((d <= distances[pos]) && (d > distances[pos + 1])))) {
    if (d > distances[pos]) {
      bounds[1] = pos;
    } else {
      bounds[0] = pos;
    }
    pos = (bounds[1] + bounds[0]) / 2;
  }

  for (int i = 0; i < pos; i++) {
    distances[i] = distances[i + 1];
    rows[i] = rows[i + 1];
  }
  distances[pos] = d;
  rows[pos] = idx;
}

static int se2_row_comp(void const* a, void const* b)
{
  return *(int*)a > *(int*)b ? 1 : -1;
}

static void se2_closest_k(int const col, int const k, int const n_nodes,
                          int const n_rows, double* mat, int* rows)
{
  double* distances = R_Calloc(k, double);
  for (int i = 0; i < k; i++) {
    distances[i] = R_PosInf;
  }

  for (int i = 0; i < n_nodes; i++) {
    if (i == col) {
      continue;
    }

    double d = se2_euclidean_dist(col, i, mat, n_rows);
    if (d < distances[0]) {
      se2_insert_dist(d, distances, i, rows, k);
    }
  }
  R_Free(distances);

  qsort(rows, k, sizeof(k), se2_row_comp);
}

void c_knn_graph(double* mat, int* k, int* n_nodes, int* n_rows, int* sp_p,
                 int* sp_i)
{
  if (*k < 1) {
    Rf_error("The k must be at least 1.");
  }

  if (*k > *n_nodes) {
    Rf_error("The k must be less than the number of nodes.");
  }

  for (int i = 0; i <= *n_nodes; i++) {
    sp_p[i] = i * *k;
  }

  for (int i = 0; i < *n_nodes; i++) {
    R_CheckUserInterrupt();
    se2_closest_k(i, *k, *n_nodes, *n_rows, mat, sp_i + sp_p[i]);
  }
}

static const R_CMethodDef cMethods[] = {
  {"knn_graph", (DL_FUNC) &c_knn_graph, 6, NULL},
  {NULL, NULL, 0}
};

static const R_CallMethodDef callMethods[] = {
  {"speakeasy2", (DL_FUNC) &call_speakeasy2, 11},
  {"order_nodes", (DL_FUNC) &call_order_nodes, 3},
  {NULL, NULL, 0}
};

void attribute_visible R_init_speakeasyr(DllInfo* info)
{
  R_registerRoutines(info, cMethods, callMethods, NULL, NULL);
  R_useDynamicSymbols(info, FALSE);
  R_forceSymbols(info, TRUE);
}
