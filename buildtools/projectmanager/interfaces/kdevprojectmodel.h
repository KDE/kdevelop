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
#ifndef KDEVPROJECTMODEL_H
#define KDEVPROJECTMODEL_H

#include <codemodel.h>
#include <ksharedptr.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qvariant.h>

class ProjectModel;
class ProjectModelItem;
class ProjectDependenceModel;
class ProjectFileModel;
class ProjectTargetModel;
class ProjectFolderModel;
class ProjectContainerModel;
class ProjectWorkspaceModel;

typedef KSharedPtr<ProjectModelItem> ProjectItemDom;
typedef KSharedPtr<ProjectDependenceModel> ProjectDependenceDom;
typedef KSharedPtr<ProjectFileModel> ProjectFileDom;
typedef KSharedPtr<ProjectTargetModel> ProjectTargetDom;
typedef KSharedPtr<ProjectFolderModel> ProjectFolderDom;
typedef KSharedPtr<ProjectWorkspaceModel> ProjectWorkspaceDom;

typedef QValueList<ProjectItemDom> ProjectItemList;
typedef QValueList<ProjectDependenceDom> ProjectDependenceList;
typedef QValueList<ProjectFileDom> ProjectFileList;
typedef QValueList<ProjectTargetDom> ProjectTargetList;
typedef QValueList<ProjectFolderDom> ProjectFolderList;
typedef QValueList<ProjectWorkspaceDom> ProjectWorkspaceList;

class ProjectModel
{
public:
    ProjectModel();
    virtual ~ProjectModel();

    template <class T> typename T::Ptr create()
    {
	typename T::Ptr ptr( new T(this) );
	return ptr;
    }

    void wipeout();
    
    inline ProjectItemList itemList() const
    { return m_items.values(); }
    
    ProjectItemDom itemByName(const QString &name) const;
    void addItem(ProjectItemDom dom);
    void removeItem(ProjectItemDom dom);
    
private:
    QMap<QString, ProjectItemDom> m_items;
};

class ProjectModelItem: public KShared
{
public:
    typedef ProjectItemDom Ptr;

    ProjectItemDom toItem();
    ProjectDependenceDom toDependence();
    ProjectFileDom toFile();
    ProjectTargetDom toTarget();
    ProjectFolderDom toFolder(); 
    ProjectWorkspaceDom toWorkspace();
        
    virtual bool isDependence() const { return false; }
    virtual bool isFile() const { return false; }
    virtual bool isTarget() const { return false; }
    virtual bool isFolder() const { return false; }
    virtual bool isWorkspace() const { return false; }

protected:
    ProjectModelItem(ProjectModel *model)
        : m_projectModel(model), m_dirty(false) {}

public:
    virtual ~ProjectModelItem() {}
    
    virtual QString shortDescription() const;

    inline ProjectModel *projectModel() const
    { return m_projectModel; }
        
    inline QString name() const
    { return m_name; }
    
    inline void setName(const QString &name)
    { m_name = name; }
    
    inline bool isDirty() const
    { return m_dirty; }
    
    inline void setDirty(bool dirty)
    { m_dirty = dirty; }
    
//
// attributes
//
    inline QMap<QString, QVariant> attributes() const
    { return m_attributes; }
    
    inline bool hasAttribute(const QString &name) const
    { return m_attributes.contains(name); }
    
    inline QVariant attribute(const QString &name) const
    { return m_attributes.contains(name) ? m_attributes[name] : QVariant(); }
    
    inline void setAttribute(const QString &name, const QVariant &value)
    { m_attributes.insert(name, value); }
    
    inline void removeAttribute(const QString &name)
    { m_attributes.remove(name); }
    
private:
    ProjectModel *m_projectModel;
    QString m_name;
    bool m_dirty;
    QMap<QString, QVariant> m_attributes;
    
    friend class ProjectModel;    
};

class ProjectDependenceModel: public ProjectModelItem
{
    typedef ProjectDependenceDom Ptr;
    
protected:
    ProjectDependenceModel(ProjectModel *projectModel)
        : ProjectModelItem(projectModel) {}
    
public:
    virtual bool isDependence() const { return true; }
    
private:
    friend class ProjectModel;
};


class ProjectFileModel: public ProjectModelItem
{
    typedef ProjectFileDom Ptr;
    
protected:
    ProjectFileModel(ProjectModel *projectModel)
        : ProjectModelItem(projectModel) {}
       
public:
    virtual bool isFile() const { return true; }
 
    virtual QString shortDescription() const;
    
    inline FileDom model(CodeModel *model) const
    { return model->fileByName(name()); }
    
private:
    friend class ProjectModel;
};

class ProjectTargetModel: public ProjectModelItem
{
    typedef ProjectTargetDom Ptr;
    
protected:
    ProjectTargetModel(ProjectModel *projectModel)
        : ProjectModelItem(projectModel) {}

public:
    virtual bool isTarget() const { return true; }

    ProjectFileList fileList() const; 
    ProjectFileDom fileByName(const QString &name) const;
    void addFile(ProjectFileDom dom);
    void removeFile(ProjectFileDom dom);
        
private:
    QMap<QString, ProjectFileDom> m_files;
    
    friend class ProjectModel;
};

class ProjectFolderModel: public ProjectModelItem
{
    typedef ProjectFolderDom Ptr;
    
protected:
    ProjectFolderModel(ProjectModel *projectModel)
        : ProjectModelItem(projectModel) {}

public:
    virtual bool isFolder() const { return true; }

    virtual QString shortDescription() const;
    
    ProjectFolderList folderList() const;    
    ProjectFolderDom folderByName(const QString &name) const;
    void addFolder(ProjectFolderDom dom);
    void removeFolder(ProjectFolderDom dom);

    ProjectFileList fileList() const;    
    ProjectFileDom fileByName(const QString &name) const;
    void addFile(ProjectFileDom dom);
    void removeFile(ProjectFileDom dom);
    
    ProjectTargetList targetList() const;    
    ProjectTargetDom targetByName(const QString &name) const;
    void addTarget(ProjectTargetDom dom);
    void removeTarget(ProjectTargetDom dom);
        
private:
    QMap<QString, ProjectFolderDom> m_folders;
    QMap<QString, ProjectFileDom> m_files;
    QMap<QString, ProjectTargetDom> m_targets;
                
private:
    friend class ProjectModel;
};

class ProjectWorkspaceModel: public ProjectFolderModel
{
    typedef ProjectWorkspaceDom Ptr;
    
protected:
    ProjectWorkspaceModel(ProjectModel *projectModel)
        : ProjectFolderModel(projectModel) {}
        
public:
    virtual bool isWorkspace() const { return false; }

    virtual QString shortDescription() const;
    
private:        
    friend class ProjectModel;
};


// casts
inline ProjectItemDom ProjectModelItem::toItem()
{ return ProjectItemDom(this); }

inline ProjectDependenceDom ProjectModelItem::toDependence()
{ return isDependence() ? ProjectDependenceDom(static_cast<ProjectDependenceModel*>(this)) : 0; }

inline ProjectFileDom ProjectModelItem::toFile()
{ return isFile() ? ProjectFileDom(static_cast<ProjectFileModel*>(this)) : 0; }

inline ProjectTargetDom ProjectModelItem::toTarget()
{ return isTarget() ? ProjectTargetDom(static_cast<ProjectTargetModel*>(this)) : 0; }

inline ProjectFolderDom ProjectModelItem::toFolder() 
{ return isFolder() ? ProjectFolderDom(static_cast<ProjectFolderModel*>(this)) : 0; }

inline ProjectWorkspaceDom ProjectModelItem::toWorkspace()
{ return isWorkspace() ? ProjectWorkspaceDom(static_cast<ProjectWorkspaceModel*>(this)) : 0; }


#endif // KDEVPROJECTMODEL_H
