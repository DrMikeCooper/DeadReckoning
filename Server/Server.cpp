#include <iostream>
#include <string>
#include <BitStream.h>
#include <thread>
#include <vector>

#include "Server.h"
#include "GameMessages.h"


void Server::StartUp()
{
	//Initialize the Raknet peer interface first
	pPeerInterface = RakNet::RakPeerInterface::GetInstance();
	//Create a socket descriptor to describe this connection
	RakNet::SocketDescriptor sd(PORT, 0);
	//Now call startup - max of 32 connections, on the assigned port
	pPeerInterface->Startup(32, &sd, 1);
	pPeerInterface->SetMaximumIncomingConnections(32);
	pPeerInterface->ApplyNetworkSimulator(0.01f, 100, 100);
	//Startup a thread to ping clients every second
	std::thread pingThread(SendClientPing, pPeerInterface);
	std::thread updateThread(UpdateThread, this, pPeerInterface);

	HandleNetworkMessages();
}

void Server::SendNewClientID(RakNet::RakPeerInterface* pPeerInterface, RakNet::SystemAddress& address)
{
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_SERVER_SET_CLIENT_ID);
	bs.Write(nextClientID);
	nextClientID++;
	pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED,
		0, address, false);

	// send the existing gameobjects to the new client
	for (auto it = m_gameObjects.begin(); it != m_gameObjects.end(); it++)
	{
		GameObject obj = it->second;
		obj.Write(pPeerInterface, address, false);
	}

	// send us to all other clients
	int id = nextClientID - 1;
	GameObject obj;
	obj.data.position = glm::vec3(0);
	obj.data.colour = GameObject::GetColour(id);
	obj.id = id;
	obj.Write(pPeerInterface, address, true);
}

void Server::HandleNetworkMessages()
{
	RakNet::Packet* packet = nullptr;
	while (true)
	{
		for (packet = pPeerInterface->Receive(); packet;
		pPeerInterface->DeallocatePacket(packet),
			packet = pPeerInterface->Receive())
		{
			switch (packet->data[0])
			{
			case ID_NEW_INCOMING_CONNECTION:
				std::cout << "A connection is incoming.\n";
				SendNewClientID(pPeerInterface, packet->systemAddress);
				break;
			case ID_DISCONNECTION_NOTIFICATION:
				std::cout << "A client has disconnected.\n";
				break;
			case ID_CONNECTION_LOST:
				std::cout << "A client lost the connection.\n";
				break;
			case ID_CLIENT_CLIENT_DATA:
			{
				// bounce the packet back to all clients
				RakNet::BitStream bs(packet->data, packet->length, false);
				pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0,
					packet->systemAddress, true);

				// read the packet and store in our list of game objects on the server
				GameObject clientData;
				clientData.Read(packet);
				m_gameObjects[clientData.id] = clientData;

				std::cout << "Client " << clientData.id <<
					" at: " << clientData.data.position.x <<
					" " << clientData.data.position.z << std::endl;

				break;
			}
			case ID_CLIENT_SPAWN_BULLET:
				OnSpawnBullet(packet);
				break;
			default:
				std::cout << "Received a message with a unknown id: " <<
					packet->data[0];
				break;
			}
		}
	}
}

void Server::OnSpawnBullet(RakNet::Packet* packet)
{
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	glm::vec3 pos, vel;
	bsIn.Read((char*)&pos, sizeof(glm::vec3));
	bsIn.Read((char*)&vel, sizeof(glm::vec3));
	SpawnObject(pos, vel);
}

void Server::SendClientPing(RakNet::RakPeerInterface* pPeerInterface)
{
	while (true)
	{
		RakNet::BitStream bs;
		bs.Write((RakNet::MessageID)GameMessages::ID_SERVER_TEXT_MESSAGE);
		bs.Write("Ping!");
		pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0,
			RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
		std::this_thread::sleep_for(std::chrono::seconds(60));
	}
}

float Server::GetElapsedTime()
{
	LARGE_INTEGER CurrentTime, ElapsedMicroseconds;
	LARGE_INTEGER Frequency;

	QueryPerformanceFrequency(&Frequency);
	QueryPerformanceCounter(&CurrentTime);

	ElapsedMicroseconds.QuadPart = CurrentTime.QuadPart - LastTime.QuadPart;
	LastTime = CurrentTime;

	//
	// We now have the elapsed number of ticks, along with the
	// number of ticks-per-second. We use these values
	// to convert to the number of elapsed microseconds.
	// To guard against loss-of-precision, we convert
	// to microseconds *before* dividing by ticks-per-second.
	//

	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;
	return ElapsedMicroseconds.QuadPart * 0.000001f;
}

void Server::UpdateThread(Server* server, RakNet::RakPeerInterface* pPeerInterface)
{
	const int deltaTime = 17;
	float timeToNextUpdate = 0; // copuntdown in seconds till we broadcast all game objects
	float updateFrequency = 0.5f; // second between updates

	while (true)
	{
		std::vector<int> deathRow;

		float dt = server->GetElapsedTime();
		timeToNextUpdate -= dt;

		for (int i = 0; i < server->m_gameObjects.size(); i++)
		{
			server->m_gameObjects[i].Update(dt);

			//check for despawn
			glm::vec3 pos = server->m_gameObjects[i].data.position;
			if (pos.x < -10 || pos.x > 10 || pos.z < -10 || pos.z > 10)
				deathRow.push_back(server->m_gameObjects[i].id);

			if (timeToNextUpdate < 0)
			{
				// broadcast to every client if we're server controlled
				if (server->m_gameObjects[i].id >= 1000)
					server->m_gameObjects[i].Write(pPeerInterface, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
			}
		}

		if (timeToNextUpdate < 0)
			timeToNextUpdate = updateFrequency;

		for (int i = 0; i < deathRow.size(); i++)
			server->Despawn(deathRow[i]);

		std::this_thread::sleep_for(std::chrono::milliseconds(deltaTime));
	}
}

void Server::SpawnObject(glm::vec3 position, glm::vec3 velocity)
{
	m_gameObjects[nextServerID] = GameObject();
	m_gameObjects[nextServerID].id = nextServerID;
	m_gameObjects[nextServerID].data.position = position;
	m_gameObjects[nextServerID].data.velocity = velocity;
	nextServerID++;
}

void Server::Despawn(int id)
{
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_SERVER_DESPAWN);
	bs.Write(id);
	pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);

	// erase from our local list
	m_gameObjects.erase(id);
}
