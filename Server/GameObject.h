#pragma once
#include <RakPeerInterface.h>
#include "../dependencies/glm/glm/glm.hpp"

// this is the chunk of data that gets sent through the network
struct GameObjectData
{
	glm::vec3 position;
	glm::vec3 velocity;
	glm::vec4 colour;
};

class GameObject
{
public:
	GameObject()
	{
		data.position = data.velocity = glm::vec3(0);
	}

	// the local position on the client
	glm::vec3 localPosition;

	GameObjectData data;
	int id;

	static glm::vec4 GetColour(int id);

	void Read(RakNet::Packet* packet);
	void Write(RakNet::RakPeerInterface* pPeerInterface, const RakNet::SystemAddress& address, bool broadcast);

	void Update(float dt);

#ifndef NETWORK_SERVER
	virtual void Draw();
#endif
};
