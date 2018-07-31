//================ Copyright (c) 2015, PG, All rights reserved. =================//
//
// Purpose:		handles network connections, multiplayer, chat etc.
//
// $NoKeywords: $nw
//===============================================================================//

#include "NetworkHandler.h"

#include "Engine.h"
#include "ConVar.h"

#include <sstream>

#ifdef MCENGINE_FEATURE_NETWORKING

#include <curl.h>

#endif

#define MC_PROTOCOL_VERSION 1
#define MC_PROTOCOL_TIMEOUT 10000

void DUMMY_ARGS_CONNECT(UString args) {;}
void DUMMY_DISCONNECT() {;}

ConVar _name_("name", "McKay");

#ifdef MCENGINE_FEATURE_NETWORKING

ConVar _connect_("connect", DUMMY_ARGS_CONNECT);
ConVar connect_duration("connect_duration", 5.0f, "Time in seconds to wait for a response from the server when trying to connect");
ConVar _disconnect_("disconnect", DUMMY_DISCONNECT);
ConVar disconnect_duration("disconnect_duration", 3.0f, "Time in seconds to wait for a gentle disconnect before dropping the connection");

ConVar _host_("host", DUMMY_DISCONNECT);
ConVar _stop_("stop", DUMMY_DISCONNECT);
ConVar host_port("host_port", 7777.0f);
ConVar host_max_clients("host_max_clients", 16.0f);
ConVar _status_("status", DUMMY_DISCONNECT);

ConVar debug_network("debug_network", false);
ConVar debug_network_time("debug_network_time", false);

ConVar _name_admin_("name_admin", "ADMIN");
ConVar _say_("say", DUMMY_ARGS_CONNECT);
ConVar _kick_("kick", DUMMY_ARGS_CONNECT);

#endif

NetworkHandler::NetworkHandler()
{
	m_bReady = false;
	m_fDebugNetworkTime = 0.0f;
	m_iLocalClientID = -1;

#ifdef MCENGINE_FEATURE_NETWORKING

	// initialize
	if (enet_initialize() != 0)
	{
		engine->showMessageError("ENet Error", "Couldn't enet_initialize()!");
		return;
	}
	if (curl_global_init(CURL_GLOBAL_WIN32 | CURL_GLOBAL_SSL) != CURLE_OK)
	{
		engine->showMessageError("CURL Error", "Couldn't curl_global_init()!");
		return;
	}

	// convar callbacks
	_host_.setCallback( fastdelegate::MakeDelegate(this, &NetworkHandler::host) );
	_stop_.setCallback( fastdelegate::MakeDelegate(this, &NetworkHandler::hostStop) );
	_status_.setCallback( fastdelegate::MakeDelegate(this, &NetworkHandler::status) );
	_connect_.setCallback( fastdelegate::MakeDelegate(this, &NetworkHandler::connect) );
	_disconnect_.setCallback( fastdelegate::MakeDelegate(this, &NetworkHandler::disconnect) );

	_say_.setCallback( fastdelegate::MakeDelegate(this, &NetworkHandler::say) );
	_kick_.setCallback( fastdelegate::MakeDelegate(this, &NetworkHandler::kick) );

	m_client = NULL;
	m_server = NULL;
	m_clientPeer = NULL;

	m_bClientConnectPending = false;
	m_fClientConnectPendingTime = 0.0f;
	m_bClientDisconnectPending = false;
	m_fClientDisconnectPendingTime = 0.0f;
	m_bClientConnectPendingAfterDisconnect = false;

	// callbacks
	m_localServerStartedListener = NULL;
	m_localServerStoppedListener = NULL;

	m_clientDisconnectedFromServerListener = NULL;
	m_clientConnectedToServerListener = NULL;

	m_clientSendInfoListener = NULL;
	m_serverSendInfoListener = NULL;

	m_clientReceiveServerInfoListener = NULL;
	m_serverReceiveClientInfoListener = NULL;

	m_clientReceiveServerPacketListener = NULL;
	m_serverReceiveClientPacketListener = NULL;
	m_serverClientChangeListener = NULL;

	m_iIDCounter = 1;	// WARNING: id 0 is reserved! (for the host client entity, = Admin)

	m_bReady = true;

#endif
}

NetworkHandler::~NetworkHandler()
{
#ifdef MCENGINE_FEATURE_NETWORKING

	if (m_clientPeer != NULL)
		enet_peer_reset(m_clientPeer);
	if (m_client != NULL)
		enet_host_destroy(m_client);
	if (isServer())
		enet_host_destroy(m_server);

	if (m_bReady)
	{
		enet_deinitialize();
		curl_global_cleanup();
	}

	m_bReady = false;

#endif
}

UString NetworkHandler::httpGet(UString url, long timeout, long connectTimeout)
{
#ifdef MCENGINE_FEATURE_NETWORKING

	CURL *curl = curl_easy_init();

	if (curl != NULL)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url.toUtf8());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "User-Agent: McEngine");
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connectTimeout);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);

		// HACKHACK: TODO: aaaaaaaaaaaaaaaaaaaaaaaaaa
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlStringWriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlReadBuffer);

		/*CURLcode res = */curl_easy_perform(curl);
		curl_easy_cleanup(curl);

		return UString(curlReadBuffer.c_str());
	}
	else
	{
		debugLog("NetworkHandler::httpGet() error, curl == NULL!\n");
		return "";
	}

#else

	return "";

#endif
}

std::string NetworkHandler::httpDownload(UString url, long timeout, long connectTimeout)
{
#ifdef MCENGINE_FEATURE_NETWORKING

	CURL *curl = curl_easy_init();

	if (curl != NULL)
	{
		curl_easy_setopt(curl, CURLOPT_URL, url.toUtf8());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, "User-Agent: McEngine");
		curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, connectTimeout);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
		curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "deflate");

		// HACKHACK: TODO: aaaaaaaaaaaaaaaaaaaaaaaaaa
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

		std::stringstream curlWriteBuffer(std::stringstream::in | std::stringstream::out | std::stringstream::binary);

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curlStringStreamWriteCallback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &curlWriteBuffer);

		CURLcode res = curl_easy_perform(curl);
		if (res != CURLE_OK)
		{
			curlWriteBuffer = std::stringstream();
			debugLog("NetworkHandler::httpDownload() error, code %i!\n", (int)res);
		}

		curl_easy_cleanup(curl);

		return curlWriteBuffer.str();
	}
	else
	{
		debugLog("NetworkHandler::httpDownload() error, curl == NULL!\n");
		return std::string("");
	}

#else

	return "";

#endif
}

void NetworkHandler::connect(UString address)
{
#ifdef MCENGINE_FEATURE_NETWORKING

	if (!m_bReady) return;

	if (address.length() < 1)
		address = "localhost";

	// destroy previous client
	if (m_clientPeer != NULL)
	{
		m_sServerAddress = address; // save address for later auto-connecting after successfully disconnecting
		m_bClientConnectPendingAfterDisconnect = true;
		disconnect();
		return;
	}

	// reset
	if (m_client != NULL)
		enet_host_destroy(m_client);

	m_client = NULL;
	m_clientPeer = NULL;

	// create the client
	m_client = enet_host_create(NULL /* create a client host */,
								1 /* only allow 1 outgoing connection */,
								2 /* allow up 2 channels to be used, 0 and 1 */,
								0 /* unlimited downstream bandwidth */,
								0 /* unlimited upstream bandwidth */);

	if (m_client == NULL)
	{
		debugLog(0xffff0000, "CLIENT: An error occurred while trying to create the client.\n");
		return;
	}

	// start the connection
	ENetAddress adr;

	// connect to the server
	enet_address_set_host(&adr, address.toUtf8());
	adr.port = host_port.getInt();

	// initiate connection, up to two channels (0 and 1)
	m_clientPeer = enet_host_connect(m_client, &adr, 2, 0);

	if (m_clientPeer == NULL)
	{
		debugLog(0xffffff00, "CLIENT: No free slot available.\n");
		return;
	}

	// wait up to connect_duration seconds for the connection to succeed
	m_bClientConnectPending = true;
	m_fClientConnectPendingTime = engine->getTime() + connect_duration.getFloat();

	m_sServerAddress = address;
	debugLog("CLIENT: Trying to connect to \"%ls\" ... (%.1f seconds(s) timeout)\n", address.length() == 0 ? L"localhost" : address.wc_str(), connect_duration.getFloat());

#endif
}

void NetworkHandler::disconnect()
{
#ifdef MCENGINE_FEATURE_NETWORKING

	if (!m_bReady) return;

	if (m_clientPeer == NULL)
	{
		debugLog("CLIENT: Not connected.\n");
		return;
	}

	// start gentle disconnect
	enet_peer_disconnect(m_clientPeer, 0);
	m_bClientDisconnectPending = true;
	m_fClientDisconnectPendingTime = engine->getTime() + disconnect_duration.getFloat();

	debugLog("CLIENT: Trying to gently disconnect... (%.1f second(s) timeout)\n", disconnect_duration.getFloat());

#endif
}

void NetworkHandler::host()
{
#ifdef MCENGINE_FEATURE_NETWORKING

	if (!m_bReady) return;

	m_vConnectedClients.clear();

	if (isServer())
		hostStop();

	debugLog("SERVER: Starting local server on port %i...\n", host_port.getInt());

	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = host_port.getInt();

	m_server = enet_host_create (&address,
								 host_max_clients.getInt(),
								 2 /* allow up to 2 channels to be used, 0 and 1 */,
								 0 /* unlimited downstream bandwidth */,
								 0 /* unlimited upstream bandwidth */);

	if (m_server == NULL)
	{
		debugLog(0xffff0000, "SERVER: An error occurred while trying to create the server.\n");
		return;
	}

	debugLog(0xff00ff00, "SERVER: Local server is up and running.\n");

	if (m_localServerStartedListener != NULL)
		m_localServerStartedListener();

#endif
}

void NetworkHandler::hostStop()
{
#ifdef MCENGINE_FEATURE_NETWORKING

	if (m_server == NULL)
	{
		debugLog("SERVER: Not running.\n");
		return;
	}

	// drop local client if one exists
	if (isClient())
		clientDisconnect();

	// notify listener
	if (m_localServerStoppedListener != NULL)
		m_localServerStoppedListener();

	debugLog("SERVER: Stopped local server.\n");

	enet_host_destroy(m_server);

	m_server = NULL;

#endif
}

void NetworkHandler::status()
{
#ifdef MCENGINE_FEATURE_NETWORKING

	// if we are the server
	if (isServer())
	{
		debugLog("\n");
		debugLog("version: %i\n", MC_PROTOCOL_VERSION);

		debugLog("hostname: <TODO>\n");

		char host_attr[255];
		enet_address_get_host_ip(&m_server->receivedAddress, host_attr, 255);

		debugLog("udp/ip: %s:%i\n", host_attr, host_port.getInt());
		for (int c=0; c<m_vConnectedClients.size(); c++)
		{
			debugLog("# %ls %i", m_vConnectedClients[c].name.wc_str(), m_vConnectedClients[c].peer->roundTripTime);
		}
		debugLog("\n");
		return;
	}

	// if we are the client
	if (isClient())
	{
		debugLog("\n");
		debugLog("version: %i\n", MC_PROTOCOL_VERSION);

		debugLog("hostname: <TODO>\n");
		debugLog("udp/ip: %ls:%i\n", m_sServerAddress.length() == 0 ? L"localhost" : m_sServerAddress.wc_str(), host_port.getInt());
		debugLog("ping: %i\n", m_clientPeer->roundTripTime);
		debugLog("\n");
	}
	else
		debugLog(" Not connected to any server.\n");

#endif
}

void NetworkHandler::update()
{
#ifdef MCENGINE_FEATURE_NETWORKING

	if (!m_bReady) return;

	// handle client
	if (isClient())
	{
		ENetEvent event;
		while (enet_host_service(m_client, &event, 0) > 0)
		{
			onClientEvent(event);
		}
	}
	else if (m_bClientConnectPendingAfterDisconnect)
	{
		m_bClientConnectPendingAfterDisconnect = false;
		connect(m_sServerAddress);
	}

	// handle server
	if (isServer())
	{
		ENetEvent event;
		while (enet_host_service(m_server, &event, 0) > 0)
		{
			onServerEvent(event);
		}

		// handle client timeouts and kicked players
		if (m_vConnectedClients.size() > 0)
		{
			for (int c=0; c<m_vConnectedClients.size(); c++)
			{
				// standard timeout (force remove)
				if ((m_server->serviceTime - m_vConnectedClients[c].peer->lastReceiveTime) > MC_PROTOCOL_TIMEOUT)
				{
					debugLog("SERVER: %s timed out.\n", m_vConnectedClients[c].name.toUtf8());

					// notify local connection listener
					if (m_serverClientChangeListener != NULL)
						m_serverClientChangeListener(m_vConnectedClients[c].id, m_vConnectedClients[c].name, false);

					enet_peer_reset(m_vConnectedClients[c].peer);

					m_vConnectedClients.erase(m_vConnectedClients.begin()+c);
					c--;

					continue;
				}

				// kicked players (allow a 1 second grace period for the client to disconnect)
				if (m_vConnectedClients[c].kickTime != 0.0f && engine->getTime() > m_vConnectedClients[c].kickTime + 1.0f)
				{
					debugLog("SERVER: %s kicked.\n", m_vConnectedClients[c].name.toUtf8());
					enet_peer_disconnect(m_vConnectedClients[c].peer, 0);

					m_vConnectedClients[c].kickTime = 0.0f;
					m_vConnectedClients[c].kickKillTime = engine->getTime();
				}

				// kicked players (if the client did not disconnect after the grace period, kill him)
				if (m_vConnectedClients[c].kickKillTime != 0.0f && engine->getTime() > m_vConnectedClients[c].kickKillTime + 1.0f)
				{
					debugLog("SERVER: %s forcefully disconnected.\n", m_vConnectedClients[c].name.toUtf8());

					// TODO: this call was not here originally, check if it causes redundant/odd behaviour
					// notify local connection listener
					if (m_serverClientChangeListener != NULL)
						m_serverClientChangeListener(m_vConnectedClients[c].id, m_vConnectedClients[c].name, false);

					enet_peer_reset(m_vConnectedClients[c].peer);

					m_vConnectedClients.erase(m_vConnectedClients.begin()+c);
					c--;

					continue;
				}
			}
		}
	}

	// handle non-responding disconnection attempts from the client
	// also handle connection timeouts
	if ((m_bClientDisconnectPending && engine->getTime() > m_fClientDisconnectPendingTime)
	 || (m_bClientConnectPending && engine->getTime() > m_fClientConnectPendingTime))
		clientDisconnect();

	if (debug_network_time.getBool() && engine->getTime() > m_fDebugNetworkTime)
	{
		m_fDebugNetworkTime = engine->getTime() + 0.5f;
		if (isClient())
			debugLog("client time = %u\n", enet_time_get());
		else if (isServer())
			debugLog("server time = %u\n", enet_time_get());
	}

#endif
}

#ifdef MCENGINE_FEATURE_NETWORKING

size_t NetworkHandler::curlStringWriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::string*)userp)->append((char*)contents, size * nmemb);
	return size * nmemb;
}

size_t NetworkHandler::curlStringStreamWriteCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
	((std::stringstream*)userp)->write((const char*)contents, (size_t) size * nmemb);
	return size * nmemb;
}

void NetworkHandler::onClientEvent(ENetEvent e)
{
	switch (e.type)
	{
	case ENET_EVENT_TYPE_RECEIVE:
		if (debug_network.getBool())
			debugLog("CLIENT: A packet of length %u was received from %s on channel %u.\n", e.packet->dataLength, e.peer->data, e.channelID);

		if (e.packet->data != NULL)
		{
			switch (*((PACKET_TYPE*)e.packet->data))
			{
			case SERVER_INFO_PACKET_TYPE:
				{
					SERVER_INFO_PACKET *sp = (SERVER_INFO_PACKET*)e.packet->data;
					m_iLocalClientID = sp->id;

					debugLog("CLIENT: Received server info (%i)\n", sp->id);

					// notify extension packet listener
					bool valid = true;
					if (m_clientReceiveServerInfoListener != NULL && sp->extension)
						valid = m_clientReceiveServerInfoListener((void*)(e.packet->data +  sizeof(SERVER_INFO_PACKET)));

					// this is the last packet we needed before starting the game
					if (valid)
					{
						if (m_clientConnectedToServerListener != NULL)
							m_clientConnectedToServerListener();
					}
					else
						disconnect(); // if the extension packet listener returned false, disconnect
				}
				break;

			case CHAT_PACKET_TYPE:
				if (!isServer()) // to avoid double logs on local hosts, since the client has already received the message through onServerEvent (and can't send it to itself in the broadcast)
				{
					chatLog(UString(((struct CHAT_PACKET*)e.packet->data)->username).substr(0, ((struct CHAT_PACKET*)e.packet->data)->usize),
							UString(((struct CHAT_PACKET*)e.packet->data)->message).substr(0, ((struct CHAT_PACKET*)e.packet->data)->msize));
				}
				break;

			case CLIENT_BROADCAST_PACKET_TYPE:
				if (m_clientReceiveServerPacketListener != NULL)
				{
					CLIENT_BROADCAST_WRAPPER *wrapper = (CLIENT_BROADCAST_WRAPPER*)e.packet->data;

					// unwrap the packet
					char *unwrappedPacket = (char*)e.packet->data;
					const int wrapperSize = sizeof(CLIENT_BROADCAST_WRAPPER);
					unwrappedPacket += wrapperSize;

					// process
					if (!m_clientReceiveServerPacketListener(wrapper->id, unwrappedPacket, e.packet->dataLength - wrapperSize))
						debugLog("CLIENT: Received unknown CLIENT_PACKET_TYPE, WTF!\n");
				}
				break;

			default:
				debugLog("CLIENT: Received unknown packet of type %i, WTF!\n", *((PACKET_TYPE*)e.packet->data));
				break;
			}
		}

		// cleanup
		enet_packet_destroy(e.packet);
		break;

	case ENET_EVENT_TYPE_CONNECT:
		if (m_bClientConnectPending)
			debugLog(0xff00ff00, "CLIENT: Connected.\n");
		else
			debugLog(0xff00ff00, "CLIENT: Connected, but without a pending connection attempt, WTF!\n");

		m_bClientConnectPending = false;

		// we are connected, publish client info
		sendClientInfo();

		// now we are just waiting for the server info packet
		debugLog("CLIENT: Retrieving server info...\n");
		break;

	case ENET_EVENT_TYPE_DISCONNECT:
		if (m_bClientDisconnectPending)
			debugLog("CLIENT: Disconnected from Server. (Reason: Client disconnected)\n");
		else
			debugLog("CLIENT: Disconnected from Server. (Reason: Server disconnected)\n");

		clientDisconnect();
		break;
	}
}

void NetworkHandler::onServerEvent(ENetEvent e)
{
	switch (e.type)
	{
	case ENET_EVENT_TYPE_CONNECT:
		{
			debugLog("SERVER: A new client connected from %x:%u.\n", e.peer->address.host, e.peer->address.port);

			// store peer info
			e.peer->data = NULL; // TODO!

			CLIENT_PEER cp;
			cp.id = m_iIDCounter++; // HACKHACK: possible overflow if the server is running for centuries
			cp.kickTime = 0.0f;
			cp.kickKillTime = 0.0f;
			cp.peer = e.peer;

			// and send the client everything about us (this includes his ID)
			sendServerInfo(cp.id, m_server, cp.peer);

			// finally, add the new client
			m_vConnectedClients.push_back(cp);
		}
		break;

	case ENET_EVENT_TYPE_RECEIVE:
		if (debug_network.getBool())
			debugLog("SERVER: A packet of length %u was received from %s on channel %u.\n", e.packet->dataLength, e.peer->data, e.channelID);

		// switch on the different packet types
		if (e.packet->data != NULL)
		{
			switch (*((PACKET_TYPE*)e.packet->data))
			{
			case CLIENT_INFO_PACKET_TYPE:
				{
					CLIENT_INFO_PACKET *cp = new CLIENT_INFO_PACKET(); // 'new' here because this is stored in e.peer->data
					*cp = *(struct CLIENT_INFO_PACKET*)e.packet->data; // TODO: wtf is even happening here
					e.peer->data = cp;

					// fill CLIENT_PEER with all the info from the client info packet
					CLIENT_PEER *pp = getClientPeerByPeer(e.peer);
					if (pp != NULL)
					{
						pp->name = UString(((struct CLIENT_INFO_PACKET*)e.peer->data)->username).substr(0, ((struct CLIENT_INFO_PACKET*)e.peer->data)->size);

						// if the client is not running the same version
						if (cp->version != MC_PROTOCOL_VERSION)
						{
							debugLog("SERVER: User is trying to connect using version %i, but the server is running version %i.\n", cp->version, MC_PROTOCOL_VERSION);
							singlecastChatMessage("CONSOLE", UString::format("Version mismatch: Server is running version %i, but you are running version %i!", MC_PROTOCOL_VERSION, cp->version), m_server, e.peer);
							pp->kickTime = engine->getTime(); // initiate a graceful kick
						}
						else
						{
							// notify extension packet listener
							bool valid = true;
							if (m_serverReceiveClientInfoListener != NULL && cp->extension)
								valid = m_serverReceiveClientInfoListener((void*)(e.packet->data + sizeof(CLIENT_INFO_PACKET)));

							// notify the listener, now that we have everything
							if (valid)
							{
								if (m_serverClientChangeListener != NULL)
									m_serverClientChangeListener(pp->id, pp->name, true);
							}
							else
								pp->kickTime = engine->getTime(); // if the extension packet listener returned false, kick this client
						}
					}
					else
						debugLog("SERVER: NULL CLIENT_PEER!\n");
				}
				break;

			case CHAT_PACKET_TYPE:
				{
					CLIENT_PEER *pp = getClientPeerByPeer(e.peer);
					if (pp != NULL)
					{
						UString username = pp->name;
						UString message = UString(((struct CHAT_PACKET*)e.packet->data)->message).substr(0, ((struct CHAT_PACKET*)e.packet->data)->msize);

						// log it
						chatLog(username, message);

						// broadcast the message
						broadcastChatMessage((struct CHAT_PACKET*)e.packet->data, m_server, e.peer);

						if (message.find("!roll") != -1)
						{
							UString rollMessage = username;
							rollMessage.append(" rolls ");
							rollMessage.append(UString::format("%i point(s)", (rand() % 101)));

							broadcastChatMessage("CONSOLE", rollMessage, m_server, NULL);

							chatLog("CONSOLE", rollMessage);
						}
					}
					else
						debugLog("SERVER: NULL CLIENT_PEER!\n");
				}
				break;

			case CLIENT_BROADCAST_PACKET_TYPE:
				{
					// HACKHACK: TODO: this is dangerous, broadcasting all client broadcast requests by default if no server listener is set
					bool accepted = true;

					if (m_serverReceiveClientPacketListener != NULL)
					{
						CLIENT_PEER *pp = getClientPeerByPeer(e.peer);
						if (pp != NULL)
						{
							// unwrap the packet
							char *unwrappedPacket = (char*)e.packet->data;
							const int wrapperSize = sizeof(PACKET_TYPE);
							unwrappedPacket += wrapperSize;

							// process
							accepted = m_serverReceiveClientPacketListener(pp->id, unwrappedPacket, e.packet->dataLength - wrapperSize);
						}
						else
							debugLog("SERVER: NULL CLIENT_PEER!\n");
					}

					// broadcast the packet to all other clients
					if (accepted && m_vConnectedClients.size() > 0)
					{
						// unwrap the packet
						char *unwrappedPacket = (char*)e.packet->data;
						const int unWrapperSize = sizeof(PACKET_TYPE);
						unwrappedPacket += unWrapperSize;

						// create wrapped packet (with id!)
						CLIENT_BROADCAST_WRAPPER wrap;
						const int wrapperSize = sizeof(CLIENT_BROADCAST_WRAPPER);
						char wrappedPacket[(sizeof(CLIENT_BROADCAST_WRAPPER) + e.packet->dataLength - unWrapperSize)];
						// the id is not set here, but for every client in the loop below
						memcpy(((char*)&wrappedPacket) + wrapperSize, unwrappedPacket, (e.packet->dataLength - unWrapperSize));
						int size = sizeof(CLIENT_BROADCAST_WRAPPER) + e.packet->dataLength - unWrapperSize;

						for (int c=0; c<m_vConnectedClients.size(); c++)
						{
							// update id
							wrap.id = m_vConnectedClients[c].id;
							memcpy(&wrappedPacket, &wrap, wrapperSize);

							// create it
							ENetPacket *packet = enet_packet_create((const void*)wrappedPacket, size, 0);

							// send it
							if (m_vConnectedClients[c].peer != e.peer)
								enet_peer_send(m_vConnectedClients[c].peer, 0, packet);
						}

						enet_host_flush(m_server);
					}
				}
				break;

			case CLIENT_PACKET_TYPE:
				if (m_serverReceiveClientPacketListener != NULL)
				{
					CLIENT_PEER *pp = getClientPeerByPeer(e.peer);
					if (pp != NULL)
					{
						// unwrap the packet
						char *unwrappedPacket = (char*)e.packet->data;
						const int wrapperSize = sizeof(PACKET_TYPE);
						unwrappedPacket += wrapperSize;

						// process
						if (!m_serverReceiveClientPacketListener(pp->id, unwrappedPacket, e.packet->dataLength - wrapperSize))
							debugLog("SERVER: Received unknown CLIENT_PACKET_TYPE, WTF!\n");
					}
					else
						debugLog("SERVER: NULL CLIENT_PEER!\n");
				}
				break;

			default:
				debugLog("SERVER: Received unknown packet of type %i, WTF!\n", *((PACKET_TYPE*)e.packet->data));
				break;
			}
		}

		// cleanup
		enet_packet_destroy(e.packet);
		break;

	case ENET_EVENT_TYPE_DISCONNECT:
		CLIENT_PEER *pp = getClientPeerByPeer(e.peer);
		if (pp != NULL)
		{
			debugLog("SERVER: %s disconnected.\n", pp->name.toUtf8());

			// notify local connection listener
			if (m_serverClientChangeListener != NULL)
				m_serverClientChangeListener(pp->id, UString(""), false);
		}
		else
			debugLog("SERVER: NULL CLIENT_PEER!\n");

		// cleanup client info
		if (e.peer->data != NULL)
		{
			delete ((CLIENT_INFO_PACKET*)e.peer->data);
			e.peer->data = NULL;
		}

		// remove the peer from the list
		for (int i=0; i<m_vConnectedClients.size(); i++)
		{
			if (m_vConnectedClients[i].peer == e.peer)
			{
				m_vConnectedClients.erase(m_vConnectedClients.begin()+i);
				break;
			}
		}

		break;
	}
}

#endif

void NetworkHandler::sendClientInfo()
{
#ifdef MCENGINE_FEATURE_NETWORKING

	debugLog("CLIENT: Sending client info...\n");
	UString localname = _name_.getString();

	// build packet
	size_t size = 0;

	// base packet
	CLIENT_INFO_PACKET cp;
	cp.version = MC_PROTOCOL_VERSION;
	for (int i=0; i<clamp<int>(localname.length(), 0, 254); i++)
	{
		cp.username[i] = localname[i];
	}
	cp.username[254] = '\0';
	cp.size = clamp<int>(localname.length(), 0, 255);
	cp.extension = false;
	size += sizeof(CLIENT_INFO_PACKET);

	// extension packet
	size_t extensionSize = 0;
	std::shared_ptr<void> extensionData = NULL;
	if (m_clientSendInfoListener != NULL)
	{
		EXTENSION_PACKET extp = m_clientSendInfoListener();

		extensionData = extp.data;
		extensionSize = extp.size;

		cp.extension = true;
	}

	// create combined packet
	char combinedPacket[(size + extensionSize)];
	memcpy(&combinedPacket, &cp, size); // copy previous data
	if (extensionSize > 0 && extensionData != NULL)
	{
		memcpy(((char*)&combinedPacket) + size, extensionData.get(), extensionSize); // add extension data
		size += extensionSize;
	}

	ENetPacket *packet = enet_packet_create((const void*) combinedPacket, size, ENET_PACKET_FLAG_RELIABLE);

	// send it
	if (packet != NULL)
	{
		enet_peer_send(m_clientPeer, 0, packet);
		enet_host_flush(m_client);
	}

#endif
}

void NetworkHandler::clientDisconnect()
{
#ifdef MCENGINE_FEATURE_NETWORKING

	if (m_bClientConnectPending)
		debugLog(0xffff0000, "CLIENT: Couldn't connect, server \"%ls\" doesn't respond.\n", m_sServerAddress.length() == 0 ? L"localhost" : m_sServerAddress.wc_str());

	// notify listener
	if (isClient() && m_clientDisconnectedFromServerListener != NULL)
		m_clientDisconnectedFromServerListener();

	if (m_clientPeer != NULL)
		enet_peer_reset(m_clientPeer);

	m_clientPeer = NULL;

	m_bClientDisconnectPending = false;
	m_bClientConnectPending = false;

#endif
}

#ifdef MCENGINE_FEATURE_NETWORKING

void NetworkHandler::sendServerInfo(unsigned int assignedID, ENetHost *host, ENetPeer *destination)
{
	debugLog("SERVER: Sending server info (%i)...\n", assignedID);

	// build packet
	size_t size = 0;

	// base packet
	SERVER_INFO_PACKET serverInfoPacket;
	serverInfoPacket.id = assignedID;
	serverInfoPacket.extension = false;
	size += sizeof(SERVER_INFO_PACKET);

	// extension packet
	size_t extensionSize = 0;
	std::shared_ptr<void> extensionData = NULL;
	if (m_serverSendInfoListener != NULL)
	{
		EXTENSION_PACKET extp = m_serverSendInfoListener();

		extensionData = extp.data;
		extensionSize = extp.size;

		serverInfoPacket.extension = true;
	}

	// create combined packet
	char combinedPacket[(size + extensionSize)];
	memcpy(&combinedPacket, &serverInfoPacket, size); // copy previous data
	if (extensionSize > 0 && extensionData != NULL)
	{
		memcpy(((char*)&combinedPacket) + size, extensionData.get(), extensionSize); // add extension data
		size += extensionSize;
	}

	ENetPacket *packet = enet_packet_create((const void*)combinedPacket, size, ENET_PACKET_FLAG_RELIABLE);

	// send it
	if (packet != NULL)
	{
		enet_peer_send(destination, 0, packet);
		enet_host_flush(host);
	}
}

void NetworkHandler::singlecastChatMessage(CHAT_PACKET *cp, ENetHost *host, ENetPeer *destination)
{
	ENetPacket *packet = enet_packet_create((const void*)cp, sizeof(CHAT_PACKET), ENET_PACKET_FLAG_RELIABLE);

	// send it
	if (packet != NULL)
	{
		enet_peer_send(destination, 0, packet);
		enet_host_flush(host);
	}
}

void NetworkHandler::singlecastChatMessage(UString username, UString message, ENetHost *host, ENetPeer *destination)
{
	CHAT_PACKET cp;

	for (int i=0; i<clamp<int>(username.length(), 0, 254); i++)
	{
		cp.username[i] = username[i];
	}
	cp.username[254] = '\0';

	for (int i=0; i<clamp<int>(message.length(), 0, 254); i++)
	{
		cp.message[i] = message[i];
	}
	cp.message[254] = '\0';

	cp.usize = clamp<int>(username.length(), 0, 255);
	cp.msize = clamp<int>(message.length(), 0, 255);

	singlecastChatMessage(&cp, host, destination);
}

void NetworkHandler::broadcastChatMessage(CHAT_PACKET *cp, ENetHost *host, ENetPeer *origin)
{
	for (int c=0; c<m_vConnectedClients.size(); c++)
	{
		if (m_vConnectedClients[c].peer != origin)
			singlecastChatMessage(cp, host, m_vConnectedClients[c].peer);
	}
}

void NetworkHandler::broadcastChatMessage(UString username, UString message, ENetHost *host, ENetPeer *origin)
{
	for (int c=0; c<m_vConnectedClients.size(); c++)
	{
		if (m_vConnectedClients[c].peer != origin)
			singlecastChatMessage(username, message, host, m_vConnectedClients[c].peer);
	}
}

#endif

void NetworkHandler::broadcast(void *data, size_t size, bool reliable)
{
#ifdef MCENGINE_FEATURE_NETWORKING

	// block invalid packets
	if (data == NULL || size < 4)
	{
		debugLog(0xffff0000, "CLIENT: Invalid clientSend(), either data == NULL or size < 4!\n");
		return;
	}

	// if we are the server:
	if (isServer() && m_vConnectedClients.size() > 0)
	{
		// create wrapped packet (with id!)
		CLIENT_BROADCAST_WRAPPER wrap;
		const int wrapperSize = sizeof(CLIENT_BROADCAST_WRAPPER);
		char wrappedPacket[(sizeof(CLIENT_BROADCAST_WRAPPER) + size)];
		// the id is not set here, but for every client in the loop below
		memcpy(((char*)&wrappedPacket) + wrapperSize, data, size);
		size += wrapperSize;

		for (int c=0; c<m_vConnectedClients.size(); c++)
		{
			// update id
			wrap.id = m_vConnectedClients[c].id;
			memcpy(&wrappedPacket, &wrap, wrapperSize);

			// create it
			ENetPacket *packet = enet_packet_create((const void*)wrappedPacket, size, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);

			// send it
			if (packet != NULL)
				enet_peer_send(m_vConnectedClients[c].peer, 0, packet);
		}

		enet_host_flush(m_server);
		return;
	}

	// if we are the client
	if (!isClient()) return;

	// create wrapped packet (without id)
	PACKET_TYPE wrap = CLIENT_BROADCAST_PACKET_TYPE;
	const int wrapperSize = sizeof(PACKET_TYPE);
	char wrappedPacket[(sizeof(PACKET_TYPE) + size)];
	memcpy(&wrappedPacket, &wrap, wrapperSize);
	memcpy(((char*)&wrappedPacket) + wrapperSize, data, size);
	size += wrapperSize;

	ENetPacket *packet = enet_packet_create((const void*)wrappedPacket, size, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);

	// send it
	if (packet != NULL)
	{
		enet_peer_send(m_clientPeer, 0, packet);
		enet_host_flush(m_client);
	}

#endif
}

void NetworkHandler::servercast(void *data, size_t size, bool reliable)
{
#ifdef MCENGINE_FEATURE_NETWORKING

	// only valid if we are a client

	if (!isClient())
		return;

	// create wrapped packet
	PACKET_TYPE wrap = CLIENT_PACKET_TYPE;
	const int wrapperSize = sizeof(PACKET_TYPE);
	char wrappedPacket[(sizeof(PACKET_TYPE) + size)];
	memcpy(&wrappedPacket, &wrap, wrapperSize);
	memcpy(((char*)&wrappedPacket) + wrapperSize, data, size);
	size += wrapperSize;

	ENetPacket *packet = enet_packet_create((const void*)wrappedPacket, size, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);

	// send it
	if (packet != NULL)
	{
		enet_peer_send(m_clientPeer, 0, packet);
		enet_host_flush(m_client);
	}

#endif
}

void NetworkHandler::clientcast(void *data, size_t size, unsigned int id, bool reliable)
{
#ifdef MCENGINE_FEATURE_NETWORKING

	// only valid if we are the server
	if (!isServer()) return;

	CLIENT_PEER *pp = getClientPeerById(id);
	if (pp == NULL)
	{
		debugLog("SERVER: Tried to clientcast(void *, %i, %i, %i) to non-existing CLIENT_PEER!\n", size, id, (int)reliable);
		return;
	}

	// create wrapped packet (with id!)
	CLIENT_BROADCAST_WRAPPER wrap;
	const int wrapperSize = sizeof(CLIENT_BROADCAST_WRAPPER);
	char wrappedPacket[(sizeof(CLIENT_BROADCAST_WRAPPER) + size)];
	memcpy(&wrappedPacket, &wrap, wrapperSize);
	memcpy(((char*)&wrappedPacket) + wrapperSize, data, size);
	size += wrapperSize;

	ENetPacket *packet = enet_packet_create((const void*)wrappedPacket, size, reliable ? ENET_PACKET_FLAG_RELIABLE : 0);

	// send it
	if (packet != NULL)
	{
		enet_peer_send(pp->peer, 0, packet);
		enet_host_flush(m_server);
	}

#endif
}

void NetworkHandler::say(UString message)
{
#ifdef MCENGINE_FEATURE_NETWORKING

	if (message.length() < 1)
		return;

	// if we are the server owner, say will generate name_admin chat message broadcasts:
	if (!isClient() && isServer())
	{
		UString localname = _name_admin_.getString();

		// send it
		broadcastChatMessage(localname, message, m_server, NULL);

		// log
		chatLog(localname, message);
		return;
	}

	// if we are the client

	if (!isClient())
	{
		debugLog("CLIENT: Not connected to any server.\n");
		return;
	}

	if (m_bClientConnectPending) // don't allow chat while we are still connecting
	{
		debugLog("CLIENT: Please wait until you are connected to the server!\n");
		return;
	}

	UString localname = _name_.getString();

	// send it
	singlecastChatMessage(localname, message, m_client, m_clientPeer);

	// log
	if (!isServer()) // to avoid double logs on local hosts
		chatLog(localname, message);

#endif
}

void NetworkHandler::kick(UString username)
{
#ifdef MCENGINE_FEATURE_NETWORKING

	if (username.length() < 1)
		return;

	if (!isServer())
	{
		debugLog("SERVER: Not running.\n");
		return;
	}

	for (int c=0; c<m_vConnectedClients.size(); c++)
	{
		if (m_vConnectedClients[c].name == username)
		{
			// send kick message
			singlecastChatMessage("CONSOLE", "You have been KICKED from the server!", m_server, m_vConnectedClients[c].peer);

			// initiate graceful disconnect
			m_vConnectedClients[c].kickTime = engine->getTime();

			// notify local connection listener
			// TODO: this gets called in the disconnect event, or in the timeout (is it really necessary to also immediately call it here? results in a redundant second message to every client)
			/*
			if (m_serverClientChangeListener != NULL)
				m_serverClientChangeListener(m_vConnectedClients[c].id, "", false);
			*/

			return;
		}
	}

	UString msg = "SERVER: Couldn't find user \"";
	msg.append(username);
	msg.append("\"\n");
	debugLog("%s", msg.toUtf8());

#endif
}

void NetworkHandler::chatLog(UString username, UString message)
{
	UString chatlog = username;
	chatlog.append(": ");
	chatlog.append(message);
	chatlog.append("\n");
	debugLog("%s", chatlog.toUtf8());
}

#ifdef MCENGINE_FEATURE_NETWORKING

NetworkHandler::CLIENT_PEER *NetworkHandler::getClientPeerByPeer(ENetPeer *peer)
{
	for (int c=0; c<m_vConnectedClients.size(); c++)
	{
		if (m_vConnectedClients[c].peer == peer)
			return (&m_vConnectedClients[c]);
	}
	return NULL;
}

NetworkHandler::CLIENT_PEER *NetworkHandler::getClientPeerById(unsigned int id)
{
	for (int c=0; c<m_vConnectedClients.size(); c++)
	{
		if (m_vConnectedClients[c].id == id)
			return (&m_vConnectedClients[c]);
	}
	return NULL;
}

#endif

int NetworkHandler::getPing()
{
#ifdef MCENGINE_FEATURE_NETWORKING

	return m_clientPeer != NULL ? (m_clientPeer->roundTripTime) : 0;

#else
	return -1;
#endif
}

bool NetworkHandler::isClient()
{
#ifdef MCENGINE_FEATURE_NETWORKING

	return m_client != NULL && m_clientPeer != NULL;

#else
	return false;
#endif
}

bool NetworkHandler::isServer()
{
#ifdef MCENGINE_FEATURE_NETWORKING

	return m_server != NULL;

#else
	return false;
#endif
}
