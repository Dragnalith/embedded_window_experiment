// WindowsProject1.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "WindowsProject1.h"

#include <string>
#include <atomic>
#include <assert.h>
#include <string.h>

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

const size_t BUF_SIZE = 256;
struct Payload {
	std::atomic<bool> is_set = false;
	HWND hwnd;
	HWND parentHwnd;
	char data[64];
};
static_assert(sizeof(Payload) < BUF_SIZE, "");

Payload* g_Payload = nullptr;
void OpenSharedMemory() {
	HANDLE hMapFile;
	LPCTSTR pBuf;

	hMapFile = OpenFileMapping(
		FILE_MAP_ALL_ACCESS,   // read/write access
		FALSE,                 // do not inherit the name
		L"Local\\drgn_test_mem");               // name of mapping object

	if (hMapFile == NULL)
	{
		_tprintf(TEXT("Could not open file mapping object (%d).\n"),
			GetLastError());
		assert(false);
	}

	pBuf = (LPTSTR)MapViewOfFile(hMapFile, // handle to map object
		FILE_MAP_ALL_ACCESS,  // read/write permission
		0,
		0,
		BUF_SIZE);

	g_Payload = (Payload*)pBuf;
	assert(g_Payload->is_set.load() == false);
	memcpy(g_Payload->data, "coucou", sizeof("coucou") + 1);
}

bool show = false;
int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

	std::wstring cmdline(lpCmdLine);
	show = (cmdline != L"noshow");
	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;
	HBRUSH hBrush = CreateSolidBrush(RGB(200, 0, 0));


	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT1));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = hBrush;
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	if (!show) {
		OpenSharedMemory();
	}
	HWND hWnd = CreateWindowW(szWindowClass, szTitle, show ? WS_OVERLAPPEDWINDOW : WS_CHILD,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, show ? nullptr : g_Payload->parentHwnd, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		DWORD err = GetLastError();
		return FALSE;
	}

	SetWindowPos(hWnd, 0, 400, 400, 300, 300, 0);



	if (show) {

		ShowWindow(hWnd, nCmdShow);
	}
	else {
		g_Payload->hwnd = hWnd;
		memcpy(g_Payload->data, "done", sizeof("done") + 1);
		g_Payload->is_set = true;
	}
	UpdateWindow(hWnd);

	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
int g_MouseX = 0;
int g_MouseY = 0;
int g_Count = 0;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	{
		g_MouseX = GET_X_LPARAM(lParam);
		g_MouseY = GET_Y_LPARAM(lParam);
		g_Count += 1;
		g_Count %= 4;
		RECT rect;
		GetWindowRect(hWnd, &rect);
		rect.left = 0;
		rect.top = 0;
		if (show) {
			InvalidateRect(hWnd, &rect, true);
		}
		else {
			InvalidateRect(g_Payload->parentHwnd, &rect, true);
		}

	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		//    Rectangle function is defined as...
		//    BOOL Rectangle(hdc, xLeft, yTop, yRight, yBottom);

		//    Drawing a rectangle with just a black pen
		//    The black pen object is selected and sent to the current device context
		//  The default brush is WHITE_BRUSH
		RECT rect;

		GetWindowRect(hWnd, &rect);
		rect.left = 0;
		rect.top = 0;
		HBRUSH hBrush = CreateSolidBrush(RGB(g_Count * 50, 100, 100));

		FillRect(hdc, &rect, hBrush);
		Rectangle(hdc, g_MouseX, g_MouseY, g_MouseX + 10, g_MouseY + 10);
		DeleteObject(hBrush);
		EndPaint(hWnd, &ps);
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

// Message handler for about box.
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
