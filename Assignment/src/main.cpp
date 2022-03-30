#include "TankRender.h"

#define DEFAULT_SCREENWIDTH 1820
#define DEFAULT_SCREENHEIGHT 1080
// main that controls the creation/destruction of an application
int WinMain(int argc, char* argv[])
{
	// explicitly control the creation of our application
	TankRender* app = new TankRender();
	app->run("Tank Rendering", DEFAULT_SCREENWIDTH, DEFAULT_SCREENHEIGHT, false);

	// explicitly control the destruction of our application
	delete app;

	return 0;
}