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

//��Ļ��С
int Windows_Width;
int Windows_Height;

HWND Normal_Window; //��Ϸ���еĴ���
HWND CheatWindow;  //���ǵ����׳���Ĵ���
RECT window_loc;  //��Ϸ���������ľ�������
HDC hDestDC;
HDC paintDC;
HFONT font;
const COLORREF rgbRed = 0x000000FF;
const COLORREF rgbGreen = 0x0000FF00;

bool modeESP = false;
bool modeAutoAim = false;

void Init() {
	//������Ϣ
	Windows_Width = *(int*)Win_Width_offset;
	Windows_Height = *(int*)Win_Height_offset;

	//ͶӰ����
	p_matrix = (P_Matrix*) perspective_matrix_offset;

	//ȡƫ�ƴ�ָ���ֵ��Ȼ������һ��Character_info���͵�ָ�룬����������Ϣ
	my_character = (Character_info*) (*(DWORD*)character_info_offset);

	character_list = (DWORD*)character_list_offset;
	character_num = (int*)character_num_offset;
}

// �������
float get_distance(float x, float y) {
	return sqrtf((x * x) + (y * y));
}

void get_eny_info() {
	// ����������� ע��i��1��ʼ
	memset(eny_X, 0, sizeof eny_X);
	memset(eny_Y, 0, sizeof eny_Y);
	memset(is_enemy, false, sizeof is_enemy);
	memset(name, 0, sizeof name);
	for (int i = 1; i < *character_num; i++) {
		DWORD* now_character_offset = (DWORD*)(*character_list + (i * 4));
		
		//���������б�ȡ����һ������
		Character_info* other_character = (Character_info*)(*now_character_offset);
		float dis = -1.0f;

		// ��֤���˴��
		if (my_character  && other_character && p_matrix && ! other_character->is_dead && WorldToScreen(other_character)) {
		
			//�Ų�����target->x,y,z
			float clipCoords_X = (other_character->x *(p_matrix->a01) ) + (other_character->y * p_matrix->a11 ) + (other_character->z * p_matrix->a21 ) + p_matrix->a31;
			float clipCoords_Y = (other_character->x * p_matrix->a02 ) + (other_character->y* p_matrix->a12 ) + (other_character->z* p_matrix->a22 ) + p_matrix->a32;
			float clipCoords_W = (other_character->x * p_matrix->a04) + (other_character->y* p_matrix->a14 ) + (other_character->z*p_matrix->a24 ) + p_matrix->a34;

			//׼������Ļ��λ��
			float camX = Windows_Width / 2.0f;
			float camY = Windows_Height / 2.0f;

			//ת���ɵ�������Ļ������
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

	//��Ļ���ĵ�
	float cen_X = Windows_Width /(float)2.0;
	float cen_Y = Windows_Height /(float)2.0;

	//��������Ļ������
	float eny_X = (cen_X * screenX / screenW);
	float eny_Y = (cen_Y * screenY / screenW);
	current_evy_dis = get_distance(eny_X, eny_Y);
	return true;
}


void AutoAim() {
	//��ʼ��
	dif_d_angle = 0.0f;
	dif_p_angle = 0.0f;

	//���˾���׼�ǵľ���
	current_evy_dis = -1.0f;
	float min_dis = -1.0f;

	// ����������� �ҵ�������Ļ׼������ĵ��� ע��i��1��ʼ
	for (int i = 1; i < *character_num; i++) {
		DWORD* target_offset = (DWORD*)(*character_list + (i * 4));
		Character_info* target = (Character_info*)(*target_offset);

		// ��֤���˴�������ӽǷ�Χ��
		if (my_character != NULL && target != NULL && my_character->team_info != target->team_info && !target->is_dead && p_matrix != NULL
			&& WorldToScreen(target)) {

			float abspos_x = target->x - my_character->x;
			float abspos_y = target->y - my_character->y;
			//float abspos_z = target->z + 0.3 - my_character->z;	//��0.3��Ϊ�˸�����׼ͷ��
			float abspos_z = target->z  - my_character->z;	//��0.3��Ϊ�˸�����׼ͷ��
			float Horizontal_distance = get_distance(abspos_x, abspos_y);	//2Dƽ�����
			// ���㸩���Ǻ�ƫת��
			if (min_dis == -1.0f || current_evy_dis < min_dis) {
				min_dis = current_evy_dis;
				// ����yaw
				float alpha = atan2f(abspos_y, abspos_x);
				// ת���ɽǶ���
				dif_d_angle = (float)(alpha * (180.0 / M_PI)) + 90;
				// ��Ҫ��90�� ��Ϊ��ҵ���ʼyaw����90��
				//closest_yaw = yaw + 90;

				// ����pitch
				float beta = atan2f(abspos_z, Horizontal_distance);
				// ת���ɽǶ���
				dif_p_angle = (float)(beta * (180.0 / M_PI));
			}
		}
	}

	// ��׼���Ƶ�����ĵ��˴�
	if (min_dis != -1.0f) {
		//UpdateAim();
		my_character->yaw = dif_d_angle;
		my_character->pitch = dif_p_angle;
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
	Normal_Window = FindWindowA(  NULL,"AssaultCube"); //��Ϸ����
	GetClientRect(Normal_Window, &window_loc);  //��ȡ���ڿͻ���������

	WNDCLASSEX game_window;
	game_window.cbSize = sizeof(WNDCLASSEX);
	game_window.style = CS_HREDRAW | CS_VREDRAW;  //���崰�ڷ��Ϊ����ˮƽ�ػ棬��ֱ�ػ�
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
		//WS_EX_TOPMOST��ָ���Ը÷�񴴽��Ĵ���Ӧ���������з���߲㴰�ڵ����沢��ͣ������L����ʹ����δ�����ʹ�ú���SetWindowPos�����ú���ȥ������
		//WS_EX_TRANSPARENT��ָ���������񴴽��Ĵ����ڴ����µ�ͬ���������ػ�ʱ���ô��ڲſ����ػ����������µ�ͬ�������ѱ��ػ����ô�����͸���ġ�
		(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED),  //Ϊ���ڼ���ָ��͸��������
		(L"CheatTest"),
		(L"CheatTest"),
		(WS_POPUP | WS_CLIPCHILDREN),
		new_window_point.x,
		new_window_point.y,
		Windows_Width,
		Windows_Height,
		0, 0, 
		game_window.hInstance,  //��ԭ������Ϸ���ڹ���
		0
	);

	hDestDC = GetDC(CheatWindow);

	COLORREF background = GetBkColor(hDestDC);
	SetLayeredWindowAttributes(CheatWindow, background, 0, LWA_COLORKEY); //����Ϊ͸��

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

		if (is_enemy[i]) { //����ǵ��ˣ�Ϊ����Ʊ߿�
			SetDCPenColor(paintDC, 0x00800080);
			SetTextColor(paintDC, 0x00800080);

			//WINGDIAPI BOOL WINAPI Ellipse(_In_ HDC hdc, _In_ int left, _In_ int top, _In_ int right, _In_ int bottom);
			//���﷢����֮ǰ�ҵ��������½��µ����꣬ͷ��Ӧ�û���һ���ط�û�ң���Ҫ�����������µ�ת��
			// ��������ֱ����һ�����б任�� ����eny_X[i]��eny_Y[i]��һ��������������ҵ���չ
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
