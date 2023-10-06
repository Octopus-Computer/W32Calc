#include "Calculator.hpp"

void Calculator::SetupCalculator(HWND hWnd)
{
    HFONT hFont = GENERATE_FONT(DEFAULT_SIZE);

    RECT size{};
    GetClientRect(hWnd, &size);

    int width = size.right - size.left, height = size.bottom - size.top;
    Vector2i boardPos = BOARD_POS(height);
    const int spacing = BUTTON_SPACING;
    Vector2i buttonSize = Vector2i(width / 4 - spacing, (height - boardPos.y) / 4 - spacing);

    for (int row = 0; row < buttonsText.size(); row++)
    {
        for (int col = 0; col < buttonsText[0].size(); col++)
        {
            HWND hButton = CreateWindowExW(
                0L,
                L"Button",                   // Predefined class; Unicode assumed
                buttonsText[row][col].c_str(),                   // Button text
                WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_OWNERDRAW,  // Styles
                (spacing + (buttonSize.x + spacing) * col) + boardPos.x,     // X position
                (spacing + (buttonSize.y + spacing) * row) + boardPos.y,                  // Y position
                buttonSize.x,                  // Button width
                buttonSize.y,                  // Button height
                hWnd,                        // Parent window
                reinterpret_cast<HMENU>(row * buttonsText[0].size() + col + 1),    // Button ID
                (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
                NULL);                       // Pointer not needed
            SendMessage(hButton, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(0, TRUE));

            buttons.push_back(hButton); // Store button handle
        }
    }

    inputBox = CreateWindowEx(0L, L"Static", input.c_str(), WS_VISIBLE | WS_CHILD, 0, 0, width, boardPos.y - 0, hWnd, NULL, NULL, NULL);
    SendMessage(inputBox, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(0, TRUE));

    DeleteObject(hFont);
}

void Calculator::HandleCustomButton(LPARAM lParam)
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

        // Draw the round rectangle button
        HPEN hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));
        HBRUSH hBrush = CreateSolidBrush(RGB(59, 59, 59));
        SelectObject(lpdis->hDC, hPen);
        SelectObject(lpdis->hDC, hBrush);
        Rectangle(lpdis->hDC, lpdis->rcItem.left, lpdis->rcItem.top, lpdis->rcItem.right, lpdis->rcItem.bottom);

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
}

void Calculator::ResizeCalculator(HWND hWnd)
{
    RECT size{};
    GetClientRect(hWnd, &size);

    int width = size.right - size.left, height = size.bottom - size.top;
    Vector2i boardPos = BOARD_POS(height);
    const int spacing = BUTTON_SPACING;
    Vector2i buttonSize = Vector2i(width / buttonsText[0].size() - spacing, (height - boardPos.y) / buttonsText.size() - spacing);

    HWND targetButton = buttons[0 * buttonsText[0].size() + 0];

    RECT button;
    GetClientRect(targetButton, &button);

    // Calculate the new font size based on the window size
    int newFontSize = ((button.right - button.left) + (button.bottom - button.top)) / 4;

    // Create a new font with the updated size
    HFONT hFont = GENERATE_FONT(newFontSize);

    for (int row = 0; row < buttonsText.size(); row++)
    {
        for (int col = 0; col < buttonsText[0].size(); col++)
        {
            HWND hButton = buttons[row * buttonsText[0].size() + col]; // Get the button handle from the stored vector

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

void Calculator::HandleButtonInput(WPARAM wParam, HWND hWnd)
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
}

void Calculator::HandleKeyboardInput(WPARAM wParam)
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
}

LRESULT Calculator::ChangeStaticColor(WPARAM wParam)
{
    HDC hdcStatic = (HDC)wParam;
    SetBkColor(hdcStatic, RGB(32, 32, 32));
    SetTextColor(hdcStatic, RGB(255, 255, 255)); 
    return (LRESULT)GetStockObject(NULL_BRUSH);  // Return the handle to a null brush to prevent background erasure
}

void Calculator::UpdateInputbox(HWND hWnd)
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
}

HFONT Calculator::GENERATE_FONT(int FontSize)
{
    LOGFONT lf = {};
    lf.lfHeight = -FontSize;
    lf.lfWeight = FW_NORMAL;
    lf.lfCharSet = DEFAULT_CHARSET;
    lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality = DEFAULT_QUALITY;
    lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
    wcscpy_s(lf.lfFaceName, L"Segoe MDL2 Assets");

    return CreateFontIndirect(&lf);
}

Vector2i Calculator::BOARD_POS(int height)
{
    return Vector2i(-(BUTTON_SPACING / 2), static_cast<int>(height / 4.0f - BUTTON_SPACING / 2));
}

bool Calculator::Filter(const std::vector<std::wstring>& filter, const wchar_t* target)
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

std::wstring Calculator::TranslateUnicode(std::wstring wstr)
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

void Calculator::InputParser(std::wstring input, double& num1, double& num2, wchar_t& op)
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

double Calculator::CalculateInput(std::wstring input)
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

void Calculator::NegateNumber(std::wstring& input)
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

void Calculator::EraseFinalNumber(std::wstring& input)
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

std::wstring Calculator::str2wstr(std::string str)
{
	std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
	return converter.from_bytes(str);
}

std::wstring Calculator::double2wstr(double number)
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
