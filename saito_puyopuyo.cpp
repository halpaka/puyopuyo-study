#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<math.h>
#include<conio.h>

#define FIELD_WIDTH 8
#define FIELD_HEIGHT 15

#define SCREEN_WIDTH FIELD_WIDTH
#define SCREEN_HEIGHT FIELD_HEIGHT + 1

#define PUYO_START_X 3
#define PUYO_START_Y 1

#define PUYO_COLOR_MAX 4
#define GAMEOVER 7

enum {
	CELL_NONE,
	CELL_WALL,
	CELL_OVER,
	CELL_PUYO_0,
	CELL_PUYO_1,
	CELL_PUYO_2,
	CELL_PUYO_3,
	CELL_MAX
};

enum {
	PUYO_ANGLE_0,
	PUYO_ANGLE_90,
	PUYO_ANGLE_180,
	PUYO_ANGLE_270,
	PUYO_ANGLE_MAX
};
int puyoSubPositions[][2] = {
	{0,-1},//PUYO_ANGLE_0
	{-1,0},//PUYO_ANGLE_90
	{0,1},//PUYO_ANGLE_180
	{1,0}//PUYO_ANGLE_270
};
int cells[FIELD_HEIGHT][FIELD_WIDTH];
int displayBuffer[SCREEN_HEIGHT][SCREEN_WIDTH];
int checked[FIELD_HEIGHT][FIELD_WIDTH];

char cellNames[][2 + 1] = {
	"・",//CELL_NONE
	"藤",//CELL_WALL
	"十",//CELL_OVER
	"斎",//CELL_PUYO_0
	"斉",//CELL_PUYO_1
	"齋",//CELL_PUYO_2
	"齊",//CELL_PUYO_3
	"Ｇ",
	"Ａ",
	"Ｍ",
	"Ｅ",
	"Ｏ",
	"Ｖ",
	"Ｅ",
	"Ｒ"
};

char Num[][1 + 1] = {
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9"
};

int puyoX = PUYO_START_X,
puyoY = PUYO_START_Y;
int puyoColor;
int subColor;
int puyoAngle;

bool lock = false;
bool game_over = false;

void display(int Score) {
	system("cls");
	memcpy(&displayBuffer[1][0], &cells[1][0], sizeof cells);
	if (!lock) {
		int subX = puyoX + puyoSubPositions[puyoAngle][0];
		int subY = puyoY + puyoSubPositions[puyoAngle][1];
		displayBuffer[puyoY][puyoX] = CELL_PUYO_0 + puyoColor;
		displayBuffer[subY][subX] = CELL_PUYO_0 + subColor;
	}

	int cal_score = Score;
	int dig = 10;
	for (int x = SCREEN_WIDTH - 1; x >= 0; x--) {
		displayBuffer[SCREEN_HEIGHT - 1][x] = cal_score % dig;
		cal_score = cal_score / dig;
	}

	if (displayBuffer[1][3] == CELL_NONE) displayBuffer[1][3] = CELL_OVER;

	if (game_over) {
		for (int i = 2; i <= 5; i++) displayBuffer[7][i] = GAMEOVER + i - 2;
		for (int i = 2; i <= 5; i++) displayBuffer[8][i] = GAMEOVER + 4 + i - 2;
	}

	for (int x = 0; x < FIELD_WIDTH; x++)
		printf("  ");
	printf("\n");
	for (int y = 1; y < SCREEN_HEIGHT - 1; y++) {
		for (int x = 0; x < FIELD_WIDTH; x++)
			printf("%s", cellNames[displayBuffer[y][x]]);
		printf("\n");
	}
	for (int x = 0; x < FIELD_WIDTH; x++)
		printf("%s ", Num[displayBuffer[SCREEN_HEIGHT-1][x]]);
}

//障害物の有無を判断する関数
bool intersectPuyoToFeild(int _puyoX, int _puyoY, int _puyoAngle) {
	if ((cells[_puyoY][_puyoX] != CELL_NONE) && (cells[_puyoY][_puyoX] != CELL_OVER))
		return true;
	int subX = _puyoX + puyoSubPositions[_puyoAngle][0];
	int subY = _puyoY + puyoSubPositions[_puyoAngle][1];
	if ((cells[subY][subX] != CELL_NONE) && (cells[_puyoY][_puyoX] != CELL_OVER))
		return true;
	return false;
}

int getPuyoConnectedCount(int _x, int _y, int _cell, int _count) {
	if (checked[_y][_x]
		|| (cells[_y][_x] != _cell)
		)
		return _count;

	_count++;
	checked[_y][_x] = true;

	for (int i = 0; i < PUYO_ANGLE_MAX; i++) {
		int x = _x + puyoSubPositions[i][0];
		int y = _y + puyoSubPositions[i][1];
		_count = getPuyoConnectedCount(x, y, _cell, _count);
	}

	return _count;
}

void erasePuyo(int _x, int _y, int _cell) {
	if (cells[_y][_x] != _cell)
		return;
	cells[_y][_x] = CELL_NONE;
	for (int i = 0; i < PUYO_ANGLE_MAX; i++) {
		int x = _x + puyoSubPositions[i][0];
		int y = _y + puyoSubPositions[i][1];
		erasePuyo(x, y, _cell);
	}
}

int Calculate_Score(int erase, int chain, int *link, int color) {
	int chain_score[19] = { 0,8,16,32,64,96,128,160,192,224,256,288,320,352,384,416,448,480,512 };
	int link_score[8] = { 0,2,3,4,5,6,7,10 };
	int color_score[5] = { 0,3,6,12,24 };
	int score = 0;
	int LinkScore = 0;
	int i = 0;

	while ((link[i] != 0) && (i != 16)) {
		if (link[i] > 11) link[i] = 11;
		LinkScore += link_score[link[i] - 4];
		link[i] = 0;
		i++;
	}

	if (chain_score[chain - 1] + LinkScore + color_score[color - 1] != 0) 
		score = erase * 10 * (chain_score[chain - 1] + LinkScore + color_score[color - 1]);
	else
		score = erase * 10;
	return score;
}

int main() {
	int score = 0;
	int erase = 0;
	int chain = 0;
	int link[16] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	int color = 0;
	int erase_color[PUYO_COLOR_MAX] = {0,0,0,0};

	srand((unsigned int)time(NULL));
	for (int y = 0; y < FIELD_HEIGHT; y++) {
		cells[y][0] = CELL_WALL;
		cells[y][FIELD_WIDTH - 1] = CELL_WALL;
	};
	for (int x = 1; x < FIELD_WIDTH - 1; x++)
		cells[FIELD_HEIGHT - 1][x] = CELL_WALL;

	puyoColor = rand() % PUYO_COLOR_MAX;
	subColor = rand() % PUYO_COLOR_MAX;

	time_t t = 0;
	while (1) {
		if (t < time(NULL)) {
			t = time(NULL);//1秒ごとに表示が更新される
			if (!lock) {
				if (!intersectPuyoToFeild(puyoX, puyoY + 1, puyoAngle)) {
					puyoY++;
				}
				else {
					int subX = puyoX + puyoSubPositions[puyoAngle][0];
					int subY = puyoY + puyoSubPositions[puyoAngle][1];
					cells[puyoY][puyoX] = CELL_PUYO_0 + puyoColor; //完全に固定
					cells[subY][subX] = CELL_PUYO_0 + subColor; 
					puyoX = PUYO_START_X;//リスポーン
					puyoY = PUYO_START_Y;
					puyoAngle = PUYO_ANGLE_0;
					puyoColor = rand() % PUYO_COLOR_MAX;
					subColor = rand() % PUYO_COLOR_MAX;

					lock = true;
				}
			}

			if (lock) {
				lock = false;
				for (int y = FIELD_HEIGHT - 3; y >= 0; y--) {
					for (int x = 1; x < FIELD_WIDTH - 1; x++) {
						if ((cells[y][x] != CELL_NONE) 
							&& (cells[y + 1][x] == CELL_NONE)) {
							int i = 1;
							while (cells[y + i + 1][x] == CELL_NONE) i++;
							cells[y + i][x] = cells[y][x];
							cells[y][x] = CELL_NONE;
							lock = true;
						}
					}
				}//ちぎれるor消えたぷよから落ちる動き

				if (!lock) {
					chain++;
					int i = 0;
					memset(checked, 0, sizeof checked);
					for (int y = 0; y < FIELD_HEIGHT - 1; y++) {
						for (int x = 1; x < FIELD_WIDTH - 1; x++) {
							if (cells[y][x] != CELL_NONE) {
								int connectpuyo = getPuyoConnectedCount(x, y, cells[y][x], 0);
								if (connectpuyo >= 4) {
									if (erase_color[cells[y][x] - CELL_PUYO_0] == 0) color++;
									erase_color[cells[y][x] - CELL_PUYO_0]++;
									erase += connectpuyo;
									link[i] = connectpuyo;
									i++;

									erasePuyo(x, y, cells[y][x]);
									
									lock = true;
								}
							}
						}
					}

					if (!lock && cells[PUYO_START_Y][PUYO_START_X] != CELL_NONE) {
						game_over = true;
						break;
					}

					if (lock) {
						int mini_score = 0;
						mini_score = Calculate_Score(erase, chain, link, color);
						score += mini_score;
					}
					else
						chain = 0;

					erase = 0;
					color = 0;
					for (int i = 0; i < PUYO_COLOR_MAX; i++) erase_color[i] = 0;
				}
			}
			display(score);
		}

		if (_kbhit()) {
			if (lock)
				int blank = _getch();
			else {
				//影武者に障害物があるかどうか判断してもらう
				int x = puyoX;
				int y = puyoY;
				int angle = puyoAngle;
				switch (_getch()) {
					//case'w':y--; break;
				case's':y++; break;
				case'a':x--; break;
				case'd':x++; break;
				case' ':angle = (++angle) % PUYO_ANGLE_MAX; break;
				}

				if (!intersectPuyoToFeild(x, y, angle)) {
					puyoX = x;
					puyoY = y;
					puyoAngle = angle;
				}
				display(score);//入力遅延を防ぐため入力を感知したら更新
			}

		}
	}
	display(score);
	return 0;
}
