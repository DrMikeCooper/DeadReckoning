#pragma once
#include "Client.h"
#include "SpriteGameObject.h"

class ExampleClient :
	public Client
{
public:
	ExampleClient();
	~ExampleClient();

	virtual GameObject* CreateGameObject()
	{
		return new SpriteGameObject();
	}
};

