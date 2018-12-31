//================ Copyright (c) 2018, PG, All rights reserved. =================//
//
// Purpose:		discord rpc handler
//
// $NoKeywords: $discord-rpc
//===============================================================================//

#ifndef DISCORDINTERFACE_H
#define DISCORDINTERFACE_H

#include "cbase.h"

#ifdef MCENGINE_FEATURE_DISCORD

#include "discord_rpc.h"

#endif

class DiscordInterface
{
public:
	DiscordInterface();
	~DiscordInterface();

#ifdef MCENGINE_FEATURE_DISCORD

	// callbacks
	static void onReady(const DiscordUser *request);
	static void onError(int errorCode, const char *message);
	static void onDisconnected(int errorCode, const char *message);
	static void onJoinGame(const char *joinSecret);
	static void onSpectateGame(const char *spectateSecret);
	static void onJoinRequest(const DiscordUser *request);

#endif

	void setRichPresence(UString key, UString value, bool pool = false);

	inline bool isReady() const {return m_bReady;}

private:
	bool m_bReady;

#ifdef MCENGINE_FEATURE_DISCORD

	DiscordRichPresence m_presence;

	UString m_sPresenceState;
	UString m_sPresenceDetails;
	UString m_sPresenceLargeImageKey;
	UString m_sPresenceLargeImageText;
	UString m_sPresenceSmallImageKey;
	UString m_sPresenceSmallImageText;
	UString m_sPresencePartyID;
	UString m_sPresenceMatchSecret;
	UString m_sPresenceJoinSecret;
	UString m_sPresenceSpectateSecret;

#endif
};

extern DiscordInterface *discord;

#endif
