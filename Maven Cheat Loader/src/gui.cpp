#include "gui.h"
#include <cstdlib>
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/ImGuiFileDialog.h" //imgui filepath

#include "globals.h"
#include <corecrt_math.h>
#include "../font/icons.h"
#include "../font/font.h"

#include <windows.h>
#include <shellapi.h>
#include "../imgui/imgui_toggle.h"
#include "imguidesign.h"//del
#include <iostream>
#include <fstream> 
#include <string>
#include <Lmcons.h>
#include <chrono>

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

bool CloseOnInject = false;//del
bool StealthInject = false;//del

bool my_checkbox_state = false;
constexpr auto butn_tall = 48;

//for login

//to here


//textbox guessing game thing (idk i was bored)

void GoodGuessMessageBox(); //need these here otherwise the section of code below will not work

void BadGuessMessageBox(); //need these here otherwise the section of code below will not work


void GuessPassword() {

	static char userInput[50] = "";

	// Textbox for the users input
	ImGui::PushItemWidth(451);
	ImGui::InputText("Enter Password Here##TextInput", userInput, sizeof(userInput));

	ImGui::Spacing();

	if (ImGui::Button("Guess The Password", ImVec2(453, 0))) {

		if (strcmp(userInput, "soasface123") == 0) {
			GoodGuessMessageBox();

			ShellExecute(0, 0, "https://pastebin.com/VXNR9Qpr", 0, 0, SW_SHOW); //Fake credit card list (ITS NOT REAL YOU TWOT)

		}
		else {
			BadGuessMessageBox();
		}
	}
}
//to here


//for custom back color
ImVec4 bgColor = ImVec4(0.13f, 0.14f, 0.18f, 1.0f);
//to here


//imgui file dialog
void OpenFileDialog(std::string& selectedFileName) {
	if (ImGui::Button("Find DLL", ImVec2(451, 50)))
		ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".dll", ".");

	if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey")) {

		if (ImGuiFileDialog::Instance()->IsOk()) {

			selectedFileName = ImGuiFileDialog::Instance()->GetFilePathName();
		}

		// close
		ImGuiFileDialog::Instance()->Close();
	}
}

std::string selectedFileName;  // varible to store the file name
//to here

//injecting stuff
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

std::string processName;
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
const char* games[] = { "GTA V", "CS2", "FiveM", "Rust","Apex Legends","Fortnite","COD Warzone","Rainbow Six Siege" };
int selectedGame = 0;

//for hwid spoofer
static bool checkBox = true;
static const char* items[]{ "Spoof For The Game Selected", "Spoof For All", "Rust", "Apex Legends", "Fortnite", "Fivem Server Unban", "COD Warzone", "Rainbow Six Siege" };
static int selectedItem = 0;
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


//ALL MESSAGE BOXS BELOW

//Loader status
void CheatStatusMessageBox() {
	//std::this_thread::sleep_for(std::chrono::seconds(1));// makes program sleep for 1 seconds
	MessageBox(NULL, "Loader Is Online!", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
}

//Update Log
void UpdateLogMessageBox() {
	//std::this_thread::sleep_for(std::chrono::seconds(1));// makes program sleep for 1 seconds
	MessageBox(NULL, "18/12/2023\n - Adding a Login Screen\n - Fixing the messy AF code\n - Added a Working Injector\n - Adding more random crap e.g. random guessing game thing idk\n - Fixing Game injector, Only For x64 Processes\n - Easier to use the injector (added file dialog)\n\n 17/12/2023\n - Cleaning Up Code\n - Just Adding more Features", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
}


//del
//right message box
void GoodGuessMessageBox() {
	MessageBox(NULL, "Success!!! How TF did you know the right password?", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
}

//wrong messagebox
void BadGuessMessageBox() {
	MessageBox(NULL, "Wrong!!!, hahahahaha you will never guess it.", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
}
//to here


//login succsess messagebox
void LoginsuccessfulMessageBox() {
	MessageBox(NULL, "Login Successful!!!", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
}

//login unsuccessful messagebox
void LoginUnsuccessfulMessageBox() {
	MessageBox(NULL, "Login Unsuccessful, Please try again.", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
}

//bad dll path
void dllUnsuccessfulMessageBox() {
	MessageBox(NULL, "The Specified DLL File does NOT exist!", "Maven Cheat Loader", MB_ICONERROR | MB_OK);
}

//good dll path
void dllSuccessfulMessageBox() {
	MessageBox(NULL, "DLL Injected successfully!", "Maven Cheat Loader", MB_ICONINFORMATION| MB_OK);
}

//process not found
void ProcessNotFoundMessageBox() {
	MessageBox(NULL, "Process not found!", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
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

		//colored text (not needed atm)
		static float Alpha = 255;
		static bool Tick = false;
		static float Speed = 1.0f;

		if (Tick || Alpha > -255)
		{
			Tick = true;
			if (!(Alpha <= 0))
				Alpha -= Speed;
			else if (Alpha <= 0)
				Tick ^= 1;
		}

		else if (!Tick || Alpha != 255)
		{
			Tick = false;
			if (!(Alpha >= 255))
				Alpha += Speed;
			else if (Alpha >= 255)
				Tick ^= 1;
		}

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


				ImGui::Text("Version: 1.1");

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

				ImGuiPP::CenterText("More Options", 1, TRUE);

				ImGui::Spacing();

				//(The only reason I added this is cause it looks coll btw)
				if (ImGui::Button("Check Loader Status", { ImGui::GetContentRegionAvail().x, butn_tall }))
				{
					CheatStatusMessageBox();
				}

				ImGui::Spacing();

				//and same with this
				if (ImGui::Button("Update Log", { ImGui::GetContentRegionAvail().x, butn_tall }))
				{
					UpdateLogMessageBox();
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
			ImGuiPP::CenterText("Injector [BETA] x64 Processes Only", 1, TRUE);

			ImGui::Spacing();
			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::PushItemWidth(451); //changes the textbox size
			ImGui::Text("                              Input Process name Below");
			InputText("##ProcessNameInput", processName);

			ImGui::Spacing();

			ImGui::PushItemWidth(451);// changes the textbox size
			ImGui::Text("                                  Input DLL path Below");
			ImGui::InputText("##DLLPathInput", &selectedFileName[0], selectedFileName.capacity(), ImGuiInputTextFlags_ReadOnly);

			ImGui::Spacing();

			OpenFileDialog(selectedFileName); //easier method to select the dll (i like it but i might remove it)

			ImGui::Spacing();

			if (ImGui::Button("Inject DLL", ImVec2(451, 50))) 
			{
				if (StealthInject) //if stealth inject is turned on
				{
					//ImGui::Text("Injection Status: N/A");
					ImGui::SameLine();
					//ImGui::Text("Injecting DLL...");
					ImGui::Separator();

					AntiAC antiAC;

					if (PathFileExists(selectedFileName.c_str()) == FALSE) {
						dllUnsuccessfulMessageBox();
						break;
					}

					DWORD procId = GetProcId(processName.c_str());
					if (procId == NULL) {
						ProcessNotFoundMessageBox();
						Sleep(3500);
						GetProcId("skinjbir", 0b10100111001);
					}
					else {
						dllSuccessfulMessageBox();
						InjectDLL(procId, selectedFileName.c_str());
					}
				}
				else
					//this is for if you toggle both | Need to fix the message box poping up twice
					if (StealthInject && CloseOnInject)
					{
						//ImGui::Text("Injection Status: N/A");
						ImGui::SameLine();
						//ImGui::Text("Injecting DLL...");
						ImGui::Separator();

						AntiAC antiAC;

						if (PathFileExists(selectedFileName.c_str()) == FALSE) {
							dllUnsuccessfulMessageBox();
							break;
						}

						DWORD procId = GetProcId(processName.c_str());
						if (procId == NULL) {
							ProcessNotFoundMessageBox();
							Sleep(3500);
							GetProcId("skinjbir", 0b10100111001);
						}
						else {
							dllSuccessfulMessageBox();
							InjectDLL(procId, selectedFileName.c_str());
							exit(0);
						}
					}
				//this is just for close on inject
				if (CloseOnInject)
				{
					//ImGui::Text("Injection Status: N/A");
					ImGui::SameLine();
					//ImGui::Text("Injecting DLL...");
					ImGui::Separator();

					if (PathFileExists(selectedFileName.c_str()) == FALSE) {
						dllUnsuccessfulMessageBox();
						break;
					}

					DWORD procId = GetProcId(processName.c_str());
					if (procId == NULL) {
						ProcessNotFoundMessageBox();
						Sleep(3500);
						GetProcId("skinjbir", 0b10100111001);
					}
					else {
						dllSuccessfulMessageBox();
						InjectDLL(procId, selectedFileName.c_str());
						exit(0);
					}
				}
				else // this below is for default injection
				{
					//ImGui::Text("Injection Status: N/A");
					ImGui::SameLine();
					//ImGui::Text("Injecting DLL...");
					ImGui::Separator();

					if (PathFileExists(selectedFileName.c_str()) == FALSE) {
						dllUnsuccessfulMessageBox();
						break;
					}

					DWORD procId = GetProcId(processName.c_str());
					if (procId == NULL) {
						ProcessNotFoundMessageBox();
						Sleep(3500);
						GetProcId("skinjbir", 0b10100111001);
					}
					else {
						dllSuccessfulMessageBox();
						InjectDLL(procId, selectedFileName.c_str());
					}
				}
			
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

