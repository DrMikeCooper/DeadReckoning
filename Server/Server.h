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
	static void UpdateThread(Server* server, RakNet::RakPeerInterface* pPeerInterface);

	void OnSpawnBullet(RakNet::Packet* packet);
	void SpawnObject(glm::vec3 position, glm::vec3 velocity);
	void Despawn(int id);

	const unsigned short PORT = 5456;

	RakNet::RakPeerInterface* pPeerInterface = nullptr;
	int nextClientID = 1;
	int nextServerID = 1000;

	std::map<int, GameObject> m_gameObjects;
};
