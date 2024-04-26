#' K-nearest neighbors graph
#'
#' @param mat a matrix to be compared column-by-column.
#' @param k how many nearest neighbors to collect.
#'
#' @return A directed sparse adjacency matrix with `k * ncol(mat)` nonzero
#'   edges. Each column has k edges connected to the k closest columns (not
#'   including itself).
#' @export
#'
#' @examples
#' if (require("scRNAseq")) {
#'   rnaseq <- scRNAseq::FletcherOlfactoryData()
#'   cell_types <- rnaseq$cluster_id
#'
#'   ## Filter genes with low expression. Remove any genes with less than 10 cells
#'   ## with at least 1 reads.
#'   counts <- assay(rnaseq, "counts")
#'   indices <- apply(counts, 1, function(gene) sum(gene > 0) > 10)
#'   counts <- counts[indices, ]
#'
#'   ## Normalize by shifted logarithm
#'   target <- median(colSums(counts))
#'   size_factors <- colSums(counts) / target
#'   counts_norm <- log(t(t(counts) / size_factors + 1))
#'
#'   ## Dimension reduction
#'   counts_norm <- t(prcomp(t(counts_norm), scale. = FALSE)$x)[1:50, ]
#'
#'   adj <- speakeasyr::knn_graph(counts_norm, k)
#' }
knn_graph <- function(mat, k) {
  if (!is.matrix(mat)) {
    stop("Matrix must be of type matrix.")
  }

  components <- .C(C_knn_graph, as.double(mat), as.integer(k),
    as.integer(ncol(mat)), as.integer(nrow(mat)),
    sp_p = integer(ncol(mat) + 1), sp_i = integer(ncol(mat) * k)
  )

  Matrix::sparseMatrix(
    i = components$sp_i, p = components$sp_p,
    dims = c(ncol(mat), ncol(mat)),
    dimnames = list(colnames(mat), colnames(mat)),
    index1 = FALSE, repr = "C"
  )
}
