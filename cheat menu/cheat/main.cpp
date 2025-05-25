#include <Windows.h>
#include <thread>

#include "gui.h"


INT APIENTRY WinMain(
	HINSTANCE instance,
	HINSTANCE previousInstance,
	PSTR arguments,
	int commandShow
)
{
	// create gui
	gui::CreateHWin("External Cheat", "Idle Slayer Cheat");
	gui::CreateDevice();
	gui::CreateImGUI();

	while (gui::EXIT)
	{
		gui::BeginRender();
		gui::Render();
		gui::EndRender();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	// destroy gui
	gui::DestroyImGUI();
	gui::DestroyDevice();
	gui::DestroyHWin();

	return EXIT_SUCCESS;
}