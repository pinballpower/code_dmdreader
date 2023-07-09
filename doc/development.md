# Development tips & tricks

## Platform

It's often best to develop on a modern PC instead of a Raspberry Pi. DMDReader has been developed mostly on Windows. Compilation is a lot faster on a PC than on a Raspberry Pi.

## Memory

Note that DMDReader uses some C++ frameworks. Modern C++ frameworks use a lot of templates. The good thing about this: You can write very clean code that is also very fast. 
The bad thing: the compiler has a lot of work to do to deal with all these templates. This means on a slow system it will take some time to compile it. You also need a lot of
memory. Use a system with at least 2GB of RAM. If you run muliple jobs in parallel, have at least 2GB RAM physical RAM per job.
