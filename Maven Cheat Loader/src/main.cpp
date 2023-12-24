#include "gui.h"
#include "globals.h"
#include <thread>
#include <iostream>
#include "antiAC.h"


//popup
void StartMessageBox() {
	MessageBox(NULL, "Loading Loader Bypass, Please Wait....", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK); //doesnt mean anything (YET) just to make it look cool
}

//code for generating the random string exe name
std::string generateRandomString(int length) {
	std::string randomString;
	static const char alphanum[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";

	srand(static_cast<unsigned int>(time(nullptr)));

	for (int i = 0; i < length; ++i) {
		randomString += alphanum[rand() % (sizeof(alphanum) - 1)];
	}

	return randomString;
}
//to here

int __stdcall wWinMain(
	HINSTANCE instance,
	HINSTANCE previousInstance,
	PWSTR arguments,
	int commandShow)

{

	StartMessageBox();// Display the message box when the application is run
	std::this_thread::sleep_for(std::chrono::seconds(3));// makes program sleep for 3 seconds


	// create gui
	std::string randomString = generateRandomString(16); //generates the random string
	gui::CreateHWindow(randomString.c_str()); //assign the random string name to the application
	gui::CreateDevice();
	gui::CreateImGui();

	while (gui::isRunning)
	{
		gui::BeginRender();
		gui::Render();
		gui::EndRender();

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	// destroy gui
	gui::DestroyImGui();
	gui::DestroyDevice();
	gui::DestroyHWindow();

	return EXIT_SUCCESS;
}