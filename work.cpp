#include "work.h"
#include "ui.h"
#include "download.h"
#include "unzip.h"
#include "changelog.h"

#include "signtool/verify.h"

#if defined(__x86_64__) || defined(_WIN64) || defined(Q_PROCESSOR_X86_64)
#define MANIFEST_URL "https://tasemnice.eu/lorris64/updater_manifest.txt"
#else
#define MANIFEST_URL "https://tasemnice.eu/lorris32/updater_manifest.txt"
#endif

#define SIGNATURE_SUFFIX ".sig"

static const char *modes[] = { "release", "dev" };
static const char *changelogs[] = { "changelog1", "changelog2" };

HANDLE Work::m_thread = 0;
volatile bool Work::m_run = true;
char Work::m_mode[20];
int Work::m_rev = -1;
HWND Work::m_window = 0;

void Work::createThread(HWND window)
{
    strcpy(m_mode, "dev");
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
    snprintf(str, sizeof(str), "Downloading (%lu kB of %lu kB)", part/1024, total/1024);
    Ui::setText(str);
}

void Work::updateVerInfo()
{
    if(__argc >= 3)
    {
        char *pch = strtok(__argv[1], "-");
        pch = strtok(NULL, "-");
        if(pch)
            snprintf(m_mode, sizeof(m_mode), "%s", pch);

        m_rev = atoi(__argv[2]);
        return;
    }


    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    HANDLE stdoutRd = NULL;
    HANDLE stdoutWr = NULL;
    HANDLE stdinRd = NULL;
    HANDLE stdinWr = NULL;

    TCHAR cmdline[] = TEXT("Lorris.exe -v");
    PROCESS_INFORMATION piProcInfo; 
    STARTUPINFO siStartInfo;
    BOOL bSuccess = FALSE;

    char str[256] = { 0 };
    DWORD dwRead;

    if (!CreatePipe(&stdoutRd, &stdoutWr, &saAttr, 0)) {
        return;
    }

    if(!SetHandleInformation(stdoutRd, HANDLE_FLAG_INHERIT, 0)) {
        goto exit;
    }

     if (!CreatePipe(&stdinRd, &stdinWr, &saAttr, 0)) {
        return;
    }

    if(!SetHandleInformation(stdinWr, HANDLE_FLAG_INHERIT, 0)) {
        goto exit;
    }

    ZeroMemory( &piProcInfo, sizeof(PROCESS_INFORMATION) );
    ZeroMemory( &siStartInfo, sizeof(STARTUPINFO) );
    siStartInfo.cb = sizeof(STARTUPINFO); 
    siStartInfo.hStdError = stdoutWr;
    siStartInfo.hStdOutput = stdoutWr;
    siStartInfo.hStdInput = stdinRd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    bSuccess = CreateProcess(NULL,
      cmdline,// command line 
      NULL,          // process security attributes 
      NULL,          // primary thread security attributes 
      TRUE,          // handles are inherited 
      CREATE_NO_WINDOW, // creation flags 
      NULL,          // use parent's environment 
      NULL,          // use parent's current directory 
      &siStartInfo,  // STARTUPINFO pointer 
      &piProcInfo);  // receives PROCESS_INFORMATION

    if(!bSuccess) {
        goto exit;
    }
    
    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);

    bSuccess = ReadFile(stdoutRd, str, sizeof(str)-1, &dwRead, NULL);
    if(bSuccess && dwRead > 0)
    {
        char *pch = strtok(str, " ,-");
        for (int cnt = 0; pch != NULL; ++cnt)
        {
            switch(cnt)
            {
                case 3: snprintf(m_mode, sizeof(m_mode), "%s", pch); break;
                case 6: m_rev = atoi(pch); break;
            }
            pch = strtok(NULL, " ,-");
        }
    }

exit:
    CloseHandle(stdoutRd);
    CloseHandle(stdoutWr);
    CloseHandle(stdinRd);
    CloseHandle(stdinWr);
}

const char *Work::matchChangelog()
{
    for(int i = 0; i < sizeof(modes)/sizeof(modes[0]); ++i)
    {
        if(strcmp(modes[i], m_mode) == 0)
            return changelogs[i];
    }
    return NULL;
}

bool Work::parseManifest(char *name, std::string &url, std::string &sha256)
{
    FILE *man = fopen(name, "r");
    if(!man)
        return true;

    const char *log = matchChangelog();
    bool hasNewer = false;

    char line[1024];
    while(fgets(line, sizeof(line), man))
    {
        if(log && strstr(line, log))
        {
            Changelog::init(line);
            continue;
        }

        if(!strstr(line, m_mode))
            continue;

        char *pch = strtok(line, " ");
        for (int cnt = 0; pch != NULL; ++cnt)
        {
            switch(cnt)
            {
                case 1:
                    if(atoi(pch) <= m_rev)
                        hasNewer = true;
                    break;
                case 2:
                    url = pch;
                    break;
                case 3:
                    sha256 = pch;
                    break;
            }
            pch = strtok(NULL, " ");
        }
    }
    fclose(man);

    if(hasNewer)
    {
        Ui::setText("You already have the newest version");
        MessageBox(NULL, TEXT("You already have the newest version"), TEXT("Error!"), 0);
        ::PostMessage(m_window, WORK_COMPLETE, 0, 0);
        return false;
    }
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

    uintptr_t res = (uintptr_t)ShellExecute(GetDesktopWindow(), TEXT("open"), str_path.c_str(), NULL, path, SW_SHOWNORMAL);
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

    std::string url, sha256;

    char manName[MAX_PATH];
    char zipName[MAX_PATH];
    *manName = *zipName = 0;

    // Download files
    try {
        Download::download(MANIFEST_URL, true, NULL, manName);

        if(!verifySignature(MANIFEST_URL, manName, NULL))
            goto exit;

        if(!parseManifest(manName, url, sha256))
            goto exit;

        if(url.empty())
            throw DLExc("Corrupted manifest file");

        Download::download(url.c_str(), true, showprogress, zipName);

        if(!verifySignature(url, zipName, sha256.c_str()))
            goto exit;

        Ui::setText("Extracting....");
        unzipFile(zipName);
    }
    catch(DLExc exc)
    {
        char txt[256];
        snprintf(txt, sizeof(txt), "ERROR: %s", exc.geterr());
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

static int hex2int(char c) {
    if(c >= '0' && c <= '9')
        return c - '0';
    else if(c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    else if(c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return -1;
}

bool Work::verifySignature(std::string origurl, const char *fn, const char *expectedSha256Str) {
    uint8_t sha256[32];
    if(!signtool_sha256sum(fn, sha256))
        throw DLExc("Failed to calculate sha256 hash");

    if(expectedSha256Str) {
        uint8_t expectedSha256[sizeof(sha256)] = { 0 };
        size_t len = strlen(expectedSha256Str);
        int c;
        for(size_t i = 0; i < sizeof(expectedSha256)*2 && i < len;) {
            if((c = hex2int(expectedSha256Str[i])) == -1)
                break;
            expectedSha256[i++/2] = c << 4;
            if((c = hex2int(expectedSha256Str[i])) == -1)
                break;
            expectedSha256[i++/2] |= c;
        }

        if(memcmp(expectedSha256, sha256, sizeof(sha256)) != 0) {
            int res = MessageBox(NULL, TEXT("The sha256 sum of downloaded file does not match. "
                "The file was either corrupted, tampered with or not generated properly. "
                "Ignore and continue?"), TEXT("Error!"), MB_YESNO | MB_DEFBUTTON2);
            if(res != IDYES) {
                Ui::setText("ERROR: SHA256 hashes do not match.");
                ::PostMessage(m_window, WORK_COMPLETE, 0, 0);
                return false;
            }
        }
    }

    origurl += SIGNATURE_SUFFIX;

    char sigfile[MAX_PATH] = { 0 };
    Download::download(origurl.c_str(), true, NULL, sigfile);

    static const uint8_t pubkey[ECC_BYTES+1] = {
        0x03,0x20,0x51,0xc3,0xa9,0x65,0xaa,0xe8,0x74,0x57,0x82,0xf6,0xff,0x05,0xd1,0x63,0x55,
        0x70,0xfe,0x8d,0xac,0xd9,0x81,0x9f,0x3d,0x1c,0xdd,0xdf,0x31,0x26,0xbb,0x02,0x7f,
    };

    int verify_res = signtool_verify(sha256, sigfile, pubkey);

    if(*sigfile)
        remove(sigfile);

    if(!verify_res) {
        int res = MessageBox(NULL, TEXT("The file signature does not match. "
            "The file was either corrupted, tampered with or not generated properly. "
            "Ignore and continue? This is unsafe!"), TEXT("Error!"), MB_YESNO | MB_DEFBUTTON2);
        if(res != IDYES) {
            Ui::setText("ERROR: The signature is invalid.");
            ::PostMessage(m_window, WORK_COMPLETE, 0, 0);
            return false;
        }
    }

    return true;
}