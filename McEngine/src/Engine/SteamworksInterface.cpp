//================ Copyright (c) 2018, PG, All rights reserved. =================//
//
// Purpose:		steamworks api wrapper
//
// $NoKeywords: $steam
//===============================================================================//

#include "SteamworksInterface.h"

#include "Engine.h"

#ifdef MCENGINE_FEATURE_STEAMWORKS

#include "steam_api.h"

#endif

#define STEAMWORKS_APPID 0

SteamworksInterface *steam = NULL;

SteamworksInterface::SteamworksInterface()
{
	steam = this;

	m_bReady = false;

	if (engine->getArgs().length() > 0 && engine->getArgs().find("nosteam") != -1) return;

#ifdef MCENGINE_FEATURE_STEAMWORKS

	// init and autorestart via steam if necessary
	// NOTE: this forces users to use steam, and if someone started without steam then it was probably intentional, therefore commented for now
	/*
	if (SteamAPI_RestartAppIfNecessary(STEAMWORKS_APPID))
	{
		debugLog("STEAM: RestartAppIfNecessary, shutting down ...\n");
		engine->shutdown();
		return;
	}
	*/

	if (!SteamAPI_Init())
	{
		debugLog("STEAM: SteamAPI_Init() failed!\n");
		return;
	}

	m_bReady = true;

	debugLog("STEAM: Logged in as \"%s\"\n", SteamFriends()->GetPersonaName());

#endif
}

SteamworksInterface::~SteamworksInterface()
{
	steam = NULL;

#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (m_bReady)
		SteamAPI_Shutdown();

#endif

	m_bReady = false;
}

void SteamworksInterface::setRichPresence(UString key, UString value)
{
#ifdef MCENGINE_FEATURE_STEAMWORKS

	if (!m_bReady) return;
	if (key.length() < 1) return;

	SteamFriends()->SetRichPresence(key.toUtf8(), value.toUtf8());

#endif
}
