if (require("igraph")) {
  seed <- 1234
  graph <- igraph::graph.famous("zachary")
  expected_membership <- c(
    9, 9, 9, 9, 1, 1, 1, 9, 3, 8, 1, 2, 9, 9, 5, 5, 1, 9, 5, 9, 5, 9, 5, 6, 7,
    7, 6, 6, 4, 6, 3, 7, 5, 5
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
