//================ Copyright (c) 2019, PG, All rights reserved. =================//
//
// Purpose:		main entry point (nintendo switch)
//
// $NoKeywords: $main
//===============================================================================//

#include "cbase.h"

#ifdef __SWITCH__

//#define MCENGINE_HORIZON_NXLINK

#ifndef MCENGINE_FEATURE_SDL
#error SDL2 is currently required for switch builds
#endif

#include "HorizonSDLEnvironment.h"
#include "ConVar.h"

#include <switch.h>

#include <unistd.h> // for close()

static int s_nxlinkSock = -1;

static void initNxLink()
{
    if (R_FAILED(socketInitializeDefault())) return;

    s_nxlinkSock = nxlinkStdio();

    if (s_nxlinkSock >= 0)
        printf("printf output now goes to nxlink server");
    else
        socketExit();
}

static void deinitNxLink()
{
    if (s_nxlinkSock >= 0)
    {
        close(s_nxlinkSock);
        socketExit();
        s_nxlinkSock = -1;
    }
}

extern "C" void userAppInit()
{
#ifdef MCENGINE_HORIZON_NXLINK

	initNxLink();

#endif
}

extern "C" void userAppExit()
{
#ifdef MCENGINE_HORIZON_NXLINK

	deinitNxLink();

#endif
}

extern int mainSDL(int argc, char *argv[], SDLEnvironment *customSDLEnvironment);

int main(int argc, char* argv[])
{
	int ret = 0;

	// TODO: user selection applet popup, somehow
	/*
	u128 userID;
	accountGetPreselectedUser(&userID);
	*/

	romfsInit();
	{
		appletSetScreenShotPermission(AppletScreenShotPermission_Enable);
		appletSetFocusHandlingMode(AppletFocusHandlingMode_SuspendHomeSleepNotify); // TODO: seems broken? no notification is received when going to sleep

		// NOTE: yuzu emulator timing bug workaround (armGetSystemTick() is way too fast), uncomment for testing
		//convar->getConVarByName("host_timescale")->setValue(0.018f);

		ret = mainSDL(argc, argv, new HorizonSDLEnvironment());
	}
	romfsExit();

	return ret;
}

#endif
