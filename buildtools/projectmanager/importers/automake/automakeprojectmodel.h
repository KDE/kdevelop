/* This file is part of KDevelop
    Copyright (C) 2004 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/
#ifndef AUTOMAKEPROJECTMODEL_H
#define AUTOMAKEPROJECTMODEL_H

#include <kdevprojectmodel.h>
//Added by qt3to4:
#include <Q3ValueList>

class AutomakeFolderModel;
class AutomakeTargetModel;
class AutomakeFileModel;

typedef KSharedPtr<AutomakeFolderModel> AutomakeFolderDom;
typedef KSharedPtr<AutomakeTargetModel> AutomakeTargetDom;
typedef KSharedPtr<AutomakeFileModel> AutomakeFileDom;

typedef Q3ValueList<AutomakeFolderDom> AutomakeFolderList;
typedef Q3ValueList<AutomakeTargetDom> AutomakeTargetList;
typedef Q3ValueList<AutomakeFileDom> AutomakeFileList;

class AutomakeTargetModel: public ProjectTargetModel
{
public:
    typedef AutomakeTargetDom Ptr;
    
protected:
    AutomakeTargetModel(ProjectModel *projectModel)
        : ProjectTargetModel(projectModel) {}
        
public: 
    static AutomakeTargetDom from(ProjectTargetDom dom)
    { return AutomakeTargetDom(dynamic_cast<AutomakeTargetModel*>(dom.data())); }

// ### use attribute/setAttribute    
    QString path;
    QString primary;
    QString prefix;
    
    QString ldflags;
    QString ldadd;
    QString libadd;
    QString dependencies;
    
private:
    friend class ProjectModel;
};

class AutomakeFileModel: public ProjectFileModel
{
public:
    typedef AutomakeFileDom Ptr;
    
protected:
    AutomakeFileModel(ProjectModel *projectModel)
        : ProjectFileModel(projectModel) {}

public:
    static AutomakeFileDom from(ProjectFileDom dom)
    { return AutomakeFileDom(dynamic_cast<AutomakeFileModel*>(dom.data())); }
    
private:
    friend class ProjectModel;
};

class AutomakeFolderModel: public ProjectFolderModel
{
public:
    typedef AutomakeFolderDom Ptr;
    
protected:
    AutomakeFolderModel(ProjectModel *projectModel)
        : ProjectFolderModel(projectModel) {}
            
public:
    static AutomakeFolderDom from(ProjectFolderDom dom)
    { return AutomakeFolderDom(dynamic_cast<AutomakeFolderModel*>(dom.data())); }
    
    QStringList subdirs() const;
    void addSubdir(const QString &path);

    QMap<QString, QString> prefixes;
    
private:
    friend class ProjectModel;
};

#endif // AUTOMAKEPROJECTMODEL_H
