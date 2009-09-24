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
#include <QtCore/QMutexLocker>

#include <KDE/KUrl>
#include <KDE/KTextEditor/Range>
#include <KDE/KTextEditor/SmartRange>

#include "documentcursor.h"
#include "simplerange.h"

class QMutex;

namespace KTextEditor { class SmartRange; class SmartCursor; class SmartInterface; }

namespace KDevelop
{
  class DocumentRange;
  class EditorIntegratorStatic;
  class HashedString;
  class IndexedString;

/**
 * Class to hold a reference to a locked KTextEditor::SmartInterface.
 * Implicitly shared.  To get an instance of this class, see EditorIntegrator::smart()
 *
 * This class is re-entrant, but not thread safe.
 */
class KDEVPLATFORMLANGUAGE_EXPORT LockedSmartInterface
{
  public:
    LockedSmartInterface(const LockedSmartInterface& lock);
    ~LockedSmartInterface();

    /// Unlock the smart interface when you've finished using it
    /// This happens automatically on deletion, so you only need to call if
    /// the object is not going to be deleted.
    void unlock() const;

    /**
     * Return the current text editor document, based on the current URL.
     */
    KTextEditor::Document* currentDocument() const;

    KTextEditor::SmartInterface* operator->() const;
    operator bool() const;

  private:
    friend class EditorIntegrator;
    LockedSmartInterface(KTextEditor::SmartInterface* iface = 0, KTextEditor::Document* doc = 0);

    class LockedSmartInterfacePrivate* const d;
};

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
class KDEVPLATFORMLANGUAGE_EXPORT EditorIntegrator
{
  friend class EditorIntegratorStatic;

public:
  EditorIntegrator();
  virtual ~EditorIntegrator();

  ///A hack working around problems while initialization, @see DUChain::documentLoadedPrepare. It used to happen that the document wasn't registered
  ///to the editor-integrator there, and thus no smart-ranges/highlighting were created. We use this to inject the document.
  void insertLoadedDocument(KTextEditor::Document* document);

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
   * Returns the text document for \a url, if one exists. The url should have been formatted using KUrl::pathOrUrl() at some point.
   */
  static KTextEditor::Document* documentForUrl(const HashedString& url);

  /**
   * Returns the text document for \a url, if one exists. The url should have been formatted using KUrl::pathOrUrl() at some point.
   * This shouuld be preferred over the method above
   */
  static KTextEditor::Document* documentForUrl(const IndexedString& url);

  /**
   * Determine if a document has been loaded yet
   */
  static bool documentLoaded(KTextEditor::Document* document);

  /// Returns the url of the currently associated editor
  IndexedString currentUrl() const;

  /// Associate this editor integrator with the editor which is currently editing the given \a url
  /// @param useSmart Whether smart-ranges should be created for this url.
  void setCurrentUrl(const IndexedString& url, bool useSmart = true);

  /// Retrieve a locked version of the SmartInterface for the current document, or null if one does not exist / is being deleted etc.
  LockedSmartInterface smart() const;

  /// Retrieve a locked version of the SmartInterface for the current document, or null if one does not exist / is being deleted etc.
  static LockedSmartInterface smart(const KUrl& url);

  /// Top range type enumeration
  enum TopRangeType {
    Highlighting /**< top range type for highlighting */,
    DefinitionUseChain /**< top range type for duchain */,
    TopRangeCount /**< top range type for counting */
  };

  /**
   * Returns a toplevel range in a document for use as \a type.
   *
   * \param type The use for which the created range will be used
   * \returns the range either found or created, if the document was valid and supports
   *          smart ranges. Zero if no smart interface is available for the document.
   *
   * Opens a range that needs to be closed using exitCurrentRange()
   */
  KTextEditor::SmartRange* topRange(const LockedSmartInterface& iface, TopRangeType type);

  /**
   * Releases a toplevel \a range.  The range should be deleted by the caller.
   */
  static void releaseTopRange(KTextEditor::SmartRange* range);

  /**
   * Releases + safely deletes a text editor range.
   *
   * \warning you must already hold the smart lock for the corresponding document.
   * \todo audit uses to check for smart lock holding
   */
  static void releaseRange(KTextEditor::SmartRange* range);

  enum Edge {
    FrontEdge /**< the front edge of a token */,
    BackEdge /**< the back edge of a token */
  };

  /**
   * Create a new persistant cursor from the given \a position.
   */
  KTextEditor::SmartCursor* createCursor(const LockedSmartInterface& iface, const KTextEditor::Cursor& position);

  /**
   * Create a new persistant cursor from the given \a token on the given \a edge.
   */
  KTextEditor::SmartCursor* createCursor(const LockedSmartInterface& iface, std::size_t token, Edge edge);

  /**
   * Apply a possibly dated range to the current smart cursor.  Performs translation on \a fromRange,
   * then applies it to smartRange.
   */
  void adjustRangeTo(const LockedSmartInterface& iface, const SimpleRange& fromRange);

  /**
   * Translate the given \a range to the current smart revision, and return the result.
   * You need to have the smart mutex locked for the result to remain valid while you process it.
   */
  SimpleRange translate(const LockedSmartInterface& iface, const SimpleRange& fromRange) const;

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
   * \returns the newly created text range, or zero if no smart interface is available for the document.
   */
  KTextEditor::SmartRange* createRange(const LockedSmartInterface& iface, const KTextEditor::Range& range, KTextEditor::SmartRange::InsertBehaviors insertBehavior = KTextEditor::SmartRange::DoNotExpand);

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
   * \returns the newly created text range, or zero if no smart interface is available for the document.
   * \overload
   */
  KTextEditor::SmartRange* createRange(const LockedSmartInterface& iface, const KTextEditor::Cursor& start, const KTextEditor::Cursor& end);

  enum RangeEdge {
    InnerEdge /**< the inner edge of a range */,
    OuterEdge /**< the outer edge of a range */
  };

  /**
   * Returns the most current text range.
   */
  KTextEditor::SmartRange* currentRange(const LockedSmartInterface& iface) const;

  /**
   * Sets the current range to \a range. It is put upon the range-stack.
   * Does nothing if the range is zero.
   */
  void setCurrentRange(const LockedSmartInterface& iface, KTextEditor::SmartRange* range);

  /**
   * Count of ranges currently on the stack.
   */
  int rangeStackSize(const LockedSmartInterface& iface) const;

  /**
   * Sets the previous range on the stack to be the new current range.
   * Does nothing if the range stack is empty.
   */
  void exitCurrentRange(const LockedSmartInterface& iface);

  /**
   * Use this to connect to notifications provided by EditorIntegratorStatic.
   */
  static QObject* notifier();

private:
  /// Remove a current associated document, eg. if the document gets closed
  /// Only to be used by EditorIntegratorStatic
  void clearCurrentDocument();

  static EditorIntegratorStatic *data();
  class EditorIntegratorPrivate* const d;

};

}
#endif // EDITORINTEGRATOR_H

