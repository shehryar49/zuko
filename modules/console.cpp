#include "pch.h"
#include "console.h"
#include <Windows.h>
PltObject nil;
PltObject init()
{
	Module* m = vm_allocModule();
	m->name = "console";
	m->members.emplace(("setTextAttribute"),PObjFromFunction("setTextAttribute",&setTextAttribute));
	m->members.emplace(("show"), PObjFromFunction("show", &show));
	m->members.emplace(("hide"), PObjFromFunction("hide", &hide));
	m->members.emplace(("gotoxy"), PObjFromFunction("gotoxy", &gotoxy));
	m->members.emplace(("getDimensions"), PObjFromFunction("getDimensions", &getDimensions));

	return PObjFromModule(m);
}

PltObject setTextAttribute(PltObject* args, int n)
{
	if (n != 1)
	{
		return Plt_Err(ARGUMENT_ERROR, "1 argument needed!");
		
	}
	if (args[0].type != PLT_INT)
	{
		return Plt_Err(TYPE_ERROR, "Integer 32 bit needed!");
		
	}
	HANDLE hand = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hand, args[0].i);
	return nil;
}
PltObject show(PltObject* args, int n)
{
	if (n != 0)
	{
		return Plt_Err(ARGUMENT_ERROR, "0 arguments needed!");
		
	}
	ShowWindow(GetConsoleWindow(), SW_SHOW);
	return nil;
}
PltObject hide(PltObject* args, int n)
{
	if (n != 0)
	{
		return Plt_Err(ARGUMENT_ERROR, "0 arguments needed!");
		
	}
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	return nil;
}
PltObject getDimensions(PltObject* args, int n)
{
	if (n != 0)
	{
		return Plt_Err(ARGUMENT_ERROR, "0 arguments needed!");
		
	}
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	int c, r;
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
	c = csbi.srWindow.Right - csbi.srWindow.Left + 1;
	r = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
	PltList* dim = vm_allocList();
	dim->push_back(PObjFromInt(r));
	dim->push_back(PObjFromInt(c));
	return PObjFromList(dim);
}

PltObject gotoxy(PltObject* args, int n)
{
	if (n != 2)
	{
		return Plt_Err(ARGUMENT_ERROR, "2 arguments needed!");
		
	}
	if (args[0].type != 'i' || args[1].type != 'i')
	{
		return Plt_Err(TYPE_ERROR, "Integer argument needed!");
		
	}
	COORD pos = { args[0].i,args[1].i };
	HANDLE hand = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hand,pos);
	return nil;
}
