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

#ifndef _PROCESSWIDGET_H_
#define _PROCESSWIDGET_H_

#include <klistbox.h>

class KProcess;
class ProcessLineMaker;

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


/**
 * This class is designed to share code between the
 * grep and application output widgets.
 */
class ProcessWidget : public KListBox
{
    Q_OBJECT

public:
    ProcessWidget(QWidget *parent, const char *name=0);
    ~ProcessWidget();

    /**
     * Returns whether a process is running in this view.
     */
    bool isRunning();
    
public slots:
    /**
     * Starts the child process.
     */
    void startJob(const QString &dir, const QString &command);
    /**
     * Kills the child processss.
     */
    void killJob();
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

protected:
    /**
     * This is called when the child process exits.
     * The flag 'normal' is true if the process exited
     * normally (i.e. not by a signal or similar), otherwise
     * the exit status can be taken from 'status'.
     */
    virtual void childFinished(bool normal, int status);

signals:
    void processExited(KProcess *); 
    void rowSelected(int row);

protected:
    virtual QSize minimumSizeHint() const;
    
protected slots:
    void slotProcessExited(KProcess*);

private:
    KProcess *childproc;
    ProcessLineMaker* procLineMaker;
};


#endif
