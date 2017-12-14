#pragma once
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <map>

#include "GameObject.h"

class Server
{
public:
	void StartUp();
	void SendNewClientID(RakNet::RakPeerInterface* pPeerInterface, RakNet::SystemAddress& address);
	void HandleNetworkMessages();
	static void SendClientPing(RakNet::RakPeerInterface* pPeerInterface);

	const unsigned short PORT = 5456;

	RakNet::RakPeerInterface* pPeerInterface = nullptr;
	int nextClientID = 1;
	std::map<int, GameObject> m_gameObjects;
};
