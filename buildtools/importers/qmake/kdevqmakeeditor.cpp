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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/
#include "kdevqmakeeditor.h"

#include <qdir.h>
#include <qfileinfo.h>
//Added by qt3to4:
#include <Q3ValueList>

#include <kgenericfactory.h>

#include <kdevprojectmodel.h>
#include <kdevproject.h>
#include <qmakedriver.h>
#include <qmakeast.h>

#include "modelcreator.h"
#include "qmakeprojectmodel.h"

using namespace ModelCreator;

K_EXPORT_COMPONENT_FACTORY(libqmakeimporter, KGenericFactory<KDevQMakeEditor>("kdevqmakeimporter"))

using namespace QMake;

KDevQMakeEditor::KDevQMakeEditor(QObject* parent, const char* name, const QStringList &): 
    KDevProjectEditor(parent, name), m_ast(0)
{
    m_project = ::qt_cast<KDevProject*>(parent);
    Q_ASSERT(m_project);
}

bool KDevQMakeEditor::hasFeature(Features f) const
{
    return f & features();
}

bool KDevQMakeEditor::addFile(ProjectFileDom /*file*/, ProjectFolderDom /*parent*/)
{
    return false;
}

bool KDevQMakeEditor::addFile(ProjectFileDom /*file*/, ProjectTargetDom /*parent*/)
{
    return false;
}

bool KDevQMakeEditor::addFolder(ProjectFolderDom /*folder*/, ProjectFolderDom /*parent*/)
{
    return false;
}

bool KDevQMakeEditor::addTarget(ProjectTargetDom /*target*/, ProjectFolderDom /*parent*/)
{
    return false;
}

bool KDevQMakeEditor::removeFile(ProjectFileDom /*file*/, ProjectFolderDom /*parent*/)
{
    return false;
}

bool KDevQMakeEditor::removeFile(ProjectFileDom /*file*/, ProjectTargetDom /*parent*/)
{
    return false;
}

bool KDevQMakeEditor::removeFolder(ProjectFolderDom /*folder*/, ProjectFolderDom /*parent*/)
{
    return false;
}

bool KDevQMakeEditor::removeTarget(ProjectTargetDom /*target*/, ProjectFolderDom /*parent*/)
{
    return false;
}

KDevQMakeEditor::Features KDevQMakeEditor::features() const
{
    return All;
}

KDevProjectEditor * KDevQMakeEditor::editor() const
{
    return (KDevProjectEditor*)(this);
}

ProjectItemDom KDevQMakeEditor::import(ProjectModel *model, const QString &fileName)
{
    kDebug() << k_funcinfo << endl;
    QFileInfo fileInfo(fileName);
    
    ProjectItemDom item;
    
    if (fileInfo.isDir()) {
        QMakeFolderDom folder = model->create<QMakeFolderModel>();
        folder->setName(fileName);
        item = folder->toItem();
        
        //building the project ast
        folder->ast = buildProjectAST(fileName);
        
    } else if (fileInfo.isFile()) {
        QMakeFileDom file = model->create<QMakeFileModel>();
        file->setName(fileName);
        item = file->toItem();
        
        //creating ast for one file in case a filename is given
        //@fixme it's unclear when does this situation happen
        if (m_ast)
            delete m_ast;
        QMake::Driver::parseFile(fileName, &m_ast);
    }
    
    return item;
}

ProjectFolderList KDevQMakeEditor::parse(ProjectFolderDom dom)
{    
    kDebug() << k_funcinfo << endl;
    ProjectFolderList folderList;
    if (!dom)
        return folderList;
    
    kDebug() << 1 << endl;
    QMakeFolderDom qmakeDom = QMakeFolderModel::from(dom);
    kDebug() << 2 << " folder is: " << qmakeDom->name() << ", ast is: " << qmakeDom->ast << endl;
    
    //-------------
    //adding scopes and function scopes
    for (Q3ValueList<QMake::AST*>::iterator it = qmakeDom->ast->statements.begin();
        it != qmakeDom->ast->statements.end(); ++it)
    {
        if ((*it)->nodeType() == AST::ProjectAST)
            newFolderDom(folderList, dom, *it);
    }
    //-------------
    kDebug() << 3 << endl;
    
    QDir d(dom->name());
    if (d.exists())
    {
    kDebug() << 4 << endl;
        const QFileInfoList *subdirs = d.entryInfoList(QDir::Dirs);
        for (QFileInfoList::const_iterator it = subdirs->constBegin(); 
            it != subdirs->constEnd(); ++it)
        {
    kDebug() << 5 << endl;
            QFileInfo *info = *it;
            if (info->isDir() && (info->fileName() != ".") && (info->fileName() != "..") )
            {
    kDebug() << 6 << " info: " << info->fileName() << endl;
                newFolderDom(folderList, dom, 0, info);
    kDebug() << 7 << endl;
            }
        }
    }
    
    return folderList;
}

QString KDevQMakeEditor::findMakefile(ProjectFolderDom dom) const
{
    return dom->name() + "/" + dom->name() + ".pro";
}

QStringList KDevQMakeEditor::findMakefiles(ProjectFolderDom /*dom*/) const
{
    return "";
}

QList<KDevProjectTargetItem*> KDevQMakeEditor::targets() const
{
    return QList<KDevProjectTargetItem*>();
}

FileItemList KDevQMakeEditor::filesForTarget(KDevProjectTargetItem *) const
{
    return FileItemList();
}

#include "kdevqmakeeditor.moc"
