/***************************************************************************
 *   Copyright (C) 2001-2002 by Bernd Gehrmann                             *
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
#include <qguardedptr.h>
#include <kgenericfactory.h>

#include "kdevproject.h"

class KDialogBase;
class AutoProjectWidget;


class AutoProjectPart : public KDevProject
{
    Q_OBJECT

public:
    AutoProjectPart( QObject *parent, const char *name, const QStringList &args );
    ~AutoProjectPart();

protected:
    virtual void openProject(const QString &dirName, const QString &projectName);
    virtual void closeProject();

    virtual QString projectDirectory();
    virtual QString projectName();
    virtual QString mainProgram();
    virtual QString activeDirectory();
    virtual QStringList allSourceFiles();
    virtual void addFile(const QString &fileName);
    virtual void removeFile(const QString &fileName);
    
private slots:
    void projectConfigWidget(KDialogBase *dlg);
    void slotAddTranslation();
    void slotBuild();
    void slotClean();
    void slotDistClean();
    void slotInstall();
    void slotMakefilecvs();
    void slotMakeMessages();
    void slotConfigure();
    void slotExecute();
    
private:
    void startMakeCommand(const QString &dir, const QString &target);
    
    QGuardedPtr<AutoProjectWidget> m_widget;
    QString m_projectName;
    friend class AutoProjectWidget;
    friend class CompilerOptionsWidget;
    friend class AddTranslationDialog;
};

typedef KGenericFactory<AutoProjectPart> AutoProjectFactory;

#endif
