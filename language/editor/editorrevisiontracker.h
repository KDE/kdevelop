/* This file is part of KDevelop
    Copyright 2008 Hamish Rodda <rodda@kde.org>

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

#ifndef EDITORREVISIONTRACKER_H
#define EDITORREVISIONTRACKER_H

#include <KUrl>
#include <KTextEditor/Cursor>

#include "language/languageexport.h"

namespace KDevelop {

/**
 * This class allows for convenient tracking of revisions for a list of
 * urls and their associated revisions at the time this object is created.
 *
 * \author Hamish Rodda \<rodda@kde.org\>
 */
class KDEVPLATFORMLANGUAGE_EXPORT EditorRevisionTracker
{
public:
  /// Constructor.
  EditorRevisionTracker();

  /// Destructor.
  ~EditorRevisionTracker();

  void addUrl(const KUrl& url);
  void addUrls(const KUrl::List& list);

  void removeUrl(const KUrl& url);
  void removeUrls(const KUrl::List& list);

  /**
   * Open the given \a url at the given \a cursor position,
   * translated to the point in time at which the \a url was added
   * to the tracker.
   *
   * \param url url to open or activate
   * \param cursor position to navigate to (after translation, if applicable)
   */
  void openUrl(const KUrl& url, const KTextEditor::Cursor& cursor);

private:
  class EditorRevisionTrackerPrivate* const d;
};

}

#endif // EDITORREVISIONTRACKER_H
