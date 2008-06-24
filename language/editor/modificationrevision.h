/*
   Copyright 2008 David Nolden <david.nolden.kdevelop@art-master.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef MODIFICATIONREVISION_H
#define MODIFICATIONREVISION_H

#include <QDateTime>
#include "../languageexport.h"
#include <kdebug.h>

class QString;

namespace KDevelop {

class IndexedString;

/**
 * Pairs together a date and a revision-number, for simpler moving around and comparison. Plus some convenience-functions.
 * Use this to track changes to files, by storing the file-modification time and the editor-revision if applicable(@see KTextEditor::SmartInterface)
 *
 * All member-functions except the IndexedString constructor directly act on the two members, without additional logic.
 *
 * Does not need a d-pointer, is only a container-class.
 * */
struct KDEVPLATFORMLANGUAGE_EXPORT ModificationRevision
{
  ///Constructs a ModificationRevision for the file referenced by the given string, which should have been constructed using KUrl::pathOrUrl at some point
  ///This is efficient, because it uses a cache to look up the modification-revision, caching file-system stats for some time
  static ModificationRevision revisionForFile(const IndexedString& url);
  
  ModificationRevision( const QDateTime& modTime = QDateTime(), int revision_ = 0 );

  bool operator <( const ModificationRevision& rhs ) const;

  bool operator==( const ModificationRevision& rhs ) const;

  bool operator!=( const ModificationRevision& rhs ) const;

  QString toString() const;

  QDateTime modificationTime;  //On-disk modification-time of a document
  int revision;        //SmartInterface revision of a document(0 if the document is not loaded)
};

}

KDEVPLATFORMLANGUAGE_EXPORT kdbgstream& operator<< (kdbgstream& s, const KDevelop::ModificationRevision& rev);

#endif
