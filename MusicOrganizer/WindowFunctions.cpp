#include "stdafx.h"
#include "windows.h"
#include "Resource.h"
#include "WindowFunctions.h"
HWND WINAPI CreateTrackbar(HWND hWnd, HWND* hwndTrack, UINT iMin, UINT iMax)
{
	*hwndTrack = CreateWindowEx(0, TRACKBAR_CLASS, TEXT("Trackbar Control"), WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_ENABLESELRANGE | TBS_HORZ,
		110, 55, 200, 30, hWnd, (HMENU)ID_TRACKBAR, (HINSTANCE)GetWindowLong(hWnd, GWL_HINSTANCE), NULL);
	SendMessage(*hwndTrack, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(iMin, iMax));
	SendMessage(*hwndTrack, TBM_SETPAGESIZE, 0, (LPARAM)2);
	SendMessage(*hwndTrack, TBM_SETPOS, 1, 100);
	return *hwndTrack;
}

int SetListViewColumns(HWND hWndLV, int colNum, int textMaxLen, char** header)
{
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

LPWSTR MBStoWS(char * source)
{
	LPWSTR dest = new WCHAR[MAX_PATH];
	const int codePage = CP_ACP;
	int alen = MultiByteToWideChar(codePage, 0, source, -1, NULL, 0);
	MultiByteToWideChar(codePage, 0, source, -1, dest, alen);
	return dest;
}

HWND CreateListView(HWND hWndParent, UINT uId)
{
	INITCOMMONCONTROLSEX icex;
	RECT rcl;

	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_LISTVIEW_CLASSES;
	InitCommonControlsEx(&icex);
	GetClientRect(hWndParent, &rcl);
	HWND hWndLV = CreateWindow(WC_LISTVIEW, TEXT("ListView"), WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_AUTOVSCROLL | LBS_NOTIFY | LVS_REPORT | WS_CLIPSIBLINGS | LVS_AUTOARRANGE | LVS_SHOWSELALWAYS, 450, 0, 600, rcl.bottom - rcl.top, hWndParent, (HMENU)uId, GetModuleHandle(NULL), NULL);
	ListView_SetExtendedListViewStyleEx(hWndLV, 0, LVS_EX_FULLROWSELECT | LVS_EX_LABELTIP | LVS_EX_GRIDLINES | LVS_EX_ONECLICKACTIVATE | LVS_EX_COLUMNSNAPPOINTS);
	return (hWndLV);
}

INT WINAPI AddListViewItems(HWND hWndLV, int colNum, int textMaxLen, char** item)
{
	int iLastIndex = ListView_GetItemCount(hWndLV);
	LVITEM lvi;
	lvi.mask = LVIF_TEXT;
	lvi.cchTextMax = textMaxLen;
	lvi.iItem = iLastIndex;
	lvi.pszText = NULL;
	lvi.iSubItem = 0;

	if (ListView_InsertItem(hWndLV, &lvi) == -1)
		return -1;
	for (int i = 0; i < colNum; i++)
	{
		if (i == 2 && item[i] != NULL)
		{
			char *temp;
			int l = 0;
			temp = strstr(item[i], "\\");
			do {
				l = strlen(temp) + 1;
				item[i] = &item[i][strlen(item[i]) - l + 2];
				temp = strstr(item[i], "\\");
			} while (temp);
		}
		if (item[i] == NULL || !strcmp(item[i], ""))
			item[i] = "none";
		ListView_SetItemText(hWndLV, iLastIndex, i, MBStoWS(item[i]));
	}
	int itemsCount = iLastIndex + 1;
	return itemsCount;
}

VOID WINAPI UpdateListViewItem(HWND hWndLV, int colNum, char** item,int selectedItemIndex)
{
	for (int i = 0; i < colNum; i++)
	{
		if (!strcmp(item[i], ""))
			item[i] = "none";
		if (i == 2 && strcmp(item[i], ""))
		{
			char *temp;
			int l = 0;
			temp = strstr(item[i], "\\");
			do {
				l = strlen(temp) + 1;
				item[i] = &item[i][strlen(item[i]) - l + 2];
				temp = strstr(item[i], "\\");
			} while (temp);
		}
		LPWSTR dest = new WCHAR[MAX_PATH];
		const int codePage = CP_ACP;
		int alen = MultiByteToWideChar(codePage, 0, item[i], -1, NULL, 0);
		MultiByteToWideChar(codePage, 0, item[i], -1, dest, alen);
		ListView_SetItemText(hWndLV, selectedItemIndex, i, dest);
	}
}

VOID WINAPI UpdateEditBoxes(std::list<char *> songList,int selectedItemIndex, MP3FileInfo *mp3fi,HWND hwndTBTitle,
	HWND hwndTBArtist,
	HWND hwndTBAlbum,
	HWND hwndTBYear,
	HWND hwndTBComment)
{
	BOOL init = false;
	id_tag mp3tag;
	wchar_t buffer[250] = { 0 };
	ZeroMemory(&mp3tag, sizeof mp3tag);

	auto iterator = songList.begin();
	char * item = *std::next(iterator, selectedItemIndex);

	if ((*mp3fi).Init(item))
	{
		ClearEditBoxes(hwndTBTitle, hwndTBArtist, hwndTBAlbum, hwndTBYear, hwndTBComment);
		init = TRUE;
		if ((*mp3fi).bHasV1Tag || (*mp3fi).bHasV2Tag)
		{
			LPWSTR ptr = buffer;
			if ((*mp3fi).szTitle != NULL)
				SetWindowText(hwndTBTitle, MBStoWS((*mp3fi).szTitle));
			if ((*mp3fi).szArtist != NULL)
				SetWindowText(hwndTBArtist, MBStoWS((*mp3fi).szArtist));
			if ((*mp3fi).szAlbum != NULL)
				SetWindowText(hwndTBAlbum, MBStoWS((*mp3fi).szAlbum));
			if ((*mp3fi).szYear != NULL)
				SetWindowText(hwndTBYear, MBStoWS((*mp3fi).szYear));
			if ((*mp3fi).szComment != NULL)
				SetWindowText(hwndTBComment, MBStoWS((*mp3fi).szComment));
		}
	}

	if (init)
	{
		(*mp3fi).Free();
		init = FALSE;
	}
}

VOID WINAPI ClearEditBoxes(HWND hwndTBTitle,
	HWND hwndTBArtist,
	HWND hwndTBAlbum,
	HWND hwndTBYear,
	HWND hwndTBComment)
{
	SetWindowText(hwndTBTitle, TEXT(""));
	SetWindowText(hwndTBArtist, TEXT(""));
	SetWindowText(hwndTBAlbum, TEXT(""));
	SetWindowText(hwndTBYear, TEXT(""));
	SetWindowText(hwndTBComment, TEXT(""));
}

char* WStoMBS(LPWSTR source)
{
	char buffer[MAX_PATH] = { 0 };
	size_t sizeRequired = WideCharToMultiByte(950, 0, source, -1, NULL, 0, NULL, NULL);
	sizeRequired = WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, source, -1, buffer, sizeof(buffer), NULL, NULL);
	return buffer;
}

VOID WINAPI AddMenus(HWND hwnd)
{
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

VOID WINAPI FilterSong(std::list <char*> *songList, std::list <char*> tempSongList,HWND hWnd, HWND hwndTBSearch, HWND *hWndLV, MP3FileInfo * mp3fi,int *itemsCount, int selectedItemIndex,
	HWND hwndLabelSongNumb,
	HWND hwndBtnAdd,
	HWND hwndRBSinger,
	HWND hwndBtnChangeTags,
	HWND hwndBtnNext,
	HWND hwndBtnPlay,
	HWND hwndBtnPrev
	)
{
	int const textMaxLen = 20;
	wchar_t buffer[250] = { 0 };
	LPWSTR ptr = buffer;
	char searchString[30];
	BOOL init = false;
	id_tag mp3tag;
	wchar_t wtext[250] = { 0 };
	ZeroMemory(&mp3tag, sizeof mp3tag);
	GetWindowText(hwndTBSearch, ptr, 30);
	ListView_DeleteAllItems(*hWndLV);
	songList->clear();
	for (int i = 0; i < tempSongList.size(); i++)
	{
		auto iterator = tempSongList.begin();
		char * tempElement = *std::next(iterator, i);
		songList->push_back(tempElement);

		if ((*mp3fi).Init(tempElement))
		{
			if ((*mp3fi).bHasV1Tag || (*mp3fi).bHasV2Tag)
			{
				char* item[4] = { (*mp3fi).szTitle, (*mp3fi).szArtist,(*mp3fi).szFilename,(*mp3fi).szAlbum };
				*itemsCount = AddListViewItems(*hWndLV, 4, textMaxLen, item);
			}
			else
			{
				char* item[4] = { NULL, NULL,(*mp3fi).szFilename ,NULL };
				*itemsCount = AddListViewItems(*hWndLV, 4, textMaxLen, item);
			}
		}
	}
	if (!wcscmp(ptr, TEXT("")))
	{
		*itemsCount = songList->size();
		wchar_t resultString[256];
		swprintf_s(resultString, L"%d/%d", selectedItemIndex + 1, tempSongList.size());
		SendMessage(hwndLabelSongNumb, WM_SETTEXT, 0, (LPARAM)resultString);
		EnableWindow(hwndBtnAdd, TRUE);
	}
	else
	{
		strcpy(searchString, WStoMBS(ptr));
		std::vector<int> items;
		int indexOfColoumn;
		if (SendMessage(hwndRBSinger, BM_GETCHECK, 0, 0))
			indexOfColoumn = 1;
		else
			indexOfColoumn = 3;
		for (int i = 0; i < tempSongList.size(); i++)
		{
			std::vector<wchar_t> bufText(256);
			ListView_GetItemText(*hWndLV, i, indexOfColoumn, &bufText[0], 256);
			wchar_t* buf = reinterpret_cast<wchar_t*>(bufText.data());
			if (wcsicmp(buf, ptr))
				items.push_back(i);
		}

		for (int i = items.size() - 1; i >= 0; i--)
		{
			std::list<char *>::iterator it = songList->begin();
			std::advance(it, items[i]);
			songList->erase(it);
			ListView_DeleteItem(*hWndLV, items[i]);

		}
		*itemsCount = songList->size();
		selectedItemIndex = -1;
		wchar_t resultString[256];

		swprintf_s(resultString, L"%d/%d", selectedItemIndex + 1, songList->size());
		SendMessage(hwndLabelSongNumb, WM_SETTEXT, 0, (LPARAM)resultString);

		SendMessage(hWnd, WM_COMMAND, ID_BUTTONSTOP, 0);
		EnableWindow(hwndBtnChangeTags, FALSE);
		EnableWindow(hwndBtnNext, FALSE);
		EnableWindow(hwndBtnPlay, FALSE);
		EnableWindow(hwndBtnPrev, FALSE);
		EnableWindow(hwndBtnAdd, FALSE);
	}
}

VOID WINAPI ChangeTags(std::list <char*> songList,int selectedItemIndex, HWND *hWndLV, MP3FileInfo * mp3fi,
	HWND hwndTBTitle,
	HWND hwndTBArtist,
	HWND hwndTBAlbum,
	HWND hwndTBYear,
	HWND hwndTBComment,
	HWND hwndBtnChangeTags)
{
	id_tag mp3tag;
	wchar_t buffer[250] = { 0 };
	LPWSTR pointer = buffer;

	auto iterator = songList.begin();
	char * item = *std::next(iterator, selectedItemIndex);

	char title[30];
	GetWindowText(hwndTBTitle, pointer, 30);
	strcpy(title, WStoMBS(pointer));

	char artist[30];
	GetWindowText(hwndTBArtist, pointer, 30);
	strcpy(artist, WStoMBS(pointer));

	char album[30];
	GetWindowText(hwndTBAlbum, pointer, 30);
	strcpy(album, WStoMBS(pointer));

	char year[5];
	GetWindowText(hwndTBYear, pointer, 5);
	strcpy(year, WStoMBS(pointer));

	char comment[30];
	GetWindowText(hwndTBComment, pointer, 30);
	strcpy(comment, WStoMBS(pointer));

	char* songItem[4] = { title, artist, item, album };
	UpdateListViewItem(*hWndLV, 4, songItem, selectedItemIndex);

	ZeroMemory(&mp3tag, sizeof mp3tag);
	(*mp3fi).Change(item, title, album, artist, year, comment);

	EnableWindow(hwndBtnChangeTags, FALSE);
	UpdateWindow(*hWndLV);
}
VOID WINAPI AddSongs(HWND hWnd, std::list <char*> *tempSongList, std::list <char*>* songList, MP3FileInfo * mp3fi, int selectedItemIndex, int* itemsCount,
	HWND hwndTBTitle,
	HWND hwndTBArtist,
	HWND hwndTBAlbum,
	HWND hwndTBYear,
	HWND hwndTBComment,
	HWND* hWndLV,
	HWND hwndLabelSongNumb)
{
	int const textMaxLen = 20;
	OPENFILENAME  ofn = { sizeof ofn };
	wchar_t szFile[1024];
	std::vector<wchar_t*> files;
	char* fileItem;

	szFile[0] = '\0';
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = 1024;
	ofn.hwndOwner = hWnd;
	ofn.lpstrFilter = TEXT("Mp3 פאיכ\0*.mp3\0Âסו פאיכû (*.*)\0*.*\0");
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT;

	if (GetOpenFileName(&ofn) == TRUE)
	{
		wchar_t resultString[256];
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

			fileItem = (char*)malloc(1000 + 1);
			strcpy(fileItem, WStoMBS(dirName));
			strcat(fileItem, "\\");
			strcat(fileItem, WStoMBS(files[i]));
			tempSongList->push_back(fileItem);
			BOOL init = false;
			id_tag mp3tag;
			ZeroMemory(&mp3tag, sizeof mp3tag);

			if ((*mp3fi).Init(fileItem))
			{
				ClearEditBoxes(hwndTBTitle, hwndTBArtist, hwndTBAlbum, hwndTBYear, hwndTBComment);
				init = TRUE;
				if ((*mp3fi).bHasV1Tag || (*mp3fi).bHasV2Tag)
				{
					char* item[4] = { (*mp3fi).szTitle, (*mp3fi).szArtist,(*mp3fi).szFilename, (*mp3fi).szAlbum };
					*itemsCount = AddListViewItems(*hWndLV, 4, textMaxLen, item);
				}
				else
				{
					char* item[4] = { NULL, NULL,(*mp3fi).szFilename, NULL };
					*itemsCount = AddListViewItems(*hWndLV, 4, textMaxLen, item);
				}
			}
			else
			{
				MessageBox(hWnd, TEXT("Unable to initialize MP3Tag class"), TEXT("Error"), MB_ICONERROR);
			}
			if (init)
			{
				(*mp3fi).Free();
				init = FALSE;
			}
		}
		songList->clear();
		for (int i = 0; i < tempSongList->size(); i++)
		{
			auto it = tempSongList->begin();
			char * tempElement = *std::next(it, i);
			songList->push_back(tempElement);
		}
		swprintf_s(resultString, L"%d/%d", selectedItemIndex + 1, tempSongList->size());
		SendMessage(hwndLabelSongNumb, WM_SETTEXT, 0, (LPARAM)resultString);
	}
}