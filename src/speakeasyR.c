#include <R.h>
#include <Rinternals.h>
#include <R_ext/Visibility.h>
#include <R_ext/Print.h>

#include <config.h>

#include <speak_easy_2.h>

#define R_MATRIX(mat, i, j, vcount) (mat)[(i) + ((j) * (vcount))]
#define R_IGRAPH_CHECK(expr) \
  do {                                               \
    igraph_error_t se2_rs = (expr);                  \
    if (IGRAPH_UNLIKELY(se2_rs != IGRAPH_SUCCESS)) { \
      IGRAPH_ERROR_NO_RETURN("", se2_rs);            \
      return;                                        \
    }                                                \
  } while (0)

static void checkInterruptFn(void* dummy)
{
  R_CheckUserInterrupt();
}

static igraph_error_t R_interruption_handler(void* data)
{
  return R_ToplevelExec(checkInterruptFn, NULL) ?
         IGRAPH_SUCCESS : IGRAPH_INTERRUPTED;
}

static void R_warning_handler(char const* reason, char const* file,
                              int line)
{
  warning("At %s:%d\n\n%s", file, line, reason);
}

static void R_error_handler(char const* reason, char const* file,
                            int line, igraph_error_t errorcode)
{
  IGRAPH_FINALLY_FREE();
  error("At %s:%d\n\n%s %s", file, line, reason, igraph_strerror(errorcode));
}

static igraph_error_t R_status_handler(const char* message, void* data)
{
  Rprintf("%s\n", message);
  return IGRAPH_SUCCESS;
}

// Initialize igraph handlers.
static void se2_init(void)
{
  igraph_set_interruption_handler(R_interruption_handler);
  igraph_set_warning_handler(R_warning_handler);
  igraph_set_error_handler(R_error_handler);
  igraph_set_status_handler(R_status_handler);
}

// Convert a matrix to an igraph graph.
static igraph_error_t se2_R_double_to_igraph(
  double* const mat, int const n_nodes,
  igraph_t* graph, igraph_vector_t* weights,
  igraph_bool_t const is_directed)
{
  igraph_vector_int_t edges;

  igraph_integer_t n_edges = 0;
  for (int i = 0; i < n_nodes; i++) {
    for (int j = 0; j < n_nodes; j++) {
      n_edges += R_MATRIX(mat, i, j, n_nodes) != 0;
    }
  }

  IGRAPH_CHECK(igraph_vector_int_init( &edges, n_edges * 2));
  IGRAPH_FINALLY(igraph_vector_int_destroy, &edges);

  IGRAPH_CHECK(igraph_vector_init(weights, n_edges));
  IGRAPH_FINALLY(igraph_vector_destroy, weights);

  n_edges = 0;
  for (int i = 0; i < n_nodes; i++) {
    for (int j = 0; j < n_nodes; j++) {
      if (R_MATRIX(mat, i, j, n_nodes) != 0) {
        VECTOR(* weights)[n_edges / 2] = R_MATRIX(mat, i, j, n_nodes);
        VECTOR(edges)[n_edges++] = i;
        VECTOR(edges)[n_edges++] = j;
      }
    }
  }

  IGRAPH_CHECK(igraph_create(graph, &edges, n_nodes, is_directed));

  igraph_vector_int_destroy( &edges);
  IGRAPH_FINALLY_CLEAN(2);

  return IGRAPH_SUCCESS;
}

static igraph_error_t se2_R_sparse_to_igraph(
  int* const sp_i, int* const sp_p,
  double* const values, int const n_nodes,
  igraph_t* graph, igraph_vector_t* weights,
  igraph_bool_t const is_directed)
{
  igraph_vector_int_t edges;
  igraph_integer_t n_edges = sp_p[n_nodes];

  IGRAPH_CHECK(igraph_vector_int_init( &edges, n_edges * 2));
  IGRAPH_FINALLY(igraph_vector_int_destroy, &edges);

  IGRAPH_CHECK(igraph_vector_init(weights, n_edges));
  IGRAPH_FINALLY(igraph_vector_destroy, weights);

  for (igraph_integer_t i = 0; i < n_nodes; i++) {
    for (igraph_integer_t j = sp_p[i]; j < sp_p[i + 1]; j++) {
      VECTOR(* weights)[j] = *values > -1 ? values[j] : 1;
      VECTOR(edges)[(j * 2)] = i;
      VECTOR(edges)[(j * 2) + 1] = (igraph_integer_t)sp_i[j];
    }
  }

  IGRAPH_CHECK(igraph_create(graph, &edges, n_nodes, is_directed));
  igraph_vector_int_destroy( &edges);

  IGRAPH_FINALLY_CLEAN(2);

  return IGRAPH_SUCCESS;
}

static igraph_error_t se2_R_adj_to_igraph(
  int* const sp_i, int* const sp_p,
  double* const values,
  int const n_nodes, se2_neighs* graph,
  bool const is_directed)
{
  igraph_t graph_i;
  igraph_vector_t weights;

  if (* sp_i >= 0) {
    IGRAPH_CHECK(se2_R_sparse_to_igraph(sp_i, sp_p, values, n_nodes, &graph_i,
                                        &weights, is_directed));
  } else {
    IGRAPH_CHECK(se2_R_double_to_igraph(values, n_nodes, &graph_i, &weights,
                                        is_directed));
  }
  IGRAPH_FINALLY(igraph_destroy, &graph_i);
  IGRAPH_FINALLY(igraph_vector_destroy, &weights);

  IGRAPH_CHECK(se2_igraph_to_neighbor_list( &graph_i, &weights, graph));

  igraph_destroy( &graph_i);
  igraph_vector_destroy( &weights);
  IGRAPH_FINALLY_CLEAN(2);

  return IGRAPH_SUCCESS;
}

static igraph_error_t se2_R_integer_to_igraph(
  int* const mat_R,
  int const n_levels, int const n_nodes,
  igraph_matrix_int_t* mat_igraph,
  bool const shift_idx)
{
  IGRAPH_CHECK(igraph_matrix_int_init(mat_igraph, n_levels, n_nodes));
  IGRAPH_FINALLY(igraph_matrix_int_destroy, mat_igraph);
  for (int i = 0; i < n_levels; i++) {
    for (int j = 0; j < n_nodes; j++) {
      MATRIX(* mat_igraph, i, j) = R_MATRIX(mat_R, i, j, n_levels) -
                                   (int)shift_idx;
    }
  }

  IGRAPH_FINALLY_CLEAN(1);
  return IGRAPH_SUCCESS;
}

static void se2_igraph_int_to_R(igraph_matrix_int_t* const mat_igraph,
                                int* mat_R, bool const shift_idx)
{
  igraph_integer_t n_levels = igraph_matrix_int_nrow(mat_igraph);
  igraph_integer_t n_nodes = igraph_matrix_int_ncol(mat_igraph);

  for (int i = 0; i < n_levels; i++) {
    for (int j = 0; j < n_nodes; j++) {
      R_MATRIX(mat_R, i, j, n_levels) = MATRIX(* mat_igraph, i, j) + shift_idx;
    }
  }
}

void c_speakeasy2(int* sp_i, int* sp_p, double* values, int* n_nodes,
                  int* discard_transient, int* independent_runs,
                  int* max_threads, int* seed, int* target_clusters,
                  int* target_partitions, int* subcluster, int* min_clust,
                  bool* verbose, bool* is_directed, int* membership)
{
  se2_init();

  se2_neighs graph;
  igraph_matrix_int_t membership_i;

  se2_options opts = {
    .discard_transient = *discard_transient,
    .independent_runs = *independent_runs,
    .max_threads = *max_threads,
    .minclust = *min_clust,
    .subcluster = *subcluster,
    .random_seed = *seed,
    .target_clusters = *target_clusters,
    .target_partitions = *target_partitions,
    .verbose = *verbose
  };

  R_IGRAPH_CHECK(se2_R_adj_to_igraph(sp_i, sp_p, values, * n_nodes, &graph,
                                     * is_directed));
  IGRAPH_FINALLY(se2_neighs_destroy, &graph);

  R_IGRAPH_CHECK(speak_easy_2( &graph, &opts, &membership_i));
  se2_neighs_destroy( &graph);
  IGRAPH_FINALLY_CLEAN(1);
  IGRAPH_FINALLY(igraph_matrix_int_destroy, &membership_i);

  se2_igraph_int_to_R( &membership_i, membership, /* inc index */ true);

  igraph_matrix_int_destroy( &membership_i);
  IGRAPH_FINALLY_CLEAN(1);
}

void c_order_nodes(int* sp_i, int* sp_p, double* values, int* n_nodes,
                   int* membership, int* n_levels, bool* is_directed,
                   int* ordering)
{
  se2_init();

  se2_neighs graph;
  igraph_matrix_int_t membership_i;
  igraph_matrix_int_t ordering_i;

  R_IGRAPH_CHECK(se2_R_integer_to_igraph(membership, * n_levels, * n_nodes,
                                         &membership_i, /* dec idx */ true));
  IGRAPH_FINALLY(igraph_matrix_int_destroy, &membership_i);

  R_IGRAPH_CHECK(se2_R_adj_to_igraph(sp_i, sp_p, values, * n_nodes, &graph,
                                     * is_directed));
  IGRAPH_FINALLY(se2_neighs_destroy, &graph);

  R_IGRAPH_CHECK(se2_order_nodes( &graph, &membership_i, &ordering_i));
  IGRAPH_FINALLY(igraph_matrix_int_destroy, &ordering_i);

  se2_igraph_int_to_R( &ordering_i, ordering, /* ind idx */ true);

  igraph_matrix_int_destroy( &membership_i);
  se2_neighs_destroy( &graph);
  igraph_matrix_int_destroy( &ordering_i);

  IGRAPH_FINALLY_CLEAN(3);
}

static R_INLINE double se2_euclidean_dist(int const i, int const j,
    double* mat, int const n_rows)
{
  double out = 0;
  double* col_i = mat + (i* n_rows);
  double* col_j = mat + (j* n_rows);
  for (int k = 0; k < n_rows; k++) {
    double el = col_i[k] - col_j[k];
    out += (el* el);
  }

  return sqrt(out);
}

static R_INLINE void se2_insert_sim(double const d, double* similarities,
                                    int const idx, int* rows, int const k)
{
  if (k == 1) {
    similarities[0] = d;
    rows[0] = idx;
    return;
  }

  int bounds[2] = {0, k};
  int pos = (k - 1) / 2;
  while (!((pos == (k - 1)) ||
           ((d >= similarities[pos]) && (d < similarities[pos + 1])))) {
    if (d < similarities[pos]) {
      bounds[1] = pos;
    } else {
      bounds[0] = pos;
    }
    pos = (bounds[1] + bounds[0]) / 2;
  }

  for (int i = 0; i < pos; i++) {
    similarities[i] = similarities[i + 1];
    rows[i] = rows[i + 1];
  }
  similarities[pos] = d;
  rows[pos] = idx;
}

static void se2_closest_k(int const col, int const k, int const n_nodes,
                          int const n_rows, double* mat, int* rows, double* vals)
{
  double* similarities = R_Calloc(k, double);

  for (int i = 0; i < n_nodes; i++) {
    if (i == col) {
      continue;
    }

    double s = 1 / se2_euclidean_dist(col, i, mat, n_rows);
    if (s > similarities[0]) {
      se2_insert_sim(s, similarities, i, rows, k);
    }
  }

  if (* vals == -1) { // Not storing weights.
    R_qsort_int(rows, 1, k);
  } else {
    int* idx = R_Calloc(k, int);
    for (int i = 0; i < k; i++) {
      idx[i] = i;
    }

    R_qsort_int_I(rows, idx, 1, k);
    for (int i = 0; i < k; i++) {
      vals[i] = similarities[idx[i]];
    }

    R_Free(idx);
  }

  R_Free(similarities);
}

void c_knn_graph(double* mat, int* k, int* n_nodes, int* n_rows, int* sp_p,
                 int* sp_i, double* sp_x)
{
  if (* k < 1) {
    Rf_error("The k must be at least 1.");
  }

  if (* k >= *n_nodes) {
    Rf_error("The k must be less than the number of nodes.");
  }

  for (int i = 0; i <= *n_nodes; i++) {
    sp_p[i] = i** k;
  }

  for (int i = 0; i < *n_nodes; i++) {
    R_CheckUserInterrupt();
    se2_closest_k(i, * k, * n_nodes, * n_rows, mat, sp_i + sp_p[i],
                  *sp_x < 0 ? sp_x : sp_x + sp_p[i]);
  }
}

static R_NativePrimitiveArgType se2_type[] = {
  INTSXP, INTSXP, REALSXP, INTSXP, INTSXP, INTSXP, INTSXP, INTSXP, INTSXP,
  INTSXP, INTSXP, INTSXP, LGLSXP, LGLSXP, INTSXP
};

static R_NativePrimitiveArgType order_type[] = {
  INTSXP, INTSXP, REALSXP, INTSXP, INTSXP, INTSXP, LGLSXP, INTSXP
};

static R_NativePrimitiveArgType knn_type[] = {
  REALSXP, INTSXP, INTSXP, INTSXP, INTSXP, INTSXP, REALSXP
};

static const R_CMethodDef cMethods[] = {
  {"speakeasy2", (DL_FUNC) &c_speakeasy2, 15, se2_type},
  {"order_nodes", (DL_FUNC) &c_order_nodes, 8, order_type},
  {"knn_graph", (DL_FUNC) &c_knn_graph, 7, knn_type},
  {NULL, NULL, 0}
};

void attribute_visible R_init_speakeasyR(DllInfo* info)
{
  R_registerRoutines(info, cMethods, NULL, NULL, NULL);
  R_useDynamicSymbols(info, FALSE);
  R_forceSymbols(info, TRUE);
}
