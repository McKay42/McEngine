//================ Copyright (c) 2018, PG, All rights reserved. =================//
//
// Purpose:		steamworks api wrapper
//
// $NoKeywords: $steam
//===============================================================================//

#ifndef STEAMWORKSINTERFACE_H
#define STEAMWORKSINTERFACE_H

#include "cbase.h"

class SteamworksInterface
{
public:
	SteamworksInterface();
	~SteamworksInterface();

	void setRichPresence(UString key, UString value);

	inline bool isReady() const {return m_bReady;}

private:
	bool m_bReady;
};

extern SteamworksInterface *steam;

#endif
