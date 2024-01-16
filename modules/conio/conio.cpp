#include "conio.h"
#include <conio.h>
ZObject init()
{
	Module* d = vm_allocModule();
	d->name = "conio";
	Module_addNativeFun(d,"kbhit", &KBHIT);
	Module_addNativeFun(d,"getch", &GETCH);
	Module_addNativeFun(d,"getche",&GETCHE);
	return ZObjFromModule(d);
}

ZObject GETCH(ZObject* args, int n)
{
	if (n != 0)
	  return Z_Err(ArgumentError, "0 arguments needed!");
	return ZObjFromInt(_getch());
}

ZObject GETCHE(ZObject* args, int n)
{
	if (n != 0)
	  return Z_Err(ArgumentError, "0 arguments needed!");
	return ZObjFromInt(_getche());
}
ZObject KBHIT(ZObject* args, int n)
{
	if (n != 0)
	  return Z_Err(ArgumentError, "0 arguments needed!");
	return ZObjFromBool(_kbhit());
}
