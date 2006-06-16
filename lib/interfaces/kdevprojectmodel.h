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

#include "kdevitemmodel.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>

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
class KDEVINTERFACES_EXPORT KDevProjectItem: public KDevItemCollection
{
public:
  KDevProjectItem(const QString &name, KDevItemGroup *parent = 0)
    : KDevItemCollection(name, parent) {}

  // Convenience function to return the current project
  KDevProject* project() const;

  virtual KDevProjectItem *itemAt(int index) const;

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
  KDevProjectFolderItem(const KUrl &dir, KDevItemGroup *parent = 0);

  virtual KDevProjectFolderItem *folder() const
  { return const_cast<KDevProjectFolderItem*>(this); }

  /** Get the url of this folder */
  const KUrl& url() const;
  virtual QIcon icon() const;

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
  KDevProjectBuildFolderItem(const KUrl &dir, KDevItemGroup *parent = 0);

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
  KUrl m_url;
};

/**
 * Object which represents a target in a build system.
 *
 * This object contains all properties specific to a target.
 */
class KDEVINTERFACES_EXPORT KDevProjectTargetItem: public KDevProjectItem
{
public:
  KDevProjectTargetItem(const QString &name, KDevItemGroup *parent = 0)
    : KDevProjectItem(name, parent) {}

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
  KDevProjectFileItem(const KUrl& file, KDevItemGroup *parent = 0);

  virtual KDevProjectFileItem *file() const
  { return const_cast<KDevProjectFileItem*>(this); }

  /** Get the url of this file. */
  const KUrl& url() const;
  void setUrl( const KUrl& );
  virtual QIcon icon() const;


private:
  KUrl m_url;
};

class KDEVINTERFACES_EXPORT KDevProjectModel: public KDevItemModel
{
  Q_OBJECT
public:
  KDevProjectModel(QObject *parent = 0);
  virtual ~KDevProjectModel();

  virtual KDevProjectItem *item(const QModelIndex &index) const;
};

#endif // KDEVPROJECTMODEL_H
//kate: space-indent on; indent-width 2; indent-mode cstyle; replace-tabs on;
