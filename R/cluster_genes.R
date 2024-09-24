#' Cluster a gene expression matrix
#'
#' @description
#' Use the Speakeasy 2 community detection algorithm to cluster genes based on
#'   their gene expression. A gene coexpression network is created by taking
#'   correlating the input gene expression matrix to genes that tend to be
#'   expressed together. This matrix is then clustered to find gene modules.
#'
#' Note: This is intended for gene expression sampled from bulk sequencing.
#'   Samples from single cell sequencing may work but will need to be
#'   preprocessed due to the greater noise-to-signal ratio. See the speakeasyR
#'   vignette for an example of single cell preprocessing. For more information
#'   about working with single cell data see:
#'   Malte D Luecken & Fabian J Theis (2019) Current Best Practices in
#'   Single‐cell Rna‐seq Analysis: a Tutorial, Molecular Systems Biology.
#'
#' @param gene_expression a matrix of gene expression data with data from
#'   multiple samples (in the form genes x samples).
#' @param k number of neighbors to include if converting to a k-nearest
#'   neighbor graph. Should be a non-negative integer less than the number of
#'   genes. If this value is not set the raw GCN is clustered. The kNN graph is
#'   a sparse directed graph with binary edges between a node and it's most
#'   similar k neighbors. Conversion to a kNN graph can provide good clustering
#'   results much faster than using the full graph in cases with a large number
#'   of genes.
#' @inheritParams cluster
#'
#' @return A membership vector. If subclustering, returns a matrix with number
#'   of rows equal to the number of recursive clustering. Each row is the
#'   membership at different hierarchical scales, such that the last rows are
#'   the highest resolution.
#' @export
#'
#' @examples
#' # Set parameters
#' set.seed(123) # For reproducibility
#' ngene <- 200
#' nsample <- 1000
#' ncluster <- 5
#'
#' # Create a function to simulate gene expression data
#' simulate_gene_expression <- function(ngene, nsample, ncluster) {
#'   # Initialize the expression matrix
#'   expr_matrix <- matrix(0, nrow = ngene, ncol = nsample)
#'
#'   # Create cluster centers for genes
#'   cluster_centers <- matrix(rnorm(ncluster * nsample, mean = 5, sd = 2),
#'     nrow = ncluster, ncol = nsample
#'   )
#'
#'   # Assign genes to clusters
#'   gene_clusters <- sample(1:ncluster, ngene, replace = TRUE)
#'
#'   for (i in 1:ngene) {
#'     cluster <- gene_clusters[i]
#'     expr_matrix[i, ] <- cluster_centers[cluster, ] +
#'       rnorm(nsample, mean = 0, sd = 1)
#'   }
#'
#'   return(list(expr_matrix = expr_matrix, gene_clusters = gene_clusters))
#' }
#'
#' # Simulate the data
#' simulated_data <- simulate_gene_expression(ngene, nsample, ncluster)
#'
#' # Extract the expression matrix and gene clusters
#' expr_matrix <- simulated_data$expr_matrix
#' gene_clusters <- simulated_data$gene_clusters
#'
#' # Cluster and test quality of results
#' modules <- cluster_genes(expr_matrix, max_threads = 2)
cluster_genes <- function(gene_expression, k = NULL, discard_transient = 3,
                          independent_runs = 10, max_threads = 0, seed = 0,
                          target_clusters = 0, target_partitions = 5,
                          subcluster = 1, min_clust = 5, verbose = FALSE) {
  gcn <- stats::cor(t(gene_expression))

  is_directed <- FALSE
  if (!is.null(k)) {
    gcn <- speakeasyR::knn_graph(gcn, k)
    is_directed <- TRUE
  }

  speakeasyR::cluster(gcn,
    discard_transient = discard_transient,
    independent_runs = independent_runs, max_threads = max_threads, seed = seed,
    target_clusters = target_clusters, target_partitions = target_partitions,
    subcluster = subcluster, min_clust = min_clust, verbose = verbose,
    is_directed = is_directed
  )
}
