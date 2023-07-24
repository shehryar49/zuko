#include "pch.h"
#include "conioWrapper.h"
#include <conio.h>
PltObject init()
{
	Module* d = vm_allocModule();
	d->name = "conio";
	d->members.emplace(("kbhit"), PObjFromFunction("conio.kbhit", &KBHIT));
	d->members.emplace(("getch"), PObjFromFunction("conio.getch", &GETCH));
	d->members.emplace(("getche"), PObjFromFunction("conio.getche", &GETCHE));
	return PObjFromModule(d);
}

PltObject GETCH(PltObject* args, int n)
{
	if (n != 0)
	{
		return Plt_Err(ARGUMENT_ERROR, "0 arguments needed!");
		
	}
	return PObjFromInt(_getch());
}

PltObject GETCHE(PltObject* args, int n)
{
	if (n != 0)
	{
		return Plt_Err(ARGUMENT_ERROR, "0 arguments needed!");
		
	}
	return PObjFromInt(_getche());
}
PltObject KBHIT(PltObject* args, int n)
{
	if (n != 0)
	{
		return Plt_Err(ARGUMENT_ERROR, "0 arguments needed!");

	}
	return PObjFromBool(_kbhit());
}
