# Par

Par is a portfolio SMT solver. It consists of a simple wrapper that
runs a number of other SMT solvers in parallel, and returns as soon as
some of them have found an answer (i.e., sat or unsat).

  Usage: par SOLVERS SOLUTIONS BENCHMARK

    SOLVERS is the (maximum) number of solvers that will be run in
    parallel. Set to 0 to use the number of hardware threads.

    SOLUTIONS is the number of solvers that need to agree on the
    answer before it is reported.

    BENCHMARK is the name of the benchmark file.

The wrapped solvers were taken from SMT-COMP 2018.
