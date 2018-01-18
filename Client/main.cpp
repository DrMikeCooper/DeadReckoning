#include "ExampleClient.h"

int main() {
	
	auto app = new ExampleClient();
	app->run("AIE", 800, 600, false);
	delete app;

	return 0;
}