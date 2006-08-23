/* This file is part of KDevelop
    Copyright (C) 2006 Hamish Rodda <rodda@kde.org>

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

#ifndef KDEVEDITORINTEGRATOR_H
#define KDEVEDITORINTEGRATOR_H

#include <kurl.h>

#include <ktexteditor/range.h>

#include "kdevdocumentcursor.h"

class QMutex;

class KDevDocumentRange;
class KDevDocumentCursor;
class KDevEditorIntegratorPrivate;

namespace KTextEditor { class SmartRange; class SmartCursor; class SmartInterface; }

/**
 * Provides facilities for easy integration of a text editor component with
 * the information parsed from a source file.
 *
 * Uses a disguised singleton + stateful design.
 *
 * \todo introduce stacks for the state?
 *
 * \todo non-loaded documents don't trigger the removeDocument call...
 */
class KDEVINTERFACES_EXPORT KDevEditorIntegrator
{
public:
  KDevEditorIntegrator();

  /**
   * Adds a text editor \a document to the integrator.
   */
  static void addDocument(KTextEditor::Document* document);

  /**
   * Retrieve the mutex which allows background parsers to lock a document
   * in place while they make their changes.
   *
  static QMutex* mutexForDocument(KTextEditor::Document* document);*/

  /**
   * Removes the text editor \a document from the integrator.
   */
  static void removeDocument(KTextEditor::Document* document);

  /**
   * Returns the text document for \a url, if one exists.
   */
  static KTextEditor::Document* documentForUrl(const KUrl& url);

  /**
   * Determine if a document has been loaded yet
   */
  static bool documentLoaded(KTextEditor::Document* document);

  const KUrl& currentUrl() const;
  void setCurrentUrl(const KUrl& currentUrl);

  /**
   * Return the current text editor document, based on the current URL.
   */
  KTextEditor::Document* currentDocument() const;

  /// Convenience function to return the SmartInterface for the current document.
  KTextEditor::SmartInterface* smart() const;

  enum TopRangeType {
    Highlighting,
    DefinitionUseChain,

    TopRangeCount
  };

  /**
   * Returns a toplevel range in \a document for use as \a type.
   *
   * \param type The use for which the created range will be used
   * \param document The text editor document to create in.  If one is not
   *                 defined, the currently active document will be used
   *                 instead.
   * \returns the range either found or created, if the document was valid and supports
   *          smart ranges.
   */
  KTextEditor::Range* topRange(TopRangeType type);

  /**
   * Releases a toplevel \a range.  The range should be deleted by the caller.
   */
  static void releaseTopRange(KTextEditor::Range* range);

  enum Edge {
    FrontEdge,
    BackEdge
  };

  /**
   * Create a new persistant cursor from the given \a position.
   */
  KTextEditor::Cursor* createCursor(const KTextEditor::Cursor& position);

  /**
   * Create a new persistant cursor from the given \a token on the given \a edge.
   */
  KTextEditor::Cursor* createCursor(std::size_t token, Edge edge);

  // Set defaults for creation of ranges
  void setNewRange(const KTextEditor::Range& range);
  void setNewStart(const KTextEditor::Cursor& position);
  void setNewEnd(const KTextEditor::Cursor& position);

  /**
   * Create a text range over \a range as a child range of the current range.
   * The returned range will become the new currentRange().
   *
   * If the current document is loaded, and it supports creating smart ranges,
   * this will be a smart range, otherwise it will be a KDevDocumentRange.
   *
   * \param range Range of text to cover.  If this is outside the parent's range, the
   * parent will be adjusted (standard behaviour of SmartRange%s).
   *
   * \returns the newly created text range.
   */
  KTextEditor::Range* createRange(const KTextEditor::Range& range);

  /**
   * Create a text range from \a start to \a end as a child range of the current range.
   * The returned range will become the new currentRange().
   *
   * If the current document is loaded, and it supports creating smart ranges,
   * this will be a smart range, otherwise it will be a KDevDocumentRange.
   *
   * \param start start of the range of text to cover.  If this is outside the parent's range, the
   * parent will be adjusted (standard behaviour of SmartRange%s).
   * \param end end of the range of text to cover.  If this is outside the parent's range, the
   * parent will be adjusted (standard behaviour of SmartRange%s).
   *
   * \returns the newly created text range.
   * \overload
   */
  KTextEditor::Range* createRange(const KDevDocumentCursor& start, const KDevDocumentCursor& end);

  /**
   * Create a text range over the marked range as a child range of the current range.
   * The returned range will become the new currentRange().
   *
   * If the current document is loaded, and it supports creating smart ranges,
   * this will be a smart range, otherwise it will be a KDevDocumentRange.
   *
   * \returns the newly created smart range.
   * \overload
   */
  KTextEditor::Range* createRange();

  enum RangeEdge {
    InnerEdge,
    OuterEdge
  };

  /**
   * Returns the current text range.
   */
  KTextEditor::Range* currentRange() const;

  /**
   * Sets the current range to \a range.
   */
  void setCurrentRange(KTextEditor::Range* range);

  /**
   * Sets the parent range to be the new current range.
   */
  void exitCurrentRange();

protected:
  static KDevEditorIntegratorPrivate* data();

  static KDevEditorIntegratorPrivate* s_data;

  KUrl m_currentUrl;
  KTextEditor::Document* m_currentDocument;
  KTextEditor::SmartInterface* m_smart;
  KTextEditor::Range* m_currentRange;
  KTextEditor::Range m_newRangeMarker;
};

#endif // EDITORINTEGRATOR_H

// kate: indent-width 2;
