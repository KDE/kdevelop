/***************************************************************************
 *   Copyright (C) 2005 by Alexander Dymo                                  *
 *   adymo@kdevelop.org                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef KDEVQMAKEEDITOR_H
#define KDEVQMAKEEDITOR_H

#include <kdevprojecteditor.h>

class KDevProject;
class QFileInfo;

namespace QMake{
class ProjectAST;
class AST;
}

class KDevQMakeEditor : public KDevProjectEditor {
Q_OBJECT
public:
    KDevQMakeEditor(QObject* parent, const char* name, const QStringList &);

    bool hasFeature(Features f) const;
    virtual bool addFile(ProjectFileDom file, ProjectFolderDom parent);
    virtual bool addFile(ProjectFileDom file, ProjectTargetDom parent);
    virtual bool addFolder(ProjectFolderDom folder, ProjectFolderDom parent);
    virtual bool addTarget(ProjectTargetDom target, ProjectFolderDom parent);
    virtual bool removeFile(ProjectFileDom file, ProjectFolderDom parent);
    virtual bool removeFile(ProjectFileDom file, ProjectTargetDom parent);
    virtual bool removeFolder(ProjectFolderDom folder, ProjectFolderDom parent);
    virtual bool removeTarget(ProjectTargetDom target, ProjectFolderDom parent);
    virtual Features features() const;

    virtual KDevProject *project() const { return m_project; }
   
    virtual KDevProjectEditor *editor() const;
    virtual ProjectFolderList parse(ProjectFolderDom dom);
    virtual ProjectItemDom import(ProjectModel *model, const QString &fileName);
    virtual QString findMakefile(ProjectFolderDom dom) const;
    virtual QStringList findMakefiles(ProjectFolderDom dom) const;
            
private:
    KDevProject *m_project;
    QMake::ProjectAST *m_ast;
};

#endif
