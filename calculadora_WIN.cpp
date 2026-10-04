#ifndef UNICODE
#define UNICODE
#endif

#include <windows.h>
#include <commctrl.h>
#include <vector>
#include <string>
#include <sstream>

#pragma comment(lib, "comctl32.lib")

enum ControlIDs {
    ID_DISPLAY = 100,
    ID_HISTORY,
    ID_BTN_0 = 1000, ID_BTN_1, ID_BTN_2, ID_BTN_3, ID_BTN_4,
    ID_BTN_5, ID_BTN_6, ID_BTN_7, ID_BTN_8, ID_BTN_9,
    ID_BTN_ADD, ID_BTN_SUB, ID_BTN_MUL, ID_BTN_DIV,
    ID_BTN_EQ, ID_BTN_C, ID_BTN_CE, ID_BTN_BACK, ID_BTN_SIGN, ID_BTN_DOT
};

HWND hDisplay, hStaticHistory, hMainWnd;
HFONT hFontMain, hFontDisplay;
std::wstring current_entry = L"0";
std::wstring history = L"";
double numA = 0, numB = 0;
wchar_t pending_op = 0;
bool next_entry_clears = true;

// Puntero al WndProc original de los botones (para subclassing)
WNDPROC g_OrigButtonProc = NULL;

struct ButtonDef {
    const wchar_t* label;
    int id;
};

ButtonDef buttons[] = {
    { L"CE", ID_BTN_CE }, { L"C", ID_BTN_C },   { L"\u232B", ID_BTN_BACK }, { L"\u00F7", ID_BTN_DIV },
    { L"7", ID_BTN_7 },   { L"8", ID_BTN_8 },   { L"9", ID_BTN_9 },         { L"\u00D7", ID_BTN_MUL },
    { L"4", ID_BTN_4 },   { L"5", ID_BTN_5 },   { L"6", ID_BTN_6 },         { L"\u2212", ID_BTN_SUB },
    { L"1", ID_BTN_1 },   { L"2", ID_BTN_2 },   { L"3", ID_BTN_3 },         { L"\u002B", ID_BTN_ADD },
    { L"\u00B1", ID_BTN_SIGN }, { L"0", ID_BTN_0 }, { L".", ID_BTN_DOT },   { L"=", ID_BTN_EQ }
};

const int NUM_COLS = 4;
const int NUM_ROWS = 5;
HWND hGridButtons[NUM_ROWS * NUM_COLS];

// ─── Lógica de la calculadora (misma que antes) ───────────────────────────────

void ClearAll() {
    current_entry = L"0";
    history = L"";
    numA = 0; numB = 0;
    pending_op = 0;
    next_entry_clears = true;
    SetWindowText(hDisplay, current_entry.c_str());
    SetWindowText(hStaticHistory, history.c_str());
}

void performCalculation() {
    if (pending_op != 0) {
        numB = _wtof(current_entry.c_str());
        double result = 0;
        switch (pending_op) {
        case L'\u002B': result = numA + numB; break;
        case L'\u2212': result = numA - numB; break;
        case L'\u00D7': result = numA * numB; break;
        case L'\u00F7': result = (numB != 0) ? numA / numB : 0; break;
        }
        std::wstringstream ws;
        ws << result;
        current_entry = ws.str();
        numA = result;
        history = L"";
        pending_op = 0;
        next_entry_clears = true;
    }
}

// ─── HandleKey: traduce una tecla física a una acción de la calculadora ───────
//
// Recibe el código de tecla virtual (VK_*) y el carácter Unicode (de WM_CHAR).
// Devuelve true si la tecla fue consumida.
//
bool HandleKey(WPARAM vk, wchar_t ch) {

    // Números 0-9 (fila superior del teclado y numpad)
    if ((ch >= L'0' && ch <= L'9') ||
        (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9)) {

        wchar_t digit = (vk >= VK_NUMPAD0 && vk <= VK_NUMPAD9)
                        ? (L'0' + (wchar_t)(vk - VK_NUMPAD0))
                        : ch;

        if (next_entry_clears) { current_entry = L""; next_entry_clears = false; }
        current_entry += digit;
        SetWindowText(hDisplay, current_entry.c_str());
        return true;
    }

    // Punto decimal  ('.' o ',' o VK_DECIMAL del numpad)
    if (ch == L'.' || ch == L',' || vk == VK_DECIMAL) {
        if (next_entry_clears) { current_entry = L""; next_entry_clears = false; }
        if (current_entry.find(L'.') == std::wstring::npos) {
            if (current_entry.empty()) current_entry = L"0";
            current_entry += L'.';
            SetWindowText(hDisplay, current_entry.c_str());
        }
        return true;
    }

    // Operadores
    WORD opId = 0;
    wchar_t opChar = 0;
    if (ch == L'+') { opId = ID_BTN_ADD; opChar = L'\u002B'; }
    else if (ch == L'-') { opId = ID_BTN_SUB; opChar = L'\u2212'; }
    else if (ch == L'*') { opId = ID_BTN_MUL; opChar = L'\u00D7'; }
    else if (ch == L'/') { opId = ID_BTN_DIV; opChar = L'\u00F7'; }
    else if (vk == VK_ADD)      { opId = ID_BTN_ADD; opChar = L'\u002B'; }
    else if (vk == VK_SUBTRACT) { opId = ID_BTN_SUB; opChar = L'\u2212'; }
    else if (vk == VK_MULTIPLY) { opId = ID_BTN_MUL; opChar = L'\u00D7'; }
    else if (vk == VK_DIVIDE)   { opId = ID_BTN_DIV; opChar = L'\u00F7'; }

    if (opId) {
        if (pending_op != 0 && !next_entry_clears) performCalculation();
        numA = _wtof(current_entry.c_str());
        pending_op = opChar;
        std::wstringstream ws;
        ws << numA << L" " << pending_op;
        history = ws.str();
        SetWindowText(hStaticHistory, history.c_str());
        next_entry_clears = true;
        return true;
    }

    // Enter / = → igual
    if (vk == VK_RETURN || ch == L'=') {
        performCalculation();
        SetWindowText(hDisplay, current_entry.c_str());
        SetWindowText(hStaticHistory, history.c_str());
        return true;
    }

    // Backspace
    if (vk == VK_BACK) {
        if (!current_entry.empty() && !next_entry_clears) {
            current_entry.pop_back();
            if (current_entry.empty() || current_entry == L"-") current_entry = L"0";
            SetWindowText(hDisplay, current_entry.c_str());
        }
        return true;
    }

    // Escape → C (clear all)
    if (vk == VK_ESCAPE) {
        ClearAll();
        return true;
    }

    // Delete → CE (clear entry)
    if (vk == VK_DELETE) {
        current_entry = L"0";
        next_entry_clears = true;
        SetWindowText(hDisplay, current_entry.c_str());
        return true;
    }

    return false;
}

// ─── Subclassing de botones ───────────────────────────────────────────────────
//
// Interceptamos WM_KEYDOWN/WM_CHAR para redirigirlos a HandleKey,
// y WM_LBUTTONUP para devolver el foco a la ventana principal.
//
LRESULT CALLBACK ButtonSubclassProc(HWND hBtn, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_KEYDOWN) {
        // WM_KEYDOWN nos da el VK; WM_CHAR nos daría el carácter — aquí usamos VK directo
        if (HandleKey(wParam, 0))
            return 0;
    }
    if (uMsg == WM_CHAR) {
        if (HandleKey(0, (wchar_t)wParam))
            return 0;
    }
    // Tras hacer click en un botón, devolvemos el foco a la ventana principal
    // para que los próximos WM_KEYDOWN lleguen a ella.
    if (uMsg == WM_LBUTTONUP) {
        LRESULT r = CallWindowProc(g_OrigButtonProc, hBtn, uMsg, wParam, lParam);
        SetFocus(hMainWnd);
        return r;
    }
    return CallWindowProc(g_OrigButtonProc, hBtn, uMsg, wParam, lParam);
}

// ─── Layout (igual que antes) ─────────────────────────────────────────────────

void LayoutControls(HWND hwnd) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    int width  = rc.right  - rc.left;
    int height = rc.bottom - rc.top;

    int margin     = 5;
    int hHistory_h = 25;
    int hDisplay_h = 50;

    SetWindowPos(hStaticHistory, NULL, margin, margin, width - (2 * margin), hHistory_h, SWP_NOZORDER);
    SetWindowPos(hDisplay, NULL, margin, margin + hHistory_h + 2, width - (2 * margin), hDisplay_h, SWP_NOZORDER);

    int grid_start_y         = margin + hHistory_h + hDisplay_h + margin + 10;
    int available_grid_height = height - grid_start_y - margin;

    int btn_w = (width - (2 * margin)) / NUM_COLS;
    int btn_h = available_grid_height / NUM_ROWS;

    for (int row = 0; row < NUM_ROWS; ++row) {
        for (int col = 0; col < NUM_COLS; ++col) {
            int index = row * NUM_COLS + col;
            HWND hBtn = hGridButtons[index];
            if (hBtn)
                SetWindowPos(hBtn, NULL, margin + (col * btn_w), grid_start_y + (row * btn_h), btn_w - 1, btn_h - 1, SWP_NOZORDER);
        }
    }
}

// ─── WindowProc principal ─────────────────────────────────────────────────────

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {

    case WM_CREATE: {
        hMainWnd = hwnd;

        INITCOMMONCONTROLSEX icex;
        icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
        icex.dwICC  = ICC_STANDARD_CLASSES;
        InitCommonControlsEx(&icex);

        hFontMain    = CreateFont(20, 0,0,0, FW_DONTCARE, FALSE,FALSE,FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH|FF_SWISS, L"Segoe UI");
        hFontDisplay = CreateFont(40, 0,0,0, FW_BOLD,     FALSE,FALSE,FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH|FF_SWISS, L"Segoe UI");

        hStaticHistory = CreateWindowEx(0, L"STATIC", L"", WS_VISIBLE|WS_CHILD|SS_RIGHT, 0,0,0,0, hwnd, (HMENU)ID_HISTORY, NULL, NULL);
        SendMessage(hStaticHistory, WM_SETFONT, (WPARAM)hFontMain, TRUE);

        hDisplay = CreateWindowEx(0, L"STATIC", L"0", WS_VISIBLE|WS_CHILD|SS_RIGHT, 0,0,0,0, hwnd, (HMENU)ID_DISPLAY, NULL, NULL);
        SendMessage(hDisplay, WM_SETFONT, (WPARAM)hFontDisplay, TRUE);

        for (int i = 0; i < NUM_ROWS * NUM_COLS; ++i) {
            HWND hBtn = CreateWindow(L"BUTTON", buttons[i].label, WS_VISIBLE|WS_CHILD|BS_PUSHBUTTON, 0,0,0,0, hwnd, (HMENU)(UINT_PTR)buttons[i].id, NULL, NULL);
            SendMessage(hBtn, WM_SETFONT, (WPARAM)hFontMain, TRUE);
            hGridButtons[i] = hBtn;

            // Subclassing: guardamos el WndProc original la primera vez
            WNDPROC orig = (WNDPROC)SetWindowLongPtr(hBtn, GWLP_WNDPROC, (LONG_PTR)ButtonSubclassProc);
            if (!g_OrigButtonProc) g_OrigButtonProc = orig;
        }

        ClearAll();
        LayoutControls(hwnd);
        break;
    }

    // ── La ventana principal también procesa teclas directamente ──
    // Esto cubre el caso en que NINGÚN botón tenga el foco (p.ej. al arrancar).
    case WM_KEYDOWN:
        HandleKey(wParam, 0);
        return 0;

    case WM_CHAR:
        HandleKey(0, (wchar_t)wParam);
        return 0;

    case WM_SIZE:
        LayoutControls(hwnd);
        InvalidateRect(hwnd, NULL, TRUE);
        break;

    case WM_COMMAND: {
        // Los clicks de ratón siguen llegando aquí como antes
        WORD id = LOWORD(wParam);
        if ((id >= ID_BTN_0 && id <= ID_BTN_9) || id == ID_BTN_DOT) {
            if (next_entry_clears) { current_entry = L""; next_entry_clears = false; }
            wchar_t charToAdd = 0;
            if (id == ID_BTN_DOT) {
                if (current_entry.find(L'.') == std::wstring::npos) charToAdd = L'.';
            } else {
                charToAdd = (id - ID_BTN_0) + L'0';
            }
            if (charToAdd) current_entry += charToAdd;
            if (current_entry.empty() || current_entry == L".") current_entry = L"0" + current_entry;
            SetWindowText(hDisplay, current_entry.c_str());
        }
        else if (id == ID_BTN_ADD || id == ID_BTN_SUB || id == ID_BTN_MUL || id == ID_BTN_DIV) {
            if (pending_op != 0 && !next_entry_clears) performCalculation();
            numA = _wtof(current_entry.c_str());
            switch (id) {
            case ID_BTN_ADD: pending_op = L'\u002B'; break;
            case ID_BTN_SUB: pending_op = L'\u2212'; break;
            case ID_BTN_MUL: pending_op = L'\u00D7'; break;
            case ID_BTN_DIV: pending_op = L'\u00F7'; break;
            }
            std::wstringstream ws;
            ws << numA << L" " << pending_op;
            history = ws.str();
            SetWindowText(hStaticHistory, history.c_str());
            next_entry_clears = true;
        }
        else if (id == ID_BTN_EQ)  { performCalculation(); SetWindowText(hDisplay, current_entry.c_str()); SetWindowText(hStaticHistory, history.c_str()); }
        else if (id == ID_BTN_C)   ClearAll();
        else if (id == ID_BTN_CE)  { current_entry = L"0"; next_entry_clears = true; SetWindowText(hDisplay, current_entry.c_str()); }
        else if (id == ID_BTN_BACK) {
            if (!current_entry.empty() && !next_entry_clears) {
                current_entry.pop_back();
                if (current_entry.empty() || current_entry == L"-") current_entry = L"0";
                SetWindowText(hDisplay, current_entry.c_str());
            }
        }
        else if (id == ID_BTN_SIGN) {
            if (current_entry != L"0") {
                if (current_entry[0] == L'-') current_entry.erase(0, 1);
                else current_entry.insert(0, L"-");
                SetWindowText(hDisplay, current_entry.c_str());
            }
        }
        break;
    }

    case WM_GETMINMAXINFO: {
        LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
        lpMMI->ptMinTrackSize.x = 280;
        lpMMI->ptMinTrackSize.y = 400;
        break;
    }

    case WM_DESTROY:
        DeleteObject(hFontMain);
        DeleteObject(hFontDisplay);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// ─── WinMain ──────────────────────────────────────────────────────────────────

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"CalcV3WindowClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(101));

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, CLASS_NAME, L"Calculadora Pro 25KB",
                               WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 350, 500,
                               NULL, NULL, hInstance, NULL);

    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return 0;
}