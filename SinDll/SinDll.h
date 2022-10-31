#pragma once

#ifdef HEADERLIBRARY_EXPORTS
#define HEADERLIBRARY_API __declspec(dllexport)
#else
#define HEADERLIBRARY_API __declspec(dllimport)
#endif

extern "C" HEADERLIBRARY_API double func(double x);
extern "C" HEADERLIBRARY_API char name[];
extern "C" HEADERLIBRARY_API int priority;
extern "C" HEADERLIBRARY_API bool isBynary;
