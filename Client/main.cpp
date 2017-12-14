#include "Client.h"

int main() {
	
	auto app = new Client();
	app->run("AIE", 800, 600, false);
	delete app;

	return 0;
}