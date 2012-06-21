#include <windows.h>
#include <commctrl.h>

#include "ui.h"
#include "download.h"

HWND Ui::m_progress = 0;
HWND Ui::m_label = 0;
HWND Ui::m_btn = 0;
HINSTANCE Ui::m_inst = 0;
ButtonState Ui::m_btn_state = BTN_CANCEL;

#define IDB_CANCEL 1001

BSTR Ui::toWString(const char* text)
{
    int a = lstrlenA(text);
    BSTR unicodestr = SysAllocStringLen(NULL, a);
    ::MultiByteToWideChar(CP_ACP, 0, text, a, unicodestr, a);
    return unicodestr;
}

void Ui::Init(HWND hWnd)
{
    // Background
    BSTR uni = toWString("ST_U");
    BSTR stat = toWString("static");
    HWND bg = CreateWindowEx(0, stat, uni,
                   (WS_CHILD | WS_VISIBLE | WS_TABSTOP),
                   0, 0, 400, 100, hWnd, NULL, m_inst, NULL);
    SetWindowText(bg, 0);

    // Progress
    m_progress = CreateWindowEx(0, PROGRESS_CLASS, NULL,
                                WS_CHILD | WS_VISIBLE,
                                20, 20, 260, 20,
                                hWnd, NULL, m_inst, NULL);
    SendMessage(m_progress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

    // Label
    m_label = CreateWindowEx(0, stat, uni,
                           WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                           20, 45, 260, 40,
                           hWnd, NULL,
                           m_inst, NULL);
    setText("Initializing...");

    // Button
    BSTR canc = toWString("Cancel");
    BSTR btn = toWString("BUTTON");
    m_btn = CreateWindowEx(NULL, btn, canc, 
                           WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                           290, 20, 95, 20, hWnd, (HMENU)IDB_CANCEL, NULL, NULL);

    ::SysFreeString(canc);
    ::SysFreeString(btn);

    ::SysFreeString(uni);
    ::SysFreeString(stat);
}

void Ui::setProgress(int val)
{
    SendMessage(m_progress, PBM_SETPOS, val, 0);
}

void Ui::setText(const char* text)
{
    BSTR unicodestr = toWString(text);
    SetWindowText(m_label, unicodestr);
    ::SysFreeString(unicodestr);
}

void Ui::processCmd(int cmd)
{
    if(cmd != IDB_CANCEL)
        return;

    switch(m_btn_state)
    {
        case BTN_RUN_LORRIS:
        {
            TCHAR path[FILENAME_MAX];
            ::GetCurrentDirectory(FILENAME_MAX, path);

            std::wstring str_path(path);
            str_path += TEXT("\\Lorris.exe");

            ::ShellExecute(GetDesktopWindow(), TEXT("open"), str_path.c_str(), NULL, path, SW_SHOWNORMAL);
            // Fallthrough
        }
        case BTN_CANCEL:
            runWorkThread = false;
            PostQuitMessage(0);
            break;
        case BTN_TRY_AGAIN:
            runWorkThread = true;
            CreateThread(NULL, 0, WorkThread, NULL, 0, 0);
            break;
    }
}

void Ui::setBtnState(ButtonState state)
{
    m_btn_state = state;
    switch(state)
    {
        case BTN_CANCEL:
            SetWindowText(m_btn, TEXT("Cancel"));
            break;
        case BTN_TRY_AGAIN:
            SetWindowText(m_btn, TEXT("Try again"));
            break;
        case BTN_RUN_LORRIS:
            SetWindowText(m_btn, TEXT("Run Lorris"));
            break;
    }
}
