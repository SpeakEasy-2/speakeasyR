if (require("igraph")) {
  seed <- 1234

  ## Generate simple highly structured graph SE2 should be able to easily
  ## cluster.
  set.seed(5678)
  n_nodes <- 40
  n_types <- 4
  mu <- 0.2
  pref <- matrix(mu, n_types, n_types)
  diag(pref) <- 1 - mu
  graph <- igraph::sample_pref(n_nodes, types = n_types, pref.matrix = pref)
  gt_membership <- igraph::V(graph)$type

  test_that("matrix works", {
    graph_i <- as.matrix(as.matrix(graph))
    actual <- speakeasyR::cluster(graph_i, seed = seed)
    expect_gt(igraph::compare(gt_membership, actual, "nmi"), 0.9)
  })

  test_that("sparse matrix works", {
    graph_i <- as.matrix(graph)
    actual <- speakeasyR::cluster(graph_i, seed = seed)
    expect_gt(igraph::compare(gt_membership, actual, "nmi"), 0.9)
  })

  test_that("igraph works", {
    actual <- speakeasyR::cluster(graph, seed = seed)
    expect_gt(igraph::compare(gt_membership, actual, "nmi"), 0.9)
  })

  test_that("Results are reproducible", {
    actual_1 <- speakeasyR::cluster(graph, seed = seed)
    graph_i <- as.matrix(graph)
    actual_2 <- speakeasyR::cluster(graph_i, seed = seed)
    expect_equal(actual_1, actual_2)
  })
}
