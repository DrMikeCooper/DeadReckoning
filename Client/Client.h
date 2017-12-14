#pragma once


#include <glm/glm.hpp>
#include <RakPeerInterface.h>
#include <unordered_map>
#include "Application.h"
#include "../Server/GameObject.h"

class Client : public aie::Application {
public:

	Client();
	virtual ~Client();

	virtual bool startup();
	virtual void shutdown();

	virtual void update(float deltaTime);
	virtual void draw();

	// Initialize the connection
	void handleNetworkConnection();
	void initialiseClientConnection();
	// Handle incoming packets
	void handleNetworkMessages();

	void onSetClientIDPacket(RakNet::Packet* packet);
	void sendClientGameObject();
	void onReceivedClientDataPacket(RakNet::Packet * packet);
	void onDespawn(RakNet::Packet * packet);

	void sendSpawnBulletPacket();

protected:

	RakNet::RakPeerInterface* m_pPeerInterface;
	const char* IP = "127.0.0.1";
	const unsigned short PORT = 5456;

	GameObject m_myGameObject;

	std::unordered_map<int, GameObject> m_otherClientGameObjects;

	glm::mat4	m_viewMatrix;
	glm::mat4	m_projectionMatrix;

	bool wasKeyDown;
};