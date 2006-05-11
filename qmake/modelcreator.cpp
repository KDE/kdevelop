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
#include "modelcreator.h"

#include <qfileinfo.h>
#include <QDir>

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
    QFileInfo f(d.absoluteFilePath(d.dirName() + ".pro"));
    QString projectFileName = f.absoluteFilePath();
    kDebug() << "projectFileName: " << projectFileName << endl;
    if (!f.exists())    //try to locate .pro file which name differs from the name of a dir
    {
        QStringList proFiles = d.entryList("*.pro");
        if (proFiles.count() > 0)
            projectFileName = d.absoluteFilePath(proFiles.first());
        else
        {
            ast = new ProjectAST(ProjectAST::Empty);
            return ast;
        }
    }
    
    QMake::Driver::parseFile(projectFileName, &ast);
    kDebug() << "parsed ast: " << ast << endl;
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
        kDebug() << "    not a subproject: " << projectAST->scopedID << endl;
        
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
        folder->setName(info->fileName());
        projectAST = buildProjectAST(info->absoluteFilePath());
        
        if (projectAST->isEmpty())
            folder->setAttribute("Icon", "folder_grey");    
    }
    
    folder->ast = projectAST;
    folderList.append(folder->toFolder());
    folderDom->addFolder(folder->toFolder());
    
    QMakeTargetDom target = folderDom->projectModel()->create<QMakeTargetModel>();
    folder->addTarget(target->toTarget());
}


}
