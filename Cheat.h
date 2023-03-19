#pragma once
#include <Windows.h>

//用到的偏移量信息
#define character_info_offset 0x57E0A8   //其中人物信息在结构中进一步细分

#define perspective_matrix_offset 0x57DFD0  //找到的4*4矩阵的首地址

#define character_list_offset 0x58AC04
#define character_num_offset 0x58AC0C

//游戏窗口信息
#define Win_Width_offset 0x591ED8
#define Win_Height_offset 0x591EDC


extern bool modeESP;
extern bool modeAutoAim;

struct P_Matrix {  //投影矩阵
	float a01, a02, a03, a04;
	float a11, a12, a13, a14;
	float a21, a22, a23, a24;
	float a31, a32, a33, a34;
};

//把没用的补全，只需要定义我们要使用的那几个变量
struct Character_info {
	//char unknown1[4];
	char meaningless_a[0x28];
	float x; 
	float y; 
	float z; 
	float d_angle;
	float p_angle;
	char meaningless_b[0x1C9];
	char name[16];
	char meaningless_c[0xF7];
	int team_info;  //找到的玩家的阵营信息
	char meaningless_d[0x8];
	int is_dead;  //描述存活状态的信息
};


void Paint_border();

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

void InitWindow();

void DeleteWindow();

void Init();
float get_distance(float, float);

bool WorldToScreen(Character_info* character);

void get_eny_info();   //通过遍历获取全部对局玩家的信息，并得到映射后的坐标

void Self_Aiming();

void UpdateAim();
