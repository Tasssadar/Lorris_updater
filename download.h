// EXAMPLE FROM http://www.dreamincode.net/forums/topic/101532-download-file-from-url/

// Header file for downloader.
#ifndef DOWNLOAD_H
#define DOWNLOAD_H

//#include <iostream>
#include <string>
#include <windows.h>
#include <wininet.h>
#include <fstream>

using namespace std;

const int MAX_ERRMSG_SIZE = 80;
const int MAX_FILENAME_SIZE = 512;
const int BUF_SIZE = 10240;             // 10 KB

// Exception class for donwload errors;
class DLExc
{
private:
    char err[MAX_ERRMSG_SIZE];
public:
    DLExc(char *exc)
    {
        if(strlen(exc) < MAX_ERRMSG_SIZE)
            strcpy(err, exc);
    }

    // Return a pointer to the error message
    const char *geterr()
    {
        return err;
    }
};


// A class for downloading files from the internet
class Download
{
private:
    static bool ishttp(char *url);
    static bool httpverOK(HINTERNET hIurl);
    static unsigned long openfile(char *url, bool reload, ofstream &fout, char *name);
public:
    static bool getfname(char *url, char *fname);
    static bool download(char *url, bool reload, void (*update)(unsigned long, unsigned long), char *name);
};

#endif
