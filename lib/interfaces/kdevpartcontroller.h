/* This file is part of the KDE project
   Copyright (C) 2002 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
   Copyright (C) 2002 Bernd Gehrmann <bernd@kdevelop.org>
   Copyright (C) 2003 Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2003 Hamish Rodda <rodda@kde.org>
   Copyright (C) 2003 Harald Fernengel <harry@kdevelop.org>
   Copyright (C) 2003 Jens Dagerbo <jens.dagerbo@swipnet.se>

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
#ifndef __KDEV_PARTCONTROLLER_H__
#define __KDEV_PARTCONTROLLER_H__


#include <kurl.h>
#include <kparts/partmanager.h>

namespace KParts
{
  class ReadOnlyPart;
}

enum DocumentState
{
	Clean,
	Modified,
	Dirty,
	DirtyAndModified
};

class KDevPartController : public KParts::PartManager
{
  Q_OBJECT

public:

  KDevPartController(QWidget *parent);

  virtual void setEncoding(const QString &serviceType) = 0;
  virtual void editDocument(const KURL &url, int lineNum=-1, int col=-1) = 0;
  virtual void showDocument(const KURL &url, const QString &context = QString::null) = 0;
  virtual void showPart( KParts::Part* part, const QString& name, const QString& shortDescription ) = 0;
  virtual KParts::Part* findOpenDocument(const KURL& url) = 0;

  virtual void saveAllFiles() = 0;
  virtual void revertAllFiles() = 0;
  /** true if the file has been modified outside KDevelop */
  virtual bool isDirty(KParts::ReadOnlyPart*) = 0;
  virtual bool closePartForWidget( const QWidget* widget ) = 0;
  virtual bool closePartForURL( const KURL & url ) = 0;

signals:

  /**
   * Emitted when a document has been saved.
   */
  void savedFile(const QString &fileName);

  /**
   * Emitted when a document has been loaded.
   */
  void loadedFile(const QString &fileName);

  /**
   * Emitted when a file has been modified outside of KDevelop
   */
  void fileDirty(const QString &fileName);
  void fileDirty( const KURL & url );
  

};



#endif
