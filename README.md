# Algorithm Engineering for Repartitioning Dynamic Graphs

This project was created as part of my Bachelor's Thesis in Informatics at the Technical University of Munich, titled _Algorithm Engineering for Repartitioning Dynamic Graphs_. The final thesis is available at [thesis.pdf](thesis.pdf).

## HOPage in code
```cpp
//Init update module
auto cm_alg = std::make_unique<crep::comp_mig_hop_age>(2, threshold);

//Init base module
auto alg = std::make_unique<crep::brp_crep_age>(uid, alpha, num_vertices, num_partitions, partition_size, augmentation, std::move(cm_alg));
alg->init();

//Init partition module
alg->init_p_alg<crep::part_simple>(crep::get_target_part_simple, "simple", ps_random, vtp_random);
```

## Complexity of HOPage
Here we break down the theoretical worst-case complexity of **_O(n^3 logn)_** that our algorithm HOPage requires *per request*.

| Operation | Complexity |
| --- | --- |
| 1) `comp_graph::change_weight()` | **O(n)** |
| 2) `comp_mig_hop_age::get_mig_vtxs()` | **O(n^3)** |
| 2a) `explore_hop()` | O(n · (logn + n + n · (n + 2n + logn))) ∈ O(n^3) |
| 2b) `charikar_age()` | O(n logn + n^2 logn + n^2 logn) ∈ O(n^2 logn) |
| 3) logn · `comp_graph::merge()` | logn · O(n^3) ∈ **O(n^3 logn)** |
| 3a) `graph::remove_edge()` | O(n) |
| 3b) `graph::change_edges()` | O(n · (n + n)) ∈ O(n^2) |
| 3c) Update `base_graph` | O(n^2 · n) ∈ O(n^3) |
| 4) `part_simple::migrate()` | O(n + (n + n + l^2 logl) + n + l logl) ∈ **O(n + l^2 logl)** |
| 5) `comp_graph::delete_comp()` | **O(n^3)** |
| 5a) `graph::remove_adjacent_edges()` | O(n^2) |
| 5b) n^2 · `graph::update_edge()` | n^2 · O(n) ∈ O(n^3) |
| 5c) Reset components | O(n) |

## Project structure
- CREP: source code
- eval: plot scripts, log data used for creating the plots in the thesis
- config: configuration files for reproducing the results in the thesis
- mdb_offline: originally contained offline copies of the traces used for evaluation

## Relevant implementation files
- CMakeLists.txt
- CREP/assign_initial.h
- CREP/brp_alg.h
- CREP/brp_crep_age.h
- CREP/charikar.h
- CREP/CMakeLists.txt
- CREP/comp_graph.cpp
- CREP/comp_graph.h
- CREP/comp_mig_age.h
- CREP/comp_mig_alg.h
- CREP/comp_mig_cc_age.h
- CREP/comp_mig_greedy_age.h
- CREP/comp_mig_hop_age.h
- CREP/create_offline_copy.cpp
- CREP/create_offline_copy.h
- CREP/data_mongo_offline.h
- CREP/data_source.h
- CREP/debug.h
- CREP/explore_neighborhood.h
- CREP/graph.cpp
- CREP/graph.h
- CREP/graph_extended.h
- CREP/logger2.h
- CREP/part_alg.h
- CREP/part_simple.h
- CREP/run_tests.cpp
- CREP/run_tests.h
- CREP/static_partitioning.h
- CREP/typedefs.h
- CREP/union_find.cpp
- CREP/union_find.h
- CREP/utility.cpp
- CREP/utility.h

## Acknowledgement

I would like to thank my advisors, Prof. Dr. Harald Räcke and Univ.-Prof. Dr. Stefan Schmid, for all the insightful discussions and their continuous support.

<br/>

Copyright © 2020 Martin Schonger  
This work is licensed under the GPLv3.
