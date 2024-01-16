#pragma once
#include "zapi.h"
#define EXPORT __declspec(dllexport)

extern "C"
{
	EXPORT ZObject init();
	EXPORT ZObject GETCH(ZObject*, int);
	EXPORT ZObject GETCHE(ZObject*, int);
	EXPORT ZObject KBHIT(ZObject*, int);
}
