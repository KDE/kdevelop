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

#ifndef _CUSTOMPROJECTPART_H_
#define _CUSTOMPROJECTPART_H_

#include <qdict.h>
#include <qguardedptr.h>

#include "kdevproject.h"

class QListViewItem;
class KDialogBase;
class CustomProjectWidget;


class CustomProjectPart : public KDevProject
{
    Q_OBJECT

public:
    CustomProjectPart( QObject *parent, const char *name, const QStringList & );
    ~CustomProjectPart();

protected:
    virtual void openProject(const QString &dirName);
    virtual void closeProject();

    virtual QString mainProgram();
    virtual QString projectDirectory();
    virtual QStringList allSourceFiles();

private slots:
    void projectConfigWidget(KDialogBase *dlg);
    void slotItemExecuted(QListViewItem *item);
    void slotBuild();
    void slotClean();
    void slotExecute();
    
private:
    void startMakeCommand(const QString &dir, const QString &target);
    
    QGuardedPtr<CustomProjectWidget> m_widget;
    friend class CustomProjectWidget;
};

#endif
