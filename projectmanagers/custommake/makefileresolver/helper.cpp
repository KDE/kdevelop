/*
   Copyright (C) 2003-2005 Jaroslaw Staniek <staniek@kde.org>
   Copyright (C) 2005 Christian Ehrlicher <Ch.Ehrlicher@gmx.de>
   Copyright (C) 2014 Kevin Funk <kfunk@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
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
    times[0].tv_sec = accessTime.toTime_t();
    times[1].tv_usec = 0;
    times[1].tv_sec = modificationTime.toTime_t();
    return utimes(filename.toLocal8Bit().constData(), times);
}
