####8-Puzzle

Shunji Lin

* A* search, 3 choices for heuristic:

1. no heuristic
2. misplaced tile heuristic
3. manhattan distance heuristic

* Open/Closed List implemented using hash table with double probing
* Priority Queue implemented with array of linked-list, array indexed by f-values
* Encoding of states as hexadecimal according to position of tiles, takes ~36 bits per state

Using manhattan distance heuristic, solves on average ~0.01s (cpu time) for board instance with 100,000 randomly generated steps, on Macbook Pro 2.3GHz intel i7, 4gb RAM




