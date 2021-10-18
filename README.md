# Path Finder

    DISCLAIMER

    The assertion formerly made regarding the 'improvement' to Dijkstra's
    algorithm has since been noted to come at a cost to its accuracy; the
    contents of the algorithm is inaccurately categorised as 'Dijkstra's' and
    should instead be regarded as a related algorithm with a notable performance
    increase over Dijkstra's Algorithm but without the 'best-path' guarantee
    offered by Dijkstra's node ordering scheme. In most cases, the FIFO variation
    on Dijkstra's Algorithm (enclosed) will outperform A* in accuracy and both
    A* and Dijkstra in speed.

    (OLD TEXT AND ALGORITHM DESCRIPTION IS INCLUDED BELOW:)

    Reimplementation of Dijkstra's algorithm and the A* algorithm with improvements
    to the former.

    Latest release: version 1.0.1, built 01/01/2017 (DD/MM/YYYY).

    Improved Dijkstra's Algorithm uses a FIFO (First-In-First-Out) method for
    ordering node evaluation. NO sorting is conducted (for improved speed). Nodes
    are evaluated in the order they are queued.

    A* algorithm is also included for comparison.

    Benchmarking is provided to compare the two path-finding algorithms on their
    performance separately.

    Current version of software will only compile on a Windows platform.

    Distribute freely and enjoy.
