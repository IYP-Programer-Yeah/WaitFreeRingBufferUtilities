[![Build Status](https://travis-ci.com/IYP-Programer-Yeah/WaitFreeRingBufferUtilities.svg?branch=master)](https://travis-ci.com/IYP-Programer-Yeah/WaitFreeRingBufferUtilities)
[![CodeFactor](https://www.codefactor.io/repository/github/iyp-programer-yeah/waitfreeringbufferutilities/badge)](https://www.codefactor.io/repository/github/iyp-programer-yeah/waitfreeringbufferutilities)

This is  a header only C++ library that provides wait-free ring buffer utilities for C++ objects.

+ Non-intrusive: All the nodes are pre-allocated inside the ring buffer.
+ Fixed size: The size of the ring buffer is fixed at compile time, and attempts to push will fail if the ring buffer is full.
+ No need for garbage collection.
+ No need for thread registeration.
+ The underlying data structure is an array.
+ Is wait-free, read further [here](https://en.wikipedia.org/wiki/Non-blocking_algorithm#Wait-freedom).
+ Provides MPMC, SPMC, MPSC, and SPSC ring buffer.
+ This is a header only library, and if you're using C++17 it does not rely on any 3rd part libraries, on C++11 you are going to need
boos optional.

# Motives

There are two main motives:

- Most of the available libraries do no support C++ objects at all, or if they do they don't support it properly.
- Most of these libraries are lock-free rather than wait-free and yeild worse performance on high thread counts.

# Blog Posts

I have two blog post on this ring buffer deisgn. One explaining the algorithm itself, and the part two providing some benchmark
comparisons:

[Part 1](https://iyp.home.blog/2019/11/14/what-to-do-when-your-ring-buffers-are-tired-of-waiting/)
[Part 2](https://iyp.home.blog/2020/04/05/what-to-do-when-your-ring-buffers-are-tired-of-waiting-pt-2/)
