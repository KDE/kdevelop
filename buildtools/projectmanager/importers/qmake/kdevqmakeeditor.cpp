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
#include "kdevqmakeeditor.h"

#include <qdir.h>
#include <qfileinfo.h>

#include <kgenericfactory.h>

#include <kdevprojectmodel.h>
#include <kdevproject.h>
#include <kdevcore.h>
#include <kurl.h>
#include <qmakedriver.h>
#include <qmakeast.h>

#include "modelcreator.h"
#include "qmakeprojectmodel.h"
#include "projectconfigurationdlg.h"
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

bool KDevQMakeEditor::addFile(ProjectFileDom file, ProjectFolderDom parent)
{
	return false;
}

bool KDevQMakeEditor::addFile( ProjectFileDom file, ProjectTargetDom parent )
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
    kdDebug(9024) << k_funcinfo << endl;
    QFileInfo fileInfo(fileName);

    ProjectItemDom item;

    if (fileInfo.isDir()) {
        QMakeFolderDom folder = model->create<QMakeFolderModel>();
        folder->setName(fileName);
        item = folder->toItem();

        //building the project ast
        folder->ast = buildProjectAST(fileName);
        folder->setAbsPath(fileName);

    } else if (fileInfo.isFile()) {
        QMakeFileDom file = model->create<QMakeFileModel>();
        file->setName(fileName);
//         file->setAbsPath(fileName);
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
    kdDebug(9024) << k_funcinfo << endl;
    ProjectFolderList folderList;
    if (!dom)
        return folderList;

    kdDebug(9024) << 1 << endl;
    QMakeFolderDom qmakeDom = QMakeFolderModel::from(dom);
    kdDebug(9024) << 2 << " folder is: " << qmakeDom->name() << ", ast is: " << qmakeDom->ast << endl;

    //-------------
    //adding scopes and function scopes
    for (QValueList<QMake::AST*>::iterator it = qmakeDom->ast->m_children.begin();
        it != qmakeDom->ast->m_children.end(); ++it)
    {
        if ((*it)->nodeType() == AST::ProjectAST)
            newFolderDom(folderList, dom, *it);
    }
    //-------------

    QDir d(qmakeDom->absPath());
    if (!qmakeDom->absPath().isEmpty() && d.exists())
    {
    kdDebug(9024) << 4 << endl;
        const QFileInfoList *subdirs = d.entryInfoList(QDir::Dirs);
        for (QFileInfoList::const_iterator it = subdirs->constBegin();
            it != subdirs->constEnd(); ++it)
        {
    kdDebug(9024) << 5 << endl;
            QFileInfo *info = *it;
            if (info->isDir() && (info->fileName() != ".") && (info->fileName() != "..") )
            {
    kdDebug(9024) << 6 << " info: " << info->fileName() << endl;
                newFolderDom(folderList, dom, 0, info);
    kdDebug(9024) << 7 << endl;
            }
        }
    }

    return folderList;
}

QString KDevQMakeEditor::findMakefile(ProjectFolderDom dom) const
{
	QString path = dom->name();
	QString project = path.section('/',-1);
	return path + "/" + project + ".pro";
}

QStringList KDevQMakeEditor::findMakefiles(ProjectFolderDom /*dom*/) const
{
    return "";
}

void KDevQMakeEditor::fillContextMenu(QPopupMenu *popup, const Context *context)
{
	kdDebug(9024) << "Ask for context menu: " << context->type() << endl;
	if( context->type() == Context::ProjectModelItemContext )
	{
		kdDebug(9024) << "Project model" << endl;
		const ProjectModelItem *modelItem = static_cast<const ProjectModelItemContext *>(context)->item();
		if( modelItem->isTarget() )
		{
			kdDebug(9024) << "Target" << endl;
			
		}
		else if( modelItem->isFile() )
		{
			kdDebug(9024) << "File" << endl;
		}
	}
	else if( context->type() == Context::FileContext )
	{
		kdDebug(9024) << "File model" << endl;
		const KURL::List urls = static_cast<const FileContext *>(context)->urls();
		
	}
}
bool KDevQMakeEditor::createProjectFile( const QString &folder )
{
	return false;
}

bool KDevQMakeEditor::configureFolder( ProjectFolderDom folder)
{
	QMakeFolderModel *model = QMakeFolderModel::from(folder);
        if( model )
        {
		ProjectConfigurationDlg dlg(model,0);
		return dlg.exec();
        }
}

#include "kdevqmakeeditor.moc"
