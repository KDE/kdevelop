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

#include <qapplication.h>
#include <qdir.h>
#include <qtimer.h>
#include <kdebug.h>
#include <klocale.h>
#include <knotifyclient.h>
#include <kprocess.h>
#include <kregexp.h>

#include "kdevcore.h"
#include "kdevtoplevel.h"
#include "kdevpartcontroller.h"

#include "makeviewpart.h"
#include "makewidget.h"


class MakeItem
{
public:
    MakeItem(int pg, const QString fn, int ln)
        : parag(pg), fileName(fn), lineNum(ln)
    {}
    int parag;
    QString fileName;
    int lineNum;
};


MakeWidget::MakeWidget(MakeViewPart *view)
    : QTextEdit(0, "make widget")
{
    setWordWrap(WidgetWidth);
    setWrapPolicy(Anywhere);
    
    childproc = new KShellProcess("/bin/sh");
    
    connect(childproc, SIGNAL(receivedStdout(KProcess*,char*,int)),
            this, SLOT(slotReceivedOutput(KProcess*,char*,int)) );
    connect(childproc, SIGNAL(receivedStderr(KProcess*,char*,int)),
            this, SLOT(slotReceivedError(KProcess*,char*,int)) );
    connect(childproc, SIGNAL(processExited(KProcess*)),
            this, SLOT(slotProcessExited(KProcess*) )) ;

    items.setAutoDelete(true);
    parags = 0;
    moved = false;
    
    m_part = view;
}


MakeWidget::~MakeWidget()
{
    delete childproc;
}


void MakeWidget::queueJob(const QString &dir, const QString &command)
{
    commandList.append(command);
    dirList.append(dir);
    if (!isRunning())
        startNextJob();
}


void MakeWidget::startNextJob()
{
    QStringList::Iterator it = commandList.begin();
    if (it == commandList.end())
        return;

    currentCommand = *it;
    commandList.remove(it);
    
    it =  dirList.begin();
    QString dir = *it;
    dirList.remove(it);
    
    clear();
    items.clear();
    parags = 0;
    moved = false;
    
    insertLine2(currentCommand, Diagnostic);
    childproc->clearArguments();
    *childproc << currentCommand;
    childproc->start(KProcess::NotifyOnExit, KProcess::AllOutput);
    
    dirstack.clear();
    dirstack.push(new QString(dir));

    m_part->topLevel()->raiseView(this);
    m_part->core()->running(m_part, true);
}



void MakeWidget::killJob()
{
    childproc->kill();
}


bool MakeWidget::isRunning()
{
    return childproc->isRunning();
}


void MakeWidget::nextError()
{
    int parag, index;
    if (moved)
	getCursorPosition(&parag, &index);
    else
        parag = 0;

    QListIterator<MakeItem> it(items);
    for (; it.current(); ++it)
        if ((*it)->parag > parag) {
            moved = true;
            parag = (*it)->parag;
            document()->removeSelection(0);
            setSelection(parag, 0, parag+1, 0, 0);
            setCursorPosition(parag, 0);
            ensureCursorVisible();
            m_part->partController()->editDocument((*it)->fileName, (*it)->lineNum);
            return;
        }
    
    KNotifyClient::beep();
}


void MakeWidget::prevError()
{
    int parag, index;
    if (moved)
        getCursorPosition(&parag, &index);
    else
        parag = 0;

    QListIterator<MakeItem> it(items);
    for (it.toLast(); it.current(); --it)
        if ((*it)->parag < parag) {
            moved = true;
            parag = (*it)->parag;
            document()->removeSelection(0);
            setSelection(parag, 0, parag+1, 0, 0);
            setCursorPosition(parag, 0);
            ensureCursorVisible();
            m_part->partController()->editDocument((*it)->fileName, (*it)->lineNum);
            return;
        }
            
    KNotifyClient::beep();
}


void MakeWidget::contentsMousePressEvent(QMouseEvent *e)
{
    QTextEdit::contentsMousePressEvent(e);
    int parag, index;
    getCursorPosition(&parag, &index);
    searchItem(parag);
}


void MakeWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Key_Return || e->key() == Key_Enter) {
        int parag, index;
        getCursorPosition(&parag, &index);
        searchItem(parag);
    } else
        QTextEdit::keyPressEvent(e);
}


void MakeWidget::searchItem(int parag)
{
    QListIterator<MakeItem> it(items);
    for (; it.current(); ++it) {
        if ((*it)->parag == parag)
            m_part->partController()->editDocument((*it)->fileName, (*it)->lineNum);
        if ((*it)->parag >= parag)
            return;
    }
}


void MakeWidget::slotReceivedOutput(KProcess *, char *buffer, int buflen)
{
    // Flush stderr buffer
    if (!stderrbuf.isEmpty()) {
        insertLine1(stderrbuf, Normal);
        stderrbuf = "";
    }
    
    stdoutbuf += QString::fromLocal8Bit(buffer, buflen);
    int pos;
    while ( (pos = stdoutbuf.find('\n')) != -1) {
        QString line = stdoutbuf.left(pos);
        insertLine1(line, Normal);
        stdoutbuf.remove(0, pos+1);
    }
}


void MakeWidget::slotReceivedError(KProcess *, char *buffer, int buflen)
{
    // Flush stdout buffer
    if (!stdoutbuf.isEmpty()) {
        insertLine1(stdoutbuf, Diagnostic);
        stdoutbuf = "";
    }
    
    stderrbuf += QString::fromLocal8Bit(buffer, buflen);
    int pos;
    while ( (pos = stderrbuf.find('\n')) != -1) {
        QString line = stderrbuf.left(pos);
        insertLine1(line, Diagnostic);
        stderrbuf.remove(0, pos+1);
    }
}


void MakeWidget::slotProcessExited(KProcess *)
{
    QString s;
    Type t;
    
    if (childproc->normalExit()) {
        if (childproc->exitStatus()) {
            s = i18n("*** Exited with status: %1 ***").arg(childproc->exitStatus());
            t = Error;
        } else {
            s = i18n("*** Success ***");
            t = Diagnostic;
	    emit m_part->commandFinished(currentCommand);
        }
    } else {
        s = i18n("*** Compilation aborted ***");
        t = Error;
    }
    
    insertLine2(s, t);

    m_part->core()->running(m_part, false);

    // Defensive programming: We emit this with a single shot timer so that we go once again
    // through the event loop. After that, we can be sure that the process is really finished
    // and its KProcess object can be reused.
    if (childproc->normalExit())
        QTimer::singleShot(0, this, SLOT(startNextJob()));
    else {
        commandList.clear();
        dirList.clear();
    }
}


void MakeWidget::insertLine1(const QString &line, Type type)
{
    // KRegExp has ERE syntax
    KRegExp enterDirRx("[^\n]*: Entering directory `([^\n]*)'$");
    KRegExp leaveDirRx("[^\n]*: Leaving directory `([^\n]*)'$");
    KRegExp errorGccRx("([^: \t]+):([0-9]+):.*");
    KRegExp errorFtnchekRx("\"(.*)\", line ([0-9]+):.*");
    KRegExp errorJadeRx("[a-zA-Z]+:([^: \t]+):([0-9]+):[0-9]+:[a-zA-Z]:.*");
    const int errorGccFileGroup = 1;
    const int errorGccRowGroup = 2;
    const int errorFtnchekFileGroup = 1;
    const int errorFtnchekRowGroup = 2;
    const int errorJadeFileGroup = 1;
    const int errorJadeRowGroup = 2;

    if (enterDirRx.match(line)) {
        QString *dir = new QString(enterDirRx.group(1));
        dirstack.push(dir);
        kdDebug(9004) << "Entering dir: " << (*dir).ascii() << endl;
        insertLine2(line, Diagnostic);
        return;
    } else if (leaveDirRx.match(line)) {
        kdDebug(9004) << "Leaving dir: " << leaveDirRx.group(1) << endl;
        QString *dir = dirstack.pop();
        kdDebug(9004) << "Now: " << (*dir) << endl;
        delete dir;
        insertLine2(line, Diagnostic);
        return;
    }
    
    QString fn;
    int row = -1;
    
    bool hasmatch = false;
    if (errorGccRx.match(line)) {
        hasmatch = true;
        fn = errorGccRx.group(errorGccFileGroup);
        row = QString(errorGccRx.group(errorGccRowGroup)).toInt()-1;
    } else if (errorFtnchekRx.match(line)) {
        kdDebug() << "Matching " << line << endl;
        hasmatch = true;
        fn = errorFtnchekRx.group(errorFtnchekFileGroup);
        row = QString(errorFtnchekRx.group(errorFtnchekRowGroup)).toInt()-1;
    } else if (errorJadeRx.match(line)) {
        hasmatch = true;
        fn = errorGccRx.group(errorJadeFileGroup);
        row = QString(errorJadeRx.group(errorJadeRowGroup)).toInt()-1;
    }
    
    if (hasmatch) {
        kdDebug(9004) << "Error in " << fn << " " << row << endl;
        if (dirstack.top())
            fn.prepend("/").prepend(*dirstack.top());
        kdDebug(9004) << "Path: " << fn << endl;
        items.append(new MakeItem(parags, fn, row));
        insertLine2(line, Error);
    } else {
        insertLine2(line, type);
    }
}


void MakeWidget::insertLine2(const QString &line, Type type)
{
    ++parags;

    QString color =
        (type==Error)? "darkRed" :
        (type==Diagnostic)? "black" :
        "darkBlue";
    bool move = textCursor()->parag() == document()->lastParag() && textCursor()->atParagEnd();

    int paraFrom, indexFrom, paraTo, indexTo;
    getSelection(&paraFrom, &indexFrom, &paraTo, &indexTo, 0);
    append(QString("<code><font color=\"%1\">%2</font></code><br>").arg(color).arg(line));
    setSelection(paraFrom, indexFrom, paraTo, indexTo, 0);
    
    if (move) {
        moveCursor(MoveEnd, false);
    }

}

#include "makewidget.moc"
