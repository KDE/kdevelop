/* This file is part of KDevelop
    Copyright (C) 2005 Roberto Raggi <roberto@kdevelop.org>

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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KDEVPROJECTMODEL_H
#define KDEVPROJECTMODEL_H

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>

#include <kurl.h>

#include "domutil.h"

namespace Koncrete
{

class Project;
class BuildProject;
class ProjectFolderItem;
class ProjectFileItem;
class ProjectTargetItem;

/**
 * Interface that allows a developer to implement the three basic types of
 * items you would see in a project
 * \li Folder
 * \li Build Target
 * \li File
 */
class KDEVPLATFORM_EXPORT ProjectItem: public QStandardItem
{
public:
  ProjectItem(const QString &name, QStandardItem *parent = 0)
    : QStandardItem(name)
  {
    if ( parent )
      parent->setChild( parent->rowCount(), this );
  }
  
  /**
   * add the item @p item to the list of children for this item
   * do not use this function if you gave the item a parent when you
   * created it
   */
  void add( ProjectItem* item ) { setChild( rowCount(), item ); }

  enum ProjectItemType {
    Folder = QStandardItem::UserType,
    File,
    Target,
    BuildFolder
  };

  // Convenience function to return the current project
  Project* project() const;

  virtual ProjectFolderItem *folder() const { return 0; }
  virtual ProjectTargetItem *target() const { return 0; }
  virtual ProjectFileItem *file() const { return 0; }

  QList<ProjectFolderItem*> folderList() const;
  QList<ProjectTargetItem*> targetList() const;
  QList<ProjectFileItem*> fileList() const;
};

/**
 * Implementation of the ProjectItem interface that is specific to a 
 * folder
 */
class KDEVPLATFORM_EXPORT ProjectFolderItem: public ProjectItem
{
public:
  ProjectFolderItem(const KUrl &dir, QStandardItem *parent = 0);

  virtual ProjectFolderItem *folder() const
  { return const_cast<ProjectFolderItem*>(this); }

  ///Reimplemented from QStandardItem
  virtual int type() const { return ProjectItem::Folder; }

  /** Get the url of this folder */
  const KUrl& url() const;

  /** Set the url of this folder */
  void setUrl( const KUrl& );

private:
  KUrl m_url;
};

/**
 * Folder which contains buildable targets as part of a buildable project
 */
class KDEVPLATFORM_EXPORT ProjectBuildFolderItem: public ProjectFolderItem
{
public:
  ProjectBuildFolderItem(const KUrl &dir, QStandardItem *parent = 0)
  : ProjectFolderItem(dir, parent) {}

  ///Reimplemented from QStandardItem
  virtual int type() const { return ProjectItem::BuildFolder; }
  
  void setIncludeDirectories( const KUrl::List& includeList );
  
  /**
   * Return a list of directories that are used as include directories
   * for all targets in this directory.
   */
  const KUrl::List& includeDirectories() const;

  /**
   * Returns an association of environment variables which have been defined
   * for all targets in this directory.
   */
  const QHash<QString, QString>& environment() const;

private:
  KUrl::List m_includeDirs; ///include directories
  QHash<QString, QString> m_env;
};

/**
 * Object which represents a target in a build system.
 *
 * This object contains all properties specific to a target.
 */
class KDEVPLATFORM_EXPORT ProjectTargetItem: public ProjectItem
{
public:
  ProjectTargetItem(const QString &name, QStandardItem *parent = 0)
    : ProjectItem(name, parent) {}

  ///Reimplemented from QStandardItem
  virtual int type() const { return ProjectItem::Target; }
  
  virtual ProjectTargetItem *target() const
  { return const_cast<ProjectTargetItem*>(this); }

  /**
   * Return a list of directories that are used as additional include directories
   * specific to this target.
   */
  virtual const KUrl::List& includeDirectories() const = 0;

  /**
   * Returns an association of additional environment variables which have been defined
   * specifically for this target.
   */
  virtual const QHash<QString, QString>& environment() const = 0;

  /**
   * Returns a list of defines passed to the compiler with -D(macro) (value)
   */
  virtual const DomUtil::PairList& defines() const = 0;
};

/**
 * Object which represents a file.
 */
class KDEVPLATFORM_EXPORT ProjectFileItem: public ProjectItem
{
public:
  ProjectFileItem(const KUrl& file, QStandardItem *parent = 0);

  ///Reimplemented from QStandardItem
  virtual int type() const { return ProjectItem::File; }

  virtual ProjectFileItem *file() const
  { return const_cast<ProjectFileItem*>(this); }

  /** Get the url of this file. */
  const KUrl& url() const;
  void setUrl( const KUrl& );

private:
  KUrl m_url;
};

class KDEVPLATFORM_EXPORT ProjectModel: public QStandardItemModel
{
  Q_OBJECT
public:
  ProjectModel(QObject *parent = 0);
  virtual ~ProjectModel();

  using QStandardItemModel::item;
  ProjectItem *item(const QModelIndex &index) const;

  void resetModel();

};

}

#endif // KDEVPROJECTMODEL_H
//kate: space-indent on; indent-width 2; indent-mode cstyle; replace-tabs on;
