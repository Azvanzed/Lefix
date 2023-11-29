#ifndef HPP_ASSERT
#define HPP_ASSERT

#include <iostream>

#define CRASH(reason, ...) { printf("[%s:%d] " reason "\n", __FILE__, __LINE__, ##__VA_ARGS__); exit(EXIT_FAILURE); }
#define ASSERT(condition, reason, ...) if (!(condition)) CRASH(reason, ##__VA_ARGS__)

#endif