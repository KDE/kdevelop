/***************************************************************************
 *   Copyright (C) 1999-2001 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _MAKEWIDGET_H_
#define _MAKEWIDGET_H_

#include <qstack.h>
#include "qtextedit.h"
#include "qbrush.h"
#include "qstylesheet.h"
#include "private/qrichtext_p.h"


class MakeViewPart;
class MakeItem;
class KProcess;


class MakeWidget : public QTextEdit
{
    Q_OBJECT
    
public:
    MakeWidget(MakeViewPart *part);
    ~MakeWidget();

    void queueJob(const QString &dir, const QString &command);
    bool isRunning();
    
public slots:
    void startNextJob();
    void killJob();
    void nextError();
    void prevError();
    
private slots:
    void slotReceivedOutput(KProcess *, char *buffer, int buflen);
    void slotReceivedError(KProcess *, char *buffer, int buflen);
    void slotProcessExited(KProcess *);

private:
    virtual void contentsMousePressEvent(QMouseEvent *e);
    virtual void keyPressEvent(QKeyEvent *e);
    void searchItem(int parag);
    
    enum Type { Normal, Error, Diagnostic };
    void insertLine1(const QString &line, Type type);
    void insertLine2(const QString &line, Type type);
    bool matchEnterDir( const QString& line, QString& dir );
    bool matchLeaveDir( const QString& line, QString& dir );
    QString getOutputColor( Type type );

    QStringList commandList;
    QStringList dirList;
    QString currentCommand;
    KProcess *childproc;
    QString stdoutbuf;
    QString stderrbuf;
    QStack<QString> dirstack;
    QList<MakeItem> items;
    int parags;
    bool moved;

    MakeViewPart *m_part;
};

#endif
