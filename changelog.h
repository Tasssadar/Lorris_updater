#ifndef CHANGELOG_H
#define CHANGELOG_H

#include <Windows.h>

class Changelog
{
public:
    static void init(char *line);

private:
    static DWORD WINAPI run(LPVOID pParam);

    static HANDLE m_thread;
    static volatile bool m_run;
    static char *m_address;
};

#endif
