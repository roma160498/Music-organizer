#pragma once
#include "stdafx.h"
#include "MP3FileInfo.h"
#include <CommCtrl.h>
#include <list>
#include <vector>
#include "commdlg.h"
HWND WINAPI CreateTrackbar(HWND, HWND *, UINT, UINT);
HWND CreateListView(HWND, UINT);
int SetListViewColumns(HWND , int , int , char** );
INT WINAPI AddListViewItems(HWND, int, int, char**);
VOID WINAPI UpdateListViewItem(HWND, int, char**, int);
VOID WINAPI UpdateEditBoxes(std::list<char *>, int, MP3FileInfo *, HWND,
	HWND,
	HWND,
	HWND,
	HWND);
VOID WINAPI ClearEditBoxes(HWND,
	HWND,
	HWND,
	HWND,
	HWND);
char* WStoMBS(LPWSTR );
LPWSTR MBStoWS(char *);
VOID WINAPI AddMenus(HWND);
VOID WINAPI FilterSong(std::list <char*>*, std::list <char*>, HWND, HWND, HWND *, MP3FileInfo *, int *, int,
	HWND,
	HWND,
	HWND,
	HWND,
	HWND,
	HWND,
	HWND
);
VOID WINAPI ChangeTags(std::list <char*>, int, HWND *, MP3FileInfo *,
	HWND,
	HWND,
	HWND,
	HWND,
	HWND,
	HWND);
VOID WINAPI AddSongs(HWND, std::list <char*>*, std::list <char*>*, MP3FileInfo *, int, int*,
	HWND,
	HWND,
	HWND,
	HWND,
	HWND,
	HWND*,
	HWND);
struct id_tag {
	char title[256];
	char album[256];
	char artist[256];
	char year[5];
	char comment[256];
};