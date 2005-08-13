/***************************************************************************
 *   Copyright (C) 1999-2001 by Matthias Hoelzer-Kluepfel                  *
 *   hoelzer@kde.org                                                       *
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *   Copyright (C) 2004 by Alexander Dymo                                  *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef _HTDIGINDEX_H_
#define _HTDIGINDEX_H_

#include <kdialogbase.h>

class QTimer;
class QLabel;
class KProcess;
class KProgress;

class ProgressDialog : public KDialogBase
{
    Q_OBJECT
public:
    ProgressDialog(bool index, QWidget *parent=0, const char *name=0);
    ~ProgressDialog();

    void addDir(const QString &dir);
    void scanDirectories();
    bool createConfig();
    bool generateIndex();

private slots:
    void htdigStdout(KProcess *proc, char *buffer, int buflen);
    void htdigExited(KProcess *proc);
    void htmergeExited(KProcess *proc);
    void cancelClicked();
    void okClicked();
    void slotDelayedStart();

private:
    void setFilesScanned(int s);
    void setFilesToDig(int d);
    void setFilesDigged(int d);
    void setState(int n);
    void done(int r);
    void startHtdigProcess(bool initial);
    void startHtmergeProcess();

    QLabel *filesLabel, *check1, *check2, *check3;
    KProgress *bar;

    int count;
    bool procdone;
    QString databaseDir;
    QString indexdir;
    QString exe;
    QStringList files;
    KProcess *proc;
    volatile int filesToDig, filesDigged, filesScanned;
    bool htdigRunning, htmergeRunning;
};

#endif
