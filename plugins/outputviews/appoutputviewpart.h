/***************************************************************************
 *   Copyright (C) 1999-2002 by Bernd Gehrmann                             *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _APPOUTPUTVIEWSPART_H_
#define _APPOUTPUTVIEWSPART_H_

#include <qpointer.h>

#include "kdevappfrontend.h"

class AppOutputWidget;

class AppOutputViewPart : public KDevAppFrontend
{
    Q_OBJECT

public:
    AppOutputViewPart( QObject *parent, const char *name, const QStringList & );
    ~AppOutputViewPart();

    virtual void startAppCommand(const QString &directory, const QString &command, bool inTerminal);
    virtual void stopApplication();
    virtual bool isRunning();
    virtual void insertStdoutLine(const QString &line);
    virtual void insertStderrLine(const QString &line);
    virtual void clearView();

signals:
    void processExited();

private slots:
    void slotStopButtonClicked(KDevPlugin*);
    void slotProcessExited();

private:
    QPointer<AppOutputWidget> m_widget;
    friend class AppOutputWidget;
};

#endif
