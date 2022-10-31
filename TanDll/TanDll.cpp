// MathLibrary.cpp : Defines the exported functions for the DLL.
#include "pch.h" // use stdafx.h in Visual Studio 2017 and earlier
#include <utility>
#include <limits.h>
#include "TanDll.h"
#include <cmath>

char name[] = "tan";
bool isBynary = false;
int priority = 3;

double func(double x) {
    return tan(x);
}