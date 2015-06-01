Abstract         {#mainpage}
========

The initial purpose of the library was studing lock-free technics and data structures. But outcome
exceeded expectetions:

- lock-free versions of queues outperformed stl version of queue more than two times in multy thread environment.
- lock-free version of `hash_map` and `hash_set` demostrated multi-thred performance far beyond of expected limits. They outperformed stl akins even in single thread environment.

The library includes performance test (`bin/perf-test`) utility that allows you to check differences. Be aware that tests compsed to check cases that are friendly for stl maps.

Supported compilers
-------------------

- gcc v4.3 and later

*Note:* in order to unlock C++11 features you need at least gcc v4.8

Supported platforms
-------------------

* amd64
* x86 - _not tested_

 