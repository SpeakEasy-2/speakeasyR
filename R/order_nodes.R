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
#'   graph <- igraph::graph.famous("zachary")
#'   memb <- speakeasyr::cluster(graph)
#'   index <- speakeasyr::order_nodes(graph, memb)
#' }
order_nodes <- function(graph, membership, is_directed = "detect") {
  graph <- se2_as_matrix_i(graph)
  if (is_directed == "detect") {
    is_directed <- !Matrix::isSymmetric(graph)
  }

  .Call(C_order_nodes, graph, membership, is_directed)
}
