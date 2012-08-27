#include <windows.h>
#include <commctrl.h>

#include "work.h"
#include "ui.h"
#include "download.h"

HWND Ui::m_progress = 0;
HWND Ui::m_label = 0;
HWND Ui::m_btn = 0;
HWND Ui::m_edit_box = 0;
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
    HWND bg = CreateWindowEx(0, TEXT("static"), TEXT("ST_U"),
                   (WS_CHILD | WS_VISIBLE | WS_TABSTOP),
                   0, 0, 700, 400, hWnd, NULL, m_inst, NULL);
    SetWindowText(bg, 0);

    // Progress
    m_progress = CreateWindowEx(0, PROGRESS_CLASS, NULL,
                                WS_CHILD | WS_VISIBLE,
                                15, 320, 560, 20,
                                hWnd, NULL, m_inst, NULL);
    SendMessage(m_progress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));

    // Label
    m_label = CreateWindowEx(0, TEXT("static"), TEXT("ST_U"),
                           WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                           15, 345, 260, 40,
                           hWnd, NULL,
                           m_inst, NULL);
    setText("Initializing...");

    // Button
    m_btn = CreateWindowEx(NULL, TEXT("BUTTON"), TEXT("Cancel"), 
                           WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                           585, 320, 95, 20, hWnd, (HMENU)IDB_CANCEL, NULL, NULL);

    // EditBox
    m_edit_box = CreateWindowEx(NULL, TEXT("EDIT"), TEXT(""),
                                WS_CHILD|WS_VISIBLE|ES_MULTILINE|WS_VSCROLL|ES_READONLY | WS_BORDER,
                                15, 15, 665, 295, hWnd, NULL, m_inst, NULL);
    setChangelog("Downloading changelog...");
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
            if(!Work::runLorris())
            {
                Ui::setText("Failed to launch Lorris!");
                return;
            }
            // fallthrough
        case BTN_CANCEL:
            Work::endThread();
            PostQuitMessage(0);
            break;
        case BTN_TRY_AGAIN:
            Work::createThread(0);
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

void Ui::setChangelog(const char* text)
{
    BSTR unicodestr = toWString(text);
    SetWindowText(m_edit_box, unicodestr);
    ::SysFreeString(unicodestr);
}
