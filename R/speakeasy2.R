is_matrix_i <- function(obj) {
  class(obj)[[1]] %in% c("dgCMatrix", "matrix")
}

#' SpeakEasy2 community detection
#'
#' @param graph A graph or adjacency matrix in a form that can be converted to
#'   'matrix' or 'Matrix::dgCMatrix' using an 'as.matrix' coercion method.
#'   Accepted types include 'matrix', 'dgCMatrix', and 'igraph::graph's.
#' @param discard_transient The number of partitions to discard before tracking.
#' @param independent_runs How many runs SpeakEasy2 should perform.
#' @param max_threads The maximum number of threads to use. By default uses the
#'   number of cores available.
#' @param seed Random seed to use for reproducible results. SpeakEasy2 uses a
#'   different random number generator than R, but if the seed is not
#'   explicitly set, R's random number generator is used create one. Because of
#'   this, setting R's RNG will also cause reproducible results.
#' @param subcluster Depth of clustering. If greater than 1, perform recursive
#'   clustering.
#' @param min_clust Smallest clusters to recursively cluster. If subcluster not
#'   set to a value greater than 1, this has no effect.
#' @param target_clusters The number of random initial labels to use.
#' @param target_partitions Number of partitions to find per independent run.
#' @param verbose Whether to provide additional information about the
#' clustering or not.
#' @param is_directed Whether the graph should be treated as directed or not.
#'   By default, if the graph is symmetric it is treated as undirected.
#'
#' @return A membership vector. If subclustering, returns a matrix with number
#'   of rows equal to the number of recursive clustering. Each row is the
#'   membership at different hierarchical scales, such that the last rows are
#'   the highest resolution.
#' @export
#'
#' @examples
#' if (require("igraph")) {
#'   graph <- igraph::graph.famous("zachary")
#'   memb <- speakeasy2(graph)
#' }
speakeasy2 <- function(graph, discard_transient = 3, independent_runs = 10,
                       max_threads = 0, seed = 0, target_clusters = 0,
                       target_partitions = 5, subcluster = 1, min_clust = 5,
                       verbose = FALSE, is_directed = "detect") {
  if (!is_matrix_i(graph)) {
    graph <- as.matrix(graph)
  }

  if (!is_matrix_i(graph)) {
    stop(paste0(
      "Could not convert graph to an appropriate type. ",
      "'as.matrix' converted graph to \"", class(graph)[[0]],
      "\". Currently implemented classes are c(\"matrix\", \"dgCMatrix\"). ",
      "Please open an issue on github to add support for new types."
    ))
  }

  if (nrow(graph) != ncol(graph)) {
    stop("Graph adjacency matrix must be square.")
  }

  if (is_directed == "detect") {
    is_directed <- !Matrix::isSymmetric(graph)
  }

  if (seed == 0) {
    seed <- sample.int(9999, 1)
  }

  .Call(
    C_speakeasy2, graph, discard_transient, independent_runs,
    max_threads, seed, target_clusters, target_partitions, subcluster,
    min_clust, verbose, is_directed
  )
}
