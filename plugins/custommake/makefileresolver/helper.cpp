/*
    SPDX-FileCopyrightText: 2003-2005 Jaroslaw Staniek <staniek@kde.org>
    SPDX-FileCopyrightText: 2005 Christian Ehrlicher <Ch.Ehrlicher@gmx.de>
    SPDX-FileCopyrightText: 2014 Kevin Funk <kfunk@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "helper.h"

#include <QDateTime>

#ifndef Q_OS_WIN
#include <sys/time.h>
#else
#include <windows.h>
#include <winsock2.h>
#endif

namespace {

#ifdef Q_OS_WIN

// Copied from kdewin.git's src/time.c

#define KDE_SECONDS_SINCE_1601	11644473600LL
#define KDE_USEC_IN_SEC			1000000LL

void UnixTimevalToFileTime(struct timeval t, LPFILETIME pft)
{
    LONGLONG ll;

    ll = Int32x32To64(t.tv_sec, KDE_USEC_IN_SEC*10) + t.tv_usec*10 + KDE_SECONDS_SINCE_1601*KDE_USEC_IN_SEC*10;
    pft->dwLowDateTime = (DWORD)ll;
    pft->dwHighDateTime = ll >> 32;
}

int utimes(const char *filename, const struct timeval times[2])
{
    FILETIME LastAccessTime;
    FILETIME LastModificationTime;
    HANDLE hFile;

    if(times) {
        UnixTimevalToFileTime(times[0], &LastAccessTime);
        UnixTimevalToFileTime(times[1], &LastModificationTime);
    }
    else {
        GetSystemTimeAsFileTime(&LastAccessTime);
        GetSystemTimeAsFileTime(&LastModificationTime);
    }

    hFile=CreateFileA(filename, FILE_WRITE_ATTRIBUTES, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, OPEN_EXISTING, 0, NULL);
    if(hFile==INVALID_HANDLE_VALUE) {
        switch(GetLastError()) {
            case ERROR_FILE_NOT_FOUND:
                errno=ENOENT;
                break;
            case ERROR_PATH_NOT_FOUND:
            case ERROR_INVALID_DRIVE:
                errno=ENOTDIR;
                break;
            case ERROR_ACCESS_DENIED:
                errno=EACCES;
                break;
            default:
                errno=ENOENT; /*what other errors can occur? */
        }
        return -1;
    }

    if(!SetFileTime(hFile, NULL, &LastAccessTime, &LastModificationTime)) {
        /*can this happen? */
        errno=ENOENT;
        return -1;
    }
    CloseHandle(hFile);
    return 0;
}

#endif

}

int Helper::changeAccessAndModificationTime(const QString& filename,
                                            const QDateTime& accessTime, const QDateTime& modificationTime)
{
    struct timeval times[2];
    times[0].tv_usec = 0;
    times[0].tv_sec = accessTime.toSecsSinceEpoch();
    times[1].tv_usec = 0;
    times[1].tv_sec = modificationTime.toSecsSinceEpoch();
    return utimes(filename.toLocal8Bit().constData(), times);
}
