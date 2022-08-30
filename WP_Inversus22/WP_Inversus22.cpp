#include <windows.h>
#include <tchar.h>
#include "stdafx.h"
#include <math.h>
#include "Resource.h"

#define BOARDSIZE 25
#define PLAYERSIZE 15

HINSTANCE g_hinst;
LPCTSTR lpszClass = L"Window Class Name";
LPCTSTR lpszWindowsName = L"Inversus";

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE hPrevinstance, LPSTR lpszCmdParam, int nCmdShow) {

	HWND hWnd;
	MSG Message;
	WNDCLASSEX WndClass;
	g_hinst = hinstance;

	WndClass.cbSize = sizeof(WndClass);
	WndClass.style = CS_HREDRAW | CS_VREDRAW;
	WndClass.lpfnWndProc = (WNDPROC)WndProc;
	WndClass.cbClsExtra = 0;
	WndClass.cbWndExtra = 0;
	WndClass.hInstance = hinstance;
	WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	WndClass.lpszMenuName = NULL;
	WndClass.lpszClassName = lpszClass;
	WndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
	RegisterClassEx(&WndClass);

	hWnd = CreateWindow
	(lpszClass, lpszWindowsName, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX |
		WS_MAXIMIZEBOX, 0, 0, 765, 785, NULL, (HMENU)NULL, hinstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	while (GetMessage(&Message, 0, 0, 0)) {
		TranslateMessage(&Message);
		DispatchMessage(&Message);
	}
	return Message.wParam;
}
struct Board {
	int Color = 0;
	bool Hatch = false;
	int BulletStock = 0;
	float BulletShapexPos[3];
	float BulletShapeyPos[3];
	POINT Position;
};
struct Player {
	int Life = 3;
	int BulletStock = 6;
	float BulletShapexPos[6];
	float BulletShapeyPos[6];
	int DeathTimer = 0;
	bool DeathFlag = false;
	int SpecialBullet = 0;
	RECT SpawnRect;
	int SpawnRectSize = 80;
	POINT Position;
};
struct Enemy {
	int Speed = 1;
	int SpawnTimer = 0;
	int DeathTimer = 80;
	bool DeathFlag = true;
	bool Explosion = false;
	RECT SpawnRect;
	int SpawnRectSize = 80;
	POINT Position;
};
struct Bullet {
	int Direction;
	POINT Position = { -100,-100 };
	bool isSpecialBullet = false;
};
LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam) {
	PAINTSTRUCT ps;
	HDC hdc, bufferdc, Jellydc;
	HBRUSH hBrush, oldBrush, hBrush2, oldBrush2;
	HPEN hPen, oldPen;
	BITMAP bmp;
	static HBITMAP hBitmap, oldBitmap, Jelly[7], Youdai;
	static RECT ClientRect;
	static Board Board[15][15];
	static Enemy Enemy[30];
	static Player Player;
	static Bullet Bullet[10][20];
	static int Bulletcount = 0;
	static int BulletChargeTimer = 0;
	static bool Collision = false;
	static float angle = 0;
	static POINT EnemyCreatePos;
	static bool Immortal = false;
	static bool Quakeflag = false;
	static int Quaketimer = 0;
	static int JellyTimer = 0, JellyTimer2 = 0, Height[7], Width[7], Dieheight, Diewidth;
	static int EnemyCount = 0;
	static int EnemyCreateCount = 0;
	static int EnemyNum = 0;
	static int EnemyCreateSpeed = 150;
	static int Combo = 0;
	static TCHAR Combostr[10];
	static int Score = 0;
	static TCHAR Scorestr[20];
	static int ComboTimer = 0;
	static bool pause = false;
	static bool main = true;
	switch (iMessage) {
	case WM_CREATE:
		SetTimer(hWnd, 1, 6, NULL);
		GetClientRect(hWnd, &ClientRect);
		Jelly[0] = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_BITMAP1));
		Jelly[1] = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_BITMAP2));
		Jelly[2] = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_BITMAP3));
		Jelly[3] = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_BITMAP4));
		Jelly[4] = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_BITMAP5));
		Jelly[5] = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_BITMAP6));
		Jelly[6] = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_BITMAP7));
		Youdai = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDB_BITMAP8));
		for (size_t i = 0; i < 7; i++) {
			GetObject(Jelly[i], sizeof(BITMAP), &bmp);
			Width[i] = bmp.bmWidth;
			Height[i] = bmp.bmHeight;
		}
		GetObject(Youdai, sizeof(BITMAP), &bmp);
		Diewidth = bmp.bmWidth;
		Dieheight = bmp.bmHeight;
		for (size_t i = 0; i < 15; i++)
		{
			for (size_t j = 0; j < 15; j++)
			{
				Board[i][j].Position.x = (i * BOARDSIZE * 2) + BOARDSIZE;
				Board[i][j].Position.y = (j * BOARDSIZE * 2) + BOARDSIZE;
				if (i > 2 && i < 12 && j >2 && j < 12) {
					Board[i][j].Color = 0;
				}
				else {
					Board[i][j].Color = 4;
				}
			}
		}
		Enemy[7].Speed = 3;
		Enemy[14].Speed = 3;
		Enemy[21].Speed = 3;
		Enemy[28].Speed = 3;
		Player.Position.x = 750 / 2;
		Player.Position.y = 750 / 2;
		break;
	case WM_TIMER:
		if (Player.Life < 1) {
			pause = true;
		}
		if (pause == false) {
			if (JellyTimer2 < 200) {
				JellyTimer2++;
			}
			else {
				JellyTimer2 = 0;
			}
			if (JellyTimer2 % 10 == 0) {
				JellyTimer++;
			}
			if (JellyTimer > 6) {
				JellyTimer = 0;
			} 

			if (ComboTimer < 250) {
				ComboTimer++;
			}
			else {
				ComboTimer = 0;
				Combo = 0;
			}
			
			// 적 생성
			EnemyCreateCount++;
			if (EnemyCreateCount % EnemyCreateSpeed == 0) {
				EnemyCreatePos.x = rand() % 750;
				EnemyCreatePos.y = rand() % 750;
				if (((EnemyCreatePos.x > Player.Position.x && EnemyCreatePos.x - 300 > Player.Position.x) || (EnemyCreatePos.x < Player.Position.x && EnemyCreatePos.x + 300 < Player.Position.x)) ||
					((EnemyCreatePos.y > Player.Position.y && EnemyCreatePos.y - 300 > Player.Position.y) || (EnemyCreatePos.y < Player.Position.y && EnemyCreatePos.y + 300 < Player.Position.y))) {
					if (EnemyCount < 29) {
						for (size_t i = 0; i < EnemyCount; i++)
						{
							if (Enemy[i].DeathFlag == true && Enemy[i].DeathTimer>78) {
								Enemy[i].Position = EnemyCreatePos;
								Enemy[i].DeathFlag = false;
								Enemy[i].SpawnRectSize = 80;
								Enemy[i].Explosion = true;
								Enemy[i].SpawnTimer = 0;
							}
						}
						EnemyCount++;
					}
				}
				else {
					EnemyCreateCount--;
				}
			}
			// 적 생성

			// 플레이어 이동-
			if (Player.DeathFlag == false && Player.Life > 0) {
				if (GetAsyncKeyState('D')) {
					if (Player.Position.x < 750 - PLAYERSIZE) {
						for (size_t i = 0; i < 15; i++)
						{
							for (size_t j = 0; j < 15; j++)
							{
								if (Board[i][j].Color > 0) {
									if (Board[i][j].Position.x - BOARDSIZE < Player.Position.x + PLAYERSIZE + 5 && Board[i][j].Position.x + BOARDSIZE > Player.Position.x - PLAYERSIZE &&
										Board[i][j].Position.y - BOARDSIZE < Player.Position.y + PLAYERSIZE && Board[i][j].Position.y + BOARDSIZE > Player.Position.y - PLAYERSIZE) {
										Collision = true;
									}
								}
								if (Board[i][j].Position.x - BOARDSIZE < Player.Position.x && Board[i][j].Position.x + BOARDSIZE > Player.Position.x &&
									Board[i][j].Position.y - BOARDSIZE < Player.Position.y && Board[i][j].Position.y + BOARDSIZE > Player.Position.y) {
									if (Board[i][j].BulletStock > 0 && Player.BulletStock < 12) {
										Player.BulletStock++;
										Player.SpecialBullet++;
										Board[i][j].BulletStock--;
									}
									else if (Board[i][j].BulletStock > 0) {
										Board[i][j].BulletStock--;
									}
								}
							}
						}
						if (Collision == false || Immortal == true) {
							Player.Position.x += 5;
						}
						else {
							Collision = false;
						}
					}
				}
				if (GetAsyncKeyState('A')) {
					if (Player.Position.x > PLAYERSIZE) {
						for (size_t i = 0; i < 15; i++)
						{
							for (size_t j = 0; j < 15; j++)
							{
								if (Board[i][j].Color > 0) {
									if (Board[i][j].Position.x - BOARDSIZE < Player.Position.x + PLAYERSIZE && Board[i][j].Position.x + BOARDSIZE > Player.Position.x - (PLAYERSIZE + 5) &&
										Board[i][j].Position.y - BOARDSIZE < Player.Position.y + PLAYERSIZE && Board[i][j].Position.y + BOARDSIZE > Player.Position.y - PLAYERSIZE) {
										Collision = true;
									}
								}
								if (Board[i][j].Position.x - BOARDSIZE < Player.Position.x && Board[i][j].Position.x + BOARDSIZE > Player.Position.x &&
									Board[i][j].Position.y - BOARDSIZE < Player.Position.y && Board[i][j].Position.y + BOARDSIZE > Player.Position.y) {
									if (Board[i][j].BulletStock > 0 && Player.BulletStock < 12) {
										Player.BulletStock++;
										Player.SpecialBullet++;
										Board[i][j].BulletStock--;
									}
									else if (Board[i][j].BulletStock > 0) {
										Board[i][j].BulletStock--;
									}
								}
							}
						}
						if (Collision == false || Immortal == true) {
							Player.Position.x -= 5;
						}
						else {
							Collision = false;
						}
					}
				}
				if (GetAsyncKeyState('S')) {
					if (Player.Position.y < 750 - PLAYERSIZE) {
						for (size_t i = 0; i < 15; i++)
						{
							for (size_t j = 0; j < 15; j++)
							{
								if (Board[i][j].Color > 0) {
									if (Board[i][j].Position.x - BOARDSIZE < Player.Position.x + PLAYERSIZE && Board[i][j].Position.x + BOARDSIZE > Player.Position.x - PLAYERSIZE &&
										Board[i][j].Position.y - BOARDSIZE < Player.Position.y + (PLAYERSIZE + 5) && Board[i][j].Position.y + BOARDSIZE > Player.Position.y - PLAYERSIZE) {
										Collision = true;
									}
								}
								if (Board[i][j].Position.x - BOARDSIZE < Player.Position.x && Board[i][j].Position.x + BOARDSIZE > Player.Position.x &&
									Board[i][j].Position.y - BOARDSIZE < Player.Position.y && Board[i][j].Position.y + BOARDSIZE > Player.Position.y) {
									if (Board[i][j].BulletStock > 0 && Player.BulletStock < 12) {
										Player.BulletStock++;
										Player.SpecialBullet++;
										Board[i][j].BulletStock--;
									}
									else if (Board[i][j].BulletStock > 0) {
										Board[i][j].BulletStock--;
									}
								}
							}

						}
						if (Collision == false || Immortal == true) {
							Player.Position.y += 5;
						}
						else {
							Collision = false;
						}
					}
				}
				if (GetAsyncKeyState('W')) {
					if (Player.Position.y > PLAYERSIZE) {
						for (size_t i = 0; i < 15; i++)
						{
							for (size_t j = 0; j < 15; j++)
							{
								if (Board[i][j].Color > 0) {
									if (Board[i][j].Position.x - BOARDSIZE < Player.Position.x + PLAYERSIZE && Board[i][j].Position.x + BOARDSIZE > Player.Position.x - PLAYERSIZE &&
										Board[i][j].Position.y - BOARDSIZE < Player.Position.y + PLAYERSIZE && Board[i][j].Position.y + BOARDSIZE > Player.Position.y - (PLAYERSIZE + 5)) {
										Collision = true;
									}
								}
								if (Board[i][j].Position.x - BOARDSIZE < Player.Position.x && Board[i][j].Position.x + BOARDSIZE > Player.Position.x &&
									Board[i][j].Position.y - BOARDSIZE < Player.Position.y && Board[i][j].Position.y + BOARDSIZE > Player.Position.y) {
									if (Board[i][j].BulletStock > 0 && Player.BulletStock < 12) {
										Player.BulletStock++;
										Player.SpecialBullet++;
										Board[i][j].BulletStock--;
									}
									else if (Board[i][j].BulletStock > 0) {
										Board[i][j].BulletStock--;
									}
								}
							}
						}
						if (Collision == false || Immortal == true) {
							Player.Position.y -= 5;
						}
						else {
							Collision = false;
						}
					}
				}
			}
			// 플레이어 이동 -

			// 적 이동, 블럭 밟을 시 검은블럭으로 변환



			for (size_t i = 0; i < 30; i++)
			{
				if (Enemy[i].DeathFlag == false) {
					Enemy[i].SpawnTimer++;
				}
				if (Enemy[i].SpawnTimer > 80) {
					if (Player.Life > 0 && Enemy[i].DeathFlag == false) {
						if (Enemy[i].Position.x > Player.Position.x) {
							Enemy[i].Position.x -= Enemy[i].Speed;
						}
						else if (Enemy[i].Position.x < Player.Position.x) {
							Enemy[i].Position.x += Enemy[i].Speed;
						}
						if (Enemy[i].Position.y > Player.Position.y) {
							Enemy[i].Position.y -= Enemy[i].Speed;
						}
						else if (Enemy[i].Position.y < Player.Position.y) {
							Enemy[i].Position.y += Enemy[i].Speed;
						}
					}
				}
				if (Enemy[i].DeathFlag == true) {
					if (Enemy[i].DeathTimer >= 79) {
						Enemy[i].Position.x = -500;
						Enemy[i].Position.y = -500;
					}
					else {
						Enemy[i].DeathTimer++;
					}
					if (Enemy[i].DeathTimer < 79) {
						for (size_t k = 0; k < 30; k++)
						{
							if (Enemy[k].Explosion == true) {
								if (Enemy[k].Position.x > Enemy[i].Position.x - 50 && Enemy[k].Position.x < Enemy[i].Position.x + 50 && Enemy[k].Position.y > Enemy[i].Position.y - 50 && Enemy[k].Position.y < Enemy[i].Position.y + 50) {
									Enemy[k].Explosion = false;
									Enemy[k].DeathFlag = true;
									Enemy[k].DeathTimer = 0;
									if (EnemyCreateSpeed > 30) {
										EnemyCreateSpeed -= 5;
									}
									EnemyCount--;
									Combo++;
									ComboTimer = 0;
									Score += 100 + (Combo * 50);
								}
							}
						}
					}
				
				}
				
			}

			for (size_t i = 0; i < 15; i++)
			{
				for (size_t j = 0; j < 15; j++)
				{
					for (size_t k = 0; k < 30; k++)
					{
						if (Enemy[k].SpawnTimer > 80) {
							if (Enemy[k].DeathFlag == false) {
								if (Board[i][j].Position.x - BOARDSIZE < Enemy[k].Position.x && Board[i][j].Position.x + BOARDSIZE > Enemy[k].Position.x &&
									Board[i][j].Position.y - BOARDSIZE < Enemy[k].Position.y && Board[i][j].Position.y + BOARDSIZE > Enemy[k].Position.y) {
									if (Board[i][j].Color == 0) {
										Board[i][j].Color = 1;
									}
									Board[i][j].Hatch = true;
									break;
								}
								else {
									Board[i][j].Hatch = false;
								}

							}
						}
					}
				}
			}

			// 적 이동, 블럭 밟을 시 검은블럭으로 변환, 해당 타일 빗금칠

			if (Quakeflag == true) {
				if (Quaketimer < 10) {
					Quaketimer++;
				}
				else {
					Quaketimer = 0;
					Quakeflag = false;
				}
			}
			// 사망 및 부활
			if (Player.DeathFlag == false) {
				for (size_t i = 0; i < 15; i++)
				{
					for (size_t j = 0; j < 15; j++)
					{
						if (Board[i][j].Color > 0) {
							if (Board[i][j].Position.x - BOARDSIZE < Player.Position.x + PLAYERSIZE && Board[i][j].Position.x + BOARDSIZE > Player.Position.x - PLAYERSIZE &&
								Board[i][j].Position.y - BOARDSIZE < Player.Position.y + PLAYERSIZE && Board[i][j].Position.y + BOARDSIZE > Player.Position.y - PLAYERSIZE) {
								if (Immortal == false) {
									Player.DeathFlag = true;
									Quakeflag = true;
								}
							}
						}
					}
				}
			}
			else if (Player.DeathFlag == true) {
				if (Player.DeathTimer < 150) {
					Player.DeathTimer++;
				}
				else {
					Player.Life--;
					Player.DeathFlag = false;
					Player.DeathTimer = 0;
					if (Player.Life > 1) {
						Player.SpawnRectSize = 80;
					}
				}
				if (Player.DeathTimer > 80) {
					Player.Position.x = 750 / 2;
					Player.Position.y = 750 / 2;
					for (size_t i = 0; i < 30; i++)
					{
						if (Enemy[i].Position.x > 255 && Enemy[i].Position.x < 495 && Enemy[i].Position.y > 255 && Enemy[i].Position.y < 495 && Enemy[i].Explosion == true) {
							Enemy[i].Explosion = false;
							Enemy[i].DeathTimer = 0;
							Enemy[i].DeathFlag = true;
							if (EnemyCreateSpeed > 30) {
								EnemyCreateSpeed -= 5;
							}
							EnemyCount--;
						}
					}
					for (size_t i = 4; i < 11; i++)
					{
						for (size_t j = 4; j < 11; j++)
						{
							if(Board[i][j].Color == 1)
							Board[i][j].Color = 0;
						}
					}
				}
			}
			// 사망 및 부활


			// 총알 이동
			for (size_t i = 0; i < 20; i++)
			{
				if (Bullet[0][i].Position.x < ClientRect.right + 150 && Bullet[0][i].Position.x > -150 && Bullet[0][i].Position.y < ClientRect.bottom + 150 && Bullet[0][i].Position.y > -150) {
					for (size_t j = 9; j > 0; j--)
					{
						Bullet[j][i] = Bullet[j - 1][i];
					} // 그라데이션
					if (Bullet[0][i].Direction == 1) {
						Bullet[0][i].Position.y -= 7;
					}
					else if (Bullet[0][i].Direction == 2) {
						Bullet[0][i].Position.y += 7;
					}
					else if (Bullet[0][i].Direction == 3) {
						Bullet[0][i].Position.x -= 7;
					}
					else if (Bullet[0][i].Direction == 4) {
						Bullet[0][i].Position.x += 7;
					}
				}
			}
			// 총알 이동

			
			// 총알 충돌판정
			for (size_t i = 0; i < 20; i++)
			{
				for (size_t j = 0; j < 15; j++)
				{
					for (size_t k = 0; k < 15; k++)
					{
						if (Board[j][k].Position.x - BOARDSIZE < Bullet[0][i].Position.x + 4 && Board[j][k].Position.x + BOARDSIZE > Bullet[0][i].Position.x - 4 &&
							Board[j][k].Position.y - BOARDSIZE < Bullet[0][i].Position.y + 4 && Board[j][k].Position.y + BOARDSIZE > Bullet[0][i].Position.y - 4) {
							if (Board[j][k].Color == 1) {
								Board[j][k].Color = 0;
							}
							if (Board[j][k].Color == 2) {
								Board[j][k].Color = 0;
								Bullet[0][i].Position.x = -100; Bullet[0][i].Position.y = -100;
							}
							if (Board[j][k].Color == 3) {
								Bullet[0][i].Position.x = -100; Bullet[0][i].Position.y = -100;
							}
						}
					}
				}
			} // 블럭

			for (size_t k = 0; k < 30; k++)
			{
				if (Enemy[k].DeathFlag == false && Enemy[k].Explosion == true) {
					for (size_t i = 0; i < 20; i++)
					{
						if (Enemy[k].Position.x - PLAYERSIZE < Bullet[0][i].Position.x + 4 && Enemy[k].Position.x + PLAYERSIZE > Bullet[0][i].Position.x - 4 &&
							Enemy[k].Position.y - PLAYERSIZE < Bullet[0][i].Position.y + 4 && Enemy[k].Position.y + PLAYERSIZE > Bullet[0][i].Position.y - 4) {


							for (size_t i = 0; i < 15; i++)
							{
								for (size_t j = 0; j < 15; j++)
								{
									if (Board[i][j].Position.x - BOARDSIZE < Enemy[k].Position.x && Board[i][j].Position.x + BOARDSIZE > Enemy[k].Position.x &&
										Board[i][j].Position.y - BOARDSIZE < Enemy[k].Position.y && Board[i][j].Position.y + BOARDSIZE > Enemy[k].Position.y) {
										if (Board[i][j].BulletStock < 3) {
											Board[i][j].BulletStock++;
										}
									}
								}
							}
							Enemy[k].DeathTimer = 0;
							Enemy[k].DeathFlag = true;
							Enemy[k].Explosion = false;
							EnemyCount--;
							Combo++;
							ComboTimer = 0;
							Score += 100 + (Combo * 50);
							if (Enemy[k].Speed > 1) {
								Player.BulletStock = 12;
								Player.SpecialBullet = 12;
							}
							if (EnemyCreateSpeed > 50) {
								EnemyCreateSpeed -= 5;
							}
							Bullet[0][i].Position.x = -100; Bullet[0][i].Position.y = -100;
						}
					}
				}// 적
				
			}
			// 총알 충돌판정

			angle += 0.07;
			for (size_t i = 0; i < 6; i++)
			{
				Player.BulletShapexPos[i] = 6 * (cos(1.08*i + angle) - sin(1.08*i + angle));
				Player.BulletShapeyPos[i] = 6 * (sin(1.08*i + angle) + cos(1.08*i + angle));
			}
			for (size_t i = 0; i < 15; i++)
			{
				for (size_t j = 0; j < 15; j++)
				{
					for (size_t k = 0; k < 3; k++)
					{
						Board[i][j].BulletShapexPos[k] = 6 * (cos(2.16*k + angle) - sin(2.16*k + angle));
						Board[i][j].BulletShapeyPos[k] = 6 * (sin(2.16*k + angle) + cos(2.16*k + angle));
					}
				}
			}
			// 플레이어 위 총알 회전

			for (size_t i = 0; i < 10; i++)
			{
				for (size_t j = 0; j < 20; j++)
				{
					if (Bullet[i][j].isSpecialBullet == true &&
						(Bullet[i][j].Position.x < 0 || Bullet[i][j].Position.y < 0 || Bullet[i][j].Position.x > ClientRect.right || Bullet[i][j].Position.y > ClientRect.bottom)) {
						Bullet[i][j].isSpecialBullet = false;
					}
				}
			}


			if (Player.BulletStock < 6) {
				BulletChargeTimer++;
			}
			if (BulletChargeTimer > 50) {
				Player.BulletStock++;
				BulletChargeTimer = 0;
			}

		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_UP:
			if (Player.DeathFlag == false) {
				if (Player.BulletStock > 0) {
					if (Bulletcount < 19) {
						if (Player.SpecialBullet > 0) {
							Bullet[0][Bulletcount].Direction = 1;
							Bullet[0][Bulletcount].Position.x = Player.Position.x - 15;
							Bullet[0][Bulletcount].Position.y = Player.Position.y;
							Bullet[0][Bulletcount].isSpecialBullet = true;
							Bulletcount++;
							Bullet[0][Bulletcount].Direction = 1;
							Bullet[0][Bulletcount].Position.x = Player.Position.x + 15;
							Bullet[0][Bulletcount].Position.y = Player.Position.y;
							Bullet[0][Bulletcount].isSpecialBullet = true;
							Bulletcount++;
							if (Immortal == false) {
								Player.BulletStock--;
								Player.SpecialBullet--;
							}
						}
						else {
							Bullet[0][Bulletcount].Direction = 1;
							Bullet[0][Bulletcount].Position.x = Player.Position.x;
							Bullet[0][Bulletcount].Position.y = Player.Position.y;
							Bulletcount++;
							if (Immortal == false) {
								Player.BulletStock--;
							}
						}
					}
					else {
						Bulletcount = 0;
					}
				}
			}
			break;
		case VK_DOWN:
			if (Player.DeathFlag == false) {
				if (Player.BulletStock > 0) {
					if (Bulletcount < 19) {
						if (Player.SpecialBullet > 0) {
							Bullet[0][Bulletcount].Direction = 2;
							Bullet[0][Bulletcount].Position.x = Player.Position.x - 15;
							Bullet[0][Bulletcount].Position.y = Player.Position.y;
							Bullet[0][Bulletcount].isSpecialBullet = true;
							Bulletcount++;
							Bullet[0][Bulletcount].Direction = 2;
							Bullet[0][Bulletcount].Position.x = Player.Position.x + 15;
							Bullet[0][Bulletcount].Position.y = Player.Position.y;
							Bullet[0][Bulletcount].isSpecialBullet = true;
							Bulletcount++;
							if (Immortal == false) {
								Player.BulletStock--;
								Player.SpecialBullet--;
							}
						}
						else {
							Bullet[0][Bulletcount].Direction = 2;
							Bullet[0][Bulletcount].Position.x = Player.Position.x;
							Bullet[0][Bulletcount].Position.y = Player.Position.y;
							if (Immortal == false) {
								Bulletcount++;
								Player.BulletStock--;
							}
						}
					}
					else {
						Bulletcount = 0;
					}
				}
			}
			break;
		case VK_LEFT:
			if (Player.DeathFlag == false) {
				if (Player.BulletStock > 0) {
					if (Bulletcount < 19) {
						if (Player.SpecialBullet > 0) {
							Bullet[0][Bulletcount].Direction = 3;
							Bullet[0][Bulletcount].Position.x = Player.Position.x;
							Bullet[0][Bulletcount].Position.y = Player.Position.y - 15;
							Bullet[0][Bulletcount].isSpecialBullet = true;
							Bulletcount++;
							Bullet[0][Bulletcount].Direction = 3;
							Bullet[0][Bulletcount].Position.x = Player.Position.x;
							Bullet[0][Bulletcount].Position.y = Player.Position.y + 15;
							Bullet[0][Bulletcount].isSpecialBullet = true;
							Bulletcount++;
							if (Immortal == false) {
								Player.BulletStock--;
								Player.SpecialBullet--;
							}
						}
						else {
							Bullet[0][Bulletcount].Direction = 3;
							Bullet[0][Bulletcount].Position.x = Player.Position.x;
							Bullet[0][Bulletcount].Position.y = Player.Position.y;
							Bulletcount++;
							if (Immortal == false) {
								Player.BulletStock--;
							}
						}
					}
					else {
						Bulletcount = 0;
					}
				}
			}
			break;
		case VK_RIGHT:
			if (Player.DeathFlag == false) {
				if (Player.BulletStock > 0) {
					if (Bulletcount < 19) {
						if (Player.SpecialBullet > 0) {
							Bullet[0][Bulletcount].Direction = 4;
							Bullet[0][Bulletcount].Position.x = Player.Position.x;
							Bullet[0][Bulletcount].Position.y = Player.Position.y - 15;
							Bullet[0][Bulletcount].isSpecialBullet = true;
							Bulletcount++;
							Bullet[0][Bulletcount].Direction = 4;
							Bullet[0][Bulletcount].Position.x = Player.Position.x;
							Bullet[0][Bulletcount].Position.y = Player.Position.y + 15;
							Bullet[0][Bulletcount].isSpecialBullet = true;
							Bulletcount++;
							if (Immortal == false) {
								Player.BulletStock--;
								Player.SpecialBullet--;
							}
						}
						else {
							Bullet[0][Bulletcount].Direction = 4;
							Bullet[0][Bulletcount].Position.x = Player.Position.x;
							Bullet[0][Bulletcount].Position.y = Player.Position.y;
							if (Immortal == false) {
								Bulletcount++;
								Player.BulletStock--;
							}
						}
					}
					else {
						Bulletcount = 0;
					}
				}
			}
			break;
		case VK_SPACE:
			if (Immortal == true) {
				Immortal = false;
			}
			else if (Immortal == false) {
				Immortal = true;
			}
			break;
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		default:
			break;
		}
		break;
	case WM_CHAR:
		pause = false;
		switch (wParam)
		{
		case '1':
			Player.Life = 3;
			Player.SpawnRectSize = 80;
			EnemyCount = 0;
			EnemyCreateCount = 0;
			EnemyNum = 0;
			EnemyCreateSpeed = 150;
			Combo = 0;
			Score = 0;
			Player.BulletStock = 6;
			Player.SpecialBullet = 0;
			Player.Position = { 375,375 };
			for (size_t i = 0; i < 30; i++)
			{
				Enemy[i].DeathTimer = 80;
				Enemy[i].DeathFlag = true;
				Enemy[i].Position = { -500,-500 };

				if (i < 15) {
					for (size_t j = 0; j < 15; j++)
					{
						Board[i][j].Hatch = false;
						Board[i][j].Color = 0;
						Board[i][j].BulletStock = 0;
					}
				}
			}
			for (size_t i = 0; i < 15; i++)
			{
				for (size_t j = 0; j < 15; j++)
				{
					if ((i > 1 && i < 13) && ((j>1&& j<4 ) || (j > 10 && j < 13))){
						Board[i][j].Color = 3;
					}
				}
			}
			pause = true;
			break;
		case '2':
			Player.Life = 3;
			Player.SpawnRectSize = 80;
			EnemyCount = 0;
			EnemyCreateCount = 0;
			EnemyNum = 0;
			EnemyCreateSpeed = 150;
			Combo = 0;
			Score = 0;
			Player.BulletStock = 6;
			Player.SpecialBullet = 0;
			Player.Position = { 375,375 };
			for (size_t i = 0; i < 30; i++)
			{
				Enemy[i].DeathTimer = 80;
				Enemy[i].DeathFlag = true;
				Enemy[i].Position = { -500,-500 };
			}
			for (size_t i = 0; i < 15; i++)
			{
				for (size_t j = 0; j < 15; j++)
				{
					Board[i][j].Position.x = (i * BOARDSIZE * 2) + BOARDSIZE;
					Board[i][j].Position.y = (j * BOARDSIZE * 2) + BOARDSIZE;
					if (i > 2 && i < 12 && j >2 && j < 12) {
						Board[i][j].Color = 0;
					}
					else {
						Board[i][j].Color = 4;
					}
				}
			}
			pause = true;
			break; 
		case '3':
			Player.Life = 3;
			Player.SpawnRectSize = 80;
			Player.SpawnRectSize = 80;
			EnemyCount = 0;
			EnemyCreateCount = 0;
			EnemyNum = 0;
			EnemyCreateSpeed = 150;
			Combo = 0;
			Score = 0;
			Player.BulletStock = 6;
			Player.SpecialBullet = 0;
			Player.Position = { 375,375 };
			for (size_t i = 0; i < 30; i++)
			{
				Enemy[i].DeathTimer = 80;
				Enemy[i].DeathFlag = true;
				Enemy[i].Position = { -500,-500 };
				if (i < 15) {
					for (size_t j = 0; j < 15; j++)
					{
						Board[i][j].Hatch = false;
						Board[i][j].Color = 0;
						Board[i][j].BulletStock = 0;
					}
				}
			}
			for (size_t i = 0; i < 15; i++)
			{
				for (size_t j = 0; j < 15; j++)
				{
					if (i * j % 2 == 0) {
						Board[i][j].Color = 2;
					}
				}
			}
			pause = true;
			break;
		case 'P': case 'p':
			if (pause == false) {
				pause = true;
			}
			else if (pause == true) {
				pause = false;
			}
			break;
		default:
			break;
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		bufferdc = CreateCompatibleDC(hdc);
		Jellydc = CreateCompatibleDC(bufferdc);
		if (hBitmap == NULL) {
			hBitmap = CreateCompatibleBitmap(hdc, ClientRect.right, ClientRect.bottom);
		}
		oldBitmap = (HBITMAP)SelectObject(bufferdc, hBitmap);
		SetBkMode(bufferdc, TRANSPARENT);

		SelectObject(Jellydc, Jelly[JellyTimer]);
		for (size_t i = 0; i < 15; i++)
		{
			for (size_t j = 0; j < 15; j++)
			{
				if (Board[i][j].Color == 1) {
					hBrush = CreateSolidBrush(RGB(100, 100, 100));
					oldBrush = (HBRUSH)SelectObject(bufferdc, hBrush);
				} // 블럭 색 1 얕은 회색 파괴가능, 관통가능
				else if (Board[i][j].Color == 2) {
					hBrush = CreateSolidBrush(RGB(0, 0, 0));
					oldBrush = (HBRUSH)SelectObject(bufferdc, hBrush);
				} // 블럭 색 2 검은색 파괴가능, 관통불가
				else if (Board[i][j].Color == 3) {
					hBrush = CreateSolidBrush(RGB(255, 0, 0));
					oldBrush = (HBRUSH)SelectObject(bufferdc, hBrush);
				} // 블럭 색 3 장애물 빨간색, 파괴불가, 관통불가
				else if (Board[i][j].Color == 4) {
					hBrush = CreateSolidBrush(RGB(50, 50, 50));
					oldBrush = (HBRUSH)SelectObject(bufferdc, hBrush);
				} // 블럭 색 4 벽 회색
				else {
					hBrush = CreateSolidBrush(RGB(255, 255, 255));
					oldBrush = (HBRUSH)SelectObject(bufferdc, hBrush);
				}// 블럭 색 0 흰색
				Rectangle(bufferdc, Board[i][j].Position.x - BOARDSIZE, Board[i][j].Position.y - BOARDSIZE, Board[i][j].Position.x + BOARDSIZE, Board[i][j].Position.y + BOARDSIZE);
				// 블럭 출력
				SelectObject(bufferdc, oldBrush);
				DeleteObject(hBrush);
			}
		}



		for (size_t i = 0; i < 30; i++)
		{
			if (Enemy[i].DeathFlag == false) {
				hPen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
				oldPen = (HPEN)SelectObject(bufferdc, hPen);
				hBrush = CreateSolidBrush(RGB(255, 0, 0));
				oldBrush = (HBRUSH)SelectObject(bufferdc, hBrush);
				HBRUSH debugFirstBrush = oldBrush;
				if (Enemy[i].SpawnTimer > 70) {
					if (Enemy[i].Speed == 1) {
						RoundRect(bufferdc, Enemy[i].Position.x - PLAYERSIZE, Enemy[i].Position.y - PLAYERSIZE, Enemy[i].Position.x + PLAYERSIZE, Enemy[i].Position.y + PLAYERSIZE, 10, 10);
					}
					else {
						hBrush2 = CreateSolidBrush(RGB(255, 0, 255));
						oldBrush2 = (HBRUSH)SelectObject(bufferdc, hBrush2);
						RoundRect(bufferdc, Enemy[i].Position.x - PLAYERSIZE, Enemy[i].Position.y - PLAYERSIZE, Enemy[i].Position.x + PLAYERSIZE, Enemy[i].Position.y + PLAYERSIZE, 10, 10);
						SelectObject(bufferdc, oldBrush2);
						DeleteObject(hBrush2);
					}
				}
				else {
					if (Enemy[i].SpawnRectSize > 0) {
						Enemy[i].SpawnRectSize--;
					}
					Enemy[i].SpawnRect.left = Enemy[i].Position.x - Enemy[i].SpawnRectSize;
					Enemy[i].SpawnRect.right = Enemy[i].Position.x + Enemy[i].SpawnRectSize;
					Enemy[i].SpawnRect.top = Enemy[i].Position.y - Enemy[i].SpawnRectSize;
					Enemy[i].SpawnRect.bottom = Enemy[i].Position.y + Enemy[i].SpawnRectSize;
					FrameRect(bufferdc, &Enemy[i].SpawnRect, hBrush);
				}
				SelectObject(bufferdc, oldBrush);
				DeleteObject(hBrush);
				SelectObject(bufferdc, oldPen);
				DeleteObject(hPen);
			}
			else if (Enemy[i].DeathFlag == true && Enemy[i].DeathTimer < 80) {
				hPen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
				oldPen = (HPEN)SelectObject(bufferdc, hPen);
				hBrush = CreateSolidBrush(RGB(255, 0, 0));
				oldBrush = (HBRUSH)SelectObject(bufferdc, hBrush);
				Ellipse(bufferdc, Enemy[i].Position.x - (PLAYERSIZE - Enemy[i].DeathTimer) + 10, Enemy[i].Position.y - (PLAYERSIZE - Enemy[i].DeathTimer) - 10, Enemy[i].Position.x + (PLAYERSIZE - Enemy[i].DeathTimer) + 10, Enemy[i].Position.y + (PLAYERSIZE - Enemy[i].DeathTimer) - 10);
				Ellipse(bufferdc, Enemy[i].Position.x - (PLAYERSIZE - Enemy[i].DeathTimer) + 30, Enemy[i].Position.y - (PLAYERSIZE - Enemy[i].DeathTimer) + 30, Enemy[i].Position.x + (PLAYERSIZE - Enemy[i].DeathTimer) + 30, Enemy[i].Position.y + (PLAYERSIZE - Enemy[i].DeathTimer) + 30);
				if (Enemy[i].DeathTimer > 20) {
					Ellipse(bufferdc, Enemy[i].Position.x - (PLAYERSIZE - Enemy[i].DeathTimer + 10) - 20, Enemy[i].Position.y - (PLAYERSIZE - Enemy[i].DeathTimer + 10) + 20, Enemy[i].Position.x + (PLAYERSIZE - Enemy[i].DeathTimer + 10) - 20, Enemy[i].Position.y + (PLAYERSIZE - Enemy[i].DeathTimer + 10) + 20);
					Ellipse(bufferdc, Enemy[i].Position.x - (PLAYERSIZE - Enemy[i].DeathTimer + 10), Enemy[i].Position.y - (PLAYERSIZE - Enemy[i].DeathTimer + 10) + 30, Enemy[i].Position.x + (PLAYERSIZE - Enemy[i].DeathTimer + 10), Enemy[i].Position.y + (PLAYERSIZE - Enemy[i].DeathTimer + 10) + 30);
				}
				else if (Enemy[i].DeathTimer > 40) {
					Ellipse(bufferdc, Enemy[i].Position.x - (PLAYERSIZE - Enemy[i].DeathTimer + 20) - 10, Enemy[i].Position.y - (PLAYERSIZE - Enemy[i].DeathTimer + 20) - 10, Enemy[i].Position.x + (PLAYERSIZE - Enemy[i].DeathTimer + 20) - 10, Enemy[i].Position.y + (PLAYERSIZE - Enemy[i].DeathTimer + 20) - 10);
					Ellipse(bufferdc, Enemy[i].Position.x - (PLAYERSIZE - Enemy[i].DeathTimer + 40) + 30, Enemy[i].Position.y - (PLAYERSIZE - Enemy[i].DeathTimer + 40) + 30, Enemy[i].Position.x + (PLAYERSIZE - Enemy[i].DeathTimer + 40) + 30, Enemy[i].Position.y + (PLAYERSIZE - Enemy[i].DeathTimer + 40) + 30);
				}
				SelectObject(bufferdc, oldBrush);
				DeleteObject(hBrush);
				SelectObject(bufferdc, oldPen);
				DeleteObject(hPen);
			}
		}
		// 적 출력

		for (size_t i = 0; i < 15; i++)
		{
			for (size_t j = 0; j < 15; j++)
			{
				if (Board[i][j].Hatch == true) {
					hBrush = CreateHatchBrush(HS_FDIAGONAL, RGB(255, 0, 0));
					oldBrush = (HBRUSH)SelectObject(bufferdc, hBrush);
					Rectangle(bufferdc, Board[i][j].Position.x - BOARDSIZE*3, Board[i][j].Position.y - BOARDSIZE * 3, Board[i][j].Position.x + BOARDSIZE * 3, Board[i][j].Position.y + BOARDSIZE * 3);
					SelectObject(bufferdc, oldBrush);
					DeleteObject(hBrush);
				}
			}
		}// 적 주변 빗금 출력

		if (Player.DeathFlag == false && Player.Life > 0) {
			if (Immortal == false) {
				hPen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
				oldPen = (HPEN)SelectObject(bufferdc, hPen);
				hBrush = CreateSolidBrush(RGB(0, 200, 230));
				oldBrush = (HBRUSH)SelectObject(bufferdc, hBrush);
				RoundRect(bufferdc, Player.Position.x - PLAYERSIZE, Player.Position.y - PLAYERSIZE, Player.Position.x + PLAYERSIZE, Player.Position.y + PLAYERSIZE, 10, 10);
				SelectObject(bufferdc, oldBrush);
				DeleteObject(hBrush);
				SelectObject(bufferdc, oldPen);
				DeleteObject(hPen);
			}
			else {
				TransparentBlt(bufferdc, Player.Position.x- PLAYERSIZE, Player.Position.y - PLAYERSIZE, Width[JellyTimer], Height[JellyTimer], Jellydc, 0, 0, Width[JellyTimer], Height[JellyTimer], RGB(160, 168, 168));
			}
			wsprintf(Combostr, L"X%d", Combo);
			TextOut(bufferdc, Player.Position.x-5, Player.Position.y - 25, Combostr, lstrlen(Combostr));
			wsprintf(Scorestr, L"SCORE : %d", Score);
			TextOut(bufferdc, Player.Position.x - 30, Player.Position.y + 15, Scorestr, lstrlen(Scorestr));
		}
		// 플레이어 출력
		
		if (Player.DeathFlag == false && Immortal == false) {
			for (size_t i = 0; i < Player.BulletStock; i++)
			{
				if (Player.SpecialBullet > i) {
					hBrush = CreateSolidBrush(RGB(0, 0, 0));
					oldBrush = (HBRUSH)SelectObject(bufferdc, hBrush);
				}
				else {
					hBrush = CreateSolidBrush(RGB(0, 230, 230));
					oldBrush = (HBRUSH)SelectObject(bufferdc, hBrush);
				}
				Ellipse(bufferdc, Player.BulletShapexPos[i] + Player.Position.x - 2, Player.BulletShapeyPos[i] + Player.Position.y - 2, Player.BulletShapexPos[i] + Player.Position.x + 2, Player.BulletShapeyPos[i] + Player.Position.y + 2);
				SelectObject(bufferdc, oldBrush);
				DeleteObject(hBrush);
			} // 플레이어 위 총알
		}
		for (size_t i = 0; i < 15; i++)
		{
			for (size_t j = 0; j < 15; j++)
			{
				for (size_t k = 0;  k < Board[i][j].BulletStock; k++)
				{
					hBrush = CreateSolidBrush(RGB(0, 230, 230));
					oldBrush = (HBRUSH)SelectObject(bufferdc, hBrush);
					Ellipse(bufferdc, Board[i][j].BulletShapexPos[k] + Board[i][j].Position.x - 4, Board[i][j].BulletShapeyPos[k] + Board[i][j].Position.y - 4,Board[i][j].BulletShapexPos[k] + Board[i][j].Position.x + 4, Board[i][j].BulletShapeyPos[k] + Board[i][j].Position.y + 4);
					SelectObject(bufferdc, oldBrush);
					DeleteObject(hBrush);
				}
			}
		} // 보드 위 특수총알
		// 돌아가는 총알 출력

		
		if (Player.Life > 0) {
			if (Player.DeathFlag == true) {
				hPen = CreatePen(PS_NULL, 0, RGB(0, 0, 0));
				oldPen = (HPEN)SelectObject(bufferdc, hPen);
				hBrush = CreateSolidBrush(RGB(0, 200, 230));
				oldBrush = (HBRUSH)SelectObject(bufferdc, hBrush);
				if (Player.DeathTimer < 80) {
					Ellipse(bufferdc, Player.Position.x - (PLAYERSIZE - Player.DeathTimer) + 10, Player.Position.y - (PLAYERSIZE - Player.DeathTimer) - 10, Player.Position.x + (PLAYERSIZE - Player.DeathTimer) + 10, Player.Position.y + (PLAYERSIZE - Player.DeathTimer) - 10);
					Ellipse(bufferdc, Player.Position.x - (PLAYERSIZE - Player.DeathTimer) + 30, Player.Position.y - (PLAYERSIZE - Player.DeathTimer) + 30, Player.Position.x + (PLAYERSIZE - Player.DeathTimer) + 30, Player.Position.y + (PLAYERSIZE - Player.DeathTimer) + 30);
					if (Player.DeathTimer > 20) {
						Ellipse(bufferdc, Player.Position.x - (PLAYERSIZE - Player.DeathTimer + 10) - 20, Player.Position.y - (PLAYERSIZE - Player.DeathTimer + 10) + 20, Player.Position.x + (PLAYERSIZE - Player.DeathTimer + 10) - 20, Player.Position.y + (PLAYERSIZE - Player.DeathTimer + 10) +20);
						Ellipse(bufferdc, Player.Position.x - (PLAYERSIZE - Player.DeathTimer + 10), Player.Position.y - (PLAYERSIZE - Player.DeathTimer + 10)+30, Player.Position.x + (PLAYERSIZE - Player.DeathTimer + 10), Player.Position.y + (PLAYERSIZE - Player.DeathTimer + 10)+30);
					}
					else if (Player.DeathTimer > 40) {
						Ellipse(bufferdc, Player.Position.x - (PLAYERSIZE - Player.DeathTimer + 20) - 10, Player.Position.y - (PLAYERSIZE - Player.DeathTimer + 20) - 10, Player.Position.x + (PLAYERSIZE - Player.DeathTimer + 20) - 10, Player.Position.y + (PLAYERSIZE - Player.DeathTimer + 20) - 10);
						Ellipse(bufferdc, Player.Position.x - (PLAYERSIZE - Player.DeathTimer + 40) + 30, Player.Position.y - (PLAYERSIZE - Player.DeathTimer + 40) + 30, Player.Position.x + (PLAYERSIZE - Player.DeathTimer + 40) + 30, Player.Position.y + (PLAYERSIZE - Player.DeathTimer + 40) + 30);
					}
				}

				if (Player.DeathTimer > 80) {
					if (Player.SpawnRectSize > 0) {
						Player.SpawnRectSize--;
					}
					Player.SpawnRect.left = Player.Position.x - Player.SpawnRectSize;
					Player.SpawnRect.right = Player.Position.x + Player.SpawnRectSize;
					Player.SpawnRect.top = Player.Position.y - Player.SpawnRectSize;
					Player.SpawnRect.bottom = Player.Position.y + Player.SpawnRectSize;
					FrameRect(bufferdc, &Player.SpawnRect, hBrush);
				}
				SelectObject(bufferdc, oldBrush);
				DeleteObject(hBrush);
				SelectObject(bufferdc, oldPen);
				DeleteObject(hPen);
			}
		}
		// 폭발이펙트, 부활 이펙트, 범위 블럭 및 적 폭파

		for (size_t i = 0; i < 9; i++)
		{
			for (size_t j = 0; j < 20; j++)
			{
				if (Bullet[i][j].isSpecialBullet == false) {
					hPen = CreatePen(NULL, 0, RGB(25 * i, 25 * i, 25 * i));
					oldPen = (HPEN)SelectObject(bufferdc, hPen);
					hBrush = CreateSolidBrush(RGB(25 * i, 25 * i, 25 * i));
					oldBrush = (HBRUSH)SelectObject(bufferdc, hBrush);
				}
				else {
					hPen = CreatePen(NULL, 0, RGB(0 ,23 * i, 23 * i));
					oldPen = (HPEN)SelectObject(bufferdc, hPen);
					hBrush = CreateSolidBrush(RGB(0, 23 * i, 23 * i));
					oldBrush = (HBRUSH)SelectObject(bufferdc, hBrush);
				}
				Rectangle(bufferdc, Bullet[i][j].Position.x - 4, Bullet[i][j].Position.y - 4, Bullet[i][j].Position.x + 4, Bullet[i][j].Position.y + 4);
				SelectObject(bufferdc, oldBrush);
				DeleteObject(hBrush);
				SelectObject(bufferdc, oldPen);
				DeleteObject(hPen);
			}
		} // 총알 출력

		if (Player.Life < 1) {
			SelectObject(Jellydc, Youdai);
			BitBlt(bufferdc, 0, 275, Diewidth, Dieheight, Jellydc, 0, 0, SRCCOPY);
			wsprintf(Scorestr, L"SCORE : %d", Score);
			TextOut(bufferdc, 330, 290, Scorestr, lstrlen(Scorestr));
		}
		
		//  게임오버
		
		if (Quakeflag == true) {
			if (Quaketimer % 2 == 0){
				BitBlt(hdc, Quaketimer, Quaketimer, ClientRect.right, ClientRect.bottom, bufferdc, 0, 0, SRCCOPY);
			}
			else {
				BitBlt(hdc, -Quaketimer, -Quaketimer, ClientRect.right, ClientRect.bottom, bufferdc, 0, 0, SRCCOPY);
			}
		}
		else {
			BitBlt(hdc, 0, 0, ClientRect.right, ClientRect.bottom, bufferdc, 0, 0, SRCCOPY);
		}
		// 더블버퍼 출력 및 지진효과

		SelectObject(bufferdc, oldBitmap); DeleteDC(bufferdc); DeleteDC(Jellydc);
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return(DefWindowProc(hWnd, iMessage, wParam, lParam));
}
