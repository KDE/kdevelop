/***************************************************************************
 *   Copyright (C) 2001 by Bernd Gehrmann                                  *
 *   bernd@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _AUTOPROJECTPART_H_
#define _AUTOPROJECTPART_H_

#include <qdict.h>

#include "kdevproject.h"

class AutoProjectWidget;


class AutoProjectPart : public KDevProject
{
    Q_OBJECT

public:
    AutoProjectPart( KDevApi *api, QObject *parent=0, const char *name=0 );
    ~AutoProjectPart();

protected:
    virtual void openProject(const QString &dirName);
    virtual void closeProject();

    virtual QString projectDirectory();
    virtual QStringList allSourceFiles();

private slots:
    void slotBuild();
    void slotClean();
    void slotConfigure();
    void slotProjectOptions();
    
private:
    void startMakeCommand(const QString &dir, const QString &target);
    
    QGuardedPtr<AutoProjectWidget> m_widget;
    friend class AutoProjectWidget;
};

#endif

