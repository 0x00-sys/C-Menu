#include "gui.h"
#include <thread>


int __stdcall wWinMain(
	HINSTANCE instance,
	HINSTANCE prev_instance,
	PWSTR arguments,
	int show_command)
{
	//create gui
	gui::create_window(
		"cheat menu",
		"cheat menu class");
	gui::create_device();
	gui::create_gui();

	while (gui::exit)
	{
		gui::begin_render();
		gui::render();
		gui::end_render();

		std::this_thread::sleep_for(
			std::chrono::milliseconds(10));
	}

	gui::destroy_gui();
	gui::destroy_device();
	gui::destroy_window();

	return EXIT_SUCCESS;
}