#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <strsafe.h>

#include "ui.h"
#include "work.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   PSTR szCmdLine, int iCmdShow)
{
    Ui::set(hInstance);

    TCHAR szAppName[] = TEXT("Lorris updater");
    HWND hWnd;
    MSG msg;
    WNDCLASSEX wc;
    
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wc.hIconSm = NULL;
    wc.hCursor = (HCURSOR)LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = szAppName;
    
    RegisterClassEx(&wc);
    
    hWnd = CreateWindowEx(0,szAppName,
                        szAppName,
                        (WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX),
                        CW_USEDEFAULT,
                        CW_USEDEFAULT,
                        400,
                        100,
                        NULL,
                        NULL,
                        hInstance,NULL);
    ShowWindow(hWnd, iCmdShow);
    UpdateWindow(hWnd);

    Work::createThread();

    while(GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_CREATE:
            Ui::Init(hWnd);
            return 0;
        case WM_DESTROY:
            Work::endThread();
            PostQuitMessage(0);
            return 0;
        case WM_COMMAND:
            Ui::processCmd(LOWORD(wParam));
            return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}
