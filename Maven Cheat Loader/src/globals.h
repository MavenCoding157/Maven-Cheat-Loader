#pragma once
//some of these includes are useless but cba to remove them
#include <cstdint>
#include <cstddef>
#include <ctime>
#include "antiAC.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_stdlib.h"
#include <string>
#include <sstream>
#include <fstream>
#include <vector>
#include <tchar.h>
#include <tlhelp32.h>
#include <WtsApi32.h>
#pragma comment(lib, "Wtsapi32.lib")
#include <iostream>
#include <windows.h>
#include <thread>
#include <d3d9.h>



namespace globals
{
	inline float Backgroundcolor[] = { 1.f, 0.f, 0.f, 1.f };
}


namespace checkboxes
{
	
}

//for injection status
inline bool Injected = false;


// hwid spoofer and cleaner below

//for spoofer
static void SpooferComingSoon() {
	MessageBox(NULL, "Coming Soon...", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
}

static void spoofer()
{
	SpooferComingSoon();
}
//to here

//for cleaner
static void CleanerComingSoon() {
	MessageBox(NULL, "Coming Soon...", "Maven Cheat Loader", MB_ICONINFORMATION | MB_OK);
}

static void cleaner()
{
	CleanerComingSoon();
}
//to here
//to here