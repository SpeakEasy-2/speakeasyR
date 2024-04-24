se2_is_matrix_i <- function(obj) {
  class(obj)[[1]] %in% c("dgCMatrix", "matrix")
}

se2_as_matrix_i <- function(adj_like) {
  if (!se2_is_matrix_i(adj_like)) {
    adj_like <- as.matrix(adj_like)
  }

  if (!se2_is_matrix_i(adj_like)) {
    stop(paste0(
      "Could not convert adj_like to an appropriate type. ",
      "'as.matrix' converted adj_like to \"", class(adj_like)[[0]],
      "\". Currently implemented classes are c(\"matrix\", \"dgCMatrix\"). ",
      "Please open an issue on github to add support for new types."
    ))
  }

  if (nrow(adj_like) != ncol(adj_like)) {
    stop("Graph adjacency matrix must be square.")
  }

  adj_like
}
