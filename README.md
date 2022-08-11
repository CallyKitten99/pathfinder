# Path Finder

    DISCLAIMER

    The assertion formerly made regarding the 'improvement' to Dijkstra's
    algorithm has since been noted to come at a cost to its accuracy; the
    contents of the algorithm is inaccurately categorised as 'Dijkstra' and
    should instead be regarded as a related algorithm with a notable performance
    increase over Dijkstra's Algorithm but without the 'best-path' guarantee
    offered by Dijkstra's node ordering scheme. In most cases, the FIFO ordering
    will outperform A* in accuracy and both A* and Dijkstra in speed.

    (SOME OLD TEXT AND ALGORITHM DESCRIPTION IS INCLUDED BELOW:)

    Reimplemented: Dijkstra's algorithm and the A* algorithm with modifications
    to the former.

    Latest release: version 1.0.1, built 01/01/2017 (DD/MM/YYYY).

    Improved Dijkstra's Algorithm coalesces nodes in a FIFO (First-In-First-Out)
    fashion, eliminating the algorithm bottleneck. Thus no sorting is conducted
    between coalition of nodes and evaluation (for improved speed). I.e, nodes
    are evaluated in the order they are queued.

    Benchmarking is provided to compare the two path-finding algorithms on their
    performances separately.

    Current version of software will only compile on a Windows platform.

    Distribute freely and enjoy.
