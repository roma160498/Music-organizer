#pragma once
#include "stdafx.h"
#include "MP3FileInfo.h"
#include <CommCtrl.h>
#include <list>

HWND WINAPI CreateTrackbar(HWND, HWND *, UINT, UINT);
int SetListViewColumns(HWND , int , int , char** );
LPWSTR MBStoWS(char *);
HWND CreateListView(HWND , UINT );
INT WINAPI AddListViewItems(HWND , int , int , char**);
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
VOID WINAPI AddMenus(HWND);
struct id_tag {
	char title[256];
	char album[256];
	char artist[256];
	char year[5];
	char comment[256];
};