#include <Windows.h>
#include<stdio.h>
#include "PlayerCheat.h"
#include<iostream>
using namespace std;
void check()
{
	while (true) {
		if (GetAsyncKeyState(VK_F1) & 1) {
			modeESP = !modeESP;
			if (modeESP == true) {
				InitWindow();
			}
			else {
				DeleteWindow();
			}
		}
		if (modeESP)
			get_eny_info();
		if (GetAsyncKeyState(VK_F2) & 1) {
			modeAutoAim = !modeAutoAim;
		}
		if (modeAutoAim)
			AutoAim();
		Sleep(1);
	}
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	if (fdwReason == DLL_PROCESS_ATTACH) {
		Init();
		CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)check, NULL, 0, NULL);
	}
	cout << "111" << endl;
	return true;
}