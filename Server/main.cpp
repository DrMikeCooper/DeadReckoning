#include <iostream>
#include <string>
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>
#include <thread>
#include <map>

#include "GameObject.h"
#include "GameMessages.h"
#include "Server.h"

Server server;

int main()
{
	//Startup the server, and start it listening to clients
	std::cout << "Starting up the server..." << std::endl;
	
	server.StartUp();
	
	return 0;
}