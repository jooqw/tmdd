#ifndef COMMON_H
#define COMMON_H

typedef unsigned long u64;
typedef unsigned int u32;
typedef unsigned short u16;
typedef unsigned char u8;
typedef long s64;
typedef int s32;
typedef short s16;
typedef char s8;

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

#define INDENT_SPACE "    "

#define LOG_OK printf(" OK\n")

void panic(const char* msg) {
    fprintf(stderr, "\nPANIC: %s\nExiting ..\n", msg);
    exit(1);
}

#endif
