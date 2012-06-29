
#include "work.h"
#include "ui.h"
#include "download.h"
#include "unzip.h"

#define MANIFEST_URL "http://dl.dropbox.com/u/54372958/lorris.txt"

HANDLE Work::m_thread = 0;
volatile bool Work::m_run = true;
char Work::m_mode[20];
int Work::m_rev = -1;
HWND Work::m_window = 0;

void Work::createThread(HWND window)
{
    strcpy(m_mode, "release");
    m_rev = -1;
    m_run = true;

    if(window)
        m_window = window;
  
    m_thread = CreateThread(NULL, 0, run, NULL, 0, 0);
}

void Work::endThread()
{
    if(!m_run)
        return;

    m_run = false;

    MSG msg;
    for(int i = 0; i < 40 && WaitForSingleObject(m_thread, 0) != WAIT_OBJECT_0; ++i)
    {
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        Sleep(50);
    }
}

void Work::showprogress(unsigned long total, unsigned long part)
{
    int val = (int) ((double)part / total * 100);
    Ui::setProgress(val);

    static char str[64];
    sprintf(str, "Downloading (%lu kB of %lu kB)", part/1024, total/1024);
    Ui::setText(str);
}

void Work::updateVerInfo()
{
    if(__argc >= 3)
    {
        char *pch = strtok(__argv[1], "-");
        pch = strtok(NULL, "-");
        if(pch)
            strcpy(m_mode, pch);

        m_rev = atoi(__argv[2]);
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
                    case 3: strcpy(m_mode, pch); break;
                    case 6: m_rev = atoi(pch); break;
                }
                pch = strtok(NULL, " ,-");
            }
        }
    }
}

bool Work::parseManifest(char *name, char *url)
{
    FILE *man = fopen(name, "r");
    if(!man)
        return true;

    for(char line[100]; fgets(line, 1000, man);)
    {
        if(!strstr(line, m_mode))
            continue;

        char *pch = strtok(line, " \n");
        for (int cnt = 0; pch != NULL; ++cnt)
        {
            switch(cnt)
            {
                case 1:
                {
                    if(atoi(pch) <= m_rev)
                    {
                        fclose(man);
                        Ui::setText("You already have the newest version");
                        MessageBox(NULL, TEXT("You already have the newest version"), TEXT("Error!"), 0);
                        ::PostMessage(m_window, WORK_COMPLETE, 0, 0);
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

bool Work::isLorrisRunning()
{
    static std::wstring path;
    if(path.empty())
    {
         TCHAR dir[FILENAME_MAX];
         ::GetCurrentDirectory(FILENAME_MAX, dir);

         path = std::wstring(dir);
         path += TEXT("\\Lorris.exe");
    }

    // check if exists
    FILE *f = fopen("Lorris.exe", "r");
    if(!f)
        return false;
    fclose(f);

    // check if writable
    f = fopen("Lorris.exe", "r+");
    if(!f)
        return true;
    fclose(f);
    return false;
}

void Work::unzipFile(char *name)
{
    TCHAR dir[FILENAME_MAX];
    ::GetCurrentDirectory(FILENAME_MAX, dir);

    std::wstring path = std::wstring(dir);
    path += TEXT("\\");

    BSTR w_name = Ui::toWString(name);

    HZIP hz = OpenZip(w_name, 0);
    ZIPENTRY ze;
    GetZipItem(hz,-1,&ze);
    int cnt = ze.index;
    for (int zi = 0; zi < cnt && m_run; ++zi)
    { 
        Ui::setProgress(zi*100/cnt);

        ZIPENTRY ze;
        GetZipItem(hz,zi,&ze);

        std::wstring filename(path);
        filename += ze.name;
        UnzipItem(hz, zi, filename.c_str());
    }
    CloseZip(hz);
    SysFreeString(w_name);
    Ui::setProgress(100);
}

bool Work::runLorris()
{
    TCHAR path[FILENAME_MAX];
    ::GetCurrentDirectory(FILENAME_MAX, path);
    
    std::wstring str_path(path);
    str_path += TEXT("\\Lorris.exe");

    int res = (int)ShellExecute(GetDesktopWindow(), TEXT("open"), str_path.c_str(), NULL, path, SW_SHOWNORMAL);
    return res > 32;
}

DWORD WINAPI Work::run(LPVOID pParam)
{
    Ui::setBtnState(BTN_CANCEL);

    if(isLorrisRunning())
    {
        Ui::setText("Quit Lorris to update!");
        while(isLorrisRunning() && m_run)
            Sleep(1000);
    }

    if(!m_run)
        return 0;

    Ui::setText("Initializing...");

    // Get lorris version if possible
    updateVerInfo();

    char manName[MAX_PATH];
    char zipName[MAX_PATH];
    *manName = *zipName = 0;

    // Download files
    try {
        
        Download::download(MANIFEST_URL, true, NULL, manName);

        char url[500];
        url[0] = 0;

        if(!parseManifest(manName, url))
            goto exit;

        if(*url == 0)
            throw DLExc("Corrupted manifest file");

        Download::download(url, true, showprogress, zipName);

        Ui::setText("Extracting....");
        unzipFile(zipName);
    }
    catch(DLExc exc)
    {
        char txt[100];
        sprintf(txt, "ERROR: %s", exc.geterr());
        Ui::setText(txt);
        Ui::setBtnState(BTN_TRY_AGAIN);
        goto exit;
    }

    Ui::setText("Complete!");
    ::PostMessage(m_window, WORK_COMPLETE, 0, 0);
    
exit:
    if(*manName) remove(manName);
    if(*zipName) remove(zipName);

    return 0;
}
