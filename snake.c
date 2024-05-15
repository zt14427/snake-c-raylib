#include <stdio.h>
#include <stdlib.h>
#include "raylib.h"
#define REC_NUM_W 19
#define REC_NUM_H 19

typedef struct {
	int ttl;
	Rectangle geom;
} GameSquare;

typedef struct {
	Color color;
	int directionR;
	int directionG;
	int directionB;
} DColor;

typedef struct {
	char direction;
	int game_speed;
	int frame;
	int len;
	int lost;
	int c0;
} GameState;

typedef struct {
	int screenw;
	int screenh;
	Color bg_color;
	Color fg_color;
	Color p_color;
	int playing;
	int score;
} StatePacket;

StatePacket TitleScreen(StatePacket GamePacket);
StatePacket GameScreen(StatePacket GamePacket);
StatePacket LoseScreen(StatePacket GamePacket);
DColor ColorCycle(DColor color_in, int band_n);
int RandomBounds(int HIGH, int LOW);
int TitleArt(int row, int col);

int main(void) {
	const int SCREENW = 480;
	const int SCREENH = 480;
	const char TITLE[] = "Snake!";
	const int FPS_TARGET = 60;

	InitWindow(SCREENW, SCREENH, TITLE);
	SetTargetFPS(FPS_TARGET);

	StatePacket GamePacket = {SCREENW, SCREENH, BLUE, RED, WHITE, 1, 0};
	
playing:
	GamePacket = TitleScreen(GamePacket);
	if (GamePacket.playing == 0)
		goto close;
	GamePacket = GameScreen(GamePacket);
	GamePacket = LoseScreen(GamePacket);
	goto playing;
close:
	CloseWindow();
}

StatePacket TitleScreen(StatePacket GamePacket) {
	int sig = 1;
	
	const int REC_W = GamePacket.screenw / (REC_NUM_W + 1);
	const int REC_H = GamePacket.screenh / (REC_NUM_H + 1);
	GameSquare recs[REC_NUM_W][REC_NUM_H] = { 0 };

	for (int row = 0; row < REC_NUM_H; row++) {
		for (int col = 0; col < REC_NUM_W; col++) {
			recs[row][col].geom.x = REC_W / 2 + REC_W * col;
			recs[row][col].geom.y = REC_H / 2 + REC_H * row;
			recs[row][col].geom.width = REC_W - 2;
			recs[row][col].geom.height = REC_H - 2;
			recs[row][col].ttl = TitleArt(row, col);
		}
	}


	InitAudioDevice();
	Music music = LoadMusicStream("snake.wav");
	PlayMusicStream(music);
	int frame = 0;
	while (sig) {
		if (frame < 400) {
			UpdateMusicStream(music);
		}
		BeginDrawing();
		ClearBackground(GamePacket.bg_color);
		for (int row = 0; row < REC_NUM_H; row++) {
			for (int col = 0; col < REC_NUM_W; col++) {
				switch (recs[row][col].ttl) {
				case 0:
					DrawRectangleRec(recs[row][col].geom, GamePacket.fg_color);
					break;
				case 1:
					DrawRectangleRec(recs[row][col].geom, GamePacket.p_color);
					break;
				}
			}
		}
		EndDrawing();
		//DrawText("Snake!", 0, 0, 150, GamePacket.fg_color);
		//DrawText("by tz", 0, 240, 50, GamePacket.fg_color);
		//EndDrawing();
		if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_D) ||
			IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_RIGHT)) {
			GamePacket.playing = 1;
			sig = 0;
		}
		if (IsKeyPressed(KEY_X) || IsKeyPressed(KEY_ESCAPE)) {
			GamePacket.playing = 0;
			sig = 0;
		}
		if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_E) || IsKeyPressed(KEY_RIGHT_CONTROL)) {
			GamePacket.fg_color.r = RandomBounds(0, 255);
			GamePacket.fg_color.g = RandomBounds(0, 255);
			GamePacket.fg_color.b = RandomBounds(0, 255);
			GamePacket.bg_color.r = RandomBounds(0, 255);
			GamePacket.bg_color.g = RandomBounds(0, 255);
			GamePacket.bg_color.b = RandomBounds(0, 255);
		}
		if (frame < 501) {
			frame++;
		}
	}
	StopMusicStream(music);
	UnloadMusicStream(music);
	CloseAudioDevice();
	return GamePacket;
}

StatePacket GameScreen(StatePacket GamePacket) {
	const int REC_W = GamePacket.screenw / (REC_NUM_W + 1);
	const int REC_H = GamePacket.screenh / (REC_NUM_H + 1);

	int PlayerW = REC_NUM_W / 2;
	int PlayerH = REC_NUM_H / 2;

	int GAME_SPEED = 10;
	

	GameSquare recs[REC_NUM_W][REC_NUM_H] = { 0 };

	for (int row = 0; row < REC_NUM_H; row++) {
		for (int col = 0; col < REC_NUM_W; col++) {
			recs[row][col].geom.x = REC_W / 2 + REC_W * col;
			recs[row][col].geom.y = REC_H / 2 + REC_H * row;
			recs[row][col].geom.width = REC_W - 2;
			recs[row][col].geom.height = REC_H - 2;
			recs[row][col].ttl = 0;
			if (row == PlayerW && col == PlayerH) {
				recs[row][col].ttl = -1;
			}
		}
	}

	// Place Fruit
	recs[1][1].ttl = -2;

	DColor P_COLOR = {GamePacket.p_color, 1, 1, 1};
	DColor F_COLOR = {GamePacket.fg_color, 1, 1, 1};
	DColor B_COLOR = {GamePacket.bg_color, 1, 1, 1};

	int frame = 0;

	GameState game = {'r', GAME_SPEED ,0 ,3, 0, 0};
	while (game.lost == 0) {
		game.frame++;
		// a little buggy since you can queue up a u-turn between update frames
		if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP))
				if (game.direction != 'd')
					game.direction = 'u';
			if (IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT))
				if (game.direction != 'r')
					game.direction = 'l';
			if (IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN))
				if (game.direction != 'u')
					game.direction = 'd';
			if (IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT))
				if (game.direction != 'l')
					game.direction = 'r';
		if (game.frame >= game.game_speed) {			
			int dy = 0;
			int dx = 0;
			switch (game.direction) {
			case 'r':
				dy = 1;
				dx = 0;
				break;
			case 'l':
				dy = -1;
				dx = 0;
				break;
			case 'u':
				dy = 0;
				dx = -1;
				break;
			case 'd':
				dy = 0;
				dx = 1;
				break;
			}
			
			int x = -1;
			int y = -1;
			for (int row = 0; row < REC_NUM_H; row++) {
				for (int col = 0; col < REC_NUM_W; col++) {
					int ttl = recs[row][col].ttl;
					if (ttl == -1) {
						x = row;
						y = col;
					}
					if (ttl > 0) {
						recs[row][col].ttl--;
					}
				}
			}

			if (recs[x + dx][y + dy].ttl > 0) {
				game.lost = 1;
			}
			// Eats Fruit
			if (recs[x + dx][y + dy].ttl == -2) {
				int placed = 0;
				while (!placed){
					int HIGH = REC_NUM_W;
					int LOW = 0;
					int rndx = LOW + (rand() % (HIGH - LOW));
					int rndy = LOW + (rand() % (HIGH - LOW));
					if (recs[rndx][rndy].ttl == 0) {
						recs[rndx][rndy].ttl = -2;
						placed = 1;
					}
				}
				
				game.len++;
				GamePacket.score++;
				if (game.game_speed > 2) {
					game.game_speed--;
				}
				
				//Increase TTLs
				for (int row = 0; row < REC_NUM_H; row++) {
					for (int col = 0; col < REC_NUM_W; col++) {
						if (recs[row][col].ttl > 0) {
							recs[row][col].ttl++;
						}
					}
				}
				
				recs[x + dx][y + dy].ttl = -1;
			} else {
				recs[x + dx][y + dy].ttl = -1;
			}
			recs[x][y].ttl = game.len;

			if (x + dx < 0 || y + dy < 0 || x+dx >= REC_NUM_W || y+dy >= REC_NUM_H) {
				game.lost = 1;
			}
			game.frame = 0;
		}
		
		
		
		BeginDrawing();
		ClearBackground(B_COLOR.color);

		for (int row = 0; row < REC_NUM_H; row++) {
			for (int col = 0; col < REC_NUM_W; col++) {
				switch (recs[row][col].ttl) {
				case 0:
					DrawRectangleRec(recs[row][col].geom, F_COLOR.color);
					break;
				case -2:
					DrawRectangleRec(recs[row][col].geom, GREEN);
					break;
				default:
					DrawRectangleRec(recs[row][col].geom, P_COLOR.color);
				}
			}
		}

		int color_trip = 1;
		if (color_trip) {
			F_COLOR = ColorCycle(F_COLOR, 0);
			B_COLOR = ColorCycle(B_COLOR, 1);
			B_COLOR = ColorCycle(B_COLOR, 2);
			P_COLOR = ColorCycle(P_COLOR, 0);
			P_COLOR = ColorCycle(P_COLOR, 1);
			P_COLOR = ColorCycle(P_COLOR, 1);
			P_COLOR = ColorCycle(P_COLOR, 2);
			P_COLOR = ColorCycle(P_COLOR, 2);
			P_COLOR = ColorCycle(P_COLOR, 2);
		}

		EndDrawing();
	}
	GamePacket.bg_color = B_COLOR.color;
	GamePacket.fg_color = F_COLOR.color;
	return GamePacket;
}

DColor ColorCycle(DColor color_in, int band_n) {
	switch (band_n) {
		case 0:
			if (color_in.color.r == 255) {
				color_in.directionR = 0;
			}
			if (color_in.color.r == 0) {
				color_in.directionR = 1;
			}
			if (color_in.directionR == 0){
				color_in.color.r--;
			} else {
				color_in.color.r++;
			}
			break;
		case 1:
			if (color_in.color.g == 255) {
				color_in.directionG = 0;
			}
			if (color_in.color.g == 0) {
				color_in.directionG = 1;
			}
			if (color_in.directionG == 0) {
				color_in.color.g--;
			}
			else {
				color_in.color.g++;
			}
			break;
		case 2:
			if (color_in.color.b == 255) {
				color_in.directionB = 0;
			}
			if (color_in.color.b == 0) {
				color_in.directionB = 1;
			}
			if (color_in.directionB == 0) {
				color_in.color.b--;
			}
			else {
				color_in.color.b++;
			}
			break;
	}
	return color_in;
}


StatePacket LoseScreen(StatePacket GamePacket) {
	const int REC_W = GamePacket.screenw / (REC_NUM_W + 1);
	const int REC_H = GamePacket.screenh / (REC_NUM_H + 1);
	GameSquare recs[REC_NUM_W][REC_NUM_H] = { 0 };

	for (int row = 0; row < REC_NUM_H; row++) {
		for (int col = 0; col < REC_NUM_W; col++) {
			recs[row][col].geom.x = REC_W / 2 + REC_W * col;
			recs[row][col].geom.y = REC_H / 2 + REC_H * row;
			recs[row][col].geom.width = REC_W - 2;
			recs[row][col].geom.height = REC_H - 2;
			recs[row][col].ttl = 0;
			if (GamePacket.score-- > 0) {
				recs[row][col].ttl = 1;
			}
		}
	}

	int sig = 1;
	while (sig) {
		BeginDrawing();
		ClearBackground(GamePacket.bg_color);
		for (int row = 0; row < REC_NUM_H; row++) {
			for (int col = 0; col < REC_NUM_W; col++) {
				switch (recs[row][col].ttl) {
				case 0:
					DrawRectangleRec(recs[row][col].geom, GamePacket.fg_color);
					break;
				case 1:
					DrawRectangleRec(recs[row][col].geom, GamePacket.p_color);
					break;
				}
			}
		}
		EndDrawing();
		//DrawText("Snake!", 0, 0, 150, GamePacket.fg_color);
		//DrawText("by tz", 0, 240, 50, GamePacket.fg_color);
		//EndDrawing();
		if (IsKeyPressed(KEY_W) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_D) ||
			IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_RIGHT)) {
			GamePacket.playing = 1;
			sig = 0;
		}
		if (IsKeyPressed(KEY_X) || IsKeyPressed(KEY_ESCAPE)) {
			GamePacket.playing = 0;
			sig = 0;
		}
		if (IsKeyPressed(KEY_Q) || IsKeyPressed(KEY_E) || IsKeyPressed(KEY_RIGHT_CONTROL)) {
			GamePacket.fg_color.r = RandomBounds(0, 255);
			GamePacket.fg_color.g = RandomBounds(0, 255);
			GamePacket.fg_color.b = RandomBounds(0, 255);
			GamePacket.bg_color.r = RandomBounds(0, 255);
			GamePacket.bg_color.g = RandomBounds(0, 255);
			GamePacket.bg_color.b = RandomBounds(0, 255);
		}

	}
	GamePacket.score = 0;
	return GamePacket;
}

int RandomBounds(int HIGH, int LOW) {
	return LOW + (rand() % (HIGH - LOW));
}

int TitleArt(int row, int col) {
	if (row == 0 && (col == 0 || col == 1 || col == 3 || col == 6 || col == 9 || col == 12 || col == 14 || col == 16 || col == 17 || col == 18))
		return 1;
	if (row == 1 && (col == 0 || col == 3 || col == 4 || col == 6 || col == 8 || col == 10 || col == 12 || col == 14 || col == 16))
		return 1;
	if (row == 2 && (col == 0 || col == 1 || col == 3 || col == 5 || col == 6 || col == 8 || col == 9 || col == 10 || col == 12 || col == 13 || col == 16 || col == 17 || col == 18))
		return 1;
	if (row == 3 && (col == 1 || col == 3 || col == 6 || col == 8 || col == 10 || col == 12 || col == 14 || col == 16))
		return 1;
	if (row == 4 && (col == 0 || col == 1 || col == 3 || col == 6 || col == 8 || col == 10 || col == 12 || col == 14 || col == 16 || col == 17 || col == 18))
		return 1;
	if (row == 6 && (col == 4 || col == 5 || col == 6 || col == 14 || col == 15 || col == 16))
		return 1;
	if (row == 7 && (col == 3 || col == 4 || col == 5 || col == 6 || col == 7 || col == 8 || col == 12 || col == 13 || col == 14 || col == 15 || col == 16 || col == 17))
		return 1;
	if (row == 8 && (col == 2 || col == 3 || col == 4 || col == 7 || col == 8 || col == 9 || col == 11 || col == 12 || col == 13 || col == 16 || col == 17 || col == 18))
		return 1;
	if (row == 9 && (col == 2 || col == 3 || col == 8 || col == 9 || col == 10 || col == 11 || col == 12 || col == 17 || col == 18))
		return 1;
	if (row == 10 && (col == 2 || col == 3 || col == 9 || col == 10 || col == 11 || col == 16 || col == 17 || col == 18))
		return 1;
	if (row == 11 && (col == 2 || col == 3 || col == 16 || col == 17))
		return 1;
	if (row == 12 && (col == 3 || col == 4 || col == 15 || col == 16 || col == 17))
		return 1;
	if (row == 13 && (col == 15 || col == 16 || col == 17))
		return 1;
	if (row == 14 && (col == 1 || col == 2 || col == 3 || col == 4 || col == 5 || col == 6 || col == 7 || col == 14 || col == 15 || col == 16))
		return 1;
	if (row == 15 && (col == 2 || col == 3 || col == 4 || col == 5 || col == 6 || col == 7 || col == 8 || col == 13 || col == 14 || col == 15))
		return 1;
	if (row == 16 && (col == 1 || col == 2 || col == 3 || col == 4 || col == 6 || col == 7 || col == 8 || col == 9 || col == 11 || col == 12 || col == 13 || col == 14))
		return 1;
	if (row == 17 && (col == 1 || col == 2 || col == 4 || col == 7 || col == 8 || col == 9 || col == 10 || col == 11 || col == 12 || col == 13))
		return 1;
	if (row == 18 && (col == 8 || col == 9 || col == 10 || col == 11 || col == 12))
		return 1;
	return 0;
}