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
#include <cstddef>
#include <vector>
#pragma comment(lib,"shell32.lib")

#define ID3LIB_LINKOPTION 3
#include "id3.h"
#include "id3/id3lib_streams.h"
#include "id3/tag.h"

#include "id3/globals.h"
#include "id3/field.h"
#include "id3/helpers.h"
#include "id3/id3lib_frame.h"
#include "id3/id3lib_strings.h"
#include "id3/io_decorators.h"
#include "id3/io_helpers.h"
#include "id3/io_strings.h"
//#include "id3/misc_support.h"
#include "id3/reader.h"
#include "id3/readers.h"
#include "id3/sized_types.h"
#include "id3/utils.h"
#include "id3/writer.h"
#include "id3/writers.h"

#include "MP3FileInfo.h"


#define ID_PIC                          1002
#define IDB_BITMAP1                     104

#pragma comment(lib,"bass.lib")
#pragma comment(lib, "ComCtl32.Lib")
#pragma comment(lib,"id3lib.lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' \
                        name='Microsoft.Windows.Common-Controls' \
                        version='6.0.0.0' processorArchitecture='*'\
 publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma warning(disable : 4996)
MP3FileInfo mp3fi;
struct id_tag {
	char title[256];
	char album[256];
	char artist[256];
	char year[5];
	char comment[256];
};
#define MAX_LOADSTRING 100
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
// Глобальные переменные:

HBRUSH hbrush;

BITMAP bm;
HDC memBit;


HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна

HWND hwndBtnPlay, hwndBtnPause, hwndBtnStop, hwndBtnDelete, hwndTrack, hwndLabelVol, hwndTBTitle, hwndBtnAdd, hwndBtnPrev, hwndBtnNext, hwndBtnOpenClose, hwndBtnSearch;
HWND hwndTBArtist, hwndTBAlbum, hwndTBYear, hwndTBComment, hwndLabelTitle, hwndLabelArtist, hwndLabelAlbum, hwndTBSearch, hwndLabelSongNumb;
HWND hwndLabelYear, hwndLabelComment, hwndBtnChangeTags;
HWND hwndRBSinger, hwndRBAlbum;
static HWND hWndLV = NULL;
int const colNum = 4;
int const textMaxLen = 20;
BOOL openFlag = true;
struct sample
{
	LPWSTR name;
} oneSong;
std::list<char *> songList;
std::list<char *> tempSongList;
int itemsCount;
bool pauseFlag = false;
int selectedItemIndex;
HINSTANCE hIns;
ULONG_PTR gdiplusToken; // GDI Token

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
LPWSTR				MBStoWS(char *);
char*				WStoMBS(LPWSTR);
void				AddMenus(HWND);
VOID WINAPI         UpdateListViewItem(HWND, int, char**);
VOID WINAPI			UpdateEditBoxes();
VOID WINAPI			ClearEditBoxes();


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
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW);
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
      CW_USEDEFAULT, 0, 1025, 340, NULL, NULL, hInstance, NULL);
   hwndBtnPlay = CreateWindow(TEXT("BUTTON"), TEXT("Play"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 10, 50, 35, hWnd, (HMENU)ID_BUTTONPLAY, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   EnableWindow(hwndBtnPlay, FALSE);
   hwndBtnPause = CreateWindow(TEXT("BUTTON"), TEXT("Pause"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 70, 10, 50, 35, hWnd, (HMENU)ID_BUTTONPAUSE, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   EnableWindow(hwndBtnPause, FALSE);
   hwndBtnStop = CreateWindow(TEXT("BUTTON"), TEXT("Stop"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 130, 10, 50, 35, hWnd, (HMENU)ID_BUTTONSTOP, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   EnableWindow(hwndBtnStop, FALSE);

   hwndBtnNext = CreateWindow(TEXT("BUTTON"), TEXT("Next"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 250, 10, 50, 35, hWnd, (HMENU)ID_BUTTONNEXT, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   EnableWindow(hwndBtnNext, FALSE);
   hwndBtnPrev = CreateWindow(TEXT("BUTTON"), TEXT("Prev"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 190, 10, 50, 35, hWnd, (HMENU)ID_BUTTONPREV, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   EnableWindow(hwndBtnPrev, FALSE);
   hwndBtnAdd = CreateWindow(TEXT("BUTTON"), TEXT("Add"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 310, 10, 50, 35, hWnd, (HMENU)ID_BUTTONADD, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);

   hwndBtnDelete = CreateWindow(TEXT("BUTTON"), TEXT("Delete"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 370, 10, 50, 35, hWnd, (HMENU)ID_BUTTONDELETE, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   EnableWindow(hwndBtnDelete, FALSE);
  
   hwndBtnOpenClose = CreateWindow(TEXT("BUTTON"), TEXT("Close"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 370, 55, 50, 25, hWnd, (HMENU)ID_BUTTONCLOP, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);

   CreateTrackbar(hWnd, 0, 100);

   hwndLabelSongNumb = CreateWindow(TEXT("static"), TEXT("0/0"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 320, 60, 50, 15, hWnd, (HMENU)(501), (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);


   hwndLabelVol = CreateWindow(TEXT("static"), TEXT("Volume"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 10, 55, 50, 15, hWnd, (HMENU)(501), (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);

   hwndLabelTitle = CreateWindow(TEXT("static"), TEXT("Title [30]"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 10, 85, 57, 17, hWnd, (HMENU)(502), (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   hwndTBTitle = CreateWindow(TEXT("Edit"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 110, 85, 310, 20, hWnd, (HMENU)ID_TEXTBOXTITLE, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), 0);

   hwndLabelArtist = CreateWindow(TEXT("static"), TEXT("Artist [30]"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 10, 115, 65, 17, hWnd, (HMENU)(503), (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   hwndTBArtist = CreateWindow(TEXT("Edit"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 110, 115, 310, 20, hWnd, (HMENU)ID_TEXTBOXARTIST, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), 0);

   hwndLabelAlbum = CreateWindow(TEXT("static"), TEXT("Album [30]"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 10, 145, 70, 17, hWnd, (HMENU)(504), (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   hwndTBAlbum = CreateWindow(TEXT("Edit"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 110, 145, 310, 20, hWnd, (HMENU)ID_TEXTBOXALBUM, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), 0);

   hwndLabelYear = CreateWindow(TEXT("static"), TEXT("Year [4]"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 10, 175, 52, 17, hWnd, (HMENU)(505), (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   hwndTBYear = CreateWindow(TEXT("Edit"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE, 110, 175, 310, 20, hWnd, (HMENU)ID_TEXTBOXYEAR, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), 0);

   hwndLabelComment = CreateWindow(TEXT("static"), TEXT("Comment [30]"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 10, 205, 90, 17, hWnd, (HMENU)(506), (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   hwndTBComment = CreateWindow(TEXT("Edit"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 110, 205, 310, 20, hWnd, (HMENU)ID_TEXTBOXCOMMENT, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), 0);

   hwndBtnChangeTags = CreateWindow(TEXT("BUTTON"), TEXT("Change tags"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 235, 90, 35, hWnd, (HMENU)ID_BUTTONTAGS, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   EnableWindow(hwndBtnChangeTags, FALSE);

   hwndBtnSearch = CreateWindow(TEXT("BUTTON"), TEXT("Filter"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 330, 235, 90, 35, hWnd, (HMENU)ID_BUTTONSEARCH, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   hwndTBSearch = CreateWindow(TEXT("Edit"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 150, 250, 170, 20, hWnd, (HMENU)ID_TEXTBOXTITLE, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), 0);
   EnableWindow(hwndBtnSearch, FALSE);
   hwndRBSinger = CreateWindow(TEXT("BUTTON"), TEXT("By singer"), WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON, 150, 230, 80, 18, hWnd, (HMENU)ID_RADBUTSINGER, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
   hwndRBAlbum = CreateWindow(TEXT("BUTTON"), TEXT("By album"), WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON, 240, 230, 80, 18, hWnd, (HMENU)ID_RADBUTALBUM, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);

   SendMessage(hwndTBTitle, EM_LIMITTEXT, 30, 0);
   SendMessage(hwndTBArtist, EM_LIMITTEXT, 30, 0);
   SendMessage(hwndTBAlbum, EM_LIMITTEXT, 30, 0);
   SendMessage(hwndTBComment, EM_LIMITTEXT, 30, 0);
   SendMessage(hwndTBYear, EM_LIMITTEXT, 4, 0);

   int const itemNum = 3;
   char* header[colNum] = { "Title", "Singer", "Name", "Album" };
   
   if ((hWndLV = CreateListView(hWnd, ID_LISTVIEW)) == NULL)
	   MessageBox(NULL, TEXT("Невозможно создать элемент ListView"), TEXT("Ошибка"), MB_OK);
   SetListViewColumns(hWndLV, colNum, textMaxLen, header);
   AddMenus(hWnd);
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
	HDC hdc = NULL;
	HBITMAP hBitmap;
	char buffer2[1000] = { 0 };
	wchar_t wtext[250] = { 0 };
	switch (message)
	{
	case WM_CREATE:
	{
		BASS_SetVolume(100);
		break;
	}
	case WM_COMMAND:
		wmId = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId)
		{
		case ID_EXIT:
		{
			PostMessage(hWnd, WM_QUIT, 0, 0);
		}
		case ID_INFOBUTTON:
		{
			MessageBoxA(hWnd, "Creator: Martseniuk Roman\nYear:2017\nThis application has basic functions to play music, filter playlist and change id3 tags.", "About", MB_OK | MB_ICONINFORMATION);
			break;
		}
		case ID_HELPBUTTON:
		{
			MessageBoxA(hWnd, "Button assignment:\nPlay - play song\nPause - make pause\nStop - stop song\nNext - play next song\nPrev - play previous song\nAdd - add song to playlist(you can add many songs at once)\nDelete - delete one song from playlist\nClose\Open - open ore closee additional panel\nChange tags - set id3 tags for song, that is sounds right now\nFilter - filter songs in playlist by input information\nTo change id3 tags you need to input some information to special boxes and press\"Change tags\".\nYou can play music by choosing it in special list which is situated on the right part of the window.\nYou can filter you playlist by singer and album. You need to choose one, input text in box below and press \"Filter\".", "Help", MB_OK | MB_ICONINFORMATION);
			break;
		}
		case ID_BUTTONPLAY:
		{
			if (!pauseFlag)
			{
				if (songList.empty()) {
					MessageBox(NULL, TEXT("Please, add song to list."), NULL, 0);
					return 1;
				}
				auto it = songList.begin();
				char * nx = *std::next(it, selectedItemIndex);
				BASS_ChannelStop(stream);
				BASS_StreamFree(stream);
				stream = BASS_StreamCreateFile(FALSE, nx, 0, 0, 0);
				if (!stream) {
					MessageBox(NULL, TEXT("Error."), NULL, 0);
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
		case ID_BUTTONNEXT:
		{
			BASS_ChannelStop(stream);
			BASS_StreamFree(stream);
			if (selectedItemIndex<itemsCount-1)
			selectedItemIndex++;
			pauseFlag = false;
			UpdateEditBoxes();
			SendMessage(hWnd, WM_COMMAND, ID_BUTTONPLAY, 0);
			
			ListView_SetItemState(hWndLV, selectedItemIndex, LVIS_SELECTED, LVIS_SELECTED);
			ListView_SetItemState(hWndLV, selectedItemIndex - 1, LVIF_STATE, LVIS_SELECTED);
			wchar_t resultString[256];
			swprintf_s(resultString, L"%d/%d", selectedItemIndex+1, songList.size());
			SendMessage(hwndLabelSongNumb, WM_SETTEXT, 0, (LPARAM)resultString);
			break;
		}
		case ID_BUTTONPREV:
		{
			BASS_ChannelStop(stream);
			BASS_StreamFree(stream);
			if (selectedItemIndex>0)
			selectedItemIndex--;
			pauseFlag = false;
			UpdateEditBoxes();
			SendMessage(hWnd, WM_COMMAND, ID_BUTTONPLAY, 0);
			ListView_SetItemState(hWndLV, selectedItemIndex, LVIS_SELECTED, LVIS_SELECTED);
			ListView_SetItemState(hWndLV, selectedItemIndex + 1, LVIF_STATE, LVIS_SELECTED);
			wchar_t resultString[256];
			swprintf_s(resultString, L"%d/%d", selectedItemIndex+1, songList.size());
			SendMessage(hwndLabelSongNumb, WM_SETTEXT, 0, (LPARAM)resultString);
			break;
		}
		case ID_BUTTONDELETE:
		{
			auto it = songList.begin();

			char * nx = *std::next(it, selectedItemIndex);
			songList.remove(nx);
			if (selectedItemIndex != -1)
			{
			
			ListView_DeleteItem(hWndLV, selectedItemIndex);
			itemsCount--;
			SendMessage(hWnd, WM_COMMAND, ID_BUTTONSTOP, 0);
		}
			SetWindowText(hwndTBTitle, TEXT(""));
			SetWindowText(hwndTBArtist, TEXT(""));
			SetWindowText(hwndTBAlbum, TEXT(""));
			SetWindowText(hwndTBYear, TEXT(""));
			SetWindowText(hwndTBComment, TEXT(""));
			break;
		}
		case ID_BUTTONADD:
		{
			OPENFILENAME  ofn = { sizeof ofn };
			wchar_t szFile[1024];       // buffer for file name
			HWND hwnd;              // owner window
			HANDLE hf;
			szFile[0] = '\0';
			ofn.lpstrFile = szFile;
			ofn.nMaxFile = 1024;
			ofn.hwndOwner = hWnd;
			ofn.lpstrFilter = TEXT("Mp3 файл\0*.mp3\0Все файлы (*.*)\0*.*\0");
			ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT;
			std::vector<wchar_t*> files;
			char* temp;
			if (GetOpenFileName(&ofn) == TRUE)
			{
				wchar_t* ptr = ofn.lpstrFile;
				wchar_t* dirName = ofn.lpstrFile;
				ptr[ofn.nFileOffset - 1] = 0;
				ptr += ofn.nFileOffset;
				while (*ptr)
				{
					files.push_back(ptr);
					ptr += (lstrlenW(ptr) + 1);
				}
				for (int i = 0; i < files.size(); i++)
				{

					temp = (char*)malloc(1000 + 1);
					strcpy(temp, WStoMBS(dirName));
					strcat(temp, "\\");
					strcat(temp, WStoMBS(files[i]));
					//songList.push_back(temp);
					tempSongList.push_back(temp);
					BOOL init = false;
					id_tag mp3tag;
					ZeroMemory(&mp3tag, sizeof mp3tag);

					if (mp3fi.Init(temp))
					{
						ClearEditBoxes();
						init = TRUE;
						if (mp3fi.bHasV1Tag || mp3fi.bHasV2Tag)
						{
							LPWSTR ptr = wtext;

							if (mp3fi.szTitle != NULL)
								SetWindowText(hwndTBTitle, MBStoWS(mp3fi.szTitle));

							if (mp3fi.szArtist != NULL)
								SetWindowText(hwndTBArtist, MBStoWS(mp3fi.szArtist));

							if (mp3fi.szAlbum != NULL)
								SetWindowText(hwndTBAlbum, MBStoWS(mp3fi.szAlbum));

							if (mp3fi.szYear != NULL)
								SetWindowText(hwndTBYear, MBStoWS(mp3fi.szYear));

							if (mp3fi.szComment != NULL)
								SetWindowText(hwndTBComment, MBStoWS(mp3fi.szComment));

							char* item[colNum] = { mp3fi.szTitle, mp3fi.szArtist,mp3fi.szFilename, mp3fi.szAlbum };
							AddListViewItems(hWndLV, colNum, textMaxLen, item);

						}
						else
						{
							char* item[colNum] = { NULL, NULL,mp3fi.szFilename, NULL };
							AddListViewItems(hWndLV, colNum, textMaxLen, item);
						}
					}
					else
					{
						MessageBox(hWnd, TEXT("Unable to initialize MP3Tag class"), TEXT("Error"), MB_ICONERROR);
						char* item[colNum] = { NULL, NULL,NULL,NULL };
						AddListViewItems(hWndLV, colNum, textMaxLen, item);
					}
					if (init) {
						mp3fi.Free();
						init = FALSE;
					}
				}
				songList.clear();
				for (int i = 0; i < tempSongList.size(); i++)
				{
					auto it = tempSongList.begin();
					char * tempElement = *std::next(it, i);
					songList.push_back(tempElement);
				}
				wchar_t resultString[256];
				swprintf_s(resultString, L"%d/%d", selectedItemIndex + 1, tempSongList.size());
				SendMessage(hwndLabelSongNumb, WM_SETTEXT, 0, (LPARAM)resultString);
			}
			break;
		}
		case ID_BUTTONTAGS:
		{
			auto it = songList.begin();
			char * nx = *std::next(it, selectedItemIndex);
			LPWSTR ptr = wtext;

				char title[30];
				GetWindowText(hwndTBTitle, ptr, 30);
				strcpy(title, WStoMBS(ptr));

				char artist[30];
				GetWindowText(hwndTBArtist, ptr, 30);
				strcpy(artist, WStoMBS(ptr));

				char album[30];
				GetWindowText(hwndTBAlbum, ptr, 30);
				strcpy(album, WStoMBS(ptr));

				char year[5];
				GetWindowText(hwndTBYear, ptr, 5);
				strcpy(year, WStoMBS(ptr));

				char comment[30];
				GetWindowText(hwndTBComment, ptr, 30);
				strcpy(comment, WStoMBS(ptr));
			
				char* item[colNum] = { title, artist, nx, album };
				UpdateListViewItem(hWndLV, colNum, item);

				
			
			
			id_tag mp3tag;
			ZeroMemory(&mp3tag, sizeof mp3tag);
			mp3fi.Change(nx, title, album, artist, year, comment);

			EnableWindow(hwndBtnChangeTags, FALSE);
			UpdateWindow(hWndLV);
			break;
		}
		case ID_BUTTONCLOP:
		{
			RECT rect;
			int width;
			GetWindowRect(hWnd, &rect);
			if (openFlag)
			{
				width = 450;
				SendMessage(hwndBtnOpenClose, WM_SETTEXT, 0, (LPARAM)TEXT("Open"));
			}
			else
			{
				width = 1025;
				SendMessage(hwndBtnOpenClose, WM_SETTEXT, 0, (LPARAM)TEXT("Close"));
			}
			openFlag = !openFlag;
			MoveWindow(hWnd, rect.left,rect.top, width, 340, TRUE);
			break;
		}
		case ID_BUTTONSEARCH:
		{
			LPWSTR ptr = wtext;
			char searchString[30];
			GetWindowText(hwndTBSearch, ptr, 30);
			if (!wcscmp(ptr, TEXT("")))
			{
				BOOL init = false;
				id_tag mp3tag;
				wchar_t wtext[250] = { 0 };
				ZeroMemory(&mp3tag, sizeof mp3tag);
				ListView_DeleteAllItems(hWndLV);
				songList.clear();
				for (int i = 0; i < tempSongList.size(); i++)
				{
					auto it = tempSongList.begin();
					char * tempElement = *std::next(it, i);
					songList.push_back(tempElement);

					if (mp3fi.Init(tempElement))
					{
						if (mp3fi.bHasV1Tag || mp3fi.bHasV2Tag)
						{
							char* item[colNum] = { mp3fi.szTitle, mp3fi.szArtist,mp3fi.szFilename,mp3fi.szAlbum };
							AddListViewItems(hWndLV, colNum, textMaxLen, item);
						}
						else
						{
							char* item[colNum] = { NULL, NULL,mp3fi.szFilename ,NULL };
							AddListViewItems(hWndLV, colNum, textMaxLen, item);
						}
					}
				}

				itemsCount = songList.size();
				wchar_t resultString[256];
				swprintf_s(resultString, L"%d/%d", selectedItemIndex + 1, tempSongList.size());
				SendMessage(hwndLabelSongNumb, WM_SETTEXT, 0, (LPARAM)resultString);
			}
			else
			{
				BOOL init = false;
				id_tag mp3tag;
				wchar_t wtext[250] = { 0 };
				ZeroMemory(&mp3tag, sizeof mp3tag);

				strcpy(searchString, WStoMBS(ptr));

				std::vector<int> items;
				songList.clear();
				ListView_DeleteAllItems(hWndLV);


				for (int i = 0; i < tempSongList.size(); i++)
				{
					auto it = tempSongList.begin();
					char * tempElement = *std::next(it, i);
					songList.push_back(tempElement);
					if (mp3fi.Init(tempElement))
					{
						if (mp3fi.bHasV1Tag || mp3fi.bHasV2Tag)
						{
							char* item[colNum] = { mp3fi.szTitle, mp3fi.szArtist,mp3fi.szFilename ,mp3fi.szAlbum };
							AddListViewItems(hWndLV, colNum, textMaxLen, item);
						}
						else
						{
							char* item[colNum] = { NULL, NULL,mp3fi.szFilename ,NULL };
							AddListViewItems(hWndLV, colNum, textMaxLen, item);
						}
					}
				}
				int indexOfColoumn;
				if (SendMessage(hwndRBSinger, BM_GETCHECK, 0, 0))
					indexOfColoumn = 1;
				else
					indexOfColoumn = 3;

				for (int i = 0; i < tempSongList.size(); i++)
				{
					std::vector<wchar_t> bufText(256);
					ListView_GetItemText(hWndLV, i, indexOfColoumn, &bufText[0], 256);
					wchar_t* buf = reinterpret_cast<wchar_t*>(bufText.data());
					if (wcsicmp(buf, ptr))
						items.push_back(i);
				}
				
				for (int i = items.size() - 1; i >= 0; i--)
				{
					std::list<char *>::iterator it = songList.begin();
					std::advance(it, items[i]);
					songList.erase(it);
					ListView_DeleteItem(hWndLV, items[i]);

				}
				itemsCount = songList.size();
				selectedItemIndex = -1;
				wchar_t resultString[256];
				
				swprintf_s(resultString, L"%d/%d", selectedItemIndex + 1, songList.size());
				SendMessage(hwndLabelSongNumb, WM_SETTEXT, 0, (LPARAM)resultString);
				
				SendMessage(hWnd, WM_COMMAND, ID_BUTTONSTOP, 0);
				EnableWindow(hwndBtnChangeTags, FALSE);
				EnableWindow(hwndBtnNext, FALSE);
				EnableWindow(hwndBtnPlay, FALSE);
				EnableWindow(hwndBtnPrev, FALSE);
				//EnableWindow(hwndBtnChangeTags, FALSE);
			}
			
			break;
		}
		case ID_RADBUTSINGER:
		{
			if (!SendMessage(hwndRBSinger, BM_GETCHECK, 0, 0))
			{
				SendMessage(hwndRBSinger, BM_SETCHECK, BST_CHECKED, 0);
				SendMessage(hwndRBAlbum, BM_SETCHECK, BST_UNCHECKED, 0);
				EnableWindow(hwndBtnSearch, TRUE);
			}
			else
			{
				SendMessage(hwndRBSinger, BM_SETCHECK, BST_UNCHECKED, 0);
				EnableWindow(hwndBtnSearch, FALSE);
			}
			break;
		}
		case ID_RADBUTALBUM:
		{
			if (!SendMessage(hwndRBAlbum, BM_GETCHECK, 0, 0))
			{
				SendMessage(hwndRBAlbum, BM_SETCHECK, BST_CHECKED, 0);
				SendMessage(hwndRBSinger, BM_SETCHECK, BST_UNCHECKED, 0);
				EnableWindow(hwndBtnSearch, TRUE);
			}
			else
			{
				SendMessage(hwndRBAlbum, BM_SETCHECK, BST_UNCHECKED, 0);
				EnableWindow(hwndBtnSearch, FALSE);
			}
			break;
		}
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_HSCROLL:
			switch (LOWORD(wParam)) {
			case TB_THUMBTRACK:
			case TB_THUMBPOSITION:
				if (hwndTrack == (HWND)lParam) {
					float dwPos = SendMessage(hwndTrack, TBM_GETPOS, 0, 100);
					float volume = 1 - ((100-dwPos) * 0.01);
					BASS_SetVolume(volume);
				}
				break;
			}
			break;
	case WM_DROPFILES: // This message is sent when we drop a file
	{
		SendMessage(hWnd, WM_COMMAND, ID_BUTTONADD, NULL);
		break;
	}
	case WM_NOTIFY:

		LPNMHDR lpnmHdr;
		lpnmHdr = (LPNMHDR)lParam;
		if ((lpnmHdr->idFrom == ID_LISTVIEW) && (lpnmHdr->code == NM_CLICK))
		{

			selectedItemIndex = ListView_GetNextItem(hWndLV, -1, LVNI_ALL | LVNI_SELECTED);
			if (selectedItemIndex >= 0)
			{
				UpdateEditBoxes();
				EnableWindow(hwndBtnChangeTags, TRUE);
				EnableWindow(hwndBtnPlay, TRUE);
				EnableWindow(hwndBtnNext, TRUE);
				EnableWindow(hwndBtnPrev, TRUE);
				EnableWindow(hwndBtnDelete, TRUE);
				BASS_ChannelStop(stream);
				BASS_StreamFree(stream);
				pauseFlag = false;
				SendMessage(hWnd, WM_COMMAND, ID_BUTTONPLAY, 0);
				pauseFlag = false;
				wchar_t resultString[256];
				swprintf_s(resultString, L"%d/%d", selectedItemIndex + 1, songList.size());
				SendMessage(hwndLabelSongNumb, WM_SETTEXT, 0, (LPARAM)resultString);
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

	hwndTrack = CreateWindowEx(0, TRACKBAR_CLASS, TEXT("Trackbar Control"), WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE | TBS_HORZ,
		110, 55, 200, 30, hWnd, (HMENU)ID_TRACKBAR, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	SendMessage(hwndTrack, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(iMin, iMax));
	SendMessage(hwndTrack, TBM_SETPAGESIZE, 0, (LPARAM)2);
	SendMessage(hwndTrack, TBM_SETPOS, 1, 100);

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
	lvc.cchTextMax = textMaxLen;

	
	for (int i = 0; i < colNum; i++)
	{
		lvc.pszText = MBStoWS(header[i]);
		index = ListView_InsertColumn(hWndLV, i, &lvc);
		
		if (index == -1)
			break;
	}
	ListView_SetColumnWidth(hWndLV, 0, 140);
	ListView_SetColumnWidth(hWndLV, 1, 140);
	ListView_SetColumnWidth(hWndLV, 2, 140);
	ListView_SetColumnWidth(hWndLV, 3, 140);
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
	HWND hWndLV = CreateWindow(WC_LISTVIEW, TEXT("HALLO"), WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | LBS_NOTIFY | LVS_REPORT | WS_CLIPSIBLINGS | LVS_AUTOARRANGE | LVS_SHOWSELALWAYS, 450, 0, 600, rcl.bottom - rcl.top, hWndParent, (HMENU)uId, GetModuleHandle(NULL), NULL);


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

	lvi.pszText = NULL;
	lvi.iSubItem = 0;

	if (ListView_InsertItem(hWndLV, &lvi) == -1)
		return FALSE;
	for (int i = 0; i < colNum; i++)
	{
		
		if (i == 2 && item[i] != NULL)
		{
			char *ssc;
			int l = 0;
			ssc = strstr(item[i], "\\");
			do {
				l = strlen(ssc) + 1;
				item[i] = &item[i][strlen(item[i]) - l + 2];
				ssc = strstr(item[i], "\\");
			} while (ssc);
		}
		if (item[i] == NULL)
			item[i] = "none";
		ListView_SetItemText(hWndLV, iLastIndex, i, MBStoWS(item[i]));
	}
	
	itemsCount = iLastIndex + 1;
	return TRUE;

}
VOID WINAPI UpdateListViewItem(HWND hWndLV, int colNum, char** item)
{
	for (int i = 0; i < colNum; i++)
	{
		if (!strcmp(item[i],""))
			item[i] = "none";
		if (i == 2 && strcmp(item[i], ""))
		{
			char *ssc;
			int l = 0;
			ssc = strstr(item[i], "\\");
			do {
				l = strlen(ssc) + 1;
				item[i] = &item[i][strlen(item[i]) - l + 2];
				ssc = strstr(item[i], "\\");
			} while (ssc);
		}
		LPWSTR dest = new WCHAR[MAX_PATH];
		const int codePage = CP_ACP; 
		int alen = MultiByteToWideChar(codePage, 0, item[i], -1, NULL, 0);
		MultiByteToWideChar(codePage, 0, item[i], -1, dest, alen);
		ListView_SetItemText(hWndLV, selectedItemIndex, i, dest);
		}
}

VOID WINAPI UpdateEditBoxes()
{
	BOOL init = false;
	id_tag mp3tag;
	wchar_t wtext[250] = { 0 };
	ZeroMemory(&mp3tag, sizeof mp3tag);

	auto it = songList.begin();

	char * nx = *std::next(it, selectedItemIndex);
	if (mp3fi.Init(nx))
	{
		ClearEditBoxes();
		init = TRUE;
		if (mp3fi.bHasV1Tag || mp3fi.bHasV2Tag)
		{

			LPWSTR ptr = wtext;

			if (mp3fi.szTitle != NULL) 
				SetWindowText(hwndTBTitle, MBStoWS(mp3fi.szTitle));
			
			if (mp3fi.szArtist != NULL) 
				SetWindowText(hwndTBArtist, MBStoWS(mp3fi.szArtist));
			
			if (mp3fi.szAlbum != NULL) 
				SetWindowText(hwndTBAlbum, MBStoWS(mp3fi.szAlbum));
			

			if (mp3fi.szYear != NULL) 
				SetWindowText(hwndTBYear, MBStoWS(mp3fi.szYear));
			
			if (mp3fi.szComment != NULL) 
				SetWindowText(hwndTBComment, MBStoWS(mp3fi.szComment));
			

			//	char* item[colNum] = { mp3fi.szTitle, mp3fi.szArtist,mp3fi.szFilename };
			//	AddListViewItems(hWndLV, colNum, textMaxLen, item);
		}
		else
		{
			//char* item[colNum] = { NULL, NULL,mp3fi.szFilename };
			//AddListViewItems(hWndLV, colNum, textMaxLen, item);
		}
	}
	else
	{
		//MessageBox(hWnd, TEXT("Unable to initialize MP3Tag class"), TEXT("Error"), MB_ICONERROR);
		//char* item[colNum] = { NULL, NULL,mp3fi.szFilename };
		//AddListViewItems(hWndLV, colNum, textMaxLen, item);
	}

	if (init) {
		mp3fi.Free();
		init = FALSE;
	}
}

VOID WINAPI ClearEditBoxes()
{
	SetWindowText(hwndTBTitle, TEXT(""));
	SetWindowText(hwndTBArtist, TEXT(""));
	SetWindowText(hwndTBAlbum, TEXT(""));
	SetWindowText(hwndTBYear, TEXT(""));
	SetWindowText(hwndTBComment, TEXT(""));
}

char* WStoMBS(LPWSTR source)
{
	char buffer2[MAX_PATH] = { 0 }; 
	size_t sizeRequired = WideCharToMultiByte(950, 0, source, -1,
		NULL, 0, NULL, NULL);
	sizeRequired = WideCharToMultiByte(CP_ACP,                // ANSI code page
		WC_COMPOSITECHECK,     // Check for accented characters
		source,         // Source Unicode string
		-1,                    // -1 means string is zero-terminated
		buffer2,          // Destination char string
		sizeof(buffer2),  // Size of buffer
		NULL,                  // No default character
		NULL);
	return buffer2;
}

LPWSTR MBStoWS(char * source)
{
	LPWSTR dest = new WCHAR[MAX_PATH];
	const int codePage = CP_ACP;
	int alen = MultiByteToWideChar(codePage, 0, source, -1, NULL, 0);
	MultiByteToWideChar(codePage, 0, source, -1, dest, alen);
	return dest;
}

void AddMenus(HWND hwnd) {

	HMENU hMenubar;
	HMENU hFile;
	HMENU hHelp;
	hMenubar = CreateMenu();
	hFile = CreateMenu();
	hHelp = CreateMenu();
	AppendMenuW(hFile, MF_STRING, ID_BUTTONADD, L"&Add song");
	AppendMenuW(hFile, MF_SEPARATOR, 0, NULL);
	AppendMenuW(hFile, MF_STRING, ID_EXIT, L"&Quit");

	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hFile, L"&File");

	AppendMenuW(hHelp, MF_STRING, ID_HELPBUTTON, L"&Help");
	AppendMenuW(hHelp, MF_STRING, ID_INFOBUTTON, L"&About");
	AppendMenuW(hMenubar, MF_POPUP, (UINT_PTR)hHelp, L"&Info");
	SetMenu(hwnd, hMenubar);
}