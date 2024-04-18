#' SpeakEasy2 community detection
#'
#' @param graph An igraph graph or an adjacency Matrix.
#' @param discard_transient The number of partitions to discard before tracking.
#' @param independent_runs How many runs SpeakEasy2 should perform.
#' @param max_threads The maximum number of threads to use.
#' @param seed Random seed to use for reproducible results.
#' @param target_clusters The number of random initial labels to use.
#' @param target_partitions Number of partitions to find per independent run.
#' @param verbose Whether to provide additional information about the
#' clustering or not.
#'
#' @return A membership list.
#' @export
#'
#' @examples
#' if (require("igraph")) {
#'   graph <- igraph::graph.famous("zachary")
#'   memb <- speakeasy2(graph)
#' }
speakeasy2 <- function(graph, discard_transient = 3, independent_runs = 10,
                       max_threads = 0, seed = 0, target_clusters = 0,
                       target_partitions = 5, verbose = FALSE) {
  graph <- Matrix::as.matrix(graph)
  if (nrow(graph) != ncol(graph)) {
    stop("Graph adjacency matrix must be square.")
  }

  if (seed == 0) {
    seed <- sample.int(9999, 1)
  }

  .C(
    "speakeasyr_speakeasy2", as.double(graph), nrow(graph),
    as.integer(discard_transient), as.integer(independent_runs),
    as.integer(max_threads), as.integer(seed), as.integer(target_clusters),
    as.integer(target_partitions), as.integer(verbose),
    membership = integer(nrow(graph)),
    PACKAGE = "speakeasyr"
  )$membership
}
