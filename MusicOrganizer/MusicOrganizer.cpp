#include "stdafx.h"
#include "windows.h"
#include "MP3FileInfo.h"
#include "MusicOrganizer.h"
#include "bass.h"
#include "commdlg.h"
#include <CommCtrl.h>
#include <list>
#include <iostream>
#include <fstream>
#include <string>
#include <ShlObj.h>
#include <cstddef>
#include <vector>
#pragma comment(lib,"bass.lib")
#pragma comment(lib, "ComCtl32.Lib")
#pragma comment(linker,"/manifestdependency:\"type='win32' \
                        name='Microsoft.Windows.Common-Controls' \
                        version='6.0.0.0' processorArchitecture='*'\
 publicKeyToken='6595b64144ccf1df' language='*'\"")
#include "WindowFunctions.h"

MP3FileInfo mp3fi;


HINSTANCE hInst;                                
WCHAR szTitle[MAX_LOADSTRING];                  
WCHAR szWindowClass[MAX_LOADSTRING];            

HWND hwndBtnPlay, hwndBtnPause, hwndBtnStop, hwndBtnDelete, hwndTrack, hwndLabelVol, hwndTBTitle, hwndBtnAdd, hwndBtnPrev, hwndBtnNext, hwndBtnOpenClose, hwndBtnSearch;
HWND hwndTBArtist, hwndTBAlbum, hwndTBYear, hwndTBComment, hwndLabelTitle, hwndLabelArtist, hwndLabelAlbum, hwndTBSearch, hwndLabelSongNumb;
HWND hwndLabelYear, hwndLabelComment, hwndBtnChangeTags;
HWND hwndRBSinger, hwndRBAlbum;

static HWND hWndLV = NULL;
int const colNum = 4;
int const textMaxLen = 20;

BOOL openFlag = true;
std::list<char *> songList;
std::list<char *> tempSongList;
int itemsCount;
bool pauseFlag = false;
int selectedItemIndex;

HCHANNEL channel;
HSTREAM stream;

using namespace std;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,_In_opt_ HINSTANCE hPrevInstance,_In_ LPWSTR    lpCmdLine,_In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
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

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MUSICORGANIZER));

    MSG msg;

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
LRESULT OnCtlColorStatic(HDC hdc, HWND hCtl)
{
	if (hCtl == hwndLabelComment)
	{
		SetBkColor(hdc, 1);

		return (LRESULT)GetStockObject(HOLLOW_BRUSH);
	}
	return 0;
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
	HWND hWnd;
	hInst = hInstance;

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPED | WS_BORDER | WS_SYSMENU,CW_USEDEFAULT, 0, 1025, 340, NULL, NULL, hInstance, NULL);
	
	hwndBtnPlay = CreateWindow(TEXT("BUTTON"), TEXT("Play"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 10, 50, 35, hWnd, (HMENU)ID_BUTTONPLAY, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	hwndBtnPause = CreateWindow(TEXT("BUTTON"), TEXT("Pause"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 70, 10, 50, 35, hWnd, (HMENU)ID_BUTTONPAUSE, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	hwndBtnStop = CreateWindow(TEXT("BUTTON"), TEXT("Stop"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 130, 10, 50, 35, hWnd, (HMENU)ID_BUTTONSTOP, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	hwndBtnNext = CreateWindow(TEXT("BUTTON"), TEXT("Next"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 250, 10, 50, 35, hWnd, (HMENU)ID_BUTTONNEXT, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	hwndBtnPrev = CreateWindow(TEXT("BUTTON"), TEXT("Prev"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 190, 10, 50, 35, hWnd, (HMENU)ID_BUTTONPREV, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	hwndBtnAdd = CreateWindow(TEXT("BUTTON"), TEXT("Add"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 310, 10, 50, 35, hWnd, (HMENU)ID_BUTTONADD, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	hwndBtnDelete = CreateWindow(TEXT("BUTTON"), TEXT("Delete"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 370, 10, 50, 35, hWnd, (HMENU)ID_BUTTONDELETE, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	hwndBtnOpenClose = CreateWindow(TEXT("BUTTON"), TEXT("Close"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 370, 55, 50, 25, hWnd, (HMENU)ID_BUTTONCLOP, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	hwndLabelSongNumb = CreateWindow(TEXT("static"), TEXT("0/0"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 320, 60, 50, 15, hWnd, (HMENU)(501), (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	hwndLabelVol = CreateWindow(TEXT("static"), TEXT("Volume"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 10, 55, 50, 15, hWnd, (HMENU)(501), (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	hwndLabelTitle = CreateWindow(TEXT("static"), TEXT("Title [30]"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 10, 85, 57, 17, hWnd, (HMENU)(502), (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	hwndTBTitle = CreateWindow(TEXT("Edit"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 110, 85, 310, 20, hWnd, (HMENU)ID_TEXTBOXTITLE, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), 0);
	hwndLabelArtist = CreateWindow(TEXT("static"), TEXT("Singer [30]"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 10, 115, 70, 17, hWnd, (HMENU)(503), (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	hwndTBArtist = CreateWindow(TEXT("Edit"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 110, 115, 310, 20, hWnd, (HMENU)ID_TEXTBOXARTIST, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), 0);
	hwndLabelAlbum = CreateWindow(TEXT("static"), TEXT("Album [30]"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 10, 145, 70, 17, hWnd, (HMENU)(504), (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	hwndTBAlbum = CreateWindow(TEXT("Edit"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 110, 145, 310, 20, hWnd, (HMENU)ID_TEXTBOXALBUM, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), 0);
	hwndLabelYear = CreateWindow(TEXT("static"), TEXT("Year [4]"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 10, 175, 52, 17, hWnd, (HMENU)(505), (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	hwndTBYear = CreateWindow(TEXT("Edit"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE, 110, 175, 310, 20, hWnd, (HMENU)ID_TEXTBOXYEAR, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), 0);
	hwndLabelComment = CreateWindow(TEXT("static"), TEXT("Comment [30]"), WS_CHILD | WS_VISIBLE | WS_TABSTOP, 10, 205, 90, 17, hWnd, (HMENU)(506), (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	hwndTBComment = CreateWindow(TEXT("Edit"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | WS_TABSTOP, 110, 205, 310, 20, hWnd, (HMENU)ID_TEXTBOXCOMMENT, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), 0);
	hwndBtnChangeTags = CreateWindow(TEXT("BUTTON"), TEXT("Change tags"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 235, 90, 35, hWnd, (HMENU)ID_BUTTONTAGS, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	hwndBtnSearch = CreateWindow(TEXT("BUTTON"), TEXT("Filter"), WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 330, 235, 90, 35, hWnd, (HMENU)ID_BUTTONSEARCH, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	hwndTBSearch = CreateWindow(TEXT("Edit"), NULL, WS_BORDER | WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, 150, 250, 170, 20, hWnd, (HMENU)ID_TEXTBOXTITLE, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), 0);
	hwndRBSinger = CreateWindow(TEXT("BUTTON"), TEXT("By singer"), WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON, 150, 230, 80, 18, hWnd, (HMENU)ID_RADBUTSINGER, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	hwndRBAlbum = CreateWindow(TEXT("BUTTON"), TEXT("By album"), WS_CHILD | WS_VISIBLE | BS_RADIOBUTTON, 240, 230, 80, 18, hWnd, (HMENU)ID_RADBUTALBUM, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);

	EnableWindow(hwndBtnPlay, FALSE);
	EnableWindow(hwndBtnPause, FALSE);
	EnableWindow(hwndBtnStop, FALSE);
	EnableWindow(hwndBtnNext, FALSE);
	EnableWindow(hwndBtnPrev, FALSE);
	EnableWindow(hwndBtnDelete, FALSE);
	EnableWindow(hwndBtnChangeTags, FALSE);
	EnableWindow(hwndBtnSearch, FALSE);

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
	CreateTrackbar(hWnd,&hwndTrack, 0, 100);

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
	HDC hdc = NULL;

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
			PostQuitMessage(0);
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
				auto iterator = songList.begin();
				char * item = *std::next(iterator, selectedItemIndex);
				BASS_ChannelStop(stream);
				BASS_StreamFree(stream);
				stream = BASS_StreamCreateFile(FALSE, item, 0, 0, 0);
				if (!stream) {
					MessageBox(NULL, TEXT("Error."), NULL, 0);
					return 1;
				}
				ListView_SetItemState(hWndLV, selectedItemIndex, LVIS_SELECTED, LVIS_SELECTED);
				ListView_SetItemState(hWndLV, selectedItemIndex + 1, LVIF_STATE, LVIS_SELECTED);
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
			wchar_t resultString[256];
			BASS_ChannelStop(stream);
			BASS_StreamFree(stream);
			if (selectedItemIndex < itemsCount - 1)
				selectedItemIndex++;
			pauseFlag = false;
			UpdateEditBoxes(songList, selectedItemIndex, &mp3fi, hwndTBTitle, hwndTBArtist, hwndTBAlbum, hwndTBYear, hwndTBComment);
			SendMessage(hWnd, WM_COMMAND, ID_BUTTONPLAY, 0);
			ListView_SetItemState(hWndLV, selectedItemIndex, LVIS_SELECTED, LVIS_SELECTED);
			ListView_SetItemState(hWndLV, selectedItemIndex - 1, LVIF_STATE, LVIS_SELECTED);
			swprintf_s(resultString, L"%d/%d", selectedItemIndex + 1, songList.size());
			SendMessage(hwndLabelSongNumb, WM_SETTEXT, 0, (LPARAM)resultString);
			break;
		}
		case ID_BUTTONPREV:
		{
			wchar_t resultString[256];
			BASS_ChannelStop(stream);
			BASS_StreamFree(stream);
			if (selectedItemIndex > 0)
				selectedItemIndex--;
			pauseFlag = false;
			UpdateEditBoxes(songList, selectedItemIndex, &mp3fi, hwndTBTitle, hwndTBArtist, hwndTBAlbum, hwndTBYear, hwndTBComment);
			SendMessage(hWnd, WM_COMMAND, ID_BUTTONPLAY, 0);
			ListView_SetItemState(hWndLV, selectedItemIndex, LVIS_SELECTED, LVIS_SELECTED);
			ListView_SetItemState(hWndLV, selectedItemIndex + 1, LVIF_STATE, LVIS_SELECTED);
			swprintf_s(resultString, L"%d/%d", selectedItemIndex + 1, songList.size());
			SendMessage(hwndLabelSongNumb, WM_SETTEXT, 0, (LPARAM)resultString);
			break;
		}
		case ID_BUTTONDELETE:
		{
			auto iterator = songList.begin();
			char * item = *std::next(iterator, selectedItemIndex);
			songList.remove(item);
			tempSongList.remove(item);
			if (selectedItemIndex != -1)
			{
				ListView_DeleteItem(hWndLV, selectedItemIndex);
				itemsCount--;
				SendMessage(hWnd, WM_COMMAND, ID_BUTTONSTOP, 0);
				selectedItemIndex = 0;
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
			AddSongs(hWnd, &tempSongList, &songList, &mp3fi, selectedItemIndex, &itemsCount,
				hwndTBTitle,
				hwndTBArtist,
				hwndTBAlbum,
				hwndTBYear,
				hwndTBComment,
				&hWndLV,
				hwndLabelSongNumb);
			break;
		}
		case ID_BUTTONTAGS:
		{
			ChangeTags(songList, selectedItemIndex, &hWndLV, &mp3fi, hwndTBTitle, hwndTBArtist, hwndTBAlbum, hwndTBYear, hwndTBComment, hwndBtnChangeTags);
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
			MoveWindow(hWnd, rect.left, rect.top, width, 340, TRUE);
			break;
		}
		case ID_BUTTONSEARCH:
		{
			FilterSong(&songList, tempSongList, hWnd, hwndTBSearch, &hWndLV, &mp3fi, &itemsCount, selectedItemIndex, hwndLabelSongNumb, hwndBtnAdd, hwndRBSinger, hwndBtnChangeTags, hwndBtnNext, hwndBtnPlay, hwndBtnPrev);
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
	{
		switch (LOWORD(wParam)) {
		case TB_THUMBTRACK:
		case TB_THUMBPOSITION:
			if (hwndTrack == (HWND)lParam) {
				float dwPos = SendMessage(hwndTrack, TBM_GETPOS, 0, 100);
				float volume = 1 - ((100 - dwPos) * 0.01);
				BASS_SetVolume(volume);
			}
			break;
		}
		break;
	}
	case WM_NOTIFY:
	{
		LPNMHDR lpnmHdr;
		lpnmHdr = (LPNMHDR)lParam;
		wchar_t resultString[256];
		if ((lpnmHdr->idFrom == ID_LISTVIEW) && (lpnmHdr->code == NM_CLICK))
		{
			selectedItemIndex = ListView_GetNextItem(hWndLV, -1, LVNI_ALL | LVNI_SELECTED);
			if (selectedItemIndex >= 0)
			{
				UpdateEditBoxes(songList, selectedItemIndex, &mp3fi, hwndTBTitle, hwndTBArtist, hwndTBAlbum, hwndTBYear, hwndTBComment);
				EnableWindow(hwndBtnChangeTags, TRUE);
				EnableWindow(hwndBtnPlay, TRUE);
				EnableWindow(hwndBtnNext, TRUE);
				EnableWindow(hwndBtnPrev, TRUE);
				EnableWindow(hwndBtnDelete, TRUE);
				BASS_ChannelStop(stream);
				BASS_StreamFree(stream);
				pauseFlag = false;
				SendMessage(hWnd, WM_COMMAND, ID_BUTTONPLAY, 0);
				swprintf_s(resultString, L"%d/%d", selectedItemIndex + 1, songList.size());
				SendMessage(hwndLabelSongNumb, WM_SETTEXT, 0, (LPARAM)resultString);
			}
		}
		break;
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		FillRect(ps.hdc, &ps.rcPaint, CreateSolidBrush(RGB(255,255,255)));
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_CTLCOLORSTATIC:
	{
		HDC hdcStatic = (HDC)wParam;
		SetTextColor(hdcStatic, RGB(0, 0, 0));
		SetBkColor(hdcStatic, RGB(255, 255, 255));
		return (INT_PTR)CreateSolidBrush(RGB(255, 255, 255));
	}

	
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
