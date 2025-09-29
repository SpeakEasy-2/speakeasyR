#include <R.h>
#include <Rinternals.h>
#include <R_ext/Visibility.h>
#include <R_ext/Print.h>

#include <config.h>

#include <speak_easy_2.h>

#define IS_WEIGHTED(g) ((g)->weights != NULL)

#define N_NEIGHBORS(a, i)                                                     \
  ((a).neigh_list ? VECTOR(*(a).sizes)[(i)] : (a).n_nodes)

#define R_MATRIX(mat, i, j, vcount) (mat)[(i) + ((j) * (vcount))]
#define R_IGRAPH_CHECK(expr) \
  do {                                               \
    igraph_error_t se2_rs = (expr);                  \
    if (IGRAPH_UNLIKELY(se2_rs != IGRAPH_SUCCESS)) { \
      IGRAPH_ERROR_NO_RETURN("", se2_rs);            \
      return R_NilValue;                             \
    }                                                \
  } while (0)

static void checkInterruptFn(void* dummy)
{
  R_CheckUserInterrupt();
}

static igraph_bool_t R_interruption_handler(void)
{
  return R_ToplevelExec(checkInterruptFn, NULL) == false;
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
  Rprintf("%s", message);
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

static igraph_error_t se2_R_unweighted_double_to_graph(
  double* const mat, se2_neighs* graph, igraph_bool_t const is_directed)
{
  igraph_integer_t const n_nodes = graph->n_nodes;
  for (igraph_integer_t i = 0; i < n_nodes; i++) {
    igraph_vector_int_t neighbors = VECTOR(* graph->neigh_list)[i];
    igraph_integer_t n_neighs = 0;
    for (igraph_integer_t j = 0; j < n_nodes; j++) {
      n_neighs += R_MATRIX(mat, j, i, n_nodes);
    }
    if (!is_directed) {
      for (igraph_integer_t j = 0; j < n_nodes; j++) {
        n_neighs += R_MATRIX(mat, i, j, n_nodes);
      }
    }
    VECTOR(* graph->sizes)[i] = n_neighs;

    igraph_vector_int_init( &VECTOR(neighbors), n_neighs);
    igraph_integer_t count = 0;
    for (igraph_integer_t j = 0; j < n_nodes; j++) {
      if (R_MATRIX(mat, j, i, n_nodes)) {
        VECTOR(neighbors)[count++] = j;
      }
    }

    if (is_directed) {
      continue;
    }

    for (igraph_integer_t j = 0; j < n_nodes; j++) {
      if (R_MATRIX(mat, i, j, n_nodes)) {
        VECTOR(neighbors)[count++] = j;
      }
    }
  }

  return IGRAPH_SUCCESS;
}

static igraph_error_t se2_R_weighted_double_to_graph(
  double* const mat, se2_neighs* graph, igraph_bool_t const is_directed)
{
  igraph_integer_t const n_nodes = graph->n_nodes;
  for (igraph_integer_t i = 0; i < n_nodes; i++) {
    igraph_vector_init( &VECTOR(* graph->weights)[i], n_nodes);
  }

  for (igraph_integer_t i = 0; i < n_nodes; i++) {
    igraph_vector_t w = VECTOR(* graph->weights)[i];
    for (igraph_integer_t j = 0; j < n_nodes; j++) {
      VECTOR(w)[j] = R_MATRIX(mat, j, i, n_nodes);
    }
  }

  if (is_directed) {
    return IGRAPH_SUCCESS;
  }

  for (igraph_integer_t i = 0; i < n_nodes; i++) {
    igraph_vector_t w = VECTOR(* graph->weights)[i];
    for (igraph_integer_t j = 0; j < n_nodes; j++) {
      VECTOR(w)[j] += R_MATRIX(mat, i, j, n_nodes);
    }
  }

  return IGRAPH_SUCCESS;
}

static igraph_error_t se2_R_directed_sparse_to_graph(
  int* const sp_i, int* const sp_p, double* const values, se2_neighs* graph)
{
  igraph_integer_t const n_nodes = graph->n_nodes;
  for (igraph_integer_t i = 0; i < n_nodes; i++) {
    igraph_integer_t const n_neighs = sp_p[i + 1] - sp_p[i];
    VECTOR(* graph->sizes)[i] = n_neighs;
    igraph_vector_int_init( &VECTOR(* graph->neigh_list)[i], n_neighs);

    if (IS_WEIGHTED(graph)) {
      igraph_vector_init( &VECTOR(* graph->weights)[i], n_neighs);
    }
  }

  for (igraph_integer_t i = 0; i < n_nodes; i++) {
    for (igraph_integer_t j = sp_p[i]; j < sp_p[i + 1]; j++) {
      VECTOR(VECTOR(* graph->neigh_list)[i])[j - sp_p[i]] = sp_i[j];

      if (IS_WEIGHTED(graph)) {
        VECTOR(VECTOR(* graph->weights)[i])[j - sp_p[i]] = values[j];
      }
    }
  }

  return IGRAPH_SUCCESS;
}

#define NEIGHBOR(a, i, j) (VECTOR(VECTOR(*(a).neigh_list)[(i)])[(j)])
#define WEIGHT(a, i, j) (VECTOR(VECTOR(*(a).weights)[(i)])[(j)])

static igraph_error_t se2_R_undirected_sparse_to_graph(
  int* const sp_i, int* const sp_p, double* const values, se2_neighs* graph)
{
  igraph_integer_t const n_nodes = graph->n_nodes;
  for (igraph_integer_t i = 0; i < n_nodes; i++) {
    VECTOR(* graph->sizes)[i] = sp_p[i + 1] - sp_p[i];
    for (igraph_integer_t j = sp_p[i]; j < sp_p[i + 1]; j++) {
      VECTOR(* graph->sizes)[sp_i[i]] += 1;
    }
  }

  for (igraph_integer_t i = 0; i < n_nodes; i++) {
    igraph_integer_t n_neighs = N_NEIGHBORS(* graph, i);
    igraph_vector_int_init( &VECTOR(* graph->neigh_list)[i], n_neighs);
    if (IS_WEIGHTED(graph)) {
      igraph_vector_init( &VECTOR(* graph->weights)[i], n_neighs);
    }
  }

  igraph_vector_int_t pos;
  IGRAPH_CHECK(igraph_vector_int_init( &pos, n_nodes));
  IGRAPH_FINALLY(igraph_vector_int_destroy, &pos);

  for (igraph_integer_t i = 0; i < n_nodes; i++) {
    for (igraph_integer_t j = sp_p[i]; j < sp_p[i + 1]; j++) {
      igraph_integer_t row = sp_i[j];
      if (IS_WEIGHTED(graph)) {
        WEIGHT(* graph, i, VECTOR(pos)[i]) = values[j];
        WEIGHT(* graph, row, VECTOR(pos)[row]) = values[j];
      }

      NEIGHBOR(* graph, i, VECTOR(pos)[i]++) = row;
      NEIGHBOR(* graph, row, VECTOR(pos)[row]++) = i;
    }
  }

  igraph_vector_int_destroy( &pos);
  IGRAPH_FINALLY_CLEAN(1);

  return IGRAPH_SUCCESS;
}

#undef NEIGHBOR
#undef WEIGHT

static igraph_error_t se2_R_sparse_to_graph(
  int* const sp_i, int* const sp_p,
  double* const values, se2_neighs* graph, igraph_bool_t const is_directed)
{
  if (is_directed) {
    return se2_R_directed_sparse_to_graph(sp_i, sp_p, values, graph);
  }

  return se2_R_undirected_sparse_to_graph(sp_i, sp_p, values, graph);
}

static igraph_error_t se2_R_adj_to_graph(
  int* const sp_i, int* const sp_p,
  double* const values,
  int const n_nodes, se2_neighs* graph,
  bool const is_directed)
{
  igraph_bool_t const is_weighted = values != NULL;
  igraph_bool_t const is_sparse = sp_i != NULL;

  graph->n_nodes = n_nodes;

  /* NOTE: We don't actually have to calculate kin or total weights because it
  will be done at the start of the SE2 algorithm. */
  graph->total_weight = 0;
  graph->kin = igraph_malloc(sizeof(* graph->kin));
  IGRAPH_CHECK_OOM(graph->kin, "");
  IGRAPH_FINALLY(igraph_free, graph->kin);
  IGRAPH_CHECK(igraph_vector_init(graph->kin, n_nodes));
  IGRAPH_FINALLY(igraph_vector_destroy, graph->kin);

  if (is_sparse || !is_weighted) {
    graph->neigh_list = igraph_malloc(sizeof(* graph->neigh_list));
    IGRAPH_CHECK_OOM(graph->neigh_list, "");
    IGRAPH_FINALLY(igraph_free, graph->neigh_list);
    IGRAPH_CHECK(igraph_vector_int_list_init(graph->neigh_list, n_nodes));
    IGRAPH_FINALLY(igraph_vector_int_list_destroy, graph->neigh_list);

    graph->sizes = igraph_malloc(sizeof(* graph->sizes));
    IGRAPH_CHECK_OOM(graph->sizes, "");
    IGRAPH_FINALLY(igraph_free, graph->sizes);
    IGRAPH_CHECK(igraph_vector_int_init(graph->sizes, n_nodes));
    IGRAPH_FINALLY(igraph_vector_int_destroy, graph->sizes);
  } else {
    graph->neigh_list = NULL;
    graph->sizes = NULL;
  }

  if (is_weighted) {
    graph->weights = igraph_malloc(sizeof(* graph->weights));
    IGRAPH_CHECK_OOM(graph->weights, "");
    IGRAPH_FINALLY(igraph_free, graph->weights);
    IGRAPH_CHECK(igraph_vector_list_init(graph->weights, n_nodes));
    IGRAPH_FINALLY(igraph_vector_list_destroy, graph->weights);
  } else {
    graph->weights = NULL;
  }

  if (is_sparse) {
    IGRAPH_CHECK(se2_R_sparse_to_graph(sp_i, sp_p, values, graph, is_directed));
  } else if (is_weighted) {
    IGRAPH_CHECK(se2_R_weighted_double_to_graph(values, graph, is_directed));
  } else {
    IGRAPH_CHECK(se2_R_unweighted_double_to_graph(values, graph, is_directed));
  }

  IGRAPH_FINALLY_CLEAN(2);

  if (is_sparse || !is_weighted) {
    IGRAPH_FINALLY_CLEAN(4);
  }

  if (is_weighted) {
    IGRAPH_FINALLY_CLEAN(2);
  }

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

SEXP c_speakeasy2(SEXP sp_i, SEXP sp_p, SEXP values, SEXP n_nodes,
                  SEXP discard_transient, SEXP independent_runs,
                  SEXP max_threads, SEXP seed, SEXP target_clusters,
                  SEXP target_partitions, SEXP subcluster, SEXP min_clust,
                  SEXP verbose, SEXP is_directed)
{
  se2_init();

  se2_neighs graph;
  SEXP membership =
    PROTECT(allocVector(INTSXP, INTEGER(n_nodes)[0] * INTEGER(subcluster)[0]));
  igraph_matrix_int_t membership_i;

  se2_options opts = {
    .discard_transient = INTEGER(discard_transient)[0],
    .independent_runs = INTEGER(independent_runs)[0],
    .max_threads = INTEGER(max_threads)[0],
    .minclust = INTEGER(min_clust)[0],
    .subcluster = INTEGER(subcluster)[0],
    .random_seed = INTEGER(seed)[0],
    .target_clusters = INTEGER(target_clusters)[0],
    .target_partitions = INTEGER(target_partitions)[0],
    .verbose = LOGICAL(verbose)[0]
  };

  igraph_bool_t const is_sparse = length(sp_i) > 1;
  igraph_bool_t const is_weighted = length(values) > 1;

  R_IGRAPH_CHECK(se2_R_adj_to_graph(is_sparse ? INTEGER(sp_i) : NULL,
                                    is_weighted ? INTEGER(sp_p) : NULL,
                                    REAL(values),
                                    INTEGER(n_nodes)[0], &graph,
                                    LOGICAL(is_directed)[0]));
  IGRAPH_FINALLY(se2_neighs_destroy, &graph);

  R_IGRAPH_CHECK(speak_easy_2( &graph, &opts, &membership_i));
  se2_neighs_destroy( &graph);
  IGRAPH_FINALLY_CLEAN(1);
  IGRAPH_FINALLY(igraph_matrix_int_destroy, &membership_i);

  se2_igraph_int_to_R( &membership_i,
                       INTEGER(membership), /* inc index */ true);

  igraph_matrix_int_destroy( &membership_i);
  IGRAPH_FINALLY_CLEAN(1);

  UNPROTECT(1);
  return membership;
}

SEXP c_order_nodes(SEXP sp_i, SEXP sp_p, SEXP values, SEXP n_nodes,
                   SEXP membership, SEXP n_levels, SEXP is_directed)
{
  se2_init();

  se2_neighs graph;
  igraph_matrix_int_t membership_i;
  igraph_matrix_int_t ordering_i;
  SEXP ordering =
    PROTECT(allocVector(INTSXP, INTEGER(n_nodes)[0] * INTEGER(n_levels)[0]));

  R_IGRAPH_CHECK(se2_R_integer_to_igraph(
                   INTEGER(membership),
                   INTEGER(n_levels)[0],
                   INTEGER(n_nodes)[0], &membership_i,
                   /* dec idx */ true));
  IGRAPH_FINALLY(igraph_matrix_int_destroy, &membership_i);

  R_IGRAPH_CHECK(se2_R_adj_to_graph(INTEGER(sp_i), INTEGER(sp_p), REAL(values),
                                    INTEGER(n_nodes)[0], &graph,
                                    LOGICAL(is_directed)[0]));
  IGRAPH_FINALLY(se2_neighs_destroy, &graph);

  R_IGRAPH_CHECK(se2_order_nodes( &graph, &membership_i, &ordering_i));
  IGRAPH_FINALLY(igraph_matrix_int_destroy, &ordering_i);

  se2_igraph_int_to_R( &ordering_i, INTEGER(ordering), /* ind idx */ true);

  igraph_matrix_int_destroy( &membership_i);
  se2_neighs_destroy( &graph);
  igraph_matrix_int_destroy( &ordering_i);

  IGRAPH_FINALLY_CLEAN(3);

  UNPROTECT(1);
  return ordering;
}

static R_INLINE double se2_euclidean_dist(int const i, int const j,
    double const* mat, int const n_rows)
{
  double out = 0;
  double const* col_i = mat + (i* n_rows);
  double const* col_j = mat + (j* n_rows);
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
                          int const n_rows, double const* mat, int* rows, double* vals)
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

SEXP c_knn_graph(SEXP mat, SEXP k, SEXP n_nodes, SEXP n_rows, SEXP sp_p,
                 SEXP sp_i, SEXP sp_x)
{
  SEXP res = PROTECT(allocVector(VECSXP, 3));

  int const k_ = INTEGER(k)[0];
  int const n_nodes_ = INTEGER(n_nodes)[0];
  int const n_rows_ = INTEGER(n_rows)[0];
  int* sp_i_ = INTEGER(sp_i);
  int* sp_p_ = INTEGER(sp_p);
  double* sp_x_ = REAL(sp_x);
  double const* mat_ = REAL(mat);

  if (k_ < 1) {
    Rf_error("The k must be at least 1.");
  }

  if (k_ >= n_nodes_) {
    Rf_error("The k must be less than the number of nodes.");
  }

  for (int i = 0; i <= n_nodes_; i++) {
    sp_p_[i] = i* k_;
  }

  for (int i = 0; i < n_nodes_; i++) {
    R_CheckUserInterrupt();
    se2_closest_k(i, k_, n_nodes_, n_rows_,
                  mat_, sp_i_ + sp_p_[i],
                  *sp_x_ < 0 ? sp_x_ : sp_x_ + sp_p_[i]);
  }

  SET_VECTOR_ELT(res, 0, sp_p);
  SET_VECTOR_ELT(res, 1, sp_i);
  SET_VECTOR_ELT(res, 2, sp_x);

  UNPROTECT(1);
  return res;
}

static const R_CallMethodDef callMethods[] = {
  {"speakeasy2", (DL_FUNC) &c_speakeasy2, 14},
  {"order_nodes", (DL_FUNC) &c_order_nodes, 7},
  {"knn_graph", (DL_FUNC) &c_knn_graph, 7},
  {NULL, NULL, 0}
};

void attribute_visible R_init_speakeasyR(DllInfo* info)
{
  R_registerRoutines(info, NULL, callMethods, NULL, NULL);
  R_useDynamicSymbols(info, FALSE);
  R_forceSymbols(info, TRUE);
}
