/***************************************************************************
 *   Copyright (C) 2003 Roberto Raggi                                      *
 *   roberto@kdevelop.org                                                  *
 *   Copyright (C) 2003 Alexander Dymo                                     *
 *   cloudtemple@mksat.net                                                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef __KDEVPART_GENERICPROJECT_H__
#define __KDEVPART_GENERICPROJECT_H__


#include <qguardedptr.h>
#include "kdevproject.h"

#include "kdevbuildsystem.h"

class GenericProjectWidget;
class BuildGroupItem;
class GenericGroupListViewItem;

class GenericProjectPart : public KDevProject
{
    Q_OBJECT
public:
    GenericProjectPart(QObject *parent, const char *name, const QStringList &);
    virtual ~GenericProjectPart();

    BuildGroupItem* mainGroup() { return m_mainBuild; }
//    void setMainGroup( BuildGroupItem* mainGroup );

    virtual void openProject(const QString &dirName, const QString &projectName);
    virtual void closeProject();
    virtual QString projectDirectory() const;
    virtual QString projectName() const;
    virtual DomUtil::PairList runEnvironmentVars() const;
    virtual QString mainProgram(bool relative = false) const;
    virtual QString runDirectory() const;
    virtual QString runArguments() const;
    virtual QString activeDirectory() const;
    virtual QString buildDirectory() const;
    virtual QStringList allFiles() const;
    virtual void addFiles(const QStringList &fileList );
    virtual void addFile(const QString &fileName);
    virtual void removeFiles ( const QStringList& fileList );
    virtual void removeFile(const QString &fileName);

    KDevBuildSystem *buildSystem() const;
signals:
    void mainGroupChanged( BuildGroupItem* group );

private:
    KDevBuildSystem *m_buildSystem;

    void loadProjectConfig(QString projectFile);

    void parseGroup(const QDomElement &el, const GenericGroupListViewItem *parentGroup);
    void parseTarget(const QDomElement &el, BuildGroupItem *group);
    void parseFile(const QDomElement &el, BuildTargetItem *target);

    GenericGroupListViewItem *createGroupItem(const QDomElement &el, const GenericGroupListViewItem *parent = 0 );
    BuildTargetItem *createTargetItem(const QDomElement &el, BuildGroupItem *group);
    BuildFileItem *createFileItem(const QDomElement &el, BuildTargetItem *target);

    void addFilePrivate( const QString & fileName, BuildTargetItem *tit);

    QGuardedPtr<GenericProjectWidget> m_widget;
    GenericGroupListViewItem* m_mainGroup;
    BuildGroupItem* m_mainBuild;

    QString m_buildDir;
    QString m_projectDir;
    QString m_projectName;
    QString m_activeDir;
    QString m_runDir;
};


#endif
