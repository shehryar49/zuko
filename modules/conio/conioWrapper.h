#pragma once
#include "C:\plutonium\PltObject.h"
#define EXPORT __declspec(dllexport)

extern "C"
{
	EXPORT PltObject init();
	EXPORT PltObject GETCH(PltObject*, int);
	EXPORT PltObject GETCHE(PltObject*, int);
	EXPORT PltObject KBHIT(PltObject*, int);
}