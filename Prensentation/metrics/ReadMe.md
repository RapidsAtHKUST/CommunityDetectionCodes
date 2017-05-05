# Metrics

## Overview

- comparing partitions => to be applied in community detection result(graph vertex, community) => overlapping communities

rand index(TP+TN/all), same as accuracy => adjusted rand index, (index-expected_index)/(max_index-expected_index) => omega index

mutual information => normalized mutual information => overlap normalized mutual information

- assuming the modular structure of communities => overlapping communities

## Overlap Normalized Mutual Information

- comm_x => random variable X_k, two possible values(combinations), probability
- comm_x, comm_y => joint distribution, four possible values(combinations), probability
- entropy function, conditional entropy
- overlapping extension, min(H(X_k|Y_l)) for all Y_l in Y
- in case of the eligible condition random variable(from comm_y) is far from being the negatives of the random variable from comm_x

## Adjusted Rand Index & Omega Index

