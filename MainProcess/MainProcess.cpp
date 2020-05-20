// MainProcess.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MainProcess.h"

#include <assert.h>
#include <sstream>
#include <atomic>
#include <string.h>
#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
ATOM                MyRegisterClass2(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

const size_t BUF_SIZE = 256;
struct Payload {
	std::atomic<bool> is_set = false;
	HWND hwnd;
	char data[64];
};
static_assert(sizeof(Payload) < BUF_SIZE, "");

Payload* g_Payload = nullptr;
void CreateSharedMemory(HWND parentHwnd) {
	HANDLE hMapFile;
	LPCTSTR pBuf;

	hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,    // use paging file
		NULL,                    // default security
		PAGE_READWRITE,          // read/write access
		0,                       // maximum object size (high-order DWORD)
		sizeof(Payload),                // maximum object size (low-order DWORD)
		L"Local\\drgn_test_mem");                 // name of mapping object

	if (hMapFile == NULL)
	{
		DWORD err = GetLastError();
		_tprintf(TEXT("Could not create file mapping object (%d).\n"),
			err);
		assert(false);
	}
	pBuf = (LPTSTR)MapViewOfFile(hMapFile,   // handle to map object
		FILE_MAP_ALL_ACCESS, // read/write permission
		0,
		0,
		BUF_SIZE);

	g_Payload = (Payload*)pBuf;
	new (g_Payload) Payload();
	assert(g_Payload->is_set.load() == false);
}

HWND childHwnd = 0;
void ReadSharedMemory() {
	while (!g_Payload->is_set.load()) {
		childHwnd = g_Payload->hwnd;
	}
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);


	// TODO: Place code here.

	// Initialize global strings
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_MAINPROCESS, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	MyRegisterClass2(hInstance);

	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MAINPROCESS));

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

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINPROCESS));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

ATOM MyRegisterClass2(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	HBRUSH hBrush = CreateSolidBrush(RGB(200, 200, 200));


	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAINPROCESS));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = nullptr;
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = L"MyClass2";
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

struct handle_data {
	unsigned long process_id;
	HWND window_handle = 0;
};

BOOL CALLBACK enum_windows_callback(HWND handle, LPARAM lParam)
{
	handle_data& data = *(handle_data*)lParam;
	unsigned long process_id = 0;
	GetWindowThreadProcessId(handle, &process_id);
	std::stringstream sstr;
	sstr << "pid: " << process_id << "\n";
	OutputDebugStringA(sstr.str().c_str());
	if (data.process_id != process_id)
		return TRUE;
	data.window_handle = handle;
	return FALSE;
}

HWND find_main_window(unsigned long process_id)
{
	handle_data data;
	data.process_id = process_id;
	data.window_handle = 0;
	EnumWindows(enum_windows_callback, (LPARAM)&data);
	return data.window_handle;
}

STARTUPINFO si;
PROCESS_INFORMATION pi;
void CreateChildWindow() {

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	TCHAR arg[1024];
	lstrcpyW(arg, L"ChildWindowProcess.exe noshow");
	//lstrcpyW(arg, L"notepad.exe");

	BOOL b = CreateProcess(NULL,   // No module name (use command line)
		arg,        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi);           // Pointer to PROCESS_INFORMATION structure

	assert(b);
}

void DestroyChildWindow() {

}

void InvalidateChildWindow() {

	RECT rect;
	GetWindowRect(childHwnd, &rect);
	rect.left = 0;
	rect.top = 0;
	InvalidateRect(childHwnd, &rect, true);
	UpdateWindow(childHwnd);
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

HWND otherHwnd;
HWND parentHwnd;
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Store instance handle in our global variable

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		0, 0, 650, 650, nullptr, nullptr, hInstance, nullptr);


	HWND hWnd2 = CreateWindowW(L"MyClass2", L"Child", WS_CHILD,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, hWnd, nullptr, hInstance, nullptr);
	otherHwnd = hWnd2;
	parentHwnd = hWnd;

	if (!hWnd)
	{
		return FALSE;
	}

	CreateSharedMemory(hWnd);
	CreateChildWindow();

	ReadSharedMemory();

	//childHwnd = find_main_window(pi.dwProcessId);
	//SetWindowLong(childHwnd, GWL_STYLE, 0);
	assert(childHwnd != 0);
	CreateSharedMemory(hWnd);
	SetParent(childHwnd, hWnd);
	SetParent(otherHwnd, hWnd);
	ShowWindow(hWnd, 0xa);
	ShowWindow(otherHwnd, 0xa);
	ShowWindow(childHwnd, 0xa);

	// Set the position after the window is shown,
	// other showing the window won't trigger a repaint
	SetWindowPos(otherHwnd, 0, 0, 0, 400, 400, 0);
	SetWindowPos(childHwnd, otherHwnd, 0, 0, 400, 400, 0);

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
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		if (otherHwnd == hWnd) {

			//    Rectangle function is defined as...
			//    BOOL Rectangle(hdc, xLeft, yTop, yRight, yBottom);

			//    Drawing a rectangle with just a black pen
			//    The black pen object is selected and sent to the current device context
			//  The default brush is WHITE_BRUSH
			RECT rect;

			GetWindowRect(hWnd, &rect);
			rect.left = 0;
			rect.top = 0;
			HBRUSH hBrush = CreateSolidBrush(RGB(0, 200, 100));
			SetDCBrushColor(hdc, RGB(0, 200, 100));
			RECT r;
			SetRect(&r, 150, 150, 250, 250);
			FillRect(hdc, &r, hBrush);
			DeleteObject(hBrush);
		}
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_LBUTTONDOWN:
	{
		if (hWnd == otherHwnd) {
			PostMessage(childHwnd, WM_LBUTTONDOWN, wParam, lParam);
			RECT rect;
			GetWindowRect(hWnd, &rect);
			rect.left = 0;
			rect.top = 0;
			InvalidateRect(parentHwnd, &rect, true);
		}
	}
	break;
	case WM_SIZE:
	{
		InvalidateChildWindow();
	}
	break;
	case WM_DESTROY:
		PostMessage(childHwnd, WM_DESTROY, 0, 0);
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
