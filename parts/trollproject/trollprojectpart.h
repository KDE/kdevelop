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

#ifndef _TROLLPROJECTPART_H_
#define _TROLLPROJECTPART_H_

#include <qdict.h>
#include <qguardedptr.h>

#include "kdevproject.h"

class KDialogBase;
class TrollProjectWidget;


class TrollProjectPart : public KDevProject
{
    Q_OBJECT

public:
    TrollProjectPart( QObject *parent, const char *name, const QStringList &args );
    ~TrollProjectPart();

protected:
    virtual void openProject(const QString &dirName);
    virtual void closeProject();

    virtual QString mainProgram();
    virtual QString projectDirectory();
    virtual QStringList allSourceFiles();

private slots:
    void projectConfigWidget(KDialogBase *dlg);
    void slotBuild();
    void slotClean();
    void slotQMake();
    void slotExecute();
    
private:
    void startMakeCommand(const QString &dir, const QString &target);
    
    QGuardedPtr<TrollProjectWidget> m_widget;
    friend class TrollProjectWidget;
};

#endif

