#include <string>
#include "changelog.h"
#include "ui.h"
#include "download.h"

volatile bool Changelog::m_run = true;
HANDLE Changelog::m_thread = 0;
char *Changelog::m_address = 0;

void Changelog::init(char *line)
{
    if(m_thread != 0)
        return;

    char *pch = strtok(line, " \n");
    for (int cnt = 0; pch != NULL; ++cnt)
    {
        if(cnt == 1)
        {
            m_address = new char[strlen(pch)+1];
            strcpy(m_address, pch);
            break;
        }
        pch = strtok(NULL, " \n");
    }

    if(m_address == 0)
    {
        Ui::setChangelog("No changelog available");
        return;
    }

    m_thread = CreateThread(NULL, 0, run, NULL, 0, 0);
}

DWORD WINAPI Changelog::run(LPVOID pParam)
{
    char filename[MAX_PATH];
    FILE *file = NULL;
    std::string result = "";

    filename[0] = 0;

    try
    {
        Download::download(m_address, true, NULL, filename);
    }
    catch(DLExc exc)
    {
        Ui::setChangelog("No changelog available");
        goto exit;
    }

    file = fopen(filename, "r");
    if(file)
    {
        bool skip = true;
        int last = 0;
        for(char line[1000]; fgets(line, 1000, file);)
        {
            int cur = ftell(file) - last - 2;
            last = ftell(file);

            if(skip)
            {
                if(strstr(line, "Version") == line)
                    skip = false;
                else
                    continue;
            }

            line[cur] = 0;

            result += line;
            result += "\r\n";
        }
        Ui::setChangelog(result.c_str());
        fclose(file);
    }

exit:
    delete[] m_address;
    return 0;
}