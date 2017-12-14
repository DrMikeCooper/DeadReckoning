#include <iostream>
#include <string>
#include <MessageIdentifiers.h>
#include <BitStream.h>
#include "GameObject.h"
#include "GameMessages.h"
#ifndef NETWORK_SERVER
#include "Gizmos.h"
using aie::Gizmos;
#endif

glm::vec4 colours[] = {
	glm::vec4(0.5,0.5,0.5,1), //grey
	glm::vec4(1,0,0,1), // red
	glm::vec4(0,1,0,1), // green
	glm::vec4(0,0,1,1), // blue
	glm::vec4(1,1,0,1), // yellow
	glm::vec4(1,0,1,1), // magenta
	glm::vec4(0,1,1,1), //cyan
	glm::vec4(0,0,0,1), //black
};

glm::vec4 GameObject::GetColour(int id)
{
	return colours[id & 7];
}

void GameObject::Read(RakNet::Packet* packet)
{
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(RakNet::MessageID));
	bsIn.Read(id);
	bsIn.Read((char*)&data, sizeof(GameObjectData));
}

void GameObject::Write(RakNet::RakPeerInterface* pPeerInterface, const RakNet::SystemAddress& address, bool broadcast)
{
	RakNet::BitStream bs;
	bs.Write((RakNet::MessageID)GameMessages::ID_CLIENT_CLIENT_DATA);
	bs.Write(id);
	bs.Write((char*)&data, sizeof(GameObjectData));
	pPeerInterface->Send(&bs, HIGH_PRIORITY, RELIABLE_ORDERED, 0,
		address, broadcast);
}

// called on each GameObject every frame on the client
void GameObject::Update(float dt)
{
	data.position += data.velocity * dt;

	// interpolate the local position towards the server position - TODO better
	localPosition = localPosition * 0.5f + data.position * 0.5f;
}

#ifndef NETWORK_SERVER
void GameObject::Draw()
{
	Gizmos::addSphere(data.position, 1.0f, 32, 32, glm::vec4(1,1,1,0));
	Gizmos::addSphere(localPosition, 0.9f, 32, 32, data.colour);

}
#endif
