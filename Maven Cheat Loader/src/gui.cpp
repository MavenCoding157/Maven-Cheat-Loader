#include "gui.h"
#include <cstdlib>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/ImGuiFileDialog.h" //imgui file dialog

#include "globals.h"
#include <corecrt_math.h>
#include "../font/icons.h"
#include "../font/font.h"

#include <windows.h>
#include <shellapi.h>
#include "../imgui/imgui_toggle.h"
#include "../imgui/imguidesign.h"
#include <iostream>
#include <fstream> 
#include <string>
#include <Lmcons.h>
#include <chrono>
#include <unordered_map>
#include <string>
#include <utility>

#include <Shlwapi.h>// for injecting and filepath stuff
#pragma comment(lib, "Shlwapi.lib")// for injecting and filepath stuff


//all credits 

// imgui: https://github.com/ocornut/imgui
// imgui file dialog: https://github.com/aiekick/ImGuiFileDialog

//to here


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter
);

long __stdcall WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter)
{
	if (ImGui_ImplWin32_WndProcHandler(window, message, wideParameter, longParameter))
		return true;

	switch (message)
	{
	case WM_SIZE: {
		if (gui::device && wideParameter != SIZE_MINIMIZED)
		{
			gui::presentParameters.BackBufferWidth = LOWORD(longParameter);
			gui::presentParameters.BackBufferHeight = HIWORD(longParameter);
			gui::ResetDevice();
		}
	}return 0;

	case WM_SYSCOMMAND: {
		if ((wideParameter & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
	}break;

	case WM_DESTROY: {
		PostQuitMessage(0);
	}return 0;

	case WM_LBUTTONDOWN: {
		gui::position = MAKEPOINTS(longParameter); // set click points
	}return 0;

	case WM_MOUSEMOVE: {
		if (wideParameter == MK_LBUTTON)
		{
			const auto points = MAKEPOINTS(longParameter);
			auto rect = ::RECT{ };

			GetWindowRect(gui::window, &rect);

			rect.left += points.x - gui::position.x;
			rect.top += points.y - gui::position.y;

			if (gui::position.x >= 0 &&
				gui::position.x <= gui::WIDTH &&
				gui::position.y >= 0 && gui::position.y <= 19)
				SetWindowPos(
					gui::window,
					HWND_TOPMOST,
					rect.left,
					rect.top,
					0, 0,
					SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER
				);
		}

	}return 0;

	}

	return DefWindowProc(window, message, wideParameter, longParameter);
}

void gui::CreateHWindow(const char* windowName) noexcept
{
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_CLASSDC;
	windowClass.lpfnWndProc = reinterpret_cast<WNDPROC>(WindowProcess);
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandleA(0);
	windowClass.hIcon = 0;
	windowClass.hCursor = 0;
	windowClass.hbrBackground = 0;
	windowClass.lpszMenuName = 0;
	windowClass.lpszClassName = "class001";
	windowClass.hIconSm = 0;

	RegisterClassEx(&windowClass);

	window = CreateWindowEx(
		0,
		"class001",
		windowName,
		WS_POPUP,
		100,
		100,
		WIDTH,
		HEIGHT,
		0,
		0,
		windowClass.hInstance,
		0
	);

	ShowWindow(window, SW_SHOWDEFAULT);
	UpdateWindow(window);
}

void gui::DestroyHWindow() noexcept
{
	DestroyWindow(window);
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

bool gui::CreateDevice() noexcept
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d)
		return false;

	ZeroMemory(&presentParameters, sizeof(presentParameters));

	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameters.EnableAutoDepthStencil = TRUE;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&presentParameters,
		&device) < 0)
		return false;

	return true;
}

void gui::ResetDevice() noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = device->Reset(&presentParameters);

	if (result == D3DERR_INVALIDCALL)
		IM_ASSERT(0);

	ImGui_ImplDX9_CreateDeviceObjects();
}

void gui::DestroyDevice() noexcept
{
	if (device)
	{
		device->Release();
		device = nullptr;
	}

	if (d3d)
	{
		d3d->Release();
		d3d = nullptr;
	}
}

void gui::CreateImGui() noexcept
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ::ImGui::GetIO(); (void)io;

	ImGuiStyle& style = ImGui::GetStyle();

	style.Colors[ImGuiCol_TitleBg] = ImColor(30, 34, 43);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImColor(30, 34, 43);
	style.Colors[ImGuiCol_TitleBgActive] = ImColor(30, 34, 43);
	style.Colors[ImGuiCol_WindowBg] = ImColor(32, 36, 47);
	style.Colors[ImGuiCol_Button] = ImColor(48, 59, 80);
	style.Colors[ImGuiCol_ButtonActive] = ImColor(48, 59, 80);
	style.Colors[ImGuiCol_ButtonHovered] = ImColor(48, 59, 80);
	style.Colors[ImGuiCol_CheckMark] = ImColor(255, 255, 255, 255);
	style.Colors[ImGuiCol_FrameBg] = ImColor(45, 55, 78);
	style.Colors[ImGuiCol_FrameBgActive] = ImColor(45, 55, 78);
	style.Colors[ImGuiCol_FrameBgHovered] = ImColor(45, 55, 78);
	style.Colors[ImGuiCol_Header] = ImColor(24, 24, 24, 255);
	style.Colors[ImGuiCol_HeaderActive] = ImColor(54, 53, 55);
	style.Colors[ImGuiCol_HeaderHovered] = ImColor(24, 24, 24, 100);
	style.Colors[ImGuiCol_ResizeGrip] = ImColor(51, 49, 50, 255);
	style.Colors[ImGuiCol_ResizeGripActive] = ImColor(54, 53, 55);
	style.Colors[ImGuiCol_ResizeGripHovered] = ImColor(51, 49, 50, 255);
	style.Colors[ImGuiCol_SliderGrab] = ImColor(249, 79, 49, 255);
	style.Colors[ImGuiCol_SliderGrabActive] = ImColor(249, 79, 49, 255);
	style.Colors[ImGuiCol_TabActive] = ImColor(32, 36, 47);
	style.Colors[ImGuiCol_Tab] = ImColor(32, 36, 47);
	style.Colors[ImGuiCol_Border] = ImColor(54, 54, 54);
	style.Colors[ImGuiCol_Separator] = ImColor(54, 54, 54);
	style.Colors[ImGuiCol_SeparatorActive] = ImColor(54, 54, 54);
	style.Colors[ImGuiCol_SeparatorHovered] = ImColor(54, 54, 54);

	static const ImWchar icons_ranges[] = { 0xf000, 0xf3ff, 0 };
	ImFontConfig icons_config;

	ImFontConfig CustomFont;
	CustomFont.FontDataOwnedByAtlas = false;

	icons_config.MergeMode = true;
	icons_config.PixelSnapH = true;
	icons_config.OversampleH = 3;
	icons_config.OversampleV = 3;

	io.Fonts->AddFontFromMemoryTTF(const_cast<std::uint8_t*>(Custom), sizeof(Custom), 20, &CustomFont);
	io.Fonts->AddFontFromMemoryCompressedTTF(font_awesome_data, font_awesome_size, 32.5f, &icons_config, icons_ranges);

	io.Fonts->AddFontDefault();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);
}

void gui::DestroyImGui() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void gui::BeginRender() noexcept
{
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);

		if (message.message == WM_QUIT)
		{
			isRunning = !isRunning;
			return;
		}
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void gui::EndRender() noexcept
{
	ImGui::EndFrame();

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

	if (device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}

	const auto result = device->Present(0, 0, 0, 0);

	// Handle loss of D3D9 device
	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
}



//bools and stuff
bool show_exit_button = true;
static auto current_tab = 0;
bool CloseOnInject = false;
bool StealthInject = false;
bool my_checkbox_state = false;
constexpr auto butn_tall = 48;

//for login

//to here


//textbox guessing game thing (idk i was bored)

void GuessPassword() {

	static char userInput[50] = "";

	// Textbox for the users input
	ImGui::PushItemWidth(451);
	ImGui::InputText("Enter Password Here##TextInput", userInput, sizeof(userInput), ImGuiInputTextFlags_Password);

	ImGui::Spacing();

	if (ImGui::Button("Guess The Password", ImVec2(453, 0))) {

		if (strcmp(userInput, "soasface123") == 0) {
			MessageBox(NULL, "Success!!! How TF did you know the right password?", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);

			ShellExecute(0, 0, "https://pastebin.com/VXNR9Qpr", 0, 0, SW_SHOW); //Fake credit card list (ITS NOT REAL YOU TWOT)

		}
		else {
			MessageBox(NULL, "Wrong!!!, hahahahaha you will never guess it.", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
		}
	}
}
//to here

//for custom back color
ImVec4 bgColor = ImVec4(0.13f, 0.14f, 0.18f, 1.0f);
//to here

//imgui file dialog | obselete for now
void OpenFileDialog(std::string& selectedFileName) {
	if (ImGui::Button("Find DLL", ImVec2(451, 50)))
		ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".dll", ".");

	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {

		if (ImGuiFileDialog::Instance()->IsOk()) {

			selectedFileName = ImGuiFileDialog::Instance()->GetFilePathName();
		}

		//close
		ImGuiFileDialog::Instance()->Close();
	}
}
//to here

//injecting stuff (64bit)
DWORD GetProcId(const char* pn, unsigned short int fi = 0b1101)
{
	DWORD procId = 0;
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap != INVALID_HANDLE_VALUE)
	{
		PROCESSENTRY32 pE;
		pE.dwSize = sizeof(pE);

		if (Process32First(hSnap, &pE))
		{
			if (!pE.th32ProcessID)
				Process32Next(hSnap, &pE);
			do
			{
				if (fi == 0b10100111001)
					std::cout << pE.szExeFile << u8"\x9\x9\x9" << pE.th32ProcessID << std::endl;
				if (!_stricmp(pE.szExeFile, pn))
				{
					procId = pE.th32ProcessID;
					break;
				}
			} while (Process32Next(hSnap, &pE));
		}
	}
	CloseHandle(hSnap);
	return procId;
}

BOOL InjectDLL(DWORD procID, const char* dllPath)
{
	BOOL WPM = 0;

	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, 0, procID);
	if (hProc == INVALID_HANDLE_VALUE)
	{
		return -1;
	}
	void* loc = VirtualAllocEx(hProc, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	WPM = WriteProcessMemory(hProc, loc, dllPath, strlen(dllPath) + 1, 0);
	if (!WPM)
	{
		CloseHandle(hProc);
		return -1;
	}
	HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)LoadLibraryA, loc, 0, 0);
	if (!hThread)
	{
		VirtualFree(loc, strlen(dllPath) + 1, MEM_RELEASE);
		CloseHandle(hProc);
		return -1;
	}
	CloseHandle(hProc);
	VirtualFree(loc, strlen(dllPath) + 1, MEM_RELEASE);
	CloseHandle(hThread);
	return 0;
}

void ImGuiPrint(const char* message) {
	ImGui::Text(message);
}

void InputText(const char* label, std::string& inputBuffer) {
	const int bufferSize = 256;
	char buffer[bufferSize];
	strcpy_s(buffer, bufferSize, inputBuffer.c_str());
	ImGui::InputText(label, buffer, bufferSize);
	inputBuffer = buffer;
}
//to here

//more injection stuff
std::string processName; //process name for injector (x64 bit)

std::string selectedFileName;
const char* selectedProcessName = nullptr;
//to here

//injector 32 bit
bool InjectDLL32bit(DWORD processId, const char* dllPath) {
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
	if (hProcess == NULL) {
		MessageBox(NULL, "Error Getting PID!", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
		return false;
	}

	LPVOID dllPathAddr = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
	if (dllPathAddr == NULL) {
		MessageBox(NULL, "Error Allocating Memory!", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
		CloseHandle(hProcess);
		return false;
	}

	WriteProcessMemory(hProcess, dllPathAddr, dllPath, strlen(dllPath) + 1, NULL);

	LPVOID loadLibraryAddr = GetProcAddress(GetModuleHandle("kernel32.dll"), "LoadLibraryA");
	if (loadLibraryAddr == NULL) {
		VirtualFreeEx(hProcess, dllPathAddr, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}

	HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddr, dllPathAddr, 0, NULL);
	if (hThread == NULL) {
		MessageBox(NULL, "Error Creating Remote Thread!", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
		VirtualFreeEx(hProcess, dllPathAddr, 0, MEM_RELEASE);
		CloseHandle(hProcess);
		return false;
	}

	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(hThread);
	VirtualFreeEx(hProcess, dllPathAddr, 0, MEM_RELEASE);
	CloseHandle(hProcess);

	//MessageBox(NULL, "DLL Injected Successfully!", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);

	return true;
}
//to here


//proccess list (for imgui) | Obselete For now...
void ProcessDropdownAndTextBox() {
	static char selectedProcess[256] = "";
	static std::string processList;

	// Get the list of running processes
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hSnap != INVALID_HANDLE_VALUE) {
		PROCESSENTRY32 pE;
		pE.dwSize = sizeof(pE);

		if (Process32First(hSnap, &pE)) {
			do {
				processList += pE.szExeFile;
				processList += '\0';
			} while (Process32Next(hSnap, &pE));
		}

		CloseHandle(hSnap);
	}

	ImGui::PushItemWidth(451);
	if (ImGui::BeginCombo("##ProcessDropdown", selectedProcess)) {
		// go through and add to the list
		const char* processes = processList.c_str();
		while (*processes) {
			bool isSelected = (strcmp(selectedProcess, processes) == 0);
			if (ImGui::Selectable(processes, isSelected)) {
				strcpy_s(selectedProcess, sizeof(selectedProcess), processes);
			}

			processes += strlen(processes) + 1;
		}

		ImGui::EndCombo();
	}

	ImGui::Text("", selectedProcess, ImGuiInputTextFlags_ReadOnly);
}
//to here


//all games for injecting (for now) | Obselete rn
static const char* games[] = { "GTA V", "gmod", "CS2"};
static int selectedGame = 0;
//to here


// for getting exe directory
std::string GetExeDirectory();

std::string GetExeDirectory() {
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");
	return std::string(buffer).substr(0, pos);
}
//to here


//for hwid spoofer
static bool checkBox = true;
static const char* items[]{ "Spoof For The Game Selected", "Spoof For All", "Rust", "Apex Legends", "Fortnite", "Fivem Server Unban", "COD Warzone", "Rainbow Six Siege" };
static int selectedItem = 0;
//to here

//update stuff
std::string CurrentVersion = "1.3";
std::string url = "https://raw.githubusercontent.com/MavenCoding157/Maven-Cheat-Loader/main/version.txt";
std::string version;
//to here


//rgb stuff (dont need but handy if you want to use it)
float hue = 0.0f;
float colorSpeed = 0.05f;

void RenderFlashingRGBText() {
	ImVec4 textColor;
	textColor.x = sinf(hue) * 0.5f + 0.5f;
	textColor.y = sinf(hue + 2.0f * 3.14159265359f / 3.0f) * 0.5f + 0.5f;
	textColor.z = sinf(hue + 4.0f * 3.14159265359f / 3.0f) * 0.5f + 0.5f;
	textColor.w = 1.0f;

	ImGui::TextColored(textColor, "Undetected");

	hue += colorSpeed;
	if (hue > 1.0f) {
		hue -= 1.0f;
	}
}

void RGBText() {
	RenderFlashingRGBText();
}


void gui::Render() noexcept
{
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ WIDTH, HEIGHT });
	ImGui::Begin(
		"Maven Cheat Loader - By MavenCoding157",
		&isRunning,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove
	);


	if (ImGui::BeginChild(
		1,
		{ ImGui::GetContentRegionAvail().x * 0.25f, ImGui::GetContentRegionAvail().y },
		true)) {
		constexpr auto button_height = 81;
		if (ImGui::Button("\n " ICON_FA_HOME " \n", { ImGui::GetContentRegionAvail().x, button_height })) { current_tab = 9; } //home/mainhub
		ImGui::Spacing();

		if (ImGui::Button("\n " ICON_FA_EYE " \n", { ImGui::GetContentRegionAvail().x, button_height })) { current_tab = 0; } //injector
		ImGui::Spacing();

		if (ImGui::Button("\n " ICON_FA_BOLT " \n", { ImGui::GetContentRegionAvail().x, button_height })) { current_tab = 10; } //spoofer
		ImGui::Spacing();

		if (ImGui::Button("\n " ICON_FA_COGS " \n", { ImGui::GetContentRegionAvail().x, button_height })) { current_tab = 5; } //settings
		ImGui::Spacing();

		if (ImGui::Button("\n " ICON_FA_QUESTION " \n", { ImGui::GetContentRegionAvail().x, button_height })) { current_tab = 6; } //about
		ImGui::Spacing();

		ImGui::EndChild();
	}

	ImGui::SameLine();

	if (ImGui::BeginChild(
		2,
		ImGui::GetContentRegionAvail(),
		true)) {

		switch (current_tab) {
		case 9:
			ImGui::BeginChild("Home", ImVec2(453, 0), true);
			{
				ImGuiPP::CenterText("Maven Cheat Loader", 1, TRUE);

				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();


				ImGui::Text("Version: 1.3");

				ImGui::Text("Game Injector Status: ");
				ImGui::SameLine();
				ImGui::TextColored(ImColor(0, 128, 0), "Undetected");

				ImGui::Text("HWID Spoofer Status: ");
				ImGui::SameLine();
				ImGui::TextColored(ImColor(252, 244, 5), "Coming Soon....");

				ImGui::Text("Clean Trace Files Status: ");
				ImGui::SameLine();
				ImGui::TextColored(ImColor(252, 244, 5), "Coming Soon....");

				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();

				ImGuiPP::CenterText("More Options", 1, TRUE);

				ImGui::Spacing();

				//(The only reason I added this is cause it looks cool btw)
				if (ImGui::Button("Check Loader Status", { ImGui::GetContentRegionAvail().x, butn_tall }))
				{
					MessageBox(NULL, "Loader Is Online!", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
				}

				ImGui::Spacing();

				//and same with this
				if (ImGui::Button("Update Log", { ImGui::GetContentRegionAvail().x, butn_tall }))
				{
					MessageBox(NULL, "21/12/2023\n\n - Actually added a loader, The current Games are: Gmod, CS2 and GTA V. However only GTA V works atm\n - Fixing/removing unnecessary and useless code\n - UI updates, Currently working on GTA 5 Trainer atm so not many updates will be coming to the loader for now\n\n 19/12/2023\n\n - Making a ImGui Based GTA V Trainer (coming Soon)\n - Added an 'Update Button'\n\n 18/12/2023\n - Adding a Login Screen\n - Fixing the messy AF code\n - Added a Working Injector\n - Adding more random crap e.g. random guessing game thing idk\n - Fixing Game injector, Only For x64 Processes\n - Easier to use the injector (added file dialog)\n - Accidentally messed up the code and had to delete some stuff so expect some temporary bugs\n\n 17/12/2023\n\n - Cleaning Up Code\n - Just Adding more Features", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
				}

				ImGui::Spacing();

				//check/update loader version
				if (ImGui::Button("Check For Updates", { ImGui::GetContentRegionAvail().x, butn_tall }))
				{
					{
						auto read_file = [](const std::string& filename) -> std::string {
							std::ifstream file(filename);
							return file.is_open() ? std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>()) : "";
							};

						version = read_file("Version.txt");
					}

					if (version.find(CurrentVersion) != std::string::npos) {
						int choice = MessageBox(nullptr, "Maven Cheat Loader is up to date", "Maven Cheat Loader", MB_OK | MB_ICONINFORMATION);
					}
					else {
						int choice = MessageBox(nullptr, "There is an update! Would you like to download it?", "Maven Cheat Loader", MB_YESNO | MB_ICONINFORMATION);
						if (choice == IDYES) {
							ShellExecute(0, 0, "https://github.com/MavenCoding157/Maven-Cheat-Loader/", 0, 0, SW_SHOW);
							exit(0);
						}
						else {
							// Do nothing (aka no update)
						}
					}
				}


				//Idk wtf this is below, was palnning to add an api or some crap. Might add later though idk yet

				//static char text[256] = "";
				//ImGui::Text("	  API Key [BETA]");
				//ImGui::SetNextItemWidth(147);
				//if (ImGui::InputText("", text, 265, ImGuiInputTextFlags_Password)) {
					 //Do something when the text is changed
				//}

				//if (ImGui::Button("Copy", { ImGui::GetContentRegionAvail().x, butn_tall })) {
					//ImGui::SetClipboardText(text);
				//}
			}
			ImGui::EndChild();
			break;

		case 0:
			ImGuiPP::CenterText("Cheat Loader [BETA]", 1, TRUE);

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::PushItemWidth(451); //changes the textbox size
			ImGui::ListBox("##Games", &selectedGame, games, IM_ARRAYSIZE(games));

			ImGui::Spacing();

			ImGui::Spacing();

			if (ImGui::Button("Inject DLL", ImVec2(451, 50)))
			{

				switch (selectedGame) {
				case 0: // GTA V
					selectedFileName = GetExeDirectory() + "\\Maven GTA V Trainer.dll";
					selectedProcessName = "GTA5.exe";
					break;
				case 1: // Gmod Injection (aka x32 bit) | Not finished yet
				{
					const char* SecondDllPath = (GetExeDirectory() + "\\ImGuiHook [DX9].dll").c_str();
					DWORD targetProcessId = GetProcId("hl2.exe");

					if (InjectDLL32bit(targetProcessId, SecondDllPath)) {
						MessageBox(nullptr, "DLL Successfully Injected", "Maven Cheat Loader", MB_OK | MB_ICONINFORMATION);
					}
					else {
						MessageBox(nullptr, "DLL Injection Failed!", "Maven Cheat Loader", MB_OK | MB_ICONINFORMATION);
					}
					break;
				}
				case 2: // CS2 | Coming Soon...
					selectedFileName = "cs2.dll";
					selectedProcessName = "cs2.exe";
					break;
				default:
					break;
				}

				//injection for all other games (aka x64 bit)
				if (selectedProcessName != nullptr) {
					DWORD procId = GetProcId(selectedProcessName);

					if (procId == 0) {
						// initilized when process isnt found
						MessageBox(NULL, "Process not found!", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
					}
					else {
						// performs the injection (below)
						MessageBox(NULL, "DLL Injected successfully!", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
						InjectDLL(procId, selectedFileName.c_str());
					}
				}
				else {
					MessageBox(nullptr, "Error", "Maven Cheat Loader", MB_OK | MB_ICONINFORMATION);
				}

				if (StealthInject) //if stealth inject is turned on
				{
					if (selectedProcessName != nullptr) {
						DWORD procId = GetProcId(selectedProcessName);

						if (procId == 0) {
							// initialized when process isn't found
							MessageBox(NULL, "Process not found!", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
						}
						else {
							//anti cheat bypass (very basic)
							AntiAC antiAC;

							antiAC.check_for_debug();

							antiAC.erase_pe_headers();
							//to here

							// Perform the injection
							MessageBox(NULL, "DLL Injected successfully!", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
							InjectDLL(procId, selectedFileName.c_str());
						}
					}
					else {
						MessageBox(nullptr, "Error", "Maven Cheat Loader", MB_OK | MB_ICONINFORMATION);
					}
				}
				else
					if (StealthInject && CloseOnInject)
					{
						if (selectedProcessName != nullptr) {
							DWORD procId = GetProcId(selectedProcessName);

							if (procId == 0) {
								// initialized when process isn't found
								MessageBox(NULL, "Process not found!", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
							}
							else {
								//anti cheat bypass (very basic)
								AntiAC antiAC;

								antiAC.check_for_debug();

								antiAC.erase_pe_headers();
								//to here

								// Perform the injection
								MessageBox(NULL, "DLL Injected successfully!", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
								InjectDLL(procId, selectedFileName.c_str());
								exit(0);
							}
						}
						else {
							MessageBox(nullptr, "Error", "Maven Cheat Loader", MB_OK | MB_ICONINFORMATION);
						}
					}
				//this is just for close on inject
				if (CloseOnInject)
				{
					if (selectedProcessName != nullptr) {
						DWORD procId = GetProcId(selectedProcessName);

						if (procId == 0) {
							// initilized when process isnt found
							MessageBox(NULL, "Process not found!", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
						}
						else {
							// performs the injection (below)
							MessageBox(NULL, "DLL Injected successfully!", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
							InjectDLL(procId, selectedFileName.c_str());
							exit(0);
						}
					}
					else {
						MessageBox(nullptr, "Error", "Maven Cheat Loader", MB_OK | MB_ICONINFORMATION);
					}
				}
			}


				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::Text("Selected Game: "); //shows selected game
				ImGui::SameLine();
				ImGui::TextColored(ImColor(0, 128, 0), "%s", games[selectedGame]);

				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();
				ImGui::Spacing();

				ImGui::Text("Status: ");
				ImGui::SameLine();
				ImGui::TextColored(ImColor(0, 128, 0), "Undetected");

				break;

		case 10:
			ImGuiPP::CenterText("Spoofer", 1, TRUE);

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::SetNextItemWidth(451);
			ImGui::Combo("", &selectedItem, items, ARRAYSIZE(items));

			ImGui::Spacing();

			if (ImGui::Button("HWID Spoofer", ImVec2(451, 50)))
			{
				spoofer();
			};

			ImGui::Spacing();

			if (ImGui::Button("Trace Cleaner", ImVec2(451, 50)))
			{
				cleaner();
			}

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::Text("Status: ");
			ImGui::SameLine();
			ImGui::TextColored(ImColor(252, 244, 5), "Coming Soon....");

			break;

		case 5:
			ImGuiPP::CenterText("Settings", 1, TRUE);

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::Toggle("Stealth Inject", &StealthInject);
			ImGui::SameLine();
			ImGui::TextColored(ImColor(252, 244, 5), "(?)");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Hides the injection process from the Anti-Cheat");

			ImGui::Spacing();
			ImGui::Toggle("Close Program On Inject", &CloseOnInject);
			ImGui::SameLine();
			ImGui::TextColored(ImColor(252, 244, 5), "(?)");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Closes the Loader on injection");

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			//for custom back color
			ImGuiPP::CenterText("Colour Settings", 1, TRUE);

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::ColorEdit4("##BackgroundColor", (float*)&bgColor, ImGuiColorEditFlags_NoInputs);
			ImGui::SameLine();
			ImGui::TextColored(ImColor(252, 244, 5), "(?)");
			if (ImGui::IsItemHovered())
				ImGui::SetTooltip("Change the Background Color");


			//this is for custom
			// this sets the background colour for the entire window
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(bgColor.x, bgColor.y, bgColor.z, bgColor.w));
			ImGui::PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(bgColor.x, bgColor.y, bgColor.z, bgColor.w));
			//to here

			break;

		case 6:
			ImGuiPP::CenterText("Made by: MavenCoding157", 1, TRUE);

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::Spacing();
			if (ImGui::Button("My GitHub", { ImGui::GetContentRegionAvail().x, butn_tall }))
				ShellExecute(0, 0, "https://github.com/MavenCoding157", 0, 0, SW_SHOW);
			ImGui::Spacing();
			if (ImGui::Button("My Youtube", { ImGui::GetContentRegionAvail().x, butn_tall }))
				ShellExecute(0, 0, "https://www.youtube.com/channel/UCkP2YjZfvZIfArYbAUyRLsg", 0, 0, SW_SHOW);
			ImGui::Spacing();
			if (ImGui::Button("FREE GAMES", { ImGui::GetContentRegionAvail().x, butn_tall }))
				ShellExecute(0, 0, "https://drive.google.com/drive/folders/1myezzlndx8HAv9wtVErD2hoIAP_5Q4g-", 0, 0, SW_SHOW);

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			ImGuiPP::CenterText("Password Guessing Game", 1, TRUE);

			GuessPassword(); //password guessing game

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			//for mouse position (Only For UI)
			ImVec2 mousePos = ImGui::GetMousePos();

			ImGui::Text("                        Mouse Position: (%.1f, %.1f)", mousePos.x, mousePos.y);
			//to here

			//this below just looks cool (DEL)
			float samples[100];
			for (int n = 0; n < 100; n++)
				samples[n] = sinf(n * 0.2f + ImGui::GetTime() * 1.5f);

			ImVec2 plotSize(452, 50);
			ImGui::PlotLines("Plot Line", samples, 100, 0, nullptr, FLT_MAX, FLT_MAX, plotSize);


			break;
			}

		ImGui::EndChild();
	}

	ImGui::End();
}

