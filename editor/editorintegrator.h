/* This file is part of KDevelop
    Copyright 2006 Hamish Rodda <rodda@kde.org>

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

#include <QtCore/QDateTime>
#include <kurl.h>

#include <ktexteditor/range.h>
#include <ktexteditor/smartrange.h>

#include <documentcursor.h>


class QMutex;

namespace KTextEditor { class SmartRange; class SmartCursor; class SmartInterface; }

namespace KDevelop
{
  class DocumentRange;
  class EditorIntegratorStatic;

/**
 * Pairs together a date and a revision-number, for simpler moving around and comparison. Plus some convenience-functions.
 * Use this to track changes to files, by storing the file-modification time and the editor-revision if applicable(@see KTextEditor::SmartInterface)
 *
 * All member-functions directly act on the two members, without additional logic.
 * 
 * Does not need a d-pointer, is only a container-class.
 * */
struct KDEVPLATFORMEDITOR_EXPORT ModificationRevision
{
  ModificationRevision( const QDateTime& modTime = QDateTime(), int revision_ = 0 );

  bool operator <( const ModificationRevision& rhs ) const;

  bool operator==( const ModificationRevision& rhs ) const;
  
  bool operator!=( const ModificationRevision& rhs ) const;

  QString toString() const;

  QDateTime modificationTime;  //On-disk modification-time of a document
  int revision;        //SmartInterface revision of a document(0 if the document is not loaded)
};

KDEVPLATFORMEDITOR_EXPORT kdbgstream& operator<< (kdbgstream& s, const ModificationRevision& rev);

/**
 * Provides facilities for easy integration of a text editor component with
 * the information parsed from a source file.
 *
 * Uses a disguised singleton + stateful design.
 *
 * \todo introduce stacks for the state?
 * \todo move private members to a private class
 * \todo non-loaded documents don't trigger the removeDocument call...
 */
class KDEVPLATFORMEDITOR_EXPORT EditorIntegrator
{
public:
  EditorIntegrator();
  virtual ~EditorIntegrator();

  /**
   * Initialise the editor integrator.
   * Only needs to be called once from the main thread.
   */
  static void initialise();

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

   KUrl currentUrl() const;
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
   * Returns a toplevel range in a document for use as \a type.
   *
   * \param type The use for which the created range will be used
   * \returns the range either found or created, if the document was valid and supports
   *          smart ranges.
   */
  KTextEditor::Range* topRange(TopRangeType type);

  /**
   * Releases a toplevel \a range.  The range should be deleted by the caller.
   */
  static void releaseTopRange(KTextEditor::Range* range);

  /**
   * Releases + safely deletes a text editor range.
   */
  static void releaseRange(KTextEditor::Range* range);

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
   * The returned range will become the new currentRange(), and will be put upon the range-stack.
   *
   * If the current document is loaded, and it supports creating smart ranges,
   * this will be a smart range, otherwise it will be a DocumentRange.
   *
   * \param range Range of text to cover.  If this is outside the parent's range, the
   * parent will be adjusted (standard behaviour of SmartRange%s).
   * \param insertBehavior: If a smart-range will be created, it will be created with the given insert-behavior
   *
   * \returns the newly created text range.
   */
  KTextEditor::Range* createRange(const KTextEditor::Range& range, KTextEditor::SmartRange::InsertBehaviors insertBehavior = KTextEditor::SmartRange::DoNotExpand);

  /**
   * Create a text range from \a start to \a end as a child range of the current range.
   * The returned range will become the new currentRange().
   *
   * If the current document is loaded, and it supports creating smart ranges,
   * this will be a smart range, otherwise it will be a DocumentRange.
   *
   * \param start start of the range of text to cover.  If this is outside the parent's range, the
   * parent will be adjusted (standard behaviour of SmartRange%s).
   * \param end end of the range of text to cover.  If this is outside the parent's range, the
   * parent will be adjusted (standard behaviour of SmartRange%s).
   *
   * \returns the newly created text range.
   * \overload
   */
  KTextEditor::Range* createRange(const KTextEditor::Cursor& start, const KTextEditor::Cursor& end);

  /**
   * Create a text range over the marked range as a child range of the current range.
   * The returned range will become the new currentRange().
   *
   * If the current document is loaded, and it supports creating smart ranges,
   * this will be a smart range, otherwise it will be a DocumentRange.
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
   * Returns the most current text range.
   */
  KTextEditor::Range* currentRange() const;

  /**
   * Sets the current range to \a range. It is put upon the range-stack.
   */
  void setCurrentRange(KTextEditor::Range* range);

  /**
   * Sets the previous range on the stack to be the new current range.
   */
  void exitCurrentRange();

  /**
   * Returns the modification-revision that contains the file-modification time,
   * and if the document is loaded, the revision-number of it's content.
   * */
  static ModificationRevision modificationRevision(const KUrl& url);
private:
  static KDevelop::EditorIntegratorStatic *data();
  class EditorIntegratorPrivate* const d;

};

}
#endif // EDITORINTEGRATOR_H

// kate: space-indent on; indent-width 4; tab-width: 4; replace-tabs on; auto-insert-doxygen on
