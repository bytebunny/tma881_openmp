[//]: # (To preview markdown file in Emacs type C-c C-c p)

# [Assignment 3: OpenMP](https://www.raum-brothers.eu/martin/Chalmers_TMA881_1920/assignments.html#openmp)
> In this assignment, we count distances between points in 3-dimensional space. One possibility to think of this is as cells, whose coordinates were obtained by diagnostics and which require further processing.

> Distances are symmetric, i.e. distance between two cells c\_1 and c\_2 and the distance in reverse direction c\_2 to c\_1 should be counted once.

## Relevant concepts


## Intended program layout

Following the structure of the task, the program is split into the following tasks:

1. Reading coordinates from a text file.
1. Computing distances between the cells and counting frequencies.
1. Printing to standard output (screen).


### Reading coordinates from a file

Knowing that the format for the triplet of coordinates **X, Y, Z** is fixed to 5 significant digits for each coordinate (e.g. `+01.330 -09.035 +03.489`), it was decided to read the numbers as **integers** (i.e. as `+1330 -9035 +3489`). However, a restriction was imposed by the task on the amount of memory that the program could use. That is why, in order to handle files that would require more than 1 GiB of memory if read at once, a partial reading procedure was adopted. **_WU DA, PLEASE CONTINUE FROM HERE, POSSIBLY IN THE SUBSECTION CALLED MEMORY MANAGEMENT._**

### Computing distances and counts

The task asked to compute distances and counts with the precision of 2 decimals and let us assume that the cells are bound to reside on the interval `[-10; 10]`. This means that the largest distance between the cells is **20 * &radic;3 ~= 34.64**, and that there are `3465` possible distances to be counted.

The distances were computed simply as

**distance = &radic; &Delta;X^2 + &Delta;Y^2 + &Delta;Z^2**

Next, the corresponding indices in the frequency array were obtained from

**index = (int) (distance/10 + 0.5)**

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
