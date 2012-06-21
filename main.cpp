#include <windows.h>
#include <commctrl.h>
#include <tchar.h>
#include <strsafe.h>

#include "ui.h"
#include "download.h"

#define MANIFEST_URL "http://dl.dropbox.com/u/54372958/lorris.txt"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

DWORD WINAPI WorkThread(LPVOID pParam);
volatile bool runWorkThread = true;

char mode[20];
int rev = -1;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   PSTR szCmdLine, int iCmdShow)
{
    Ui::set(hInstance);
    strcpy(mode, "release");

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
    
    CreateThread(NULL, 0, WorkThread, NULL, 0, 0);

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
            runWorkThread = false;
            PostQuitMessage(0);
            return 0;
        case WM_COMMAND:
            Ui::processCmd(LOWORD(wParam));
            return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

void showprogress(unsigned long total, unsigned long part)
{
    int val = (int) ((double)part / total * 100);
    Ui::setProgress(val);

    static char str[200];
    sprintf(str, "Downloading (%lu kB of %lu kB)", part/1024, total/1024);
    Ui::setText(str);
}

static void updateVerInfo()
{
    if(__argc >= 3)
    {
        char *pch = strtok(__argv[1], "-");
        pch = strtok(NULL, "-");
        if(pch)
            strcpy(mode, pch);

        rev = atoi(__argv[2]);
    }
    else
    {
        FILE *file = _popen("Lorris.exe -v", "r");
        if(!file)
            return;

        char str[200];
        *str = 0;
        fgets(str, 200, file);   
        fclose(file);

        if(*str != 0)
        {
            char *pch = strtok(str, " ,-");
            for (int cnt = 0; pch != NULL; ++cnt)
            {
                switch(cnt)
                {
                    case 3: strcpy(mode, pch); break;
                    case 6: rev = atoi(pch); break;
                }
                pch = strtok(NULL, " ,-");
            }
        }
    }
}

static bool parseManifest(char *url)
{
    char name[MAX_PATH];
    Download::getfname(url, name);

    FILE *man = fopen(name, "r");
    for(char line[100]; fgets(line, 1000, man);)
    {
        if(!strstr(line, mode))
            continue;

        char *pch = strtok(line, " \n");
        pch = strtok(NULL, " \n");
        for (int cnt = 1; pch != NULL; ++cnt)
        {
            switch(cnt)
            {
                case 1:
                {
                    if(atoi(pch) <= rev)
                    {
                        fclose(man);
                        MessageBox(NULL, TEXT("You already have the newest version"), TEXT("Error!"), 0);
                        PostQuitMessage(0);
                        return false;
                    }
                    break;
                }
                case 2: strcpy(url, pch); break;
            }
            pch = strtok(NULL, " \n");
        }
    }
    fclose(man);
    return true;
}

DWORD WINAPI WorkThread(LPVOID pParam)
{
    // Get lorris version if possible
    updateVerInfo();

    // Download files
    try {
        Download::download(MANIFEST_URL, true, NULL);

        char url[500];
        *url = 0;

        if(!parseManifest(url))
            return 0;
        
        Download::download(url, true, showprogress);
    }
    catch(DLExc exc)
    {
        Ui::setText(exc.geterr());
        return 0;
    }
    return 0;
}

