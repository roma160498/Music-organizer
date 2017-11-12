#pragma once

#include "resource.h"



ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
HWND WINAPI         CreateTrackbar(HWND, UINT, UINT);
HWND                CreateListView(HWND, UINT);
int                 SetListViewColumns(HWND, int, int, char**);
BOOL WINAPI         AddListViewItems(HWND, int, int, char**);
LPWSTR				MBStoWS(char *);
char*				WStoMBS(LPWSTR);
VOID WINAPI			AddMenus(HWND);
VOID WINAPI         UpdateListViewItem(HWND, int, char**);
VOID WINAPI			UpdateEditBoxes();
VOID WINAPI			ClearEditBoxes();