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

#ifndef _SCRIPTPROJECTPART_H_
#define _SCRIPTPROJECTPART_H_

#include <qdict.h>
#include <qguardedptr.h>

#include "kdevproject.h"

class QListViewItem;
class QStringList;
class KDialogBase;
class ScriptProjectWidget;


class ScriptProjectPart : public KDevProject
{
    Q_OBJECT

public:
    ScriptProjectPart( QObject *parent, const char *name, const QStringList &args );
    ~ScriptProjectPart();

protected:
    virtual void openProject(const QString &dirName, const QString &projectName);
    virtual void closeProject();

    virtual QString projectDirectory();
    virtual QString projectName();
    virtual QString mainProgram(bool relative = false);
    virtual QString activeDirectory();
    virtual QStringList allFiles();
    virtual void addFile(const QString &fileName);
	virtual void addFiles ( const QStringList& fileList );
    virtual void removeFile(const QString &fileName);
    virtual void removeFiles ( const QStringList& fileList );
    virtual QString buildDirectory();
    virtual QString runDirectory();
    virtual QString runArguments();
    virtual DomUtil::PairList runEnvironmentVars();

private slots:
    void projectConfigWidget(KDialogBase *dlg);
    void slotNewFile();
     
private:
    QString m_projectDirectory;
    QString m_projectName;
    QStringList m_sourceFiles;

    friend class ScriptNewFileDialog;
};

#endif
