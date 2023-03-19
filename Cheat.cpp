#include <Windows.h>
#include <math.h>
#include "Cheat.h"

#define M_PI 3.1415926

DWORD* character_info_ptr;
DWORD* character_list;
int* character_num;

Character_info* my_character;
P_Matrix* p_matrix;

float current_evy_dis = -1.0f;
float dif_d_angle = -1.0f;
float dif_p_angle = -1.0f;

float eny_X[32];
float eny_Y[32];

bool is_enemy[32];
char name[32][16];

//屏幕大小
int Windows_Width;
int Windows_Height;

HWND Normal_Window; //游戏运行的窗口
HWND CheatWindow;  //我们的作弊程序的窗口
RECT window_loc;  //游戏窗口所处的矩形区域
HDC hDestDC;
HDC paintDC;
HFONT font;
const COLORREF rgbRed = 0x000000FF;
const COLORREF rgbGreen = 0x0000FF00;

bool modeESP = false;
bool modeAutoAim = false;

void Init() {
	//窗口信息
	Windows_Width = *(int*)Win_Width_offset;
	Windows_Height = *(int*)Win_Height_offset;

	//投影矩阵
	p_matrix = (P_Matrix*) perspective_matrix_offset;

	//取偏移处指针的值，然后声明一个Character_info类型的指针，访问人物信息
	my_character = (Character_info*) (*(DWORD*)character_info_offset);

	character_list = (DWORD*)character_list_offset;
	character_num = (int*)character_num_offset;
}

// 计算距离
float get_distance(float x, float y) {
	return sqrtf((x * x) + (y * y));
}

void get_eny_info() {
	// 遍历所有玩家 注意i从1开始
	memset(eny_X, 0, sizeof eny_X);
	memset(eny_Y, 0, sizeof eny_Y);
	memset(is_enemy, false, sizeof is_enemy);
	memset(name, 0, sizeof name);
	for (int i = 1; i < *character_num; i++) {
		DWORD* now_character_offset = (DWORD*)(*character_list + (i * 4));
		
		//遍历人物列表，取出来一个人物
		Character_info* other_character = (Character_info*)(*now_character_offset);
		float dis = -1.0f;

		// 保证敌人存活
		if (my_character  && other_character && p_matrix && ! other_character->is_dead && WorldToScreen(other_character)) {
		
			//脚部坐标target->x,y,z
			float clipCoords_X = (other_character->x *(p_matrix->a01) ) + (other_character->y * p_matrix->a11 ) + (other_character->z * p_matrix->a21 ) + p_matrix->a31;
			float clipCoords_Y = (other_character->x * p_matrix->a02 ) + (other_character->y* p_matrix->a12 ) + (other_character->z* p_matrix->a22 ) + p_matrix->a32;
			float clipCoords_W = (other_character->x * p_matrix->a04) + (other_character->y* p_matrix->a14 ) + (other_character->z*p_matrix->a24 ) + p_matrix->a34;

			//准心在屏幕的位置
			float camX = Windows_Width / 2.0f;
			float camY = Windows_Height / 2.0f;

			//转换成敌人在屏幕的坐标
			eny_X[i] = camX + (camX * clipCoords_X / clipCoords_W);
			eny_Y[i] = camY - (camY * clipCoords_Y / clipCoords_W);

			strcpy_s(name[i], other_character->name);
	/*		eny_X[i] = xhead;
			eny_Y[i] = yhead;*/
			if (other_character->team_info == my_character->team_info) {
				is_enemy[i] = false;
			}
			else {
				is_enemy[i] = true;
			}
		}
	}
	UpdateWindow(CheatWindow);
}


bool WorldToScreen(Character_info* character) {
	float screenX = (character->x* p_matrix->a01 ) + (character->y* p_matrix->a11) + (character->z* p_matrix->a21) + p_matrix->a31;
	float screenY = (character->x* p_matrix->a02 ) + (character->y* p_matrix->a12) + (character->z* p_matrix->a22) + p_matrix->a32;
	float screenW = (character->x* p_matrix->a04) + (character->y* p_matrix->a14) + (character->z* p_matrix->a24) + p_matrix->a34;
	if (screenW <= (float) 0.1) {
		return false;
	}

	//屏幕中心点
	float cen_X = Windows_Width /(float)2.0;
	float cen_Y = Windows_Height /(float)2.0;

	//敌人在屏幕的坐标
	float eny_X = (cen_X * screenX / screenW);
	float eny_Y = (cen_Y * screenY / screenW);
	current_evy_dis = get_distance(eny_X, eny_Y);
	return true;
}


void Self_Aiming() {
	
	//初始化
	const float PI = 3.1415926;
	dif_d_angle = 0.0;
	dif_p_angle = 0.0;
	float min_dis = 9999; //先初始化一个min_dis,后面根据计算的值进行更新

	// 遍历玩家列表，找到需要我们调整视角最小的敌人并锁定
	for (int i = 1; i < *character_num; i++) {
		//获取之前找到的人物信息和任务列表的偏移
		DWORD* now_character_offset = (DWORD*)(*character_list + (i * 4));
		Character_info* other_character = (Character_info*)(*now_character_offset);

		if (my_character  && other_character  && my_character->team_info != other_character->team_info && !other_character->is_dead && p_matrix && WorldToScreen(other_character)) {
			//根据坐标计算距离
			float current_eny_x = other_character->x - my_character->x;
			float current_eny_y = other_character->y - my_character->y;
			float current_eny_z = other_character->z  - my_character->z;	
			float sq_distance = get_distance(current_eny_x, current_eny_y);	
			
			// 计算俯仰角和偏转角
			if (min_dis == 9999 || current_evy_dis < min_dis) {
				min_dis = current_evy_dis; 

				// 计算需要水平移动的偏转角
				float alpha = atan2f(current_eny_y, current_eny_x);
				dif_d_angle = (float)(alpha * (180.0 / PI)) + 90;
				// 需要加90度，之前不加进游戏发现老是跟正确位置差90，
				//应该是游戏设置问题

				// 计算需要垂直移动的俯仰角
				float beta = atan2f(current_eny_z, sq_distance);
				dif_p_angle = (float)(beta * (180.0 / PI));
			}
		}
	}

	// 将准星移到需要锁定的敌人身上
	if (min_dis != 9999) {
		my_character->d_angle = dif_d_angle;
		my_character->p_angle = dif_p_angle;
	}
}


LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDOWN:
		break;
	case WM_PAINT:
		Paint_border();
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}



void InitWindow() {
	Normal_Window = FindWindowA(  NULL,"AssaultCube"); //游戏窗口
	GetClientRect(Normal_Window, &window_loc);  //获取窗口客户区的坐标

	WNDCLASSEX game_window;
	game_window.cbSize = sizeof(WNDCLASSEX);
	game_window.style = CS_HREDRAW | CS_VREDRAW;  //定义窗口风格为可以水平重绘，垂直重绘
	game_window.hInstance = 0;
	game_window.lpszClassName = (L"CheatTest");
	game_window.lpfnWndProc = WindowProcedure;
	game_window.hIcon = NULL;
	game_window.hIconSm = NULL;
	game_window.hCursor = NULL;
	game_window.lpszMenuName = NULL;
	game_window.cbClsExtra = 0;
	game_window.cbWndExtra = 0;
	game_window.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	RegisterClassEx(&game_window);

	POINT new_window_point;
	new_window_point.x = window_loc.left;
	new_window_point.y = window_loc.top;

	ClientToScreen(Normal_Window, &new_window_point);

	CheatWindow = CreateWindowEx(
		//WS_EX_TOPMOST：指明以该风格创建的窗口应放置在所有非最高层窗口的上面并且停留在其L，即使窗口未被激活。使用函数SetWindowPos来设置和移去这个风格。
		//WS_EX_TRANSPARENT：指定以这个风格创建的窗口在窗口下的同属窗口已重画时，该窗口才可以重画。由于其下的同属窗口已被重画，该窗口是透明的。
		(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED),  //为窗口加上指定透明的属性
		(L"CheatTest"),
		(L"CheatTest"),
		(WS_POPUP | WS_CLIPCHILDREN),
		new_window_point.x,
		new_window_point.y,
		Windows_Width,
		Windows_Height,
		0, 0, 
		game_window.hInstance,  //与原来的游戏窗口关联
		0
	);

	hDestDC = GetDC(CheatWindow);

	COLORREF background = GetBkColor(hDestDC);
	SetLayeredWindowAttributes(CheatWindow, background, 0, LWA_COLORKEY); //设置为透明

	ShowWindow(CheatWindow, SW_SHOWNORMAL);
	UpdateWindow(CheatWindow);

	paintDC = CreateCompatibleDC(0);
	SetTextAlign(paintDC, TA_CENTER | TA_NOUPDATECP);
	SetBkColor(paintDC, RGB(0, 0, 0));
	SetBkMode(paintDC, TRANSPARENT);
}



void Paint_border()
{
	HBITMAP graph = CreateCompatibleBitmap(hDestDC, Windows_Width, Windows_Height);
	SelectObject(paintDC, graph);
	//HGDIOBJ oldbitmap = SelectObject(paintDC, graph);
	BitBlt(paintDC, 0, 0, Windows_Width, Windows_Height, 0, 0, 0, WHITENESS);
	for (int i = 1; i < (*character_num); i++) {

		SelectObject(paintDC, GetStockObject(DC_PEN));
		is_enemy[i] ? SetDCPenColor(paintDC, rgbRed) : SetDCPenColor(paintDC, rgbGreen);

		if (is_enemy[i]) { //如果是敌人，为其绘制边框
			SetDCPenColor(paintDC, 0x00800080);
			SetTextColor(paintDC, 0x00800080);

			//WINGDIAPI BOOL WINAPI Ellipse(_In_ HDC hdc, _In_ int left, _In_ int top, _In_ int right, _In_ int bottom);
			//这里发现我之前找的是人物下脚下的坐标，头部应该还有一个地方没找，需要进行左右上下的转换
			// 因此这里就直接用一个进行变换了 ，将eny_X[i]、eny_Y[i]这一个点进行上下左右的扩展
			int left = eny_X[i] - Windows_Width / 30, top = eny_Y[i] - Windows_Height / 7;
			int right = eny_X[i] + Windows_Width / 30, bottom = eny_Y[i];

			Ellipse(paintDC, left , top, right, bottom);
			//Ellipse(paintDC, eny_X[i] - Windows_Width / 30, eny_Y[i] - Windows_Height / 7, eny_X[i] + Windows_Width / 30, eny_Y[i]);
			SelectObject(paintDC, font);
			TextOutA(paintDC, eny_X[i], eny_Y[i], name[i], strlen(name[i]));
		}

		const COLORREF rgbRed = 0x000000FF;
	}
	BitBlt(hDestDC, 0, 0, Windows_Width, Windows_Width, paintDC, 0, 0, SRCCOPY);
	DeleteObject(graph);
}




void DeleteWindow()
{
	DeleteDC(hDestDC);
	DeleteObject(paintDC);
	DestroyWindow(CheatWindow);
}
