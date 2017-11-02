// MusicOrganizer.cpp: определяет точку входа для приложения.
//

#include "stdafx.h"
#include "windows.h"

#include <CommCtrl.h>
#include "MusicOrganizer.h"
#include "bass.h"
#include "commdlg.h"
#include <list>
#include <iostream>
#include <fstream>
#include <string>
#include <ShlObj.h>
#pragma comment(lib,"shell32.lib")
#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment(lib,"GdiPlus.lib")

#define ID3LIB_LINKOPTION 3
#include "id3.h"
#include "id3/globals.h"
#include "id3/tag.h"
#pragma comment(lib,"id3lib.lib")
#include "MP3FileInfo.h"





#pragma comment(lib,"bass.lib")
#pragma comment(lib, "ComCtl32.Lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' \
                        name='Microsoft.Windows.Common-Controls' \
                        version='6.0.0.0' processorArchitecture='*'\
 publicKeyToken='6595b64144ccf1df' language='*'\"")
//#pragma warning(disable : 4996)
MP3FileInfo mp3fi;
struct id_tag {
	char title[256];
	char album[256];
	char artist[256];
	char year[5];
	char comment[256];
};
#define MAX_LOADSTRING 100

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

HWND hwndBtnPlay, hwndBtnPause, hwndBtnStop, hwndBtnDelete, hwndTrack, hwndLabelVol, hwndTBTitle, hwndBtnAdd, hwndBtnPrev, hwndBtnNext;
HWND hwndTBArtist, hwndTBAlbum, hwndTBYear, hwndTBComment, hwndLabelTitle, hwndLabelArtist, hwndLabelAlbum;
HWND hwndLabelYear, hwndLabelComment, hwndBtnChangeTags;
static HWND hWndLV = NULL;
int const colNum = 3;
int const textMaxLen = 20;

//WCHAR szFile[MAX_PATH];

struct sample
{
	LPWSTR name;
} oneSong;
//std::list<sample> songList;
std::list<char *> songList;
struct
{
	char ID[3];
	char Title[30];
	char Artist[30];
	char Album[30];
	char Year[4];
	char Comment[30];
	char Genre;
}TAG;
int itemsCount;
bool pauseFlag = false;
int selectedItemIndex;

HCHANNEL channel;
HSTREAM stream, streamTemp;
HSAMPLE samp;
// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
HWND WINAPI         CreateTrackbar(HWND, UINT, UINT); 
HWND                CreateListView(HWND, UINT);
int                 SetListViewColumns(HWND, int, int, char**);
BOOL WINAPI         AddListViewItems(HWND, int, int, char**);


using namespace std;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: разместите код здесь.

    // Инициализация глобальных строк
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_MUSICORGANIZER, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);


	if (HIWORD(BASS_GetVersion()) != BASSVERSION) {
		MessageBox(NULL, TEXT("Ошибка версии BASS."), NULL, 0);
		return 1;
	}
	if (!BASS_Init(-1, 22050, BASS_DEVICE_3D, 0, NULL)) {
		MessageBox(NULL, TEXT("Не удалось инициализировать BASS."), NULL, 0);
		return 1;
	}

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MUSICORGANIZER));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MUSICORGANIZER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_MUSICORGANIZER);
    wcex.lpszClassName  = (LPWSTR)szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить дескриптор экземпляра в глобальной переменной

   HWND hWnd;

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPED | WS_BORDER | WS_SYSMENU,
      CW_USEDEFAULT, 0, 875, 370, NULL, NULL, hInstance, NULL);

   hwndBtnPlay = CreateWindow(TEXT("BUTTON"), TEXT("Play"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 10, 50, 35, hWnd, (HMENU)ID_BUTTONPLAY,(HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);      
   EnableWindow(hwndBtnPlay, FALSE);
   hwndBtnPause = CreateWindow(TEXT("BUTTON"), TEXT("Pause"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 70, 10, 50, 35, hWnd, (HMENU)ID_BUTTONPAUSE, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   EnableWindow(hwndBtnPause, FALSE);
   hwndBtnStop = CreateWindow(TEXT("BUTTON"),TEXT("Stop"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 130, 10, 50, 35, hWnd, (HMENU)ID_BUTTONSTOP, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   EnableWindow(hwndBtnStop, FALSE);
   
   hwndBtnNext = CreateWindow(TEXT("BUTTON"), TEXT("Next"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 190, 10, 50, 35, hWnd, (HMENU)ID_BUTTONADD, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   EnableWindow(hwndBtnNext, FALSE);
   hwndBtnPrev = CreateWindow(TEXT("BUTTON"), TEXT("Prev"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 250, 10, 50, 35, hWnd, (HMENU)ID_BUTTONADD, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   EnableWindow(hwndBtnPrev, FALSE);
   hwndBtnAdd = CreateWindow(TEXT("BUTTON"), TEXT("Add"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 310, 10, 50, 35, hWnd, (HMENU)ID_BUTTONADD, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);

   hwndBtnDelete = CreateWindow(TEXT("BUTTON"),TEXT("Delete"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 370, 10, 50, 35, hWnd, (HMENU)ID_BUTTONDELETE, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   EnableWindow(hwndBtnDelete, FALSE);
   CreateTrackbar(hWnd, 0, 100);

   hwndLabelVol = CreateWindow(TEXT("static"), TEXT("Volume"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 10, 55, 50, 15, hWnd, (HMENU)(501), (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);

   hwndLabelTitle = CreateWindow(TEXT("static"), TEXT("Title[30]"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 80, 55, 52, 17, hWnd, (HMENU)(502), (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   hwndTBTitle = CreateWindow(TEXT("Edit"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 170, 55, 180, 20, hWnd, (HMENU)ID_TEXTBOXTITLE, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), 0);

   hwndLabelArtist = CreateWindow(TEXT("static"), TEXT("Artist[30]"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 80, 85, 60, 17, hWnd, (HMENU)(503), (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   hwndTBArtist = CreateWindow(TEXT("Edit"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 170, 85, 180, 20, hWnd, (HMENU)ID_TEXTBOXARTIST, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), 0);

   hwndLabelAlbum = CreateWindow(TEXT("static"), TEXT("Album[30]"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 80, 115, 65, 17, hWnd, (HMENU)(504), (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   hwndTBAlbum = CreateWindow(TEXT("Edit"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 170, 115, 180, 20, hWnd, (HMENU)ID_TEXTBOXALBUM, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), 0);

   hwndLabelYear = CreateWindow(TEXT("static"), TEXT("Year[4]"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 80, 145, 52, 17, hWnd, (HMENU)(505), (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   hwndTBYear = CreateWindow(TEXT("Edit"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE, 170, 145, 180, 20, hWnd, (HMENU)ID_TEXTBOXYEAR, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), 0);

   hwndLabelComment = CreateWindow(TEXT("static"), TEXT("Comment[30]"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 80, 175, 85, 17, hWnd, (HMENU)(506), (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   hwndTBComment = CreateWindow(TEXT("Edit"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 170, 175, 180, 20, hWnd, (HMENU)ID_TEXTBOXCOMMENT, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), 0);

   hwndBtnChangeTags = CreateWindow(TEXT("BUTTON"), TEXT("Change tags"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 80, 220, 90, 35, hWnd, (HMENU)ID_BUTTONTAGS, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   EnableWindow(hwndBtnChangeTags, FALSE);

   SendMessage(hwndTBTitle, EM_LIMITTEXT, 30, 0);
   SendMessage(hwndTBArtist, EM_LIMITTEXT, 30, 0);
   SendMessage(hwndTBAlbum, EM_LIMITTEXT, 30, 0);
   SendMessage(hwndTBComment, EM_LIMITTEXT, 30, 0);
   SendMessage(hwndTBYear, EM_LIMITTEXT, 4, 0);

   int const itemNum = 3;
   char* header[colNum] = { "Num", "Title", "Author" };
   
   if ((hWndLV = CreateListView(hWnd, ID_LISTVIEW)) == NULL)
	   MessageBox(NULL, TEXT("Невозможно создать элемент ListView"), TEXT("Ошибка"), MB_OK);
   SetListViewColumns(hWndLV, colNum, textMaxLen, header);
   
   ShowWindow(hWndLV, SW_SHOWDEFAULT);
   
   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;
	char buffer2[1000] = { 0 };
	wchar_t wtext[250] = { 0 };
	switch (message)
	{
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);

		
		switch (wmId)
		{
		case ID_BUTTONPLAY:
		{
			if (!pauseFlag)
			{

				if (songList.empty()) {
					MessageBox(NULL, TEXT("Добавьте песню."), NULL, 0);
					return 1;
				}
				auto it = songList.begin();

				char * nx = *std::next(it, selectedItemIndex);
				//oneSong = *nx;
				BASS_ChannelStop(stream);
				BASS_StreamFree(stream);
				//char buffer2[1000] = { 0 };
//				wcstombs(buffer2, songList.get_allocator , 1000);
				stream = BASS_StreamCreateFile(FALSE, nx, 0, 0, 0);
				if (!stream) {
					MessageBox(NULL, TEXT("Ошибка потока воспроизведения."), NULL, 0);
					return 1;
				}
			}
			BASS_ChannelPlay(stream, FALSE);
			EnableWindow(hwndBtnPlay, FALSE);
			EnableWindow(hwndBtnPause, TRUE);
			EnableWindow(hwndBtnStop, TRUE);
			break;
		}
		case ID_BUTTONPAUSE:
		{
			BASS_ChannelPause(stream);
			EnableWindow(hwndBtnPlay, TRUE);
			EnableWindow(hwndBtnPause, FALSE);
			pauseFlag = true;
			break;
		}
		case ID_BUTTONSTOP:
		{
			BASS_ChannelStop(stream);
			BASS_StreamFree(stream);
			EnableWindow(hwndBtnPlay, TRUE);
			EnableWindow(hwndBtnPause, FALSE);
			EnableWindow(hwndBtnStop, FALSE);
			pauseFlag = false;
			break;
		}
		case ID_BUTTONADD:
		{
			OPENFILENAME  ofn;
			char szFile[260];       // buffer for file name
			HWND hwnd;              // owner window
			HANDLE hf;
			ZeroMemory(&ofn, sizeof(OPENFILENAME));
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = hWnd;
			ofn.lpstrFilter = TEXT("Mp3 файл\0*.mp3\0Все файлы (*.*)\0*.*\0");
			ofn.lpstrFile = (LPWSTR)szFile;
			ofn.nMaxFile = sizeof(szFile);
			ofn.nFilterIndex = 1;
			ofn.lpstrFileTitle = NULL;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = NULL;
			ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
			if (GetOpenFileName(&ofn) == TRUE)
			{
				//LPWSTR a = ofn.lpstrFile;
				wcstombs(buffer2, ofn.lpstrFile, 1000);
				
				char* temp = (char*)malloc(1000 + 1);
				strcpy(temp, buffer2);
				songList.push_back(temp);
		/*		LPWSTR ptr = wtext;

				mbstowcs(wtext, temp, strlen(temp) + 1);//Plus null
				oneSong.name = wtext;
				//tempSong.name = ofn.lpstrFile;// temp;
				songList.push_back(oneSong);*/
			}



			BOOL init = false;
			id_tag mp3tag;
			ZeroMemory(&mp3tag, sizeof mp3tag);
			//char buffer2[500] = {0};
			// First arg is the pointer to destination char, second arg is
			// the pointer to source wchar_t, last arg is the size of char buffer
//			wcstombs(buffer2, oneSong.name, 1000);
			
			if (mp3fi.Init(buffer2))
			{

				init = TRUE;
				// If MP3 is there use ID3Lib ...
				if (mp3fi.bHasV1Tag || mp3fi.bHasV2Tag)
				{
					
					LPWSTR ptr = wtext;
					
					mbstowcs(wtext, mp3fi.szTitle, strlen(mp3fi.szTitle) + 1);//Plus null
					SetWindowText(hwndTBTitle, ptr);
					mbstowcs(wtext, mp3fi.szArtist, strlen(mp3fi.szArtist) + 1);//Plus null
					SetWindowText(hwndTBArtist, ptr);
					mbstowcs(wtext, mp3fi.szAlbum, strlen(mp3fi.szAlbum) + 1);//Plus null
					SetWindowText(hwndTBAlbum, ptr);
					

					if (mp3fi.szYear != NULL) {
						mbstowcs(wtext, mp3fi.szYear, strlen(mp3fi.szYear) + 1);//Plus null
						SetWindowText(hwndTBYear, ptr);
					}
					if (mp3fi.szComment != NULL) {
						mbstowcs(wtext, mp3fi.szComment, strlen(mp3fi.szComment) + 1);//Plus null
						SetWindowText(hwndTBComment, ptr);
					}
					char buf[3];
					char* songNumber = itoa(itemsCount + 1, buf, 10);
					//char buffer[500];
					// First arg is the pointer to destination char, second arg is
					// the pointer to source wchar_t, last arg is the size of char buffer
					//wcstombs(buffer, *TAG.Title, 500);

					char* item[colNum] = { songNumber, mp3fi.szTitle, mp3fi.szArtist };
					AddListViewItems(hWndLV, colNum, textMaxLen, item);
				}
			}
			else
				MessageBox(hWnd, TEXT("Unable to initialize MP3Tag class"), TEXT("Error"), MB_ICONERROR);

			if (init) {
				mp3fi.Free();
				init = FALSE;
			}





			/*LPOFSTRUCT lpOpenStruct;
			HANDLE handletemp = CreateFile(oneSong.name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (handletemp != INVALID_HANDLE_VALUE)
			{
				SetFilePointer(handletemp, -128, NULL, FILE_END);
				if (ReadFile(handletemp, &TAG, 128, NULL, NULL) == 0) {
					MessageBox(NULL, TEXT("Ошибка чтения."), NULL, 0);
					CloseHandle(handletemp);
					return 1;
				}
				wchar_t wtext[20];
				
				/*LPWSTR ptr = wtext;
				mbstowcs(wtext, TAG.Title, strlen(TAG.Title) + 1);//Plus null
				SetWindowText(hwndTBTitle, ptr);
				mbstowcs(wtext, TAG.Artist, strlen(TAG.Artist) + 1);//Plus null
				SetWindowText(hwndTBArtist, ptr);
				mbstowcs(wtext, TAG.Album, strlen(TAG.Album) + 1);//Plus null
				SetWindowText(hwndTBAlbum, ptr);
				mbstowcs(wtext, TAG.Year, strlen(TAG.Year) + 1);//Plus null
				SetWindowText(hwndTBYear, ptr);
				mbstowcs(wtext, TAG.Comment, strlen(TAG.Comment) + 1);//Plus null
				SetWindowText(hwndTBComment, ptr);
				char buf[3];
				char* songNumber = itoa(itemsCount + 1, buf, 10);
				//char buffer[500];
				// First arg is the pointer to destination char, second arg is
				// the pointer to source wchar_t, last arg is the size of char buffer
				//wcstombs(buffer, *TAG.Title, 500);
				
				char* item[colNum] = { songNumber, TAG.Title, TAG.Artist };
				AddListViewItems(hWndLV, colNum, textMaxLen, item);*/
			/*
			}
			CloseHandle(handletemp);*/

			break;
		}
		
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Добавьте сюда любой код прорисовки, использующий HDC...
            EndPaint(hWnd, &ps);
        }
        break;
	case WM_NOTIFY:

		LPNMHDR lpnmHdr;
		lpnmHdr = (LPNMHDR)lParam;
		if ((lpnmHdr->idFrom == ID_LISTVIEW) && (lpnmHdr->code == NM_CLICK))
		{

			selectedItemIndex = ListView_GetNextItem(hWndLV, -1, LVNI_ALL | LVNI_SELECTED);
			if (selectedItemIndex >= 0)
			{

				/*auto it = songList.begin();

				char * nx = *std::next(it, selectedItemIndex);
				//oneSong = *nx;
				LPOFSTRUCT lpOpenStruct;
				HANDLE handletemp = CreateFile(oneSong.name, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if (handletemp != INVALID_HANDLE_VALUE)
				{
					SetFilePointer(handletemp, -128, NULL, FILE_END);
					if (ReadFile(handletemp, &TAG, 128, NULL, NULL) == 0) {
						MessageBox(NULL, TEXT("Ошибка чтения."), NULL, 0);
						CloseHandle(handletemp);
						return 1;
					}
					/*SetWindowText(hwndTBTitle, TAG.Title);
					SetWindowText(hwndTBArtist, TAG.Artist);
					SetWindowText(hwndTBAlbum, TAG.Album);
					SetWindowText(hwndTBYear, TAG.Year);
					SetWindowText(hwndTBComment, TAG.Comment);*/
			
			/*	}
				CloseHandle(handletemp);*/
				EnableWindow(hwndBtnChangeTags, TRUE);
				EnableWindow(hwndBtnPlay, TRUE);
				EnableWindow(hwndBtnDelete, TRUE);
				BASS_ChannelStop(stream);
				BASS_StreamFree(stream);
				pauseFlag = false;
			}
		}
		break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

HWND WINAPI CreateTrackbar(HWND hWnd, UINT iMin, UINT iMax)
{

	hwndTrack = CreateWindowEx(0, TRACKBAR_CLASS, TEXT("Trackbar Control"), WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE | TBS_VERT,
		15, 80, 30, 200, hWnd, (HMENU)ID_TRACKBAR, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	SendMessage(hwndTrack, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(iMin, iMax));
	SendMessage(hwndTrack, TBM_SETPAGESIZE, 0, (LPARAM)2);


	SetFocus(hwndTrack);

	return hwndTrack;

}
int SetListViewColumns(HWND hWndLV, int colNum, int textMaxLen, char** header)
{
	RECT rcl;
	GetClientRect(hWndLV, &rcl);

	int index = -1;

	LVCOLUMN lvc;
	lvc.mask = LVCF_TEXT | LVCF_WIDTH;
	//lvc.cx = 495 / colNum;
	lvc.cchTextMax = textMaxLen;
	wchar_t wtext[15];
	LPWSTR ptr = wtext;

	
	for (int i = 0; i < colNum; i++)
	{
		mbstowcs(wtext, header[i], strlen(header[i]) + 1);//Plus null
		lvc.pszText = wtext;// header[i];
		index = ListView_InsertColumn(hWndLV, i, &lvc);
		
		if (index == -1)
			break;
	}
	ListView_SetColumnWidth(hWndLV, 0, 50);
	ListView_SetColumnWidth(hWndLV, 1, 180);
	ListView_SetColumnWidth(hWndLV, 2, 180);
	return index;

}

HWND CreateListView(HWND hWndParent, UINT uId)
{
	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);

	RECT rcl;
	GetClientRect(hWndParent, &rcl);
	HWND hWndLV = CreateWindow(WC_LISTVIEW, TEXT("HALLO"), WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | LBS_NOTIFY | LVS_REPORT | WS_CLIPSIBLINGS | LVS_AUTOARRANGE | LVS_SHOWSELALWAYS, 450, 0, 500, rcl.bottom - rcl.top, hWndParent, (HMENU)uId, GetModuleHandle(NULL), NULL);


	ListView_SetExtendedListViewStyleEx(hWndLV, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_ONECLICKACTIVATE);

	return (hWndLV);
}

BOOL WINAPI AddListViewItems(HWND hWndLV, int colNum, int textMaxLen, char** item)
{
	int iLastIndex = ListView_GetItemCount(hWndLV);



	LVITEM lvi;
	lvi.mask = LVIF_TEXT;
	lvi.cchTextMax = textMaxLen;
	lvi.iItem = iLastIndex;

	wchar_t wtext[20];
	mbstowcs(wtext, item[0], strlen(item[0]) + 1);//Plus null
	LPWSTR ptr[3];
	ptr[0]= wtext;

	lvi.pszText = ptr[0];// item[0];
	lvi.iSubItem = 0;

	if (ListView_InsertItem(hWndLV, &lvi) == -1)
		return FALSE;
	for (int i = 1; i < colNum; i++)
	{
		mbstowcs(wtext, item[i], strlen(item[i]) + 1);//Plus null
		ptr[i] = wtext;
		ListView_SetItemText(hWndLV, iLastIndex, i, ptr[i]/*item[i]*/);
	}

	itemsCount = iLastIndex + 1;
	return TRUE;

}