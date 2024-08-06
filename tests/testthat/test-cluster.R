if (require("igraph")) {
  seed <- 1234
  graph <- igraph::graph.famous("zachary")
  expected_membership <- c(
    3, 3, 3, 3, 4, 4, 4, 3, 1, 8, 4, 2, 3, 3, 6, 6, 4, 3, 6, 3, 6, 3, 6, 9,
    7, 7, 9, 9, 5, 9, 1, 7, 6, 6
  )

  test_that("matrix works", {
    graph_i <- as.matrix(as.matrix(graph))
    actual <- speakeasyR::cluster(graph_i, seed = seed)
    expect_equal(actual, expected_membership)
  })

  test_that("sparse matrix works", {
    graph_i <- as.matrix(graph)
    actual <- speakeasyR::cluster(graph_i, seed = seed)
    expect_equal(actual, expected_membership)
  })

  test_that("igraph works", {
    actual <- speakeasyR::cluster(graph, seed = seed)
    expect_equal(actual, expected_membership)
  })
}
