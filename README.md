[//]: # (To preview markdown file in Emacs type C-c C-c p)

# [Assignment 3: OpenMP](https://www.raum-brothers.eu/martin/Chalmers_TMA881_1920/assignments.html#openmp)
> In this assignment, we count distances between points in 3-dimensional space. One possibility to think of this is as cells, whose coordinates were obtained by diagnostics and which require further processing.

> Distances are symmetric, i.e. distance between two cells c\_1 and c\_2 and the distance in reverse direction c\_2 to c\_1 should be counted once.

## Relevant concepts
* **Elementary functions.** Complicated mathematical operations should be avoided when possible. In this assignment, it is better to multiply the coordinate differences by itself rather than square them when computing the cell distance. 
* **Use of intrinsics.** In order to speed up the computation, we can make use of intrinsics for computing the distances between the coordinates. To this end, SSE functions like `__m128 _mm_sqrt_ps`or `__m128 _mm_sqrt_ps` can be employed. This will be considered for implementation if the program has difficulties passing the performance tests.
* **Inlining.** We've learned that providing additional information to the compiler about external functions can be advantageous, especially if function is called multiple times during runtime. 
* **Synchronization of parallel computations.** Since distances do not need to be stored, it is only necessary to increment the corresponding entries in the frequency array. To this end, when computing the distances between cells in different blocks, a suitable sum `reduction` may be used.
* **Efficient reading and parsing of the input file.** Since the format of the coordinates in the input file is fixed, we can optimise the reading process. To this end, we implemented a custom character to short conversion instead of using the standard library function `strtol`. This improved the performance significantly.  Because of the external limitation of the program not using more than 1GiBi of memory, we need special treatments for large input files. For details, see the section **Reading coordinates from a file**.
* **Efficient computation of the distances.** Due to accuracy tolerance it is possible to streamline the computation of the distances. Because of the design decision to store the coordinates as integers, it is rather cheap to compute individual coordinate differences and square them. The `sqrtf` function is called only once, and the result is then rounded and cast into an integer. In order to round the answer, we decided not to use the standard `round` function, as it proved costly. Instead, we round the distance manually. 
* **Efficient use of memory and cache.** Everything we've learned about memory and locality applies in this assignment as well. Ideally we would like to perform as many operations with data loaded from memory as possible to avoid reloading it. For example, the coordinates of a specific cell are likely to be contained within a cache line, making computation of the coordinate difference and squaring it efficient. 

## Intended program layout

Following the structure of the task, the program is split into the following tasks:

1. Reading coordinates from a text file.
1. Computing distances between the cells and counting frequencies.
1. Printing to standard output (screen).


### Reading coordinates from a file

Knowing that the format for the triplet of coordinates **X, Y, Z** is fixed to 5 significant digits for each coordinate (e.g. `+01.330 -09.035 +03.489`), it was decided to read the numbers as **shorts** (i.e. as `+1330 -9035 +3489`). Since the coordinates are between `-10` and `10`, it is possible to store them as signed shorts between `-10000` and `10000`. However, a restriction was imposed by the task on the amount of memory that the program could use. That is why, in order to handle files that would require more than 1 GiB of memory if read at once, a partial reading procedure was adopted. In the extreme, for 2^32 cells the file takes about 100 GB of disk space. If the cells are to be stored in the memory as integers, we would need 48GB of memory if the coordinates are stored as integers or 24 GB of memory when we store them as shorts. Either way, it is not possible to handle such large input at once. The strategy here can be to decide (based e.g. on the size of the input file) to split the reading into blocks that will fit into the memory. For the sake of completeness of the results, it is necessary to load two blocks of cells into memory at once. After computing the distances between the blocks, we keep one of them and parse next block of the input. We repeat this until all the distances between cells from the first block and the rest are computed. After that we load next block and treat it as the base block. Note that once a cell block is in memory, we can use this opportunity and compute the distance between the coordinates in that block, saving time. This way we are able to compute distances between all the points without parsing the input too many times. 

### Memory management
The programm cannot access more than 1 GiBi of memory, in other words, 

### Computing distances and counts

The task asked to compute distances and counts with the precision of 2 decimals and let us assume that the cells are bound to reside on the interval `[-10; 10]`. This means that the largest distance between the cells is **20 * &radic;3 ~= 34.64**, and that there are `3465` possible distances to be counted.

The distances were computed simply as

**distance = &radic; &Delta;X^2 + &Delta;Y^2 + &Delta;Z^2**

Next, the corresponding indices in the frequency array were obtained from

**index = (short) (distance/10 + 0.5)**

Note that division by `10` is performed to retain only 4 significant digits (present in number `3465`) and `0.5` was added to round up to the nearest integer instead of the largest integer not larger than `distance`. Then, the counter of the corresponding element in the frequency array was incremented:

**++freqArray[ index ]**


#### Parallelisation

Looping over cells was performed in parallel by means of the `parallel for` pragma presented in the lecture. Since distances are symmetric, the computations involved the triangular part of the matrix illustrated below.

![distance_matrix](./figures/distance_illustration.png)

Here, indices `I` and `J` denote cells (up to `N` cells) between which the distance is computed. Therefore, the matrix traversal is performed by means of two nested loops:
```
for ( I = 0; I < N; ++I ){
    for ( J = 0; J < N; ++J ){
        // compute distance.
        // compute index.
        // increment count: ++freqArray[ index ]
    }
}
```
The **private** variables are `I, J`, `distance` and `index`. The **shared** variables are the number of points `N` and the array of frequencies `freqArray`, to which the reduction is done via `+:freqArray[:3465]`.

Lastly, no sorting is necessary for printing, as the elements of `freqArray` already represent distances in ascending order.
