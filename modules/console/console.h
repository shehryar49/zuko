#pragma once
#include "C:\plutonium\PltObject.h"

#define EXPORT __declspec(dllexport)
extern "C"
{
	EXPORT PltObject init();
	EXPORT PltObject setTextAttribute(PltObject*,int);
	EXPORT PltObject show(PltObject*, int );
	EXPORT PltObject hide(PltObject*, int );
	EXPORT PltObject setPixel(PltObject*, int );
	EXPORT PltObject gotoxy(PltObject*, int );
	EXPORT PltObject getDimensions(PltObject*, int );

}