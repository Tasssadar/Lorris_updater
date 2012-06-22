#ifndef UI_H
#define UI_H

#include <windows.h>

DWORD WINAPI WorkThread(LPVOID pParam);

enum ButtonState
{
    BTN_CANCEL,
    BTN_TRY_AGAIN,
    BTN_RUN_LORRIS
};

class Ui
{
public:
    static void Init(HWND hWnd);

    static void set(HINSTANCE inst)
    {
        m_inst = inst;
    }

    static void setProgress(int val);
    static void setText(const char* text);
    static void processCmd(int cmd);
    static BSTR toWString(const char* text);
    static void setBtnState(ButtonState state);

private:
    static HINSTANCE m_inst;
    static HWND m_progress;
    static HWND m_label;
    static HWND m_btn;
    static ButtonState m_btn_state;
};

#endif