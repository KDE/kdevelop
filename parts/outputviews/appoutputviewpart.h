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

#ifndef _APPOUTPUTVIEWSPART_H_
#define _APPOUTPUTVIEWSPART_H_

#include <qguardedptr.h>

#include "kdevappfrontend.h"
#include "KDevAppFrontendIface.h"


class AppOutputWidget;

class AppOutputViewPart : public KDevAppFrontend
{
    Q_OBJECT

public:
    AppOutputViewPart( QObject *parent, const char *name, const QStringList & );
    ~AppOutputViewPart();

protected:
    virtual void startAppCommand(const QString &command);
    virtual bool isRunning();
    virtual void insertStdoutLine(const QString &line);
    virtual void insertStderrLine(const QString &line);

private:
    QGuardedPtr<AppOutputWidget> m_widget;
    KDevAppFrontendIface *m_dcop;
    friend class AppOutputWidget;
};

#endif
