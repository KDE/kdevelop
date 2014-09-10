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

#ifndef KDEVPLATFORM_MODIFICATIONREVISION_H
#define KDEVPLATFORM_MODIFICATIONREVISION_H

#include <QtCore/QDateTime>
#include <kdebug.h>
#include <language/languageexport.h>
#include "../backgroundparser/documentchangetracker.h"

class QString;

namespace KDevelop {

class IndexedString;

KDEVPLATFORMLANGUAGE_EXPORT extern const int cacheModificationTimesForSeconds;

/**
 * Pairs together a date and a revision-number, for simpler moving around and comparison. Plus some convenience-functions.
 * Use this to track changes to files, by storing the file-modification time and the editor-revision if applicable (@see KTextEditor::MovingInterface)
 *
 * All member-functions except the IndexedString constructor directly act on the two members, without additional logic.
 *
 * Does not need a d-pointer, is only a container-class.
 * 
 * It is safe to store this class in the disk-mapped internal duchain data structures.
 * 
 * */
class KDEVPLATFORMLANGUAGE_EXPORT ModificationRevision
{
  public:	
	///Constructs a ModificationRevision for the file referenced by the given string, which should have been constructed using QUrl::pathOrUrl at some point
	///This is efficient, because it uses a cache to look up the modification-revision, caching file-system stats for some time
	static ModificationRevision revisionForFile(const IndexedString& fileName);

	///You can use this when you want to make sure that any cached on-disk modification-time is discarded
    ///and it's re-read from disk on the next access.
    ///Otherwise, the on-disk modification-times are re-used for a specific amount of time
	static void clearModificationCache(const IndexedString& fileName);

	///The default-revision is 0, because that is the kate moving-revision for cleanly opened documents
	ModificationRevision( const QDateTime& modTime = QDateTime(), int revision_ = 0 );

	bool operator <( const ModificationRevision& rhs ) const;

	bool operator==( const ModificationRevision& rhs ) const;

	bool operator!=( const ModificationRevision& rhs ) const;

	QString toString() const;

	uint modificationTime;  //On-disk modification-time of a document in time_t format
    int revision;        //MovingInterface revision of a document
    
private:
  friend class DocumentChangeTracker;
    static void setEditorRevisionForFile(const IndexedString& filename, int revision);
    static void clearEditorRevisionForFile(const IndexedString& filename);
};

}

KDEVPLATFORMLANGUAGE_EXPORT kdbgstream& operator<< (kdbgstream& s, const KDevelop::ModificationRevision& rev);

#endif
