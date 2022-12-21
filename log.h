#ifndef _LOG_H_
#define _LOG_H_
#include <stdio.h>
#include <stdlib.h>

#define LOG(...) printf(__VA_ARGS__); printf("\n");
#define ASSERT(label, x) if ((x)) {printf("\033[0;32mAssertion %s passed", label); printf("\033[0m\n");} else {printf("\033[0;31mAssertion %s failed", label); printf("\033[0m\n"); exit(-1);}
#define ERROR(...) printf("\033[0;31mFatal: "); printf(__VA_ARGS__); printf("\033[0m\n"); exit(-1);
#endif