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

#include "kdevbuildtool.h"

class QListViewItem;
class QStringList;
class KDialogBase;
class ScriptProjectWidget;


class ScriptProjectPart : public KDevBuildTool
{
    Q_OBJECT

public:
    ScriptProjectPart( QObject *parent, const char *name, const QStringList &args );
    ~ScriptProjectPart();
    QStringList distFiles() const;

protected:
    virtual void openProject(const QString &dirName, const QString &projectName);
    virtual void closeProject();

    virtual QString projectDirectory() const;
    virtual QString projectName() const;
    virtual QString mainProgram() const;
    virtual QString activeDirectory() const;
    virtual QStringList allFiles() const;
    virtual void addFile(const QString &fileName);
	virtual void addFiles ( const QStringList& fileList );
    virtual void removeFile(const QString &fileName);
    virtual void removeFiles ( const QStringList& fileList );
    virtual QString buildDirectory() const;
    virtual QString runDirectory() const;
    virtual QString debugArguments() const;
    virtual QString runArguments() const;
    virtual DomUtil::PairList runEnvironmentVars() const;

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
