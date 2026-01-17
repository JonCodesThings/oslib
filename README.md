# oslib
## a C98 library of common OS-independent utilities

This library provides a few common utilities with an OS-independent API.

For license see LICENSE.md

## Feature Modules

oslib is split into a few different modules.

### platform.h
Contains compiler and OS-specific definitions for types used by other modules.

### timer.h
A platform independent API for creating and managing high-precision timers.

### io.h
Provides common file I/O utilities such as reading or writing to files or searching directories.

### hot_reload.h
Provides an API for hot-reloading for two different scenarios: dynamic libraries and normal files.
For dynamic libraries this allows for function pointers to be queried.