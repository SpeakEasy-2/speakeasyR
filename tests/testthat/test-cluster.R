seed <- 1234
graph <- igraph::graph.famous("zachary")
expected_membership <- matrix(c(
  8, 8, 8, 8, 0, 0, 0, 8, 5, 4, 0, 6, 8, 8, 3, 3, 0, 8, 3, 8, 3, 8, 3, 7, 2,
  2, 1, 7, 2, 1, 5, 2, 3, 3
), 1, 34)

test_that("matrix works", {
  graph_i <- as.matrix(as.matrix(graph))
  actual <- speakeasyr::cluster(graph_i, seed = seed)
  expect_equal(actual, expected_membership)
})

test_that("sparse matrix works", {
  graph_i <- as.matrix(graph)
  actual <- speakeasyr::cluster(graph_i, seed = seed)
  expect_equal(actual, expected_membership)
})

test_that("igraph works", {
  actual <- speakeasyr::cluster(graph, seed = seed)
  expect_equal(actual, expected_membership)
})
