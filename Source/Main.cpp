#include <Windows.h>
#include <iostream>
#include <strsafe.h>
#include <codecvt>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <dwmapi.h>

#define WINDOW_RATIO 1.6625f
#define BOARD_ROWS buttonTexts.size()
#define BOARD_COLUMNS buttonTexts[0].size()
#define BUTTON_SPACING 10
#define BOARD_POS(h) Vector2i(-(BUTTON_SPACING / 2), static_cast<int>(h / 4.0f - BUTTON_SPACING / 2)) 
#define DEFAULT_SIZE 16
#define DEFAULT_FONT(s) CreateFont(s, 0, 0, 0, 0, FALSE, FALSE, FALSE, ANSI_CHARSET, \
								OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, \
								DEFAULT_PITCH | FF_DONTCARE, L"Segoe MDL2 Assets")
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

void InputParser(std::wstring input, double& num1, double& num2, wchar_t& op)
{
	try
	{
		size_t pos = std::wstring::npos;

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
			}

			num1 = std::stod(temp);
			temp = L"";

			for (int i = pos + 1; i < input.size(); i++)
			{
				temp += input[i];
			}

			if (temp.empty()) num2 = num1;
			else num2 = std::stod(temp);

			temp = L"";

			op = input[pos];
		}
		else
			num1 = std::stod(input);
	}
	catch (std::exception& e)
	{
		MessageBoxW(NULL, str2wstr(e.what()).c_str(), L"Error!", MB_ICONEXCLAMATION | MB_OK);
	}
}

double CalculateInput(std::wstring input)
{
	double num1 = 0, num2 = 0, answer = 0;
	wchar_t op;
	InputParser(input, num1, num2, op);

	if (num1 != 0 && num2 != 0)
	{
		switch (op)
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
	else
		answer = num1;

	return answer;
}

void NegateNumber(std::wstring& input)
{
	if (wcscmp(input.c_str(), L"0") == 0)
		return;

	double num1 = 0, num2 = 0;
	wchar_t op = L'\0';

	InputParser(input, num1, num2, op);

	if (op == L'\0')
	{
		// Only one number is present
		num1 = -num1;
		input = double2wstr(num1);
	}
	else
	{
		// Two numbers are present
		num2 = -num2;
		input = double2wstr(num1) + op + double2wstr(num2);
	}
}

void EraseFinalNumber(std::wstring& input)
{
	double num1 = 0, num2 = 0;
	wchar_t op = L'\0';

	InputParser(input, num1, num2, op);

	if (op == L'\0')
	{
		input = L"0";
	}
	else
	{
		input = double2wstr(num1) + op;
	}
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

std::unordered_map<wchar_t, wchar_t> UnicodeMap =
{
	{ L'\uE948', L'+' }, // Calculator Addition
	{ L'\uE949', L'-' }, // Calculator Subtract
	{ L'\uE947', L'*' }, // Calculator Multiply
	{ L'\uE94A', L'/' }, // Calculator Divide
};

std::wstring TranslateUnicode(std::wstring wstr)
{
	std::wstring out = wstr;
	for (int i = 0; i < out.size(); i++)
	{
		auto it = UnicodeMap.find(out[i]);
		if (it != UnicodeMap.end()) {
			out[i] = it->second;
		}
	}

	return out;
}

std::vector<std::vector<std::wstring>> buttonTexts =
{
	{L"\uE94D", L"CE", L"C", L"\uE94F"},
	{L"7",      L"8",  L"9", L"\uE94A"},
	{L"4",      L"5",  L"6", L"\uE947"},
	{L"1",      L"2",  L"3", L"\uE949"},
	{L"\uE94E", L"0",  L".", L"\uE948"}
};

bool Filter(const std::vector<std::wstring>& filter, const wchar_t* target)
{
	for (const auto& str : filter)
	{
		if (wcscmp(str.c_str(), target) == 0)
		{
			return true; // Match found
		}
	}
	return false; // No match found
}


std::vector<HWND> buttons;
HWND inputBox;
std::wstring input = L"0", prevInput, answer;

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_CREATE:
	{
		HFONT hFont = DEFAULT_FONT(DEFAULT_SIZE);

		RECT size{};
		GetClientRect(hWnd, &size);

		int width = size.right - size.left, height = size.bottom - size.top;
		Vector2i boardPos = BOARD_POS(height);
		const int spacing = BUTTON_SPACING;
		Vector2i buttonSize = Vector2i(width / 4 - spacing, (height - boardPos.y) / 4 - spacing);

		for (int row = 0; row < BOARD_ROWS; row++)
		{
			for (int col = 0; col < BOARD_COLUMNS; col++)
			{
				HWND hButton = CreateWindowExW(
					0L,
					L"Button",                   // Predefined class; Unicode assumed
					buttonTexts[row][col].c_str(),                   // Button text
					WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,  // Styles
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
		SendMessage(inputBox, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(0, TRUE));

		DeleteObject(hFont);

		break;
	}

	case WM_DRAWITEM:
	{
		LPDRAWITEMSTRUCT lpdis = reinterpret_cast<LPDRAWITEMSTRUCT>(lParam);

		if (lpdis->CtlID >= 1 && lpdis->CtlID <= buttons.size())
		{
			TEXTMETRIC tm;
			GetTextMetrics(lpdis->hDC, &tm);

			WCHAR symbol[3];
			GetWindowText(lpdis->hwndItem, symbol, 3);

			SIZE textSize;
			GetTextExtentPoint32(lpdis->hDC, symbol, wcslen(symbol), &textSize);

			// Calculate button dimensions
			int buttonWidth = lpdis->rcItem.right - lpdis->rcItem.left;
			int buttonHeight = lpdis->rcItem.bottom - lpdis->rcItem.top;

			// Define the roundness of the rectangle (change this value to adjust the roundness)
			int cornerRadius = 10;

			// Draw the round rectangle button
			HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
			HBRUSH hBrush = CreateSolidBrush(RGB(59, 59, 59));
			SelectObject(lpdis->hDC, hPen);
			SelectObject(lpdis->hDC, hBrush);
			RoundRect(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, lpdis->rcItem.right, lpdis->rcItem.bottom, cornerRadius, cornerRadius);

			// Set the background mode and background color for the text
			SetBkMode(lpdis->hDC, TRANSPARENT);
			SetBkColor(lpdis->hDC, RGB(59, 59, 59));

			// Calculate the position to center the text inside the button
			int textX = (buttonWidth - textSize.cx) / 2;
			int textY = (buttonHeight - textSize.cy) / 2;

			// Set the text color to white
			SetTextColor(lpdis->hDC, RGB(255, 255, 255));

			// Draw the text inside the button
			DrawText(lpdis->hDC, symbol, -1, &lpdis->rcItem, DT_SINGLELINE | DT_CENTER | DT_VCENTER);

			DeleteObject(hPen);
			DeleteObject(hBrush);
		}

		break;
	}

	case WM_CTLCOLORSTATIC:
	{
		HDC hdcStatic = (HDC)wParam;
		SetBkColor(hdcStatic, RGB(32, 32, 32));  // Set the background color (red in this example)
		SetTextColor(hdcStatic, RGB(255, 255, 255));  // Set the text color (green in this example)
		return (LRESULT)GetStockObject(NULL_BRUSH);  // Return the handle to a null brush to prevent background erasure
	}

	case WM_CHAR:
	{
		int keyCode = (int)wParam;
		std::wstring op = L"+-*/";

		switch (keyCode)
		{
		case VK_BACK:
			if(input.length() > 0)
		{
			input = input.substr(0, input.length() - 1);
			SetWindowText(inputBox, input.c_str());
		}
			break;

		case 0x0D:
			answer = double2wstr(CalculateInput(input));
			input = L"0";
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

		/* Uncomment if you want when user resize window ratio will use
		{
			RECT size{};
			GetWindowRect(hWnd, &size);

			int width = min(size.right - size.left, size.bottom - size.top), height = width * WINDOW_RATIO;
			SetWindowPos(hWnd, NULL, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
		}
		*/

		{
			RECT size{};
			GetClientRect(hWnd, &size);

			int width = size.right - size.left, height = size.bottom - size.top;
			Vector2i boardPos = BOARD_POS(height);
			const int spacing = BUTTON_SPACING;
			Vector2i buttonSize = Vector2i(width / BOARD_COLUMNS - spacing, (height - boardPos.y) / BOARD_ROWS - spacing);

			HWND targetButton = buttons[0 * BOARD_COLUMNS + 0];

			RECT button;
			GetClientRect(targetButton, &button);

			// Calculate the new font size based on the window size
			int newFontSize = ((button.right - button.left) + (button.bottom - button.top)) / 4;

			// Create a new font with the updated size
			HFONT hFont = DEFAULT_FONT(newFontSize);

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

			SetWindowPos(inputBox, NULL, 0, 0, width, boardPos.y - 0, SWP_NOZORDER);
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

		// special button char
		std::vector<std::wstring> special = { L"\uE94D", L"CE", L"C", L"\uE94F", L"\uE94E" };
		std::vector<std::wstring> op = { L"\uE948", L"\uE949", L"\uE947", L"\uE94A" };

		// Check if the button ID corresponds to one of our buttons
		if (buttonID >= 1 && buttonID <= buttons.size())
		{
			int buttonIndex = buttonID - 1;
			HWND hButton = buttons[buttonIndex];

			WCHAR temp[3];
			GetWindowText(hButton, temp, 3);

			if (!Filter(special, temp))
			{
				if (input.size() != 0 && input[0] == L'0') input.erase(0, 1);
				input += TranslateUnicode(temp);
			}
			else
			{
				if (wcscmp(temp, special[0].c_str()) == 0)
				{
					NegateNumber(input);
				}
				else if (wcscmp(temp, special[1].c_str()) == 0)
				{
					EraseFinalNumber(input);
				}
				else if (wcscmp(temp, special[2].c_str()) == 0)
				{
					input.clear();
				}
				else if (wcscmp(temp, special[3].c_str()) == 0)
				{
					if (!input.empty()) input.pop_back();
				}
				else if (wcscmp(temp, special[4].c_str()) == 0)
				{
					answer = double2wstr(CalculateInput(input));
					input = answer;
				}

				
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
		if (input != prevInput) {
			if (answer.empty()) {
				SetWindowTextW(inputBox, input.c_str());
				RECT inputBoxRect = {};
				GetClientRect(inputBox, &inputBoxRect);
				InvalidateRect(hWnd, &inputBoxRect, TRUE);
				prevInput = input;
			}
			else {
				SetWindowTextW(inputBox, (prevInput + L"=" + answer).c_str());
				RECT inputBoxRect = {};
				GetClientRect(inputBox, &inputBoxRect);
				InvalidateRect(hWnd, &inputBoxRect, TRUE);
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
	wcex.hbrBackground = CreateSolidBrush(RGB(32, 32, 32));
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"Window";
	wcex.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex)) ErrorExit(L"RegisterClassEx");

	int width = 320, height = width * WINDOW_RATIO;

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
