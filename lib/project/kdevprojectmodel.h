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

class KDevProject;
class KDevBuildProject;
class KDevProjectFolderItem;
class KDevProjectFileItem;
class KDevProjectTargetItem;

/**
 * Interface that allows a developer to implement the three basic types of
 * items you would see in a project
 * \li Folder
 * \li Build Target
 * \li File
 */
class KDEVINTERFACES_EXPORT KDevProjectItem: public QStandardItem
{
public:
  KDevProjectItem(const QString &name, QStandardItem *parent = 0)
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
  void add( KDevProjectItem* item ) { setChild( rowCount(), item ); }

  enum ProjectItemType {
    Folder = QStandardItem::UserType,
    File,
    Target,
    BuildFolder
  };

  // Convenience function to return the current project
  KDevProject* project() const;

  virtual KDevProjectFolderItem *folder() const { return 0; }
  virtual KDevProjectTargetItem *target() const { return 0; }
  virtual KDevProjectFileItem *file() const { return 0; }

  QList<KDevProjectFolderItem*> folderList() const;
  QList<KDevProjectTargetItem*> targetList() const;
  QList<KDevProjectFileItem*> fileList() const;
};

/**
 * Implementation of the KDevProjectItem interface that is specific to a 
 * folder
 */
class KDEVINTERFACES_EXPORT KDevProjectFolderItem: public KDevProjectItem
{
public:
  KDevProjectFolderItem(const KUrl &dir, QStandardItem *parent = 0);

  virtual KDevProjectFolderItem *folder() const
  { return const_cast<KDevProjectFolderItem*>(this); }

  ///Reimplemented from QStandardItem
  virtual int type() const { return KDevProjectItem::Folder; }

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
class KDEVINTERFACES_EXPORT KDevProjectBuildFolderItem: public KDevProjectFolderItem
{
public:
  KDevProjectBuildFolderItem(const KUrl &dir, QStandardItem *parent = 0)
  : KDevProjectFolderItem(dir, parent) {}

  ///Reimplemented from QStandardItem
  virtual int type() const { return KDevProjectItem::BuildFolder; }
  
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
class KDEVINTERFACES_EXPORT KDevProjectTargetItem: public KDevProjectItem
{
public:
  KDevProjectTargetItem(const QString &name, QStandardItem *parent = 0)
    : KDevProjectItem(name, parent) {}

  ///Reimplemented from QStandardItem
  virtual int type() const { return KDevProjectItem::Target; }
  
  virtual KDevProjectTargetItem *target() const
  { return const_cast<KDevProjectTargetItem*>(this); }

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
class KDEVINTERFACES_EXPORT KDevProjectFileItem: public KDevProjectItem
{
public:
  KDevProjectFileItem(const KUrl& file, QStandardItem *parent = 0);

  ///Reimplemented from QStandardItem
  virtual int type() const { return KDevProjectItem::File; }

  virtual KDevProjectFileItem *file() const
  { return const_cast<KDevProjectFileItem*>(this); }

  /** Get the url of this file. */
  const KUrl& url() const;
  void setUrl( const KUrl& );

private:
  KUrl m_url;
};

class KDEVINTERFACES_EXPORT KDevProjectModel: public QStandardItemModel
{
  Q_OBJECT
public:
  KDevProjectModel(QObject *parent = 0);
  virtual ~KDevProjectModel();

  using QStandardItemModel::item;
  KDevProjectItem *item(const QModelIndex &index) const;

};

#endif // KDEVPROJECTMODEL_H
//kate: space-indent on; indent-width 2; indent-mode cstyle; replace-tabs on;
