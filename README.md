# oslib - a C98 library of common OS-independent utilities

This library provides a few common utilities with an OS-independent API.

# Modules

oslib is split into a few different modules.

## platform.h
Contains compiler and OS-specific definitions for types used by other modules.

## timer.h
A platform independent API for creating high-precision timers.

## io.h
Provides common file I/O utilities such as reading or writing to files or searching directories.

## hot_reload.h
Enables hot-reloading for two different scenarios: dynamic libraries and normal files.