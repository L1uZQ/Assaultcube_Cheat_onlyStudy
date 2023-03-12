#include <Windows.h>
#include <math.h>
#include "PlayerCheat.h"

#define M_PI 3.1415926

DWORD* character_info_ptr;
DWORD* character_list;
int* character_num;

Character_info* my_character;
P_Matrix* p_matrix;

float closest_dis = -1.0f;
float closest_yaw = -1.0f;
float closest_pitch = -1.0f;

float val_xhead[32];
float val_yhead[32];

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
	memset(val_xhead, 0, sizeof val_xhead);
	memset(val_yhead, 0, sizeof val_yhead);
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
			float xhead = camX + (camX * clipCoords_X / clipCoords_W);
			float yhead = camY - (camY * clipCoords_Y / clipCoords_W);

			strcpy_s(name[i], other_character->name);
			val_xhead[i] = xhead;
			val_yhead[i] = yhead;
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
	closest_dis = get_distance(eny_X, eny_Y);
	return true;
}


void AutoAim() {
	//初始化
	closest_yaw = 0.0f;
	closest_pitch = 0.0f;

	//敌人距离准星的距离
	closest_dis = -1.0f;
	float current_dis = -1.0f;

	// 遍历所有玩家 找到距离屏幕准星最近的敌人 注意i从1开始
	for (int i = 1; i < *character_num; i++) {
		DWORD* target_offset = (DWORD*)(*character_list + (i * 4));
		Character_info* target = (Character_info*)(*target_offset);

		// 保证敌人存活且在视角范围内
		if (my_character != NULL && target != NULL && my_character->team_info != target->team_info && !target->is_dead && p_matrix != NULL
			&& WorldToScreen(target)) {

			float abspos_x = target->x - my_character->x;
			float abspos_y = target->y - my_character->y;
			float abspos_z = target->z + 0.3 - my_character->z;	//加0.3是为了更好瞄准头部
			float Horizontal_distance = get_distance(abspos_x, abspos_y);	//2D平面距离
			// 如果当前敌人离准星最近，计算偏航角和偏仰角
			if (current_dis == -1.0f || closest_dis < current_dis) {
				current_dis = closest_dis;
				// 计算yaw
				float azimuth_xy = atan2f(abspos_y, abspos_x);
				// 转换成角度制
				float yaw = (float)(azimuth_xy * (180.0 / M_PI));
				// 需要加90度 因为玩家的起始yaw就是90°
				closest_yaw = yaw + 90;

				// 计算pitch
				float azimuth_z = atan2f(abspos_z, Horizontal_distance);
				// 转换成角度制
				closest_pitch = (float)(azimuth_z * (180.0 / M_PI));
			}
		}
	}

	// 将准星移到最近的敌人处
	if (current_dis != -1.0f) {
		//UpdateAim();
		my_character->yaw = closest_yaw;
		my_character->pitch = closest_pitch;
	}
}

//void UpdateAim() {
//
//
//}


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
	//SelectObject(paintDC, graph);
	HGDIOBJ oldbitmap = SelectObject(paintDC, graph);
	BitBlt(paintDC, 0, 0, Windows_Width, Windows_Height, 0, 0, 0, WHITENESS);
	for (int i = 1; i < (*character_num); i++) {

		SelectObject(paintDC, GetStockObject(DC_PEN));
		is_enemy[i] ? SetDCPenColor(paintDC, rgbRed) : SetDCPenColor(paintDC, rgbGreen);

		//因为我们之前算的是按人物下脚下的坐标映射，需要进行左右上下的转换
		//WINGDIAPI BOOL WINAPI Ellipse(_In_ HDC hdc, _In_ int left, _In_ int top, _In_ int right, _In_ int bottom);
		Ellipse(paintDC, val_xhead[i]- Windows_Width/30, val_yhead[i] - Windows_Height/7, val_xhead[i] + Windows_Width / 30, val_yhead[i]);

		if (is_enemy[i]) { //如果是敌人，为其绘制边框
			SetTextColor(paintDC, 0x00800080);
		}
		const COLORREF rgbRed = 0x000000FF;
		//is_enemy[i] ? SetTextColor(paintDC, rgbRed) : SetTextColor(paintDC, rgbGreen);

		SelectObject(paintDC, font);
		TextOutA(paintDC, val_xhead[i], val_yhead[i], name[i], strlen(name[i]));
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
