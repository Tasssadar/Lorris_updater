#ifndef WORK_H
#define WORK_H

#include <windows.h>

#define WORK_COMPLETE (WM_APP+1)

class Work
{
public:
    static void createThread(HWND window);
    static void endThread();
    static bool runWork() { return m_run; }
    static bool runLorris();

private:
    static void showprogress(unsigned long total, unsigned long part);
    static void updateVerInfo();
    static bool parseManifest(char *name, char *url);
    static bool isLorrisRunning();
    static void unzipFile(char *name);
    static DWORD WINAPI run(LPVOID pParam);
    static const char *matchChangelog();

    static HANDLE m_thread;
    static volatile bool m_run;
    static char m_mode[20];
    static int m_rev;
    static HWND m_window;
};
#endif
