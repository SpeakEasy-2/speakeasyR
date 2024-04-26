#' Group nodes by community
#'
#' Useful for viewing community structure of a graph in a heatmap.
#'
#' Reorders the a graph to group nodes in the same community together.
#' Communities are ordered by size, so nodes in the largest community are
#' first. Within a community, nodes are order by highest-to-lowest degree.
#'
#' If membership is in matrix form (the output from speakeasyr::cluster with
#' subcluster > 1) a matrix is returned, with the first level based on the
#' first level of membership. At the second level, nodes are still grouped by
#' the first level community, but each first level community is reorganized
#' into the second level communities.
#'
#' See vignette for a multilevel example.
#'
#' @param graph The graph or adjacency matrix the membership vector was created
#'   for.
#' @param membership A vector or matrix listing node communities. The output
#'   from speakeasyr::cluster (should also work for other clustering algorithms
#'   that return membership in the same format).
#' @param is_directed Whether the graph should be treated as directed or not.
#'   By default, if the graph is symmetric it is treated as undirected.
#'
#' @return An index vector or matrix. The number of rows are equal to the value
#'   of subcluster passed to speakeasyr::cluster.
#' @export
#'
#' @examples
#' if (require("igraph")) {
#'   n_nodes <- 1000
#'   n_types <- 10
#'   mu <- 0.3 # Mixing parameter (likelihood an edge is between communities).
#'   pref <- matrix(mu, n_types, n_types)
#'   diag(pref) <- 1 - mu
#'   g <- igraph::preference.game(n_nodes, types = n_types, pref.matrix = pref)
#'   # Use a dense matrix representation to easily apply index.
#'   adj <- as(g[], "matrix")
#'   memb <- speakeasyr::cluster(adj, seed = 222)
#'   ordering <- speakeasyr::order_nodes(adj, memb)
#'   heatmap(adj[ordering, ordering], scale = "none", Rowv = NA, Colv = NA)
#' }
order_nodes <- function(graph, membership, is_directed = "detect") {
  graph <- se2_as_matrix_i(graph)
  if (is_directed == "detect") {
    is_directed <- !Matrix::isSymmetric(graph)
  }

  .Call(C_order_nodes, graph, membership, is_directed)
}
