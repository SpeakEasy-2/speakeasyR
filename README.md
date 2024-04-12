# speakeasyr Community Detection
  [![R-CMD-check](https://github.com/SpeakEasy-2/r-speakeasy2/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/SpeakEasy-2/r-speakeasy2/actions/workflows/R-CMD-check.yaml)

This packages provides R functions for running the SpeakEasy 2 community detection algorithm using the [SpeakEasy2 C library](https://github.com/speakeasy-2/libspeakeasy2). See the [Genome Biology article.](https://genomebiology.biomedcentral.com/articles/10.1186/s13059-023-03062-0).

SpeakEasy 2 (SE2) is a graph community detection algorithm that aims to be performant on large graphs and robust, returning consistent results across runs. SE2 does not require precognition about the number of communities in the network. Additionally, while the user can provide parameters to alter how the algorithm is run, the default option work well on a wide arrange of graphs and tweaking options generally has little affect on the results, reducing the risk of influencing the algorithm.

The core algorithm is written in C, providing speed and keeping the memory requirments low. This implementation can take advantage of multiple computing cores without increasing memory usage. SE2 can detect community structure across scales, making it a good choice for biological data, which is often organized hierchical structure.

Graphs can be passed to the algorithm as adjacency matrices using the Matrix library, igraph graphs, or any data that can coerced into a matrix.

## Installation

## Building from source
