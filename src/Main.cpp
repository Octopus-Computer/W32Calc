#include "Calculator.hpp"
#include <windows.h>

const wchar_t g_szClassName[] =L"myWindowClass";

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

int main()
{
	HINSTANCE hInstance = GetModuleHandle(NULL);

    WNDCLASSEX wc;
    HWND hwnd;
    MSG Msg;

    //Step 1: Registering the Window Class
    wc.cbSize        = sizeof(WNDCLASSEX);
    wc.style         = 0;
    wc.lpfnWndProc   = WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = g_szClassName;
    wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

    if(!RegisterClassEx(&wc))
    {
        MessageBox(NULL,L"Window Registration Failed!",L"Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // Step 2: Creating the Window
    hwnd = CreateWindowEx(
        WS_EX_CLIENTEDGE,
        g_szClassName,
       L"The title of my window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 1000,
        NULL, NULL, hInstance, NULL);

    if(hwnd == NULL)
    {
        MessageBox(NULL,L"Window Creation Failed!",L"Error!",
            MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Step 3: The Message Loop
    while(GetMessage(&Msg, NULL, 0, 0) > 0)
    {
        TranslateMessage(&Msg);
        DispatchMessage(&Msg);
    }
    return Msg.wParam;
}