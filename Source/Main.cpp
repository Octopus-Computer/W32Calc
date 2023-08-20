#include <Windows.h>
#include <iostream>
#include <strsafe.h>
#include <codecvt>
#include <sstream>
#include <vector>

#define WINDOW_RATIO 1.5f
#define BOARD_ROWS 4
#define BOARD_COLUMNS 4 
#define DEFAULT_SIZE 16
#define BOARD_POS(h) Vector2i(0, static_cast<int>(h / 4.0f))

struct Vector2i
{
	int x, y;

	Vector2i() : x(0), y(0) {}
	Vector2i(int x, int y) : x(x), y(y) {}
};

std::wstring str2wstr(std::string str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(str);
}

std::wstring double2wstr(double number)
{
	std::wstringstream ss;
	ss << number;

	std::wstring str = ss.str();

	// Find the position of the decimal point
	size_t decimalPos = str.find(L'.');

	// If a decimal point exists
	if (decimalPos != std::wstring::npos) {
		// Find the position of the last non-zero digit after the decimal point
		size_t lastNonZeroPos = str.find_last_not_of(L'0');

		// If there are trailing '0' digits after the decimal point
		if (lastNonZeroPos > decimalPos) {
			// Remove the trailing '0' digits
			str = str.substr(0, lastNonZeroPos + 1);
		}
	}

	return str;
}

double CalculatorInputParser(std::wstring input)
{
	double answer = 0;
	try
	{
		size_t pos = std::wstring::npos;
		double num1 = 0, num2 = 0;

		std::wstring operators = L"+-*/", temp = L"";

		for (size_t i = 0; i < input.length(); i++) {
			if (operators.find(input[i]) != std::wstring::npos) {
				pos = i;
			}
		}

		if (pos != std::wstring::npos)
		{
			for (int i = 0; i < pos; i++)
			{
				temp += input[i];
				std::cout << "Num1 " << i << std::endl;
			}

			num1 = std::stod(temp);
			temp = L"";

			for (int i = pos + 1; i < input.size(); i++)
			{
				temp += input[i];
				std::cout << "Num2 " << i << std::endl;
			}

			if (temp.empty()) num2 = num1;
			else num2 = std::stod(temp);

			temp = L"";

			switch (input[pos])
			{
			case L'+':
				answer = num1 + num2;
				break;
			case L'-':
				answer = num1 - num2;
				break;

			case L'*':
				answer = num1 * num2;
				break;

			case L'/':
				answer = num1 / num2;
				break;
			}
		}
		else answer = std::stod(input);
	}
	catch (std::exception& e)
	{
		MessageBoxW(NULL, str2wstr(e.what()).c_str(), L"Error!", MB_ICONEXCLAMATION | MB_OK);
	}

	return answer;
}

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

std::vector<std::vector<std::wstring>> buttonTexts =
{
	{L"7", L"8", L"9", L"/"},
	{L"4", L"5", L"6", L"*"},
	{L"1", L"2", L"3", L"-"},
	{L"=", L"0", L".", L"+"}
};

std::vector<HWND> buttons;
HWND inputBox;
std::wstring input, prevInput, answer;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
	{

		HFONT hFont = CreateFont(DEFAULT_SIZE, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET,
			OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			DEFAULT_PITCH | FF_DONTCARE, TEXT("Segoe UI"));

		RECT size{};
		GetClientRect(hWnd, &size);

		int width = size.right - size.left, height = size.bottom - size.top;
		Vector2i boardPos = BOARD_POS(height);
		const int spacing = 10;
		Vector2i buttonSize = Vector2i(width / 4 - spacing, (height - boardPos.y) / 4 - spacing);

		for (int row = 0; row < BOARD_ROWS; row++)
		{
			for (int col = 0; col < BOARD_COLUMNS; col++)
			{
				HWND hButton = CreateWindowExW(
					0L,
					L"Button",                   // Predefined class; Unicode assumed
					buttonTexts[row][col].c_str(),                   // Button text
					WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles
					(spacing + (buttonSize.x + spacing) * col) + boardPos.x,     // X position
					(spacing + (buttonSize.y + spacing) * row) + boardPos.y,                  // Y position
					buttonSize.x,                  // Button width
					buttonSize.y,                  // Button height
					hWnd,                        // Parent window
					reinterpret_cast<HMENU>(row * BOARD_COLUMNS + col + 1),    // Button ID
					(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
					NULL);                       // Pointer not needed
				SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(0, TRUE));

				buttons.push_back(hButton); // Store button handle
			}
		}

		inputBox = CreateWindowEx(0L, L"Static", input.c_str(), WS_VISIBLE | WS_CHILD, 0, 0, width, boardPos.y - 0, hWnd, NULL, NULL, NULL);

		DeleteObject(hFont);
	}

	case WM_CHAR:
	{
		int keyCode = (int)wParam;
		std::wstring op = L"+-*/";

		switch (keyCode)
		{
		case VK_BACK:
			if (!input.empty()) input.pop_back();
			break;

		case 0x0d:
			answer = double2wstr(CalculatorInputParser(input));
			input.clear();
			break;

		default:
			for (int k = 0; k < op.size(); k++)
			{
				if (wParam == op[k]) input += op[k];
			}

			if (keyCode >= 0x30 && keyCode <= 0x39) /* ranges 0...9 */
			{
				input += (wchar_t)keyCode;
			}

			break;
		}
		break;
	}

	case WM_SIZE:
	{
		{
			RECT size{};
			GetWindowRect(hWnd, &size);

			int width = min(size.right - size.left, size.bottom - size.top), height = width * WINDOW_RATIO;
			SetWindowPos(hWnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
		}

		{
			RECT size{};
			GetClientRect(hWnd, &size);

			int width = size.right - size.left, height = size.bottom - size.top;
			Vector2i boardPos = BOARD_POS(height);
			const int spacing = 10;
			Vector2i buttonSize = Vector2i(width / BOARD_COLUMNS - spacing, (height - boardPos.y) / BOARD_ROWS - spacing);

			int targetRow = 2; // Row index of the button
			int targetColumn = 3; // Column index of the button

			HWND targetButton = buttons[targetRow * BOARD_COLUMNS + targetColumn];

			RECT button;
			GetClientRect(targetButton, &button);

			// Calculate the new font size based on the window size
			int newFontSize = ((button.right - button.left) + (button.bottom - button.top)) / 4;

			// Create a new font with the updated size
			HFONT hFont = CreateFont(
				newFontSize,          // Font height
				0,                    // Font width
				0,                    // Font escapement
				0,                    // Font orientation
				FW_NORMAL,            // Font weight
				FALSE,                // Font italic
				FALSE,                // Font underline
				FALSE,                // Font strikeout
				DEFAULT_CHARSET,      // Character set
				OUT_DEFAULT_PRECIS,   // Output precision
				CLIP_DEFAULT_PRECIS,  // Clipping precision
				DEFAULT_QUALITY,      // Font quality
				DEFAULT_PITCH,        // Font pitch and family
				L"Segoe UI"               // Font face name
			);

			for (int row = 0; row < BOARD_ROWS; row++)
			{
				for (int col = 0; col < BOARD_COLUMNS; col++)
				{
					HWND hButton = buttons[row * BOARD_COLUMNS + col]; // Get the button handle from the stored vector

					SetWindowPos(
						hButton,
						NULL,
						(spacing + (buttonSize.x + spacing) * col) + boardPos.x,
						(spacing + (buttonSize.y + spacing) * row) + boardPos.y,
						buttonSize.x,
						buttonSize.y,
						SWP_NOZORDER);
					// Set the new font for the button
					SendMessage(hButton, WM_SETFONT, WPARAM(hFont), TRUE);

					// Invalidate the button to trigger a repaint
					InvalidateRect(hButton, nullptr, TRUE);
				}
			}

			SetWindowPos( inputBox, NULL, 0, 0, width, boardPos.y - 0, SWP_NOZORDER);
			// Set the new font for the button
			SendMessage(inputBox, WM_SETFONT, WPARAM(hFont), TRUE);

			// Invalidate the button to trigger a repaint
			InvalidateRect(inputBox, nullptr, TRUE);
		}

		break;
	}

	case WM_COMMAND:
	{
		// Handle button press
		int buttonID = LOWORD(wParam);

		// Check if the button ID corresponds to one of our buttons
		if (buttonID >= 1 && buttonID <= buttons.size())
		{
			int buttonIndex = buttonID - 1;
			HWND hButton = buttons[buttonIndex];

			WCHAR temp[2];
			GetWindowText(hButton, temp, 2);

			if (wcscmp(temp, L"=") != 0)
			{
				input += temp;
			}
			else
			{
				answer = double2wstr(CalculatorInputParser(input));
				input = answer;
			}

			SetFocus(hWnd);
		}

		break;
	}

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
	{
		if (input != prevInput)
		{
			if (answer.empty())
			{
				SetWindowTextW(inputBox, input.c_str());
				prevInput = input;
			}
			else
			{
				SetWindowTextW(inputBox, (prevInput + L"=" + answer).c_str());
				prevInput = input;
			}

			answer.clear();
		}

		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
}
	return 0;
}

#ifdef _CONSOLE
int main()
#elif _WINDOWS
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowcexmd)
#endif
{
#ifdef _CONSOLE
	HINSTANCE hInstance = GetModuleHandle(NULL);
#endif

	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = 0;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"Window";
	wcex.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex)) ErrorExit(L"RegisterClassEx");

	int width = 200, height = width * WINDOW_RATIO;

	// Step 2: Creating the Window
	HWND hWnd = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		L"Window",
		L"Calculator",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height,
		NULL, NULL, hInstance, NULL);

	if (hWnd == nullptr) ErrorExit(L"CreateWindowEx");

#ifdef _WINDOWS 
	ShowWindow(hwnd, nCmdShow); 
#elif _CONSOLE
	ShowWindow(hWnd, SW_SHOW);
#endif
	UpdateWindow(hWnd);

	MSG Msg{};
	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}