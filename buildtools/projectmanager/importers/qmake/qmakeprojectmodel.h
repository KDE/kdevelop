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
#ifndef QMAKEFOLDERMODEL_H
#define QMAKEFOLDERMODEL_H

#include <kdevprojectmodel.h>

class QMakeFolderModel;
class QMakeTargetModel;
class QMakeFileModel;

typedef KSharedPtr<QMakeFolderModel> QMakeFolderDom;
typedef KSharedPtr<QMakeTargetModel> QMakeTargetDom;
typedef KSharedPtr<QMakeFileModel> QMakeFileDom;

typedef QValueList<QMakeFolderDom> QMakeFolderList;
typedef QValueList<QMakeTargetDom> QMakeTargetList;
typedef QValueList<QMakeFileDom> QMakeFileList;


namespace QMake {
class ProjectAST;
class AssignmentAST;
}

class QMakeFolderModel: public ProjectFolderModel {
protected:
    QMakeFolderModel(ProjectModel* projectModel);
    ~QMakeFolderModel();

public:
    typedef QMakeFolderDom Ptr;

    static QMakeFolderDom from(ProjectFolderDom dom)
    { return QMakeFolderDom(dynamic_cast<QMakeFolderModel*>(dom.data())); }

    bool isRoot() const { return m_isRoot; }
    void setRoot() { m_isRoot = true; } 
    
    QMake::ProjectAST *ast;
    
private:
    bool m_isRoot;
friend class ProjectModel;
};

class QMakeFileModel: public ProjectFileModel {
protected:
    QMakeFileModel(ProjectModel* projectModel);

public:
    typedef QMakeFileDom Ptr;
    
    static QMakeFileDom from(ProjectFileDom dom)
    { return QMakeFileDom(dynamic_cast<QMakeFileModel*>(dom.data())); }
friend class ProjectModel;
};

class QMakeTargetModel: public ProjectTargetModel {
protected:
    QMakeTargetModel(ProjectModel* projectModel);
    ~QMakeTargetModel();

public:
    typedef QMakeTargetDom Ptr;
    
    static QMakeTargetDom from(ProjectTargetDom dom)
    { return QMakeTargetDom(dynamic_cast<QMakeTargetModel*>(dom.data())); }
    
    QMake::AssignmentAST *ast;
friend class ProjectModel;
};

#endif
