#pragma once
#include <Windows.h>

//�õ���ƫ������Ϣ
#define character_info_offset 0x57E0A8   //����������Ϣ�ڽṹ�н�һ��ϸ��

#define perspective_matrix_offset 0x57DFD0  //�ҵ���4*4������׵�ַ

#define character_list_offset 0x58AC04
#define character_num_offset 0x58AC0C

//��Ϸ������Ϣ
#define Win_Width_offset 0x591ED8
#define Win_Height_offset 0x591EDC


extern bool modeESP;
extern bool modeAutoAim;

struct P_Matrix {  //ͶӰ����
	float a01, a02, a03, a04;
	float a11, a12, a13, a14;
	float a21, a22, a23, a24;
	float a31, a32, a33, a34;
};

//��û�õĲ�ȫ��ֻ��Ҫ��������Ҫʹ�õ��Ǽ�������
struct Character_info {
	//char unknown1[4];
	char unknown1[0x28];
	float x; 
	float y; 
	float z; 
	float yaw;
	float pitch;
	char unknown3[0x1C9];
	char name[16];
	char unknown4[0xF7];
	int team_info;  //�ҵ�����ҵ���Ӫ��Ϣ
	char unknown5[0x8];
	int is_dead;  //���״̬
};



void Paint_border();

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

void InitWindow();

void DeleteWindow();

void Init();
float get_distance(float, float);

bool WorldToScreen(Character_info* character);

void get_eny_info();   //ͨ��������ȡȫ���Ծ���ҵ���Ϣ�����õ�ӳ��������

void AutoAim();

void UpdateAim();