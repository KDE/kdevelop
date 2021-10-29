/*
    SPDX-FileCopyrightText: 2006 Andreas Pakulat <apaku@gmx.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QMAKEMODELITEMS_H
#define QMAKEMODELITEMS_H

#include <project/projectmodel.h>

class QMakeProjectFile;

class QMakeFolderItem : public KDevelop::ProjectBuildFolderItem
{
public:
    QMakeFolderItem( KDevelop::IProject*,const KDevelop::Path&, KDevelop::ProjectBaseItem* parent = nullptr );
    ~QMakeFolderItem() override;

    void addProjectFile(QMakeProjectFile* file);
    QList<QMakeProjectFile*> projectFiles() const;

private:
    QList<QMakeProjectFile*> m_projectFiles;
};

class QMakeTargetItem : public KDevelop::ProjectExecutableTargetItem
{
public:
    QMakeTargetItem( QMakeProjectFile* pro, KDevelop::IProject*,
                     const QString& s, KDevelop::ProjectBaseItem* parent );
    ~QMakeTargetItem() override;

    QUrl builtUrl() const override;
    QUrl installedUrl() const override;

private:
    QMakeProjectFile *m_pro;
};


#endif

