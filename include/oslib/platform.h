#ifndef OSLIB_PLATFORM_H
#define OSLIB_PLATFORM_H

//NOTE: @Jon
//Specific typedefs for data type sizes for different compilers

//TODO: @Jon
//Add more compilers to this as required

#ifdef _MSC_VER
#if _WIN32
typedef char i8;
typedef short i16;
typedef int i32;
typedef long long i64;

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;

typedef float f32;
typedef double f64;
#endif
#endif

#endif