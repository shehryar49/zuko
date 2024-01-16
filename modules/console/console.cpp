
#include "console.h"
#include <Windows.h>

ZObject nil;
ZObject init()
{
	nil.type = Z_NIL;
	Module* m = vm_allocModule();
	m->name = "console";
	Module_addNativeFun(m,"setTextAttribute",&setTextAttribute);
	Module_addNativeFun(m,"show", &show);
	Module_addNativeFun(m,"hide", &hide);
	Module_addNativeFun(m,"gotoxy", &gotoxy);
	Module_addNativeFun(m,"getDimensions", &getDimensions);

	return ZObjFromModule(m);
}

ZObject setTextAttribute(ZObject* args, int n)
{
	if (n != 1)
		return Z_Err(ArgumentError, "1 argument needed!");
	if (args[0].type != Z_INT)
		return Z_Err(TypeError, "Integer 32 bit needed!");

	HANDLE hand = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hand, args[0].i);
	return nil;
}
ZObject show(ZObject* args, int n)
{
	if (n != 0)
	  return Z_Err(ArgumentError, "0 arguments needed!");
		
	ShowWindow(GetConsoleWindow(), SW_SHOW);
	return nil;
}
ZObject hide(ZObject* args, int n)
{
	if (n != 0)
		return Z_Err(ArgumentError, "0 arguments needed!");
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	return nil;
}
ZObject getDimensions(ZObject* args, int n)
{
	if (n != 0)
	  return Z_Err(ArgumentError, "0 arguments needed!");
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int c, r;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	c = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	r = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	ZList* dim = vm_allocList();
	ZList_push(dim,ZObjFromInt(r));
	ZList_push(dim,ZObjFromInt(c));
	return ZObjFromList(dim);
}

ZObject gotoxy(ZObject* args, int n)
{
	if (n != 2)
		return Z_Err(ArgumentError, "2 arguments needed!");
	if (args[0].type != 'i' || args[1].type != 'i')
		return Z_Err(TypeError, "Integer argument needed!");
		
	COORD pos = { args[0].i,args[1].i };
	HANDLE hand = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hand,pos);
	return nil;
}
