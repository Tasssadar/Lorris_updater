#ifndef WORK_H
#define WORK_H

#include <windows.h>

class Work
{
public:
    static void createThread();
    static void endThread();
    static bool runWork() { return m_run; }

private:
    static void showprogress(unsigned long total, unsigned long part);
    static void updateVerInfo();
    static bool parseManifest(char *name, char *url);
    static bool isLorrisRunning();
    static void unzipFile(char *name);
    static DWORD WINAPI run(LPVOID pParam);

    static HANDLE m_thread;
    static volatile bool m_run;
    static char m_mode[20];
    static int m_rev;
};
#endif
