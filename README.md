[![Build Status](https://travis-ci.com/IYP-Programer-Yeah/WaitFreeRingBufferUtilities.svg?branch=master)](https://travis-ci.com/IYP-Programer-Yeah/WaitFreeRingBufferUtilities)
[![CodeFactor](https://www.codefactor.io/repository/github/iyp-programer-yeah/waitfreeringbufferutilities/badge)](https://www.codefactor.io/repository/github/iyp-programer-yeah/waitfreeringbufferutilities)
[![codecov](https://codecov.io/gh/IYP-Programer-Yeah/WaitFreeRingBufferUtilities/branch/master/graph/badge.svg)](https://codecov.io/gh/IYP-Programer-Yeah/WaitFreeRingBufferUtilities)

This is a header-only library that provides wait-free ring buffer utilities for C++ objects.

+ Non-intrusive: All the nodes are pre-allocated inside the ring buffer.
+ Fixed size: The size of the ring buffer is fixed at compile time, and attempts to push will fail if the ring buffer is full.
+ No need for garbage collection.
+ No need for thread registeration.
+ The underlying data structure is an array.
+ Is wait-free, read further [here](https://en.wikipedia.org/wiki/Non-blocking_algorithm#Wait-freedom).
+ Provides MPMC, SPMC, MPSC, and SPSC ring buffers.
+ This is a header only library, and if you're using C++17 it does not rely on any 3rd party libraries, on C++11 you are going to need
Boost optional.

# Motives

## Most of the available libraries do not support C++ objects

Most of the available libraries do not support C++ objects at all, or if they do they don't support them properly.
Available libraries are usually writen in C and support pointers only. This usually results in difficulties in memory management and
disables RAII. It also requires a thread safe object pool that usually yeilds worse performance than that of a single ring-buffer (Even
though this design on part of the C libraries adds overhead, this overhead is not accounted for on the benchmarks).

## Most of the available libraries are lock-free

Being lock-free doesn't necessarily gaurantee that threads are not going to wait on each other even though there are no locks. Wait-free does gaurantee this. This gaurantee reduces the overhead of each thread on the other, this is also supported by the benchmarks.

# Blog Posts

I have two blog post on this ring buffer design. One explaining the algorithm itself, and the part two providing some benchmark
comparisons:

+ [Part 1](https://iyp.home.blog/2019/11/14/what-to-do-when-your-ring-buffers-are-tired-of-waiting/)
+ [Part 2](https://iyp.home.blog/2020/04/05/what-to-do-when-your-ring-buffers-are-tired-of-waiting-pt-2/)
