#include "R.h"
#include "Rinternals.h"
#include "R_ext/Visibility.h"

#include "igraph.h"
#include "speak_easy_2.h"

#define R_MATRIX(m, i, j, vcount) matrix[(i) + ((j) * (vcount))]

static void matrix_to_igraph(double const* matrix, int const n_nodes,
                             igraph_t* graph, igraph_vector_t* weights)
{
  igraph_vector_int_t edges;

  igraph_integer_t n_edges = 0;
  for (int i = 0; i < n_nodes; i++) {
    for (int j = 0; j < n_nodes; j++) {
      n_edges += R_MATRIX(matrix, i, j, n_nodes) != 0;
    }
  }

  igraph_vector_int_init(&edges, n_edges * 2);
  igraph_vector_init(weights, n_edges);

  n_edges = 0;
  for (int i = 0; i < n_nodes; i++) {
    for (int j = 0; j < n_nodes; j++) {
      if (R_MATRIX(matrix, i, j, n_nodes) != 0) {
        VECTOR(*weights)[n_edges / 2] = R_MATRIX(matrix, i, j, n_nodes);
        VECTOR(edges)[n_edges++] = i;
        VECTOR(edges)[n_edges++] = j;
      }
    }
  }

  /* TODO: determine directedness elsewhere. */
  igraph_create(graph, &edges, n_nodes, IGRAPH_DIRECTED);

  igraph_vector_int_destroy(&edges);
}

void speakeasyr_speakeasy2(double* matrix, int* n_nodes,
                           int* discard_transient,
                           int* independent_runs, int* max_threads, int* seed,
                           int* target_clusters, int* target_partitions,
                           int* verbose, int* membership)
{
  igraph_t graph;
  igraph_vector_t weights;
  igraph_matrix_int_t membership_i;
  se2_options opts = {
    .discard_transient = *discard_transient,
    .independent_runs = *independent_runs,
    .max_threads = *max_threads,
    /* .minclust = *min_clust, */
    /* .subcluster = *subcluster, */
    .random_seed = *seed,
    .target_clusters = *target_clusters,
    .target_partitions = *target_partitions,
    .verbose = *verbose
  };

  matrix_to_igraph(matrix, *n_nodes, &graph, &weights);
  speak_easy_2(&graph, &weights, &opts, &membership_i);

  for (int j = 0; j < *n_nodes; j++) {
    /* TODO: Add in subclustering */
    membership[j] = MATRIX(membership_i, 0, j);
  }

  igraph_matrix_int_destroy(&membership_i);
  igraph_vector_destroy(&weights);
  igraph_destroy(&graph);
}

static const R_CallMethodDef callMethods[] = {
  {"speakeasy2", (DL_FUNC) &call_speakeasy2, 9},
  {NULL, NULL, 0}
};

void attribute_visible R_init_speakeasyr(DllInfo* info)
{
  R_registerRoutines(info, NULL, callMethods, NULL, NULL);
  R_useDynamicSymbols(info, FALSE);
  R_forceSymbols(info, TRUE);
}
