/***************************************************************************
                             processview.h - base class for output views
                             -------------------                                         
    copyright            : (C) 1999 by The KDevelop Team
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef PROCESSVIEW_H
#define PROCESSVIEW_H

#include <qlistbox.h>
#include "component.h"

class KProcess;


class ProcessListBoxItem : public QListBoxText
{
public:
    enum Type { Diagnostic, Normal, Error };
    
    ProcessListBoxItem(const QString &s, Type type);

    virtual bool isCustomItem();
    
private:
    virtual void paint(QPainter *p);
    Type t;
};


class ProcessView : public QListBox, public Component
{
    Q_OBJECT
public:
    ProcessView(QWidget *parent, const char *name=0);
    ~ProcessView();

    /**
     * Inserts one line from stdin into the listbox. This can
     * be overridden by subclasses to implement
     * syntax highlighting.
     */
    virtual void insertStdoutLine(const QString &line);
    /**
     * Inserts one line from stderr into the listbox. This can
     * be overridden by subclasses to implement
     * syntax highlighting. By default, a ProcessListBoxItem
     * is used.
     */
    virtual void insertStderrLine(const QString &line);
    /**
     * This is called when the child process exits.
     * The flag 'normal' is true if the process exited
     * normally (i.e. not by a signal or similar), otherwise
     * the exit status can be taken from 'status'.
     */
    virtual void childFinished(bool normal, int status);
    /**
     * Clears the child process's arguments and changes
     * into the directory dir.
     */
    void prepareJob(const QString &dir);
    /**
     * Starts the child process.
     */
    void startJob();
    /**
     * Kills the child processss.
     */
    void killJob();
    /**
     * Sets the next argument for the child process.
     */
    KProcess &operator<<(const QString& arg);
    /**
     * Returns whether a process is running in this view.
     */
    bool isRunning();

signals:
    void processExited(KProcess *); 
    void rowSelected(int row);

protected:
    // Component notifications:
    virtual void compilationAborted();
    
protected slots:
    void slotReceivedOutput(KProcess*, char*, int);
    void slotReceivedError(KProcess*, char*, int);
    void slotProcessExited(KProcess*);

private:
    void flushStdoutBuf();
    void flushStderrBuf();
    KProcess *childproc;
    QString stdoutbuf;
    QString stderrbuf;
};


#endif
