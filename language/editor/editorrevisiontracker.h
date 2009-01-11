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

#include <QtCore/QObject>
#include <KDE/KUrl>
#include <KDE/KTextEditor/Cursor>

#include "../languageexport.h"

namespace KDevelop {

class IDocument;

/**
 * This class allows for convenient tracking of revisions for a list of
 * urls and their associated revisions at the time this object is created.
 *
 * \author Hamish Rodda \<rodda@kde.org\>
 */
class KDEVPLATFORMLANGUAGE_EXPORT EditorRevisionTracker : public QObject
{
  Q_OBJECT

public:
  /// Constructor.
  EditorRevisionTracker(QObject* parent = 0);

  /// Destructor.
  ~EditorRevisionTracker();

  /// Add a URL to be tracked. \param url url to track.
  Q_SCRIPTABLE void addUrl(const KUrl& url);

  /// Add a list of URLs to be tracked. \param list urls to track.
  Q_SCRIPTABLE void addUrls(const KUrl::List& list);

  /// Stop tracking the given \a url. \param url url to remove from the tracking list.
  Q_SCRIPTABLE void removeUrl(const KUrl& url);

  /// Stop tracking the given url \a list. \param list urls to stop tracking.
  Q_SCRIPTABLE void removeUrls(const KUrl::List& list);

  /// Retrieve the list of tracked urls.
  Q_SCRIPTABLE KUrl::List urls() const;

  /**
   * Translate the given \a cursor position to the point in time at which
   * the \a url was added to the tracker.
   *
   * \param url url of the cursor
   * \param cursor position to translate
   * \returns the translated cursor, or an unchanged cursor if translation is not
   *          available.
   */
  Q_SCRIPTABLE KTextEditor::Cursor translateCursor(const KUrl& url, const KTextEditor::Cursor& cursor) const;

  /**
   * Translate the given text \a range to the point in time at which
   * the \a url was added to the tracker.
   *
   * \param url url of the cursor
   * \param range text range to translate
   * \returns the translated range, or an unchanged range if translation is not
   *          available.
   */
  Q_SCRIPTABLE KTextEditor::Range translateRange(const KUrl& url, const KTextEditor::Range& range) const;

  /**
   * Open the given \a url at the given \a cursor position,
   * translated to the point in time at which the \a url was added
   * to the tracker.
   *
   * \param url url to open or activate
   * \param cursor position to navigate to (after translation, if applicable)
   */
  Q_SCRIPTABLE void openUrl(const KUrl& url, const KTextEditor::Cursor& cursor) const;

private Q_SLOTS:
  void documentLoaded(KTextEditor::Document* document);
  void documentClosed(KTextEditor::Document* document);
  void documentUrlChanged(KDevelop::IDocument* document);

private:
  class EditorRevisionTrackerPrivate* const d;
};

}

#endif // EDITORREVISIONTRACKER_H
