//================ Copyright (c) 2018, PG, All rights reserved. =================//
//
// Purpose:		discord rpc handler
//
// $NoKeywords: $discord-rpc
//===============================================================================//

#include "DiscordInterface.h"

#include "Engine.h"

#define DISCORD_APPID "0"
#define DISCORD_STEAMWORKS_APPID "0"

DiscordInterface *discord = NULL;

DiscordInterface::DiscordInterface()
{
	discord = this;

	m_bReady = false;

	if (engine->getArgs().length() > 0 && engine->getArgs().find("nodiscord") != -1) return;

#ifdef MCENGINE_FEATURE_DISCORD

	memset(&m_presence, 0, sizeof(DiscordRichPresence));

	DiscordEventHandlers handlers;
	memset(&handlers, 0, sizeof(handlers));

	handlers.ready = DiscordInterface::onReady;
	handlers.errored = DiscordInterface::onError;
	handlers.disconnected = DiscordInterface::onDisconnected;
	handlers.joinGame = DiscordInterface::onJoinGame;
	handlers.spectateGame = DiscordInterface::onSpectateGame;
	handlers.joinRequest = DiscordInterface::onJoinRequest;

	Discord_Initialize(DISCORD_APPID, &handlers, 1, DISCORD_STEAMWORKS_APPID);

	m_bReady = true;

#endif
}

DiscordInterface::~DiscordInterface()
{
	discord = NULL;

#ifdef MCENGINE_FEATURE_DISCORD

	if (m_bReady)
		Discord_Shutdown();

#endif

	m_bReady = false;
}

#ifdef MCENGINE_FEATURE_DISCORD

void DiscordInterface::onReady(const DiscordUser *request)
{
	// unused
}

void DiscordInterface::onError(int errorCode, const char *message)
{
	debugLog("DISCORD: Error, %i, %s\n", errorCode, message);
}

void DiscordInterface::onDisconnected(int errorCode, const char *message)
{
	// unused
}

void DiscordInterface::onJoinGame(const char *joinSecret)
{
	// unused
}

void DiscordInterface::onSpectateGame(const char *spectateSecret)
{
	// unused
}

void DiscordInterface::onJoinRequest(const DiscordUser *request)
{
	// unused
}

#endif

void DiscordInterface::setRichPresence(UString key, UString value, bool pool)
{
#ifdef MCENGINE_FEATURE_DISCORD

	if (!m_bReady || value.length() > 128) return;

	if (key == "state")
	{
		m_sPresenceState = value;
		m_presence.state = m_sPresenceState.toUtf8();
	}
	else if (key == "details")
	{
		m_sPresenceDetails = value;
		m_presence.details = m_sPresenceDetails.toUtf8();
	}
	else if (key == "startTimestamp")
		m_presence.startTimestamp = (int64_t)value.toLong();
	else if (key == "endTimestamp")
		m_presence.endTimestamp = (int64_t)value.toLong();
	else if (key == "largeImageKey" && value.length() <= 32)
	{
		m_sPresenceLargeImageKey = value;
		m_presence.largeImageKey = m_sPresenceLargeImageKey.toUtf8();
	}
	else if (key == "largeImageText")
	{
		m_sPresenceLargeImageText = value;
		m_presence.largeImageText = m_sPresenceLargeImageText.toUtf8();
	}
	else if (key == "smallImageKey" && value.length() <= 32)
	{
		m_sPresenceSmallImageKey = value;
		m_presence.smallImageKey = m_sPresenceSmallImageKey.toUtf8();
	}
	else if (key == "smallImageText")
	{
		m_sPresenceSmallImageText = value;
		m_presence.smallImageText = m_sPresenceSmallImageText.toUtf8();
	}
	else if (key == "partyId")
	{
		m_sPresencePartyID = value;
		m_presence.partyId = m_sPresencePartyID.toUtf8();
	}
	else if (key == "partySize")
		m_presence.partySize = value.toInt();
	else if (key == "partyMax")
		m_presence.partyMax = value.toInt();
	else if (key == "matchSecret")
	{
		m_sPresenceMatchSecret = value;
		m_presence.matchSecret = m_sPresenceMatchSecret.toUtf8();
	}
	else if (key == "joinSecret")
	{
		m_sPresenceJoinSecret = value;
		m_presence.joinSecret = m_sPresenceJoinSecret.toUtf8();
	}
	else if (key == "spectateSecret")
	{
		m_sPresenceSpectateSecret = value;
		m_presence.spectateSecret = m_sPresenceSpectateSecret.toUtf8();
	}
	else if (key == "instance")
		m_presence.instance = (int8_t)value.toInt();

	if (!pool)
		Discord_UpdatePresence(&m_presence);

#endif
}
