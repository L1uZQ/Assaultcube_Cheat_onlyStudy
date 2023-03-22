#include <Windows.h>
#include "Cheat.h"
#include<iostream>
using namespace std;
void run()
{
	while (true) {
		if (GetAsyncKeyState(VK_F6)) {
			Show_enyBorder = !Show_enyBorder;
			if (Show_enyBorder == true) {
				InitWindow();
			}
			else {
				DeleteWindow();
			}
		}
		if (Show_enyBorder)get_eny_info();

		if (GetAsyncKeyState(VK_F7) ) {
			Open_self_Aiming = !Open_self_Aiming;
		}
		if (Open_self_Aiming) Self_Aiming();

		Sleep(5);
	}
}

bool WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		Init();
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)run, NULL, 0, NULL);
	}
	return true;
}