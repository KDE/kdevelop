/***************************************************************************
                     processview.h - log window for compiler output
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


class ProcessView : public QListBox, public Component
{
    Q_OBJECT
public:
    ProcessView(QWidget *parent, const char *name=0);
    ~ProcessView();

    /**
     * Inserts one line into the listbox. This can
     * be overridden by subclasses to implement
     * syntax highlighting.
     */
    virtual void insert(const QString &line);
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

signals:
    void processExited(KProcess *); 
    void rowSelected(int row);

protected slots:
    void slotReceivedOutput(KProcess*, char*, int);
    void slotProcessExited(KProcess*);

private:
    KProcess *childproc;
    QString buf;
};

#endif
