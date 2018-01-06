

#include "Main.h"

#define WS_CUSTOM WS_OVERLAPPEDWINDOW // WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX

/* Global Variables */

HINSTANCE	g_hInstance				= 0;
LPTSTR		g_strTitle				= TEXT("Path Finder");
LPTSTR		g_strClass				= TEXT("PATHFINDER");

HWND		g_hWnd					= 0;
HMENU		g_hMenu					= 0;
HMENU		g_hMenu_File			= 0;
HMENU		g_hMenu_Edit			= 0;
HMENU		g_hMenu_Edit_Generate	= 0;
HMENU		g_hMenu_Display			= 0;
HMENU		g_hMenu_Help			= 0;

MSG			g_Msg					= {};
PATH		g_Path					= {};

HWND		g_hwndNewPrompt			= 0;
HWND		g_htxtNewWidth			= 0;
HWND		g_htxtNewHeight			= 0;

HWND		g_hbtnCompute			= 0;
HWND		g_hbtnRedraw			= 0;
HWND		g_htxtDelay				= 0;

HWND		g_hcmbIllustrate		= 0;
HWND		g_hcmbMethod			= 0;

HWND		g_hwndHelp				= 0;

BYTE		g_bCreatingWall			= 0;
BYTE		g_bAltData				= 0;
WORD		g_wReserved				= 0;
DWORD		g_dwMethod				= METHOD_DIJKSTRA;

MAP	*		g_pMap					= 0;

char		g_str[1024];

/* Forward declarations */

static BOOL __stdcall	Initialise();
static void __stdcall	CleanUp();
LRESULT CALLBACK		WndProc(HWND, UINT, WPARAM, LPARAM);

int __stdcall WinMain(
	HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine,
    int nCmdShow)
{
	// Set random seed
	srand(GetTickCount());
	
	// Store global handle
	g_hInstance = hInstance;
	
	// Start up
	if (!Initialise())
	{
		CleanUp();
		ExitProcess(0);
	}
	
	// Enter program loop
	while (g_Msg.message != WM_QUIT)
	{
		// Check for messages
		if (PeekMessage(&g_Msg, 0, 0, 0, PM_REMOVE))
		{
			// Handle messages
			TranslateMessage(&g_Msg);
			DispatchMessage(&g_Msg);
		}
		else
		{
			// Conduct updates
			if (g_pMap) {
				if (g_bCreatingWall == 1) {
					NODE * node = MapGetAtCursor(g_pMap);
					if (node && node->s != 3) {
						node->s = 3;
						MapRenderNode(g_pMap, node);
					} else Sleep(1);
				} else if (g_bCreatingWall == 2) {
					NODE * node = MapGetAtCursor(g_pMap);
					if (node && node->s == 3) {
						node->s = 0;
						MapRenderNode(g_pMap, node);
					} else Sleep(1);
				} else Sleep(1);
			} else Sleep(1);
		}
	}
	
	// Clean up and exit
	CleanUp();
	ExitProcess(0);
}

static BOOL __stdcall Initialise()
{
	WNDCLASSEX wcex;
	RECT rct;
	DWORD dwScreenWidth, dwScreenHeight;
	
	// Register class
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= g_hInstance;
	wcex.hIcon			= LoadIcon(0, IDI_APPLICATION);
	wcex.hCursor		= LoadCursor(0, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)COLOR_WINDOW;
	wcex.lpszMenuName	= 0;
	wcex.lpszClassName	= g_strClass;
	wcex.hIconSm		= LoadIcon(0, IDI_APPLICATION);
	if (!RegisterClassEx(&wcex)) {
		MessageBox(0, TEXT("Failed to register class.\nThe program will now terminate."), g_strTitle, MB_ICONHAND);
		return FALSE;
	}
	
	// Get screen metrics
	dwScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	dwScreenHeight = GetSystemMetrics(SM_CYSCREEN);
	
	// Create menu
	g_hMenu = CreateMenu();
	if (!g_hMenu) {
		MessageBoxA(0, "Failed to create menu.", g_strTitle, MB_ICONHAND);
		return FALSE;
	}
	g_hMenu_File = CreatePopupMenu();
	if (!g_hMenu_File) {
		DestroyMenu(g_hMenu);
		MessageBoxA(0, "Failed to create popup-menu.", g_strTitle, MB_ICONHAND);
		return FALSE;
	}
	g_hMenu_Edit = CreatePopupMenu();
	if (!g_hMenu_Edit) {
		DestroyMenu(g_hMenu_File);
		DestroyMenu(g_hMenu);
		MessageBoxA(0, "Failed to create popup-menu.", g_strTitle, MB_ICONHAND);
		return FALSE;
	}
	g_hMenu_Display = CreatePopupMenu();
	if (!g_hMenu_Display) {
		DestroyMenu(g_hMenu_Edit);
		DestroyMenu(g_hMenu_File);
		DestroyMenu(g_hMenu);
		MessageBoxA(0, "Failed to create popup-menu.", g_strTitle, MB_ICONHAND);
		return FALSE;
	}
	g_hMenu_Edit_Generate = CreatePopupMenu();
	if (!g_hMenu_Edit_Generate) {
		DestroyMenu(g_hMenu_Display);
		DestroyMenu(g_hMenu_Edit);
		DestroyMenu(g_hMenu_File);
		DestroyMenu(g_hMenu);
		MessageBoxA(0, "Failed to create popup-menu.", g_strTitle, MB_ICONHAND);
		return FALSE;
	}
	g_hMenu_Help = CreatePopupMenu();
	if (!g_hMenu_Help) {
		DestroyMenu(g_hMenu_Edit_Generate);
		DestroyMenu(g_hMenu_Display);
		DestroyMenu(g_hMenu_Edit);
		DestroyMenu(g_hMenu_File);
		DestroyMenu(g_hMenu);
		MessageBoxA(0, "Failed to create popup-menu.", g_strTitle, MB_ICONHAND);
		return FALSE;
	}
	AppendMenu(g_hMenu,					MF_POPUP,	(UINT_PTR)g_hMenu_File,				"&File");
	AppendMenu(g_hMenu,					MF_POPUP,	(UINT_PTR)g_hMenu_Edit,				"&Edit");
	AppendMenu(g_hMenu,					MF_POPUP,	(UINT_PTR)g_hMenu_Display,			"&Display");
	AppendMenu(g_hMenu,					MF_POPUP,	(UINT_PTR)g_hMenu_Help,				"&Help");
	AppendMenu(g_hMenu_File,			0,			(UINT_PTR)IDM_FILE_NEW,				"&New");
	AppendMenu(g_hMenu_File,			0,			(UINT_PTR)IDM_FILE_OPEN,			"&Open");
	AppendMenu(g_hMenu_File,			0,			(UINT_PTR)IDM_FILE_SAVE_AS,			"Save &As");
	AppendMenu(g_hMenu_File,			0,			(UINT_PTR)IDM_FILE_CLOSE,			"&Close");
	AppendMenu(g_hMenu_File,			0,			(UINT_PTR)IDM_FILE_EXIT,			"&Exit");
	AppendMenu(g_hMenu_Edit,			0,			(UINT_PTR)IDM_EDIT_CLEAR,			"&Clear");
	AppendMenu(g_hMenu_Edit,			MF_POPUP,	(UINT_PTR)g_hMenu_Edit_Generate,	"&Generate");
	AppendMenu(g_hMenu_Edit_Generate,	0,			(UINT_PTR)IDM_EDIT_GENERATE_MAZE,	"&Maze");
	AppendMenu(g_hMenu_Edit_Generate,	0,			(UINT_PTR)IDM_EDIT_GENERATE_RANDOM,	"&Random");
	AppendMenu(g_hMenu_Display,			0,			(UINT_PTR)IDM_DISPLAY_SHOWWORK,		"&Show Work");
	AppendMenu(g_hMenu_Help,			0,			(UINT_PTR)IDM_HELP_HOWTOUSE,		"&How To Use");
	
	// Create window
	rct.left = (dwScreenWidth/2)-300;
	rct.top = (dwScreenHeight/2)-200;
	rct.right = rct.left+600;
	rct.bottom = rct.top+400;
	AdjustWindowRect(&rct, WS_CUSTOM, TRUE);
	g_hWnd = CreateWindowExA(
		0, g_strClass, g_strTitle,
		WS_CUSTOM | WS_VISIBLE,
		rct.left, rct.top,
		rct.right - rct.left,
		rct.bottom - rct.top,
		0, g_hMenu, g_hInstance, 0);
	if (!g_hWnd) {
		DestroyMenu(g_hMenu_Display);
		DestroyMenu(g_hMenu_Edit);
		DestroyMenu(g_hMenu_File);
		DestroyMenu(g_hMenu);
		MessageBoxA(0, "Failed to create window.", g_strTitle, MB_ICONHAND);
		return FALSE;
	}
	
	// Create search-method combo-box
	g_hcmbMethod = CreateWindowEx(
		0, TEXT("COMBOBOX"), 0,
		CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_VISIBLE,
		416, 16, 168, 32, g_hWnd, (HMENU)IDCB_METHOD, g_hInstance, 0);
	SendMessage(g_hcmbMethod, CB_ADDSTRING, 0, (LPARAM)TEXT("Dijkstra"));
	SendMessage(g_hcmbMethod, CB_ADDSTRING, 0, (LPARAM)TEXT("A*"));
	SendMessage(g_hcmbMethod, CB_SETCURSEL, 0, 0);
	
	// Create illustrate combo-box
	g_hcmbIllustrate = CreateWindowEx(
		0, TEXT("COMBOBOX"), 0,
		CBS_DROPDOWNLIST | CBS_HASSTRINGS | WS_CHILD | WS_VISIBLE,
		416, 160, 168, 32, g_hWnd, (HMENU)IDCB_ILLUSTRATE, g_hInstance, 0);
	SendMessage(g_hcmbIllustrate, CB_ADDSTRING, 0, (LPARAM)TEXT("No illustration"));
	SendMessage(g_hcmbIllustrate, CB_ADDSTRING, 0, (LPARAM)TEXT("Delay"));
	SendMessage(g_hcmbIllustrate, CB_ADDSTRING, 0, (LPARAM)TEXT("Time-out"));
	SendMessage(g_hcmbIllustrate, CB_SETCURSEL, 0, 0);
	
	// Create "Compute Path" button
	g_hbtnCompute = CreateWindowEx(0, TEXT("BUTTON"), "Compute Path",
		WS_CHILD | WS_VISIBLE, 416, 64, 168, 32,
		g_hWnd, (HMENU)IDB_COMPUTE_PATH, g_hInstance, 0);
	
	// Create "Redraw Map" button
	g_hbtnRedraw = CreateWindowEx(0, TEXT("BUTTON"), "Redraw Map",
		WS_CHILD | WS_VISIBLE, 416, 112, 168, 32,
		g_hWnd, (HMENU)IDB_REDRAW, g_hInstance, 0);
	
	// Create "Delay" text box
	g_htxtDelay = CreateWindowEx(0, TEXT("EDIT"), "0",
		WS_CHILD | WS_VISIBLE | ES_NUMBER, 512, 208, 72, 32,
		g_hWnd, 0, g_hInstance, 0);
	
	// Create "New Map" prompt
	rct.left = (dwScreenWidth/2)-200;
	rct.top = (dwScreenHeight/2)-100;
	rct.right = rct.left+400;
	rct.bottom = rct.top+200;
	AdjustWindowRect(&rct, WS_CAPTION, FALSE);
	g_hwndNewPrompt = CreateWindowEx(
		WS_EX_TOPMOST, g_strClass, "New",
		WS_CHILD | WS_POPUP | WS_CAPTION,
		rct.left, rct.top,
		rct.right - rct.left,
		rct.bottom - rct.top,
		g_hWnd, 0, g_hInstance, 0);
	if (!g_hwndNewPrompt) {
		MessageBoxA(0, "Failed to create New Map prompt.", g_strTitle, MB_ICONHAND);
		return FALSE;
	}
	
	// Create interface inside "New Map" prompt
	g_htxtNewWidth = CreateWindowEx(
		WS_EX_CLIENTEDGE, TEXT("EDIT"), 0,
		WS_CHILD | WS_VISIBLE | ES_NUMBER, 200, 32, 136, 32,
		g_hwndNewPrompt, (HMENU)IDT_NEW_WIDTH, g_hInstance, 0);
	g_htxtNewHeight = CreateWindowEx(
		WS_EX_CLIENTEDGE, TEXT("EDIT"), 0,
		WS_CHILD | WS_VISIBLE | ES_NUMBER, 200, 96, 136, 32,
		g_hwndNewPrompt, (HMENU)IDT_NEW_HEIGHT, g_hInstance, 0);
	CreateWindowEx(
		0, TEXT("BUTTON"), "Create",
		WS_CHILD | WS_VISIBLE, 32, 160, 64, 32,
		g_hwndNewPrompt, (HMENU)IDB_NEW_CREATE, g_hInstance, 0);
	CreateWindowEx(
		0, TEXT("BUTTON"), "Cancel",
		WS_CHILD | WS_VISIBLE, 304, 160, 64, 32,
		g_hwndNewPrompt, (HMENU)IDB_NEW_CANCEL, g_hInstance, 0);
	
	// Create help window
	rct.left = (dwScreenWidth/2)-300;
	rct.top = (dwScreenHeight/2)-200;
	rct.right = rct.left+600;
	rct.bottom = rct.top+400;
	AdjustWindowRect(&rct, WS_CAPTION, FALSE);
	g_hwndHelp = CreateWindowEx(
		WS_EX_TOPMOST, g_strClass, "Help",
		WS_CHILD | WS_POPUP | WS_CAPTION,
		rct.left, rct.top,
		rct.right - rct.left,
		rct.bottom - rct.top,
		g_hWnd, 0, g_hInstance, 0);
	if (!g_hwndHelp) {
		MessageBoxA(0, "Failed to create Help dialog.", g_strTitle, MB_ICONHAND);
		return FALSE;
	}
	
	// Create help window interface
	CreateWindowEx(
		0, TEXT("BUTTON"), "Close",
		WS_CHILD | WS_VISIBLE, 520, 352, 64, 32,
		g_hwndHelp, (HMENU)IDB_CLOSE_HOWTOUSE, g_hInstance, 0);
	
	// Return true
	return TRUE;
}

void __stdcall CleanUp()
{
	// Destroy map
	MapDestroy(g_pMap);
	g_pMap = 0;
	PathDestroy(&g_Path);
	
	// Destroy window
	if (g_hWnd) {
		DestroyWindow(g_hWnd);
		g_hWnd = 0;
	}
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
		if (hWnd == g_hWnd)
		{
			PAINTSTRUCT ps;
			RECT client;
			RECT rct;
			
			// Get client rectangle
			GetClientRect(g_hWnd, &client);
			
			// Begin painting to window
			BeginPaint(g_hWnd, &ps);
			
			// If a map exists
			if (g_pMap) {
				// Render map
				sprintf(g_str, "Statistics:\n>> Time stats (microsecs):\n"
					"  Search: %llu\n"
					"  Render: %llu\n"
					">> G Scores:\n"
					"  Path: %u\n"
					"  Highest: %u\n",
					g_pMap -> uiSearchTime,
					g_pMap -> uiDrawTime,
					g_pMap -> uiPathG,
					g_pMap -> uiHighG);
				BackBufferPresent(g_pMap -> pBackBuffer);
			} else {
				sprintf(g_str, "No statistics.");
			}
			
			// Make text background transparent
			SetBkMode(ps.hdc, TRANSPARENT);
			
			// Draw "Illustrate Delay" text
			rct.left = client.right-184;
			rct.top = client.top+208;
			rct.right = client.right;
			rct.bottom = client.bottom;
			DrawText(ps.hdc, "Illustrate\ndelay:", -1, &rct, 0);
			
			// Erase statistics region
			rct.left = client.right-184;
			rct.top = client.top+256;
			rct.right = client.right;
			rct.bottom = client.bottom;
			FillRect(ps.hdc, &rct, (HBRUSH)COLOR_WINDOW);
			
			// Draw statistics
			DrawText(ps.hdc, g_str, -1, &rct, 0);
			
			// End painting to window
			EndPaint(g_hWnd, &ps);
		}
		else if (hWnd == g_hwndNewPrompt)
		{
			PAINTSTRUCT ps;
			RECT rct;
			
			// Get client rectangle
			GetClientRect(g_hwndNewPrompt, &rct);
			
			// Begin painting to window
			BeginPaint(g_hwndNewPrompt, &ps);
			
			// Make text background transparent
			SetBkMode(ps.hdc, TRANSPARENT);
			
			// Draw "Map Width" text
			rct.left = 16;
			rct.top = 32;
			rct.right = 200;
			rct.bottom = 64;
			DrawText(ps.hdc, "Map Width:", -1, &rct, 0);
			
			// Draw "Map Height" text
			rct.top = 96;
			rct.bottom = 144;
			DrawText(ps.hdc, "Map Height:", -1, &rct, 0);
			
			// End painting to window
			EndPaint(g_hwndNewPrompt, &ps);
		}
		else if (hWnd == g_hwndHelp)
		{
			PAINTSTRUCT ps;
			RECT rct;
			
			// Get client rectangle
			GetClientRect(g_hwndHelp, &rct);
			
			// Begin painting to window
			BeginPaint(g_hwndHelp, &ps);
			
			// Make text background transparent
			SetBkMode(ps.hdc, TRANSPARENT);
			
			// Set text bounds
			rct.left += 16;
			rct.top += 16;
			rct.right -= 16;
			rct.bottom -= 16;
			DrawText(ps.hdc, "Path Finder (Version 1.0.1)\n"
				"Written by CallyKitten99.\n\n"
				"Use the 'S' key to set the Start Node to the node pointed to by the cursor.\n"
				"Use the SPACE key to set the Destination Node.\n"
				"Left-clicking anywhere on the map creates an impassable node in that area.\n"
				"Right-clicking anywhere on the map removes an impassable node.\n"
				"Nodes coloured in black are impassable nodes.\n"
				"Nodes coloured in grey are passable.\n"
				"Nodes coloured in aqua-blue are OPEN (EXPLORED) nodes.\n"
				"Nodes coloured in blue are CLOSED (EXPLORED) nodes.\n"
				"A path formed between the Start and Destination nodes is represented by a green line.\n"
				"When delayed/time-out algorithm interrendering is enabled, use the END key to end the search.",
				-1, &rct, DT_WORDBREAK);
			
			// End painting to window
			EndPaint(g_hwndHelp, &ps);
		}
		break;
	
	case WM_COMMAND:		
		switch (LOWORD(wParam))
		{
		case IDB_COMPUTE_PATH:
			if (g_dwMethod >> 1) {
				MENUITEMINFO info;
			
				g_bAltData = TRUE;
				info.cbSize = sizeof(MENUITEMINFO);
				info.fState = MFS_CHECKED;
				info.fMask = MIIM_STATE;
				SetMenuItemInfo(g_hMenu_Display, IDM_DISPLAY_SHOWWORK, FALSE, &info);
				
				SendMessage(g_htxtDelay, WM_GETTEXT, (WPARAM)1024, (LPARAM)g_str);
				METHOD_SET_TIME_PARAM(g_dwMethod, atoi(g_str));
			}
			SetFocus(g_hWnd);
			MapComputePath(g_pMap, &g_Path, g_dwMethod);
			InvalidateRect(g_hWnd, 0, FALSE);
			MapRender(g_pMap);
			PathRender(&g_Path, g_pMap);
			break;
		
		case IDB_REDRAW:
			SetFocus(g_hWnd);
			MapRender(g_pMap);
			PathRender(&g_Path, g_pMap);
			break;
			
		case IDM_FILE_NEW:
			ShowWindow(g_hwndNewPrompt, SW_SHOW);
			break;
		
		case IDM_EDIT_CLEAR:
			MapReset(g_pMap);
			MapRender(g_pMap);
			break;
		
		case IDM_EDIT_GENERATE_RANDOM:
			MapGenerateRandom(g_pMap);
			MapRender(g_pMap);
			break;
		
		case IDM_EDIT_GENERATE_MAZE:
			MapGenerateMaze(g_pMap);
			InvalidateRect(g_hWnd, 0, FALSE);
			MapRender(g_pMap);
			break;
			
		case IDM_DISPLAY_SHOWWORK:
		{
			MENUITEMINFO info;
			
			g_bAltData = !g_bAltData;
			if (g_bAltData) info.fState = MFS_CHECKED;
			else info.fState = MFS_UNCHECKED;
			info.cbSize = sizeof(MENUITEMINFO);
			info.fMask = MIIM_STATE;
			SetMenuItemInfo(g_hMenu_Display, IDM_DISPLAY_SHOWWORK, FALSE, &info);
			MapRender(g_pMap);
			PathRender(&g_Path, g_pMap);
		}
			break;
			
		case IDCB_METHOD:
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				/* If the user makes a selection from the list:
				Send CB_GETCURSEL message to get the index of the selected list item.
				Send CB_GETLBTEXT message to get the item. */
				{
					int i;
				
					i = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, 0, 0);
					switch (i)
					{
					case 0:
						METHOD_SET_METHOD(g_dwMethod, METHOD_DIJKSTRA);
						break;
					
					case 1:
						METHOD_SET_METHOD(g_dwMethod, METHOD_ASTAR);
						break;
						
					default:
						MessageBoxA(g_hWnd, "Unknown error.", g_strTitle, MB_ICONHAND);
						break;
					}
				}
				break;
				
			case CBN_CLOSEUP:
				SetFocus(g_hWnd);
				break;
			
			default: break;
			}
			break;
		
		case IDCB_ILLUSTRATE:
			switch (HIWORD(wParam))
			{
			case CBN_SELCHANGE:
				{
					int i;
				
					i = SendMessage((HWND)lParam, (UINT)CB_GETCURSEL, 0, 0);
					switch (i)
					{
					case 0:
						METHOD_SET_ILLLUSTRATE(g_dwMethod, METHOD_ILLUSTATE_NONE);
						break;
					
					case 1:
						METHOD_SET_ILLLUSTRATE(g_dwMethod, METHOD_ILLUSTATE_DELAY);
						break;
					
					case 2:
						METHOD_SET_ILLLUSTRATE(g_dwMethod, METHOD_ILLUSTATE_TIMEOUT);
						break;
						
					default:
						MessageBoxA(g_hWnd, "Unknown error.", g_strTitle, MB_ICONHAND);
						break;
					}
				}
				break;
				
			case CBN_CLOSEUP:
				SetFocus(g_hWnd);
				break;
			
			default: break;
			}
			break;
			
		case IDB_NEW_CREATE:
		{
			UINT uiWidth, uiHeight;
			MAP * pNew;
			RECT rct;
			
			// Retrieve and convert width
			SendMessage(g_htxtNewWidth, (UINT)WM_GETTEXT, (WPARAM)1024, (LPARAM)g_str);
			uiWidth = strtoul(g_str, 0, 10);
			if (!uiWidth) {
				MessageBox(g_hWnd, "Please enter a valid map width.", g_strTitle, MB_ICONEXCLAMATION);
				break;
			}
			
			// Retrieve and convert height
			SendMessage(g_htxtNewHeight, (UINT)WM_GETTEXT, (WPARAM)1024, (LPARAM)g_str);
			uiHeight = strtoul(g_str, 0, 10);
			if (!uiHeight) {
				MessageBox(g_hWnd, "Please enter a valid map height.", g_strTitle, MB_ICONEXCLAMATION);
				break;
			}
			
			// Hide the "New" prompt
			ShowWindow(g_hwndNewPrompt, SW_HIDE);
			
			// Create map
			pNew = MapCreate(uiWidth, uiHeight);
			if (!pNew) {
				MessageBox(g_hWnd, "Failed to create a new map.", g_strTitle, MB_ICONHAND);
				break;
			}
			MapDestroy(g_pMap);
			g_pMap = pNew;
			
			// Set rectangle
			GetClientRect(g_hWnd, &rct);
			rct.right -= 200;
			
			// Set its DC
			MapSetViewport(g_pMap, &rct);
			
			// Render the map
			MapRender(g_pMap);
			
			// Invalidate client area and erase
			InvalidateRect(g_hWnd, 0, TRUE);
		}
		break;
		
		case IDB_NEW_CANCEL:
			ShowWindow(g_hwndNewPrompt, SW_HIDE);
			break;
			
		case IDM_FILE_CLOSE:
			MapDestroy(g_pMap);
			g_pMap = 0;
			InvalidateRect(g_hWnd, 0, TRUE);
			break;
		
		case IDM_FILE_EXIT:
			PostQuitMessage(0);
			break;
		
		case IDM_HELP_HOWTOUSE:
			ShowWindow(g_hwndHelp, SW_SHOW);
			break;
		
		case IDB_CLOSE_HOWTOUSE:
			ShowWindow(g_hwndHelp, SW_HIDE);
			break;
		
		case IDM_FILE_OPEN:
		{
			OPENFILENAME ofn;
			
			memset(g_str, 0, 1024);
			
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = g_hWnd;
			ofn.hInstance = g_hInstance;
			ofn.lpstrFilter = "Map File\0*.MAP\0";
			ofn.lpstrCustomFilter = 0;
			ofn.nMaxCustFilter = 0;
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = g_str;
			ofn.nMaxFile = 1024;
			ofn.lpstrFileTitle = 0;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = 0;
			ofn.lpstrTitle = 0;
			ofn.Flags = OFN_FILEMUSTEXIST;
			ofn.nFileOffset = 0;
			ofn.nFileExtension = 0;
			ofn.lpstrDefExt = 0;
			ofn.lCustData = 0;
			ofn.lpfnHook = 0;
			ofn.lpTemplateName = 0;
			ofn.pvReserved = 0;
			ofn.dwReserved = 0;
			ofn.FlagsEx = 0;
			
			if (GetOpenFileName(&ofn)) {
				MAP * pMap;
				
				// Open map
				pMap = MapOpen(g_str);
				if (pMap) {
					// Replace
					MapDestroy(g_pMap);
					g_pMap = pMap;
				}
			}
		}
			break;
		
		case IDM_FILE_SAVE_AS:
		{
			OPENFILENAME ofn;
			
			memset(g_str, 0, 1024);
			
			ofn.lStructSize = sizeof(OPENFILENAME);
			ofn.hwndOwner = g_hWnd;
			ofn.hInstance = g_hInstance;
			ofn.lpstrFilter = "Map File\0*.MAP\0";
			ofn.lpstrCustomFilter = 0;
			ofn.nMaxCustFilter = 0;
			ofn.nFilterIndex = 1;
			ofn.lpstrFile = g_str;
			ofn.nMaxFile = 1024;
			ofn.lpstrFileTitle = 0;
			ofn.nMaxFileTitle = 0;
			ofn.lpstrInitialDir = 0;
			ofn.lpstrTitle = 0;
			ofn.Flags = 0;
			ofn.nFileOffset = 0;
			ofn.nFileExtension = 0;
			ofn.lpstrDefExt = 0;
			ofn.lCustData = 0;
			ofn.lpfnHook = 0;
			ofn.lpTemplateName = 0;
			ofn.pvReserved = 0;
			ofn.dwReserved = 0;
			ofn.FlagsEx = 0;
			
			if (GetSaveFileName(&ofn)) {
				char * pc = g_str;
				char * pd = 0;
				
				// Terminate string correctly
				for (; *pc; pc ++) {
					if (*pc == '.') pd = pc;
				} if (pd) {
					pd ++; if (*pd == 'M' || *pd == 'm') {
						pd ++; if (*pd == 'A' || *pd == 'a') {
							pd ++; if (*pd == 'P' || *pd == 'p') {
								pd ++; if (!(*pd)) goto L_FSA_Save;
							}
						}
					}
				} *pc = '.';
				*(++pc) = 'm';
				*(++pc) = 'a';
				*(++pc) = 'p';
				*(++pc) = 0;
				
L_FSA_Save:
				// Save map
				MapSave(g_pMap, g_str);
			}
		}
			break;
		
		default: break;
		}
		break;
	
	case WM_SIZE:
		if (hWnd == g_hWnd)
		{
			RECT rct;
			BOOL bResize = FALSE;
			
			// Get client rectangle
			GetClientRect(g_hWnd, &rct);
			
			// Ensure it is big enough to accommodate map and panel
			if (rct.right - rct.left < 600) {
				rct.right = rct.left + 600;
				bResize = TRUE;
			} if (rct.bottom - rct.top < 400) {
				rct.bottom = rct.top + 400;
				bResize = TRUE;
			} if (bResize) {
				// Resize if necessary
				AdjustWindowRect(&rct, WS_CUSTOM, TRUE);
				SetWindowPos(g_hWnd, 0, rct.left, rct.top, rct.right - rct.left, rct.bottom - rct.top, SWP_NOMOVE);
			}
			
			// Reposition interface
			SetWindowPos(g_hcmbMethod,		0,	rct.right - 184, rct.top + 16, 168, 32, 0);
			SetWindowPos(g_hbtnCompute,		0,	rct.right - 184, rct.top + 64, 168, 32, 0);
			SetWindowPos(g_hbtnRedraw,		0,	rct.right - 184, rct.top + 112, 168, 32, 0);
			SetWindowPos(g_hcmbIllustrate,	0,	rct.right - 184, rct.top + 160, 168, 32, 0);
			SetWindowPos(g_htxtDelay,		0,	rct.right - 88 , rct.top + 208, 64, 32, 0);
			
			// Set viewport
			rct.right -= 200;
			MapSetViewport(g_pMap, &rct);
			
			return 0;
		}
		break;
		
	case WM_LBUTTONDOWN:
		SetFocus(g_hWnd);
		g_bCreatingWall = 1;
		break;
		
	case WM_RBUTTONDOWN:
		SetFocus(g_hWnd);
		g_bCreatingWall = 2;
		break;
		
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		g_bCreatingWall = 0;
		break;
		
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'S':
			// Set start node
			if (g_pMap) { // If there is an existing map
				NODE * n;
				
				// Get node under cursor
				n = MapGetAtCursor(g_pMap);
				
				// If the node is valid, use it
				if (n) {
					// Render old node normally
					MapRenderNode(g_pMap, g_pMap -> pStart);
					
					// Render new node
					MapRenderStartNode(g_pMap, n);
					
					// Store new start node
					g_pMap -> pStart = n;
				}
			}
			break;
		case VK_SPACE:
			// Set end node
			if (g_pMap) { // If there is an existing map
				NODE * n;
				
				// Get node under cursor
				n = MapGetAtCursor(g_pMap);
				
				// If the node is valid, use it
				if (n) {
					// Render old node normally
					MapRenderNode(g_pMap, g_pMap -> pEnd);
					
					// Render new node
					MapRenderEndNode(g_pMap, n);
					
					// Store new end node
					g_pMap -> pEnd = n;
				}
			}
			break;
			
		case 'P':
			// Get details of node
			if (g_pMap) { // If there is an existing map
				NODE * n;
				
				// Get node under cursor
				n = MapGetAtCursor(g_pMap);
				
				if (n) { // If the node is valid
					sprintf(g_str, "(%u, %u)\n"
						"G: %u\nH: %u\nF: %u\nState: %u",
						n -> x, n -> y, n -> g, n -> h, n -> f, n -> s);
					MessageBoxA(g_hWnd, g_str, "Info:", MB_ICONASTERISK);
				}
			}
			break;
		}
		break;
		
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

