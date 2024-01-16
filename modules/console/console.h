#pragma once
#include "zapi.h"

#define EXPORT __declspec(dllexport)
extern "C"
{
	EXPORT ZObject init();
	EXPORT ZObject setTextAttribute(ZObject*,int);
	EXPORT ZObject show(ZObject*, int );
	EXPORT ZObject hide(ZObject*, int );
	EXPORT ZObject setPixel(ZObject*, int );
	EXPORT ZObject gotoxy(ZObject*, int );
	EXPORT ZObject getDimensions(ZObject*, int );

}
