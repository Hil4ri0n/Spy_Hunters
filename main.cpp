#define _CRT_SECURE_NO_WARNINGS
#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<time.h>
#include<direct.h>
#include <windows.h>
#include <tchar.h>
#include <shlwapi.h>
#include <stdlib.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH	640
#define SCREEN_HEIGHT	480
#define LEFTBORDER SCREEN_WIDTH * 0.27
#define RIGHTBORDER SCREEN_WIDTH * 0.73

typedef struct {
	int numberOfFiles;
	char firstFile[MAX_PATH];
}directoryData;

typedef struct {
	int x;
	int y;
}punkt;

typedef struct {
	double x;
	double y;
}carPosition;

typedef struct {
	SDL_Surface* screen, * charset;
	SDL_Surface* eti;
	SDL_Texture* scrtex;
	SDL_Window* window;
	SDL_Renderer* renderer;
}sdlElements;

typedef struct {
	int up;
	int down;
	int right;
	int left;
}directions;

typedef struct {
	punkt pozycjaPocisku;
	punkt punktWystrzalu;
	int czyWystrzelono;
}Pocisk;

typedef struct {
	carPosition pozycja;
	int hp;
	int czyIstnieje;
}Pojazd;



void DrawString(SDL_Surface* screen, int x, int y, const char* text, SDL_Surface* charset);
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y);
void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color);
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color);
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor);
void rysujDroge(SDL_Surface* screen, int x, int y, int szerokosc, int dlugosc);
void rysujOtoczenie(SDL_Surface* screen, int trees[][2], int bushes[][2]);
void zwolnij_przycisk(directions& directions);
const wchar_t* getWchar(const char* c);
directoryData getDirData(char* path);
void zapisz(int wynik, punkt pozycjaGracza, double czas, Pojazd przeciwnik, Pojazd& cywil);
void wczytaj(char* fileName, punkt &pozycjaGracza, int& wynik, double& czas, Pojazd& przeciwnik, Pojazd& cywil);
void saveMenu(sdlElements screenControls, punkt& pozycjaGracza, int& punkty, double& czas, Pojazd& przeciwnik, Pojazd& cywil);
void printInformation(sdlElements screenControls, double worldTime, double fps, int wynik);
void handleEvent(directions& directions, int& quit, int& pauza, punkt& pozycjaGracza, int& wynik,
	double& worldTime, sdlElements screenControls, int& load, Pojazd& przeciwnik, Pojazd& cywil, Pocisk &pocisk);
void printEnemyCar(SDL_Surface* screen, int PositionX, int PositionY);
void printCivilianCar(SDL_Surface* screen, int PositionX, int PositionY);
void enemyCarMove(Pojazd& przeciwnik, punkt pozycjaGracza);
void CivilianMove(Pojazd& cywil, punkt pozycjaGracza);
int czyZderzenie(Pojazd przeciwnik, punkt pozycjaGracza);
void zderzenie(Pojazd& przeciwnik, punkt& pozycjaGracza, int& wynik);
void spawnCar(int& enemy);
void unik(carPosition& przeciwnik, carPosition& cywil);
void trafienie(Pocisk& pocisk, Pojazd& przeciwnik);
void wystrzelPocisk(Pocisk& pocisk, punkt pozycjaGracza);
void  przesunPocisk(Pocisk& pocisk);
void rysujPocisk(punkt pozycjaPocisku, sdlElements screenControls);
void zniszczenie(Pojazd& przeciwnik, Pojazd& cywil, int& zatrzymaniePkt, int& wynik);
void akcjePocisku(Pocisk& pocisk, sdlElements screenControls, Pojazd& przeciwnik, Pojazd& cywil, int& zatrzymaniePkt, int& wynik);

// narysowanie napisu txt na powierzchni screen, zaczynaj¹c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj¹ca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
		};
	};


// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt œrodka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
	};


// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};


// rysowanie linii o d³ugoœci l w pionie (gdy dx = 0, dy = 1) 
// b¹dŸ poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};


// rysowanie prostok¹ta o d³ugoœci boków l i k
// draw a rectangle of size l by k
void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

void rysujDroge(SDL_Surface* screen, int x, int y, int szerokosc, int dlugosc) {
	int kolor = SDL_MapRGB(screen->format, 26, 26, 26);
	int bialy = SDL_MapRGB(screen->format, 255, 255, 255);
	DrawRectangle(screen,  x, y, szerokosc, dlugosc, kolor, kolor);
	DrawRectangle(screen, (int)(x + 0.425 * szerokosc), 0, (int)(szerokosc * 0.025), dlugosc, bialy, bialy);
	DrawRectangle(screen, (int)(x + 0.525 * szerokosc), 0, (int)(szerokosc * 0.025), dlugosc, bialy, bialy);
}

void rysujOtoczenie(SDL_Surface* screen, int trees[][2], int bushes[][2]) {
	
	SDL_Surface* tree = SDL_LoadBMP("./pixel_tree1.bmp");
	SDL_Surface* bush = SDL_LoadBMP("./pixelBush.bmp");

	for(int i = 0; i < 8; i++) {
		if (bushes[i][1] > SCREEN_HEIGHT) bushes[i][1] = 0;
		if (trees[i][1] > SCREEN_HEIGHT) trees[i][1] = 0;
		DrawSurface(screen, bush, bushes[i][0], bushes[i][1]++);
		DrawSurface(screen, tree, trees[i][0], trees[i][1]++);
	}
}

void zwolnij_przycisk(directions& directions) {
	const Uint8* state = SDL_GetKeyboardState(NULL);
	if (!state[SDL_SCANCODE_UP]) directions.up = 0;
	if (!state[SDL_SCANCODE_DOWN]) directions.down = 0;
	if (!state[SDL_SCANCODE_RIGHT]) directions.right = 0;
	if (!state[SDL_SCANCODE_LEFT]) directions.left = 0;
}

const wchar_t* getWchar(const char* c){
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	mbstowcs(wc, c, cSize);

	return wc;
}



directoryData getDirData(char* path) {
	WIN32_FIND_DATA ffd;
	TCHAR szDir[MAX_PATH];
	HANDLE hFind = INVALID_HANDLE_VALUE;
	int i = 0;
	do {
		szDir[i] = path[i];
		i++;
	} while (path[i]);
	szDir[i] = path[i];

	directoryData data;
	int numberOfFiles = 0;
	hFind = FindFirstFile(szDir, &ffd);
	if (hFind == INVALID_HANDLE_VALUE) {
		if(_mkdir("save") == -1) data.numberOfFiles = -1;
		else data.numberOfFiles = numberOfFiles;
		return data;
	}
	
	do {
		if (wcscmp(ffd.cFileName, getWchar(".")) != 0 && wcscmp(ffd.cFileName, getWchar("..")) != 0) {
			if(numberOfFiles == 0)  wcstombs(data.firstFile, ffd.cFileName, wcslen(ffd.cFileName) + 1);
			numberOfFiles++;
		}
	} while (FindNextFile(hFind, &ffd) != 0);
	data.numberOfFiles = numberOfFiles;
	return data;
}


void zapisz(int wynik, punkt pozycjaGracza, double czas, Pojazd przeciwnik, Pojazd& cywil) {
	
	char path[] = "save\\*";
	directoryData dirData = getDirData(path);
	if (dirData.numberOfFiles == -1) return;
		if (dirData.numberOfFiles == 5) {
			char firstFilePath[MAX_PATH];
			sprintf(firstFilePath, "save\\%s", dirData.firstFile);
			remove(firstFilePath);
		}

		time_t t = time(NULL);
		struct tm tm = *localtime(&t);
		char fileName[40]{};
		sprintf(fileName, "save\\%d-%02d-%02d %02d-%02d-%02d.txt", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
		FILE* file = NULL;
		do {
			fopen_s(&file, fileName, "w");
		} while (file == NULL);
		fprintf(file, "%d\n", wynik);
		fprintf(file, "%d\n", pozycjaGracza.x);
		fprintf(file, "%d\n", pozycjaGracza.y);
		fprintf(file, "%lf\n", czas);
		fprintf(file, "%lf\n", przeciwnik.pozycja.x);
		fprintf(file, "%lf\n", przeciwnik.pozycja.y);
		fprintf(file, "%lf\n", cywil.pozycja.x);
		fprintf(file, "%lf\n", cywil.pozycja.y);
		fclose(file);
}



void wczytaj(char* fileName, punkt& pozycjaGracza, int& wynik, double& czas, Pojazd& przeciwnik, Pojazd& cywil) {
	FILE* file = NULL;
	char path[MAX_PATH];
	sprintf(path, "save\\%s", fileName);
	do {
		fopen_s(&file, path, "r");
	} while (file == NULL);

	fscanf_s(file, "%d\n", &wynik);
	fscanf_s(file, "%d\n", &pozycjaGracza.x);
	fscanf_s(file, "%d\n", &pozycjaGracza.y);
	fscanf_s(file, "%lf\n", &czas);
	fscanf_s(file, "%lf\n", &przeciwnik.pozycja.x);
	fscanf_s(file, "%lf\n", &przeciwnik.pozycja.y);
	fscanf_s(file, "%lf\n", &cywil.pozycja.x);
	fscanf_s(file, "%lf\n", &cywil.pozycja.y);
	fclose(file);
}

void saveMenu(sdlElements screenControls, punkt& pozycjaGracza, int& punkty, double& czas, Pojazd& przeciwnik, Pojazd& cywil) {

		WIN32_FIND_DATA ffd;
		TCHAR szDir[MAX_PATH];
		HANDLE hFind = INVALID_HANDLE_VALUE;
		char path[] = "save\\*";
		int i = 0;
		do {
			szDir[i] = path[i];
			i++;
		} while (path[i]);
		szDir[i] = path[i];

		int czarny = SDL_MapRGB(screenControls.screen->format, 0x00, 0x00, 0x00);
		SDL_FillRect(screenControls.screen, NULL, czarny);

		hFind = FindFirstFile(szDir, &ffd);

		SDL_Event event;
		char text[MAX_PATH];
		if (hFind == INVALID_HANDLE_VALUE) {
			sprintf(text, "Brak zapisanych gier");
			DrawString(screenControls.screen, (int)(SCREEN_WIDTH * 0.25), 16, text, screenControls.charset);
			SDL_UpdateTexture(screenControls.scrtex, NULL, screenControls.screen->pixels, screenControls.screen->pitch);
			SDL_RenderCopy(screenControls.renderer, screenControls.scrtex, NULL, NULL);
			SDL_RenderPresent(screenControls.renderer);
			while (!(SDL_PollEvent(&event) && (event.type == SDL_KEYDOWN)));
			return;
		}

		char* fileName = NULL;
		char** saveList = (char**)malloc(5 * sizeof(char*));
		int numberOfFiles = 0;
		if (saveList != NULL) {
			do {
				if (wcscmp(ffd.cFileName, getWchar(".")) != 0 && wcscmp(ffd.cFileName, getWchar("..")) != 0) {
					fileName = (char*)malloc(MAX_PATH * sizeof(char));
					wcstombs(fileName, ffd.cFileName, wcslen(ffd.cFileName) + 1);
					saveList[numberOfFiles] = fileName;
					numberOfFiles++;
				}
			} while (FindNextFile(hFind, &ffd) != 0);


			for (int i = 0; i < numberOfFiles; i++) {
				sprintf(text, "%d. %s", i + 1, saveList[i]);
				printf(text, "%d. %s", i + 1, saveList[i]);
				printf("\n");
				DrawString(screenControls.screen, (int)(SCREEN_WIDTH * 0.25), i * 16, text, screenControls.charset);
			}
		}
		
		SDL_UpdateTexture(screenControls.scrtex, NULL, screenControls.screen->pixels, screenControls.screen->pitch);
		SDL_RenderCopy(screenControls.renderer, screenControls.scrtex, NULL, NULL);
		SDL_RenderPresent(screenControls.renderer);

		
		int choice = 0;
		while (!choice) {
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_KEYDOWN) {
					if (event.key.keysym.sym == SDLK_1 && numberOfFiles >= 1) {
						choice = 1;
						wczytaj(saveList[0], pozycjaGracza, punkty, czas, przeciwnik, cywil);
					}
					else if (event.key.keysym.sym == SDLK_2 && numberOfFiles >= 2) {
						choice = 1;
						wczytaj(saveList[1], pozycjaGracza, punkty, czas, przeciwnik, cywil);
					}
					else if (event.key.keysym.sym == SDLK_3 && numberOfFiles >= 3) {
						choice = 1;
						wczytaj(saveList[2], pozycjaGracza, punkty, czas, przeciwnik, cywil);
					}
					else if (event.key.keysym.sym == SDLK_4 && numberOfFiles >= 4) {
						choice = 1;
						wczytaj(saveList[3], pozycjaGracza, punkty, czas, przeciwnik, cywil);
					}
					else if (event.key.keysym.sym == SDLK_5 && numberOfFiles >= 5) {
						choice = 1;
						wczytaj(saveList[4], pozycjaGracza, punkty, czas, przeciwnik, cywil);
					}
					else if (event.key.keysym.sym == SDLK_ESCAPE) {
						choice = 1;
					}
				}
			}
		}
		for (int i = 0; i < numberOfFiles; i++) {
			free(saveList[i]);
		}
		free(saveList);
}

void printEnemyCar(SDL_Surface* screen, int PositionX, int PositionY) {
	SDL_Surface* enemyCar;
	enemyCar = SDL_LoadBMP("./enemyCar.bmp");
	DrawSurface(screen, enemyCar, PositionX, PositionY);
	SDL_FreeSurface(enemyCar);
}

void printCivilianCar(SDL_Surface* screen, int PositionX, int PositionY) {
	SDL_Surface* Car;
	Car = SDL_LoadBMP("./Car6.bmp");
	DrawSurface(screen, Car, PositionX, PositionY);
	SDL_FreeSurface(Car);
}

void enemyCarMove(Pojazd& przeciwnik, punkt pozycjaGracza) {
	if (pozycjaGracza.y + 93 >= przeciwnik.pozycja.y && (pozycjaGracza.x >= przeciwnik.pozycja.x + 38 || pozycjaGracza.x + 38 <= przeciwnik.pozycja.x)) {
		if (pozycjaGracza.x < przeciwnik.pozycja.x) przeciwnik.pozycja.x -= 0.3;
		else przeciwnik.pozycja.x += 0.3;
	}
	if (przeciwnik.pozycja.x > RIGHTBORDER) przeciwnik.pozycja.x = RIGHTBORDER;
	if (przeciwnik.pozycja.x < LEFTBORDER) przeciwnik.pozycja.x = LEFTBORDER;
	przeciwnik.pozycja.y += 0.1;
}

void CivilianMove(Pojazd& cywil, punkt pozycjaGracza) {
	cywil.pozycja.y += 0.1;
	if (cywil.pozycja.y >= SCREEN_HEIGHT * 1.1) cywil.pozycja.y = -70;
}

int czyZderzenie(Pojazd przeciwnik, punkt pozycjaGracza) {
	if (pozycjaGracza.x <= przeciwnik.pozycja.x + 38 && pozycjaGracza.x + 38 > przeciwnik.pozycja.x) {
		if (pozycjaGracza.y + 85 >= przeciwnik.pozycja.y && pozycjaGracza.y < przeciwnik.pozycja.y + 85) {
			return 1;
		}
	}
	return 0;
}

void zderzenie(Pojazd& przeciwnik, punkt& pozycjaGracza, int &wynik) {
	przeciwnik.pozycja.y = -50;
	przeciwnik.pozycja.x = SCREEN_WIDTH/2;
	wynik -= 2000;
}

void spawnCar(int &enemy) {
	static int time;
	time ++;
	if (time >= 1000) {
		time = 0;
		enemy = 1;
	}
}

void unik(carPosition& przeciwnik, carPosition& cywil) {
	if (przeciwnik.y + 115 >= cywil.y && przeciwnik.y <= cywil.y + 115) {
		if (przeciwnik.x <= cywil.x + 80 && przeciwnik.x + 80 >= cywil.x) {
			if (cywil.x >= SCREEN_WIDTH / 2 && przeciwnik.x <= SCREEN_WIDTH / 2) cywil.x++;
			if (cywil.x <= SCREEN_WIDTH / 2 && przeciwnik.x >= SCREEN_WIDTH / 2) cywil.x--;
			if (cywil.x >= SCREEN_WIDTH / 2 && przeciwnik.x >= SCREEN_WIDTH / 2) cywil.y--;
			if (cywil.x <= SCREEN_WIDTH / 2 && przeciwnik.x <= SCREEN_WIDTH / 2) cywil.y--;
			if (cywil.x >= RIGHTBORDER) {
				cywil.x = RIGHTBORDER;
				cywil.y--;
			}
			if (cywil.x <= LEFTBORDER) {
				cywil.x = LEFTBORDER;
				cywil.y--;
			}
		}
	}
}

void trafienie(Pocisk& pocisk, Pojazd& przeciwnik) {
	if (pocisk.pozycjaPocisku.x >= przeciwnik.pozycja.x - 10 && pocisk.pozycjaPocisku.x <= przeciwnik.pozycja.x + 30) {
		if (pocisk.pozycjaPocisku.y <= przeciwnik.pozycja.y + 50) {
			pocisk.czyWystrzelono = 0;
			przeciwnik.hp -= 20;
		}
	}
}

void wystrzelPocisk(Pocisk& pocisk, punkt pozycjaGracza) {
	pocisk.czyWystrzelono = 1;
	pocisk.punktWystrzalu.x = pozycjaGracza.x;
	pocisk.punktWystrzalu.y = pozycjaGracza.y;
	pocisk.pozycjaPocisku.x = pozycjaGracza.x;
	pocisk.pozycjaPocisku.y = pozycjaGracza.y;
}

void  przesunPocisk(Pocisk& pocisk) {
	pocisk.pozycjaPocisku.y -= 2;
	if (pocisk.pozycjaPocisku.y <= 0) pocisk.czyWystrzelono = 0;
}

void rysujPocisk(punkt pozycjaPocisku, sdlElements screenControls) {
	int kolor = SDL_MapRGB(screenControls.screen->format, 184, 3, 255);
	DrawLine(screenControls.screen, pozycjaPocisku.x, pozycjaPocisku.y, 25, 0, 1, kolor);
}

void zniszczenie(Pojazd& przeciwnik, Pojazd& cywil, int& zatrzymaniePkt, int& wynik) {
	if (przeciwnik.hp == 0) {
		przeciwnik.czyIstnieje = 0;
		przeciwnik.pozycja.y = -50;
		przeciwnik.pozycja.x = SCREEN_WIDTH / 2;
		przeciwnik.hp = 100;
		wynik += 500;
	}
	else if (cywil.hp == 0) {
		cywil.czyIstnieje = 0;
		cywil.pozycja.y = -50;
		cywil.pozycja.x = SCREEN_WIDTH / 2;
		zatrzymaniePkt = 1;
		cywil.hp = 20;
	}
}

void akcjePocisku(Pocisk &pocisk, sdlElements screenControls, Pojazd& przeciwnik, Pojazd& cywil, int& zatrzymaniePkt, int& wynik) {
	if (pocisk.czyWystrzelono) {
		rysujPocisk(pocisk.pozycjaPocisku, screenControls);
		trafienie(pocisk, przeciwnik);
		trafienie(pocisk, cywil);
		przesunPocisk(pocisk);
		if (pocisk.punktWystrzalu.y - pocisk.pozycjaPocisku.y > 190) pocisk.czyWystrzelono = 0;
		zniszczenie(przeciwnik,cywil,zatrzymaniePkt, wynik);
	}
}

void printInformation(sdlElements screenControls, double worldTime, double fps, int wynik) {

	int kolorObramowania = SDL_MapRGB(screenControls.screen->format, 232, 209, 2);
	int kolorWypelnienia = SDL_MapRGB(screenControls.screen->format, 26, 26, 26);
	char text[280];

	DrawRectangle(screenControls.screen, (int)(SCREEN_WIDTH * 0.18), 4, (int)(SCREEN_WIDTH * 0.7), 20, kolorObramowania, kolorWypelnienia);
	sprintf(text, "Czas trwania = %.1lf s  %.0lf klatek / s Wynik: %d", worldTime, fps, wynik);
	DrawString(screenControls.screen, screenControls.screen->w / 2 - strlen(text) * 8 / 2, 10, text, screenControls.charset);
	DrawRectangle(screenControls.screen, (int)(SCREEN_WIDTH * 0.79), (int)(SCREEN_HEIGHT * 0.64), (int)(SCREEN_WIDTH / 5), (int)(SCREEN_HEIGHT * 0.28), kolorObramowania, kolorWypelnienia);
	sprintf(text, "\030\031\032\033 - ruch");
	DrawString(screenControls.screen, (int)(SCREEN_WIDTH * 0.8), (int)(SCREEN_HEIGHT * 0.65), text, screenControls.charset);
	sprintf(text, "Esc - wyjscie");
	DrawString(screenControls.screen, (int)(SCREEN_WIDTH * 0.8), (int)(SCREEN_HEIGHT * 0.65) + 16, text, screenControls.charset);
	sprintf(text, "n - nowa gra");
	DrawString(screenControls.screen, (int)(SCREEN_WIDTH * 0.8), (int)(SCREEN_HEIGHT * 0.65) + 32, text, screenControls.charset);
	sprintf(text, "s - zapisz");
	DrawString(screenControls.screen, (int)(SCREEN_WIDTH * 0.8), (int)(SCREEN_HEIGHT * 0.65) + 48, text, screenControls.charset);
	sprintf(text, "l - wczytaj");
	DrawString(screenControls.screen, (int)(SCREEN_WIDTH * 0.8), (int)(SCREEN_HEIGHT * 0.65) + 64, text, screenControls.charset);
	sprintf(text, "p - pauza");
	DrawString(screenControls.screen, (int)(SCREEN_WIDTH * 0.8), (int)(SCREEN_HEIGHT * 0.65) + 80, text, screenControls.charset);
	sprintf(text, "spacja - strzal");
	DrawString(screenControls.screen, (int)(SCREEN_WIDTH * 0.8), (int)(SCREEN_HEIGHT * 0.65) + 96, text, screenControls.charset);
}

void handleEvent(directions& directions, int& quit, int& pauza, punkt& pozycjaGracza, int& wynik, 
	double& worldTime, sdlElements screenControls, int& load, Pojazd& przeciwnik, Pojazd& cywil, Pocisk& pocisk) {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
			else if (event.key.keysym.sym == SDLK_UP && !pauza) {
				directions.up = 1;
			}
			else if (event.key.keysym.sym == SDLK_DOWN && !pauza) {
				directions.down = 1;
			}
			else if (event.key.keysym.sym == SDLK_RIGHT && !pauza) {
				directions.right = 1;
			}
			else if (event.key.keysym.sym == SDLK_LEFT && !pauza) {
				directions.left = 1;
			}
			else if (event.key.keysym.sym == SDLK_n) {
				pozycjaGracza.x = (int)(SCREEN_WIDTH / 2);
				pozycjaGracza.y = (int)(SCREEN_HEIGHT * 0.75);
				wynik = 0;
				worldTime = 0;
			}
			else if (event.key.keysym.sym == SDLK_p) {
				if (!pauza) pauza = 1;
				else pauza = 0;
			}
			else if (event.key.keysym.sym == SDLK_s) {
				zapisz(wynik, pozycjaGracza, worldTime, przeciwnik, cywil);
			}
			else if (event.key.keysym.sym == SDLK_l) {
				saveMenu(screenControls, pozycjaGracza, wynik, worldTime, przeciwnik, cywil);
				load = 1;
			}
			else if (event.key.keysym.sym == SDLK_SPACE) {
				wystrzelPocisk(pocisk, pozycjaGracza);
			}
			break;
		case SDL_KEYUP:
			zwolnij_przycisk(directions);
			break;
		case SDL_QUIT:
			quit = 1;
			break;
		};
	};
}

// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	int t1, t2, quit, frames, rc;
	double delta, worldTime, fpsTimer, fps, distance;
	sdlElements screenControls;

	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	// tryb pe³noekranowy / fullscreen mode
	//rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
	    //                            &window, &renderer);
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
	                              &screenControls.window, &screenControls.renderer);
	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
		};
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(screenControls.renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(screenControls.renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(screenControls.window, "Piotr Kolasiñski, 193275");


	screenControls.screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
	                              0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	screenControls.scrtex = SDL_CreateTexture(screenControls.renderer, SDL_PIXELFORMAT_ARGB8888,
	                           SDL_TEXTUREACCESS_STREAMING,
	                           SCREEN_WIDTH, SCREEN_HEIGHT);


	// wy³¹czenie widocznoœci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp
	screenControls.charset = SDL_LoadBMP("./cs8x8.bmp");
	if(screenControls.charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screenControls.screen);
		SDL_DestroyTexture(screenControls.scrtex);
		SDL_DestroyWindow(screenControls.window);
		SDL_DestroyRenderer(screenControls.renderer);
		SDL_Quit();
		return 1;
		};
	SDL_SetColorKey(screenControls.charset, true, 0x000000);

	screenControls.eti = SDL_LoadBMP("./pixelCar.bmp");
	if(screenControls.eti == NULL) {
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		SDL_FreeSurface(screenControls.charset);
		SDL_FreeSurface(screenControls.screen);
		SDL_DestroyTexture(screenControls.scrtex);
		SDL_DestroyWindow(screenControls.window);
		SDL_DestroyRenderer(screenControls.renderer);
		SDL_Quit();
		return 1;
		};
	
	int bushes[8][2] = { {(int)(SCREEN_WIDTH / 5), (int)(SCREEN_HEIGHT / 2)}, {(int)(SCREEN_WIDTH / 15),(int)(SCREEN_HEIGHT / 8)},
						{(int)(SCREEN_WIDTH / 12), (int)(SCREEN_HEIGHT * 0.6)}, {(int)(SCREEN_WIDTH / 6), (int)(SCREEN_HEIGHT * 0.87)},
						{(int)(SCREEN_WIDTH * 0.8), (int)(SCREEN_HEIGHT / 2.5)}, {(int)(SCREEN_WIDTH * 0.95), (int)(SCREEN_HEIGHT / 8)},
						{(int)(SCREEN_WIDTH * 0.9), (int)(SCREEN_HEIGHT * 0.67)}, {(int)(SCREEN_WIDTH * 0.8), (int)(SCREEN_HEIGHT * 0.85)} };

	int trees[8][2] = { {(int)(SCREEN_WIDTH / 15), (int)(SCREEN_HEIGHT / 3)}, {(int)(SCREEN_WIDTH / 5),(int)(SCREEN_HEIGHT / 8)},
						{(int)(SCREEN_WIDTH / 6), (int)(SCREEN_HEIGHT * 0.7)}, {(int)(SCREEN_WIDTH / 14), (int)(SCREEN_HEIGHT * 0.87)},
						{(int)(SCREEN_WIDTH * 0.9), (int)(SCREEN_HEIGHT / 2.5)}, {(int)(SCREEN_WIDTH * 0.8), (int)(SCREEN_HEIGHT / 8)},
						{(int)(SCREEN_WIDTH * 0.8), (int)(SCREEN_HEIGHT * 0.67)}, {(int)(SCREEN_WIDTH * 0.95), (int)(SCREEN_HEIGHT * 0.85)} };

	int czarny = SDL_MapRGB(screenControls.screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screenControls.screen->format, 59, 99, 29);
	int czerwony = SDL_MapRGB(screenControls.screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screenControls.screen->format, 0x11, 0x11, 0xCC);
	int kolorObramowania = SDL_MapRGB(screenControls.screen->format, 232, 209, 2);
	int kolorWypelnienia = SDL_MapRGB(screenControls.screen->format, 26, 26, 26);

	t1 = SDL_GetTicks();

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;
	distance = 0;

	///////////////////////////////////////////////////////////////
	 
	punkt pozycjaGracza;
	pozycjaGracza.x = (int)(SCREEN_WIDTH / 2);
	pozycjaGracza.y = (int)(SCREEN_HEIGHT * 0.75);
	int przesuniecie = 0;
	int wynik = 0;
	int lewaGranica = (int)(SCREEN_WIDTH * 0.27);
	int prawaGranica = (int)(SCREEN_WIDTH * 0.73);
	int prawaKolizja = (int)(SCREEN_WIDTH * 0.78);
	int lewaKolizja = (int)(SCREEN_WIDTH * 0.22);
	int pauza = 0;
	Pojazd przeciwnik;
	przeciwnik.pozycja.x = SCREEN_WIDTH/2;
	przeciwnik.pozycja.y = 0;
	przeciwnik.hp = 100;
	przeciwnik.czyIstnieje = 0;
	int load = 0;
	Pojazd cywil;
	cywil.pozycja.x = SCREEN_WIDTH / 3.35;
	cywil.pozycja.y = -70;
	cywil.czyIstnieje = 0;
	cywil.hp = 20;
	Pocisk pocisk;
	pocisk.czyWystrzelono = 0;
	int zatrzymaniePkt = 0;
	
	directions directions{};

	while(!quit) {
		t2 = SDL_GetTicks();

		delta = (t2 - t1) * 0.001;
		t1 = t2;

		if(!pauza) worldTime += delta;

		distance += delta;

		if (pozycjaGracza.x > lewaGranica && pozycjaGracza.x < prawaGranica && !pauza && !zatrzymaniePkt) {
			wynik += 1;
			
		}
		else {
			zatrzymaniePkt++;
			if (zatrzymaniePkt == 5000) zatrzymaniePkt = 0;
		}

		if (!pauza) {
			SDL_FillRect(screenControls.screen, NULL, zielony);
			rysujDroge(screenControls.screen, (int)(SCREEN_WIDTH * 0.25), 0, (int)(SCREEN_WIDTH / 2), SCREEN_HEIGHT);
			rysujOtoczenie(screenControls.screen, trees, bushes);
			akcjePocisku(pocisk, screenControls, przeciwnik, cywil, zatrzymaniePkt, wynik);
			DrawSurface(screenControls.screen, screenControls.eti, pozycjaGracza.x, pozycjaGracza.y);
		}

		if (load) {
			SDL_FillRect(screenControls.screen, NULL, zielony);
			rysujDroge(screenControls.screen, (int)(SCREEN_WIDTH * 0.25), 0, (int)(SCREEN_WIDTH / 2), SCREEN_HEIGHT);
			rysujOtoczenie(screenControls.screen, trees, bushes);
			DrawSurface(screenControls.screen, screenControls.eti, pozycjaGracza.x, pozycjaGracza.y);
			printEnemyCar(screenControls.screen, (int)(przeciwnik.pozycja.x), (int)(przeciwnik.pozycja.y));
			printCivilianCar(screenControls.screen, cywil.pozycja.x, cywil.pozycja.y);
			load = 0;
		}

		


		if (!pauza) {

			if (przeciwnik.pozycja.y >= SCREEN_HEIGHT * 1.1) przeciwnik.pozycja.y = 0;
			if (przeciwnik.czyIstnieje == 1) {
				printEnemyCar(screenControls.screen, (int)(przeciwnik.pozycja.x), (int)(przeciwnik.pozycja.y));
				enemyCarMove(przeciwnik, pozycjaGracza);
			}
			else spawnCar(przeciwnik.czyIstnieje);

			if (czyZderzenie(przeciwnik, pozycjaGracza)) {
				zderzenie(przeciwnik, pozycjaGracza, wynik);
				przeciwnik.czyIstnieje = 0;
			}

			unik(przeciwnik.pozycja, cywil.pozycja);
			printCivilianCar(screenControls.screen, cywil.pozycja.x, cywil.pozycja.y);
			CivilianMove(cywil, pozycjaGracza);

			if (czyZderzenie(cywil, pozycjaGracza)) {
				zderzenie(cywil, pozycjaGracza, wynik);
			}
		}

		fpsTimer += delta;
		if (fpsTimer > 0.5) {
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.5;
		}

		printInformation(screenControls, worldTime, fps, wynik);

		SDL_UpdateTexture(screenControls.scrtex, NULL, screenControls.screen->pixels, screenControls.screen->pitch);
//		SDL_RenderClear(renderer);
		SDL_RenderCopy(screenControls.renderer, screenControls.scrtex, NULL, NULL);
		SDL_RenderPresent(screenControls.renderer);

		// obs³uga zdarzeñ (o ile jakieœ zasz³y) / handling of events (if there were any)
		handleEvent(directions, quit, pauza, pozycjaGracza, wynik, worldTime, screenControls, load, przeciwnik, cywil, pocisk);
		frames++;


		////////////////////////////////////////////////////////////////////
		if (directions.up) {
			pozycjaGracza.y--;
			if (pozycjaGracza.y < SCREEN_HEIGHT * 0.1) pozycjaGracza.y = (int)(SCREEN_HEIGHT * 0.1);
		}
		if (directions.down) {
			pozycjaGracza.y++;
			if (pozycjaGracza.y > SCREEN_HEIGHT * 0.9) pozycjaGracza.y = (int)(SCREEN_HEIGHT * 0.9);
		}
		if (directions.right) {
			pozycjaGracza.x++;
			if (pozycjaGracza.x > prawaKolizja) {
				pozycjaGracza.x = (int)(SCREEN_WIDTH / 2);
				wynik -= 1000;
			}
		}
		if (directions.left) {
			pozycjaGracza.x--;
			if (pozycjaGracza.x < lewaKolizja) {
				pozycjaGracza.x = (int)(SCREEN_WIDTH / 2);
				wynik -= 1000;
			}
		}

		
		//enemyAttack(przeciwnikX, przeciwnikY, pozycjaX, pozycjaY);
	};

	
	

	// zwolnienie powierzchni / freeing all surfaces
	SDL_FreeSurface(screenControls.charset);
	SDL_FreeSurface(screenControls.screen);
	SDL_DestroyTexture(screenControls.scrtex);
	SDL_DestroyRenderer(screenControls.renderer);
	SDL_DestroyWindow(screenControls.window);

	SDL_Quit();
	return 0;
	};
