/***************************************************************************
 *   Copyright (C) 1999, 2000 by Bernd Gehrmann                            *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _OUTPUTVIEWS_H_
#define _OUTPUTVIEWS_H_

#include <qguardedptr.h>
#include "kdevcomponent.h"


class MakeWidget;
class CProject;

class MakeView : public KDevComponent
{
    Q_OBJECT

public:
    MakeView( QObject *parent=0, const char *name=0 );
    ~MakeView();

protected:
    virtual void setupGUI();
    virtual void commandRequested(const QString &command);
    virtual void projectOpened(CProject *prj);
    virtual void projectClosed();
    
private:
    QGuardedPtr<MakeWidget> m_widget;
    friend class MakeWidget;

    CProject *m_prj;
};


class AppOutputWidget;

class AppOutputView : public KDevComponent
{
    Q_OBJECT

public:
    AppOutputView( QObject *parent=0, const char *name=0 );
    ~AppOutputView();

protected:
    virtual void setupGUI();
    virtual void stopButtonClicked();
    
private:
    QGuardedPtr<AppOutputWidget> m_widget;
    friend class AppOutputWidget;
};
#endif
