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
#include "modelcreator.h"

#include <qfileinfo.h>
#include <qdir.h>
#include <qvaluelist.h>

#include <kdebug.h>

#include <kdevproject.h>
#include <qmakedriver.h>
#include <qmakeast.h>

using namespace QMake;

namespace ModelCreator {

QMake::ProjectAST *buildProjectAST(const QString &projectDir)
{
    QMake::ProjectAST *ast = 0;

    QDir d(projectDir);
    QFileInfo f(d.absFilePath(d.dirName() + ".pro"));
    QString projectFileName = f.absFilePath();
    kdDebug(9024) << "projectFileName: " << projectFileName << endl;
    if (!f.exists())    //try to locate .pro file which name differs from the name of a dir
    {
        QStringList proFiles = d.entryList("*.pro");
        if (proFiles.count() > 0)
            projectFileName = d.absFilePath(proFiles.first());
        else
        {
            ast = new ProjectAST(ProjectAST::Empty);
            return ast;
        }
    }

    QMake::Driver::parseFile(projectFileName, &ast);
    kdDebug(9024) << "parsed ast: " << ast << endl;
    return ast;
}


void newFolderDom(ProjectFolderList &folderList,
    ProjectFolderDom &folderDom, QMake::AST *ast, QFileInfo *info)
{
    QMakeFolderDom folder = folderDom->projectModel()->create<QMakeFolderModel>();
    QMake::ProjectAST *projectAST = 0;
    if (ast)
    {
        //scope, function scope, etc.
        projectAST = (QMake::ProjectAST*)(ast);
        kdDebug(9024) << "    not a subproject: " << projectAST->scopedID << endl;

        if (projectAST->isScope())
        {
            folder->setName(projectAST->scopedID);
            folder->setAttribute("Icon", "folder_yellow");
        }
        else if (projectAST->isFunctionScope())
        {
            folder->setName(projectAST->scopedID + "(" + projectAST->args + ")");
            folder->setAttribute("Icon", "folder_green");
        }
        else
        {
            folder->setName("");
            folder->setAttribute("Icon", "folder_red");
        }
    }
    else
    {
        folder->setName(info->absFilePath());
        projectAST = buildProjectAST(info->absFilePath());

        if (projectAST->isEmpty())
            folder->setAttribute("Icon", "folder_grey");
    }

    folder->ast = projectAST;
    if (info)
        folder->setAbsPath(info->absFilePath());
    QString mode;
	QStringList sources = folder->readAssignment("TEMPLATE", mode);

	if( !sources.isEmpty() )
	{
		folderList.append(folder->toFolder());
		folderDom->addFolder(folder->toFolder());
		if( sources[0] != "SUBDIRS" || sources[0] != "subdirs" )
		{
			addFileItem( "SOURCES", "source_cpp", folder );
			addFileItem( "HEADERS", "source_h", folder );
			addFileItem( "FORMS", "dlgedit", folder );
			addFileItem( "LEXSOURCES", "dlgedit", folder );
			addFileItem( "YACCSOURCES", "dlgedit", folder );
		
			QStringList assns = folder->assignmentNames();
			assns.remove("SOURCES");
			assns.remove("HEADERS");
			assns.remove("FORMS");
			assns.remove("LEXSOURCES");
			assns.remove("YACCSOURCES");
			QStringList::ConstIterator it = assns.begin();
			for( ; it != assns.end(); ++it)
				addFileItem( *it, "gear", folder);
		}
	}
}

void addFileItem( const QString &field, const QString &icon, QMakeFolderDom &folder )
{
	QString mode;
	QStringList sources = folder->readAssignment(field, mode);
	if( !sources.isEmpty() )
	{
		bool hasFiles = false;
		QMakeTargetDom target = folder->projectModel()->create<QMakeTargetModel>();
		target->setName(field);
		target->setAttribute("Icon", icon);
		QStringList::ConstIterator srcIt = sources.begin();
		for(; srcIt != sources.end(); ++srcIt)
		{
			QMakeFileDom fileModel = folder->projectModel()->create<QMakeFileModel>();
			QFileInfo fileItemInfo( folder->name(), *srcIt);
			if( fileItemInfo.exists() && fileItemInfo.isFile())
			{
				fileModel->setName(fileItemInfo.absFilePath());
				hasFiles = true;
			}
			target->addFile(fileModel->toFile());
		}
		if( hasFiles )
			folder->addTarget(target->toTarget());
	}
}

void newFileDom(QMakeFileList &fileList,
                QMakeFileDom &fileDom, QMake::AST *ast, QFileInfo *info)
{
	kdDebug(9024) << "Add assignment." <<  fileDom->name() << endl;
	QMake::AssignmentAST *assnAST = 0;
	if (ast)
	{
		assnAST = (QMake::AssignmentAST*)(ast);
	}
}
	
}
