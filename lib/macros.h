#ifndef __MACRO__
#define __MACRO__

#include <iostream>
#include "gl_include.h"

#define CAT(X,Y) X ## Y 

#define PRINT(X) std::cout << X 
#define PRINTLN(X) std::cout << X << std::endl

#define VERBOSE1(X) << #X << ": " << X << ", "
#define VERBOSE2(X, ...) VERBOSE1(X) VERBOSE1(__VA_ARGS__) 
#define VERBOSE3(X, ...) VERBOSE1(X) VERBOSE2(__VA_ARGS__) 
#define VERBOSE4(X, ...) VERBOSE1(X) VERBOSE3(__VA_ARGS__) 
#define VERBOSE5(X, ...) VERBOSE1(X) VERBOSE4(__VA_ARGS__) 
#define VERBOSE6(X, ...) VERBOSE1(X) VERBOSE5(__VA_ARGS__) 
#define VERBOSE7(X, ...) VERBOSE1(X) VERBOSE6(__VA_ARGS__) 
#define VERBOSE8(X, ...) VERBOSE1(X) VERBOSE7(__VA_ARGS__) 
#define VERBOSE(N, ...) std::cout CAT(VERBOSE, N) (__VA_ARGS__) << std::endl 

#define VERBOSE_VEC1(X, N) << X[N] << ", "
#define VERBOSE_VEC2(N, X) VERBOSE_VEC1(X, N) VERBOSE_VEC1((N-1), X) 
#define VERBOSE_VEC3(N, X) VERBOSE_VEC1(X, N) VERBOSE_VEC2((N-1), X) 
#define VERBOSE_VEC4(N, X) VERBOSE_VEC1(X, N) VERBOSE_VEC3((N-1), X) 
#define VERBOSE_VEC5(N, X) VERBOSE_VEC1(X, N) VERBOSE_VEC4((N-1), X) 
#define VERBOSE_VEC6(N, X) VERBOSE_VEC1(X, N) VERBOSE_VEC5((N-1), X) 
#define VERBOSE_VEC7(N, X) VERBOSE_VEC1(X, N) VERBOSE_VEC6((N-1), X) 
#define VERBOSE_VEC8(N, X) VERBOSE_VEC1(X, N) VERBOSE_VEC7((N-1), X) 
#define VERBOSE_VEC(N, X) std::cout << #X << ": " CAT(VERBOSE_VEC, N) (N, X) << std::endl

#define GLM_VERBOSE(X) std::cout << #X << ": " << glm::to_string(X) << std::endl;

#endif
