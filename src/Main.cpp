#include "Calculator.hpp"
#include <Windows.h>
#include <strsafe.h>
#include <dwmapi.h>

void ErrorExit(LPCWSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(dw);
}

Calculator W32Calc;

// Step 4: the Window Procedure
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
		case WM_CREATE:
			W32Calc.SetupCalculator(hwnd);
			break;

		case WM_DRAWITEM:
			W32Calc.HandleCustomButton(lParam);
			break;

		case WM_SIZE:
			W32Calc.ResizeCalculator(hwnd);
			break;

		case WM_COMMAND:
			W32Calc.HandleButtonInput(wParam, hwnd);
			break;

		case WM_CHAR:
			W32Calc.HandleKeyboardInput(wParam);
			break;

		case WM_CTLCOLORSTATIC:
			return W32Calc.ChangeStaticColor(wParam);

        case WM_CLOSE:
            DestroyWindow(hwnd);
        	break;
        case WM_DESTROY:
            PostQuitMessage(0);
        	break;
        default:
			W32Calc.UpdateInputbox(hwnd);
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = CreateSolidBrush(RGB(32, 32, 32));
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"Window";
	wcex.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex)) 	(L"RegisterClassEx");

	int width = 320, height = width * 1.6625f;

	// Step 2: Creating the Window
	HWND hWnd = CreateWindowEx(
		WS_EX_DLGMODALFRAME,
		L"Window",
		L"Calculator",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height,
		NULL, NULL, hInstance, NULL);

	if (hWnd == nullptr) ErrorExit(L"CreateWindowEx");

	BOOL UseDarkMode = TRUE;
	DwmSetWindowAttribute(hWnd, DWMWINDOWATTRIBUTE::DWMWA_USE_IMMERSIVE_DARK_MODE, &UseDarkMode, sizeof(UseDarkMode));

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	MSG Msg{};
	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}