#include <iostream>
#include <string>
#include <BitStream.h>
#include <thread>

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

	//Startup a thread to ping clients every second
	std::thread pingThread(SendClientPing, pPeerInterface);

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
			default:
				std::cout << "Received a message with a unknown id: " <<
					packet->data[0];
				break;
			}
		}
	}
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