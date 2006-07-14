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

#ifndef EDITORINTEGRATOR_H
#define EDITORINTEGRATOR_H

#include <QObject>

#include <kurl.h>

#include <ktexteditor/range.h>

#include "lexer.h"
#include "documentcursor.h"

class AST;
class DocumentRange;
class DocumentCursor;

namespace KTextEditor { class SmartRange; class SmartCursor; class SmartInterface; }

/**
 * Provides facilities for easy integration of a text editor component with
 * the information parsed from a source file.
 *
 * Uses a disguised singleton + stateful design.
 *
 * \todo introduce stacks for the state?
 */
class EditorIntegrator : public QObject
{
  Q_OBJECT

public:
  EditorIntegrator(TokenStream* tokenStream);

  static void addParsedSource(Lexer* lexer, TokenStream* tokenStream);

  /**
   * Adds a text editor \a document to the integrator.  If it is deleted, it will automatically
   * be removed.
   */
  static void addDocument(KTextEditor::Document* document);

  /**
   * Returns the text document for \a url, if one exists.
   */
  static KTextEditor::Document* findDocument(const KUrl& url);

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
   * Deletes a toplevel \a range.
   */
  static void deleteTopRange(KTextEditor::Range* range);

  enum Edge {
    FrontEdge,
    BackEdge
  };

  /**
   * Finds the location and \a file where the given \a token was parsed from.  This function
   * does not change any of the EditorIntegrator's state.
   *
   * \param token token to locate
   * \param edge set to FrontEdge to return the start position of the token, BackEdge to return the end position.
   *
   * \returns the requested cursor relating to the start or end of the given token.
   */
  DocumentCursor findPosition(const Token& token, Edge edge = BackEdge) const;

  /**
   * Finds the location and \a file where the given \a token was parsed from.  This function
   * does not change any of the EditorIntegrator's state.
   *
   * \param token token to locate
   * \param edge set to FrontEdge to return the start position of the token, BackEdge to return the end position.
   *
   * \returns the requested cursor relating to the start or end of the given token.
   */
  DocumentCursor findPosition(std::size_t token, Edge edge = BackEdge) const;

  /**
   * Create a new persistant cursor from the given \a position.
   */
  KTextEditor::Cursor* createCursor(const DocumentCursor& position);

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
   * this will be a smart range, otherwise it will be a DocumentRange.
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
  KTextEditor::Range* createRange(const DocumentCursor& start, const DocumentCursor& end);

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
   * Create a new text range encompassing the given AST \a node.
   * The returned range will become the new currentRange().
   *
   * If the current document is loaded, and it supports creating smart ranges,
   * this will be a smart range, otherwise it will be a DocumentRange.
   *
   * \returns the newly created smart range.
   * \overload
   */
  KTextEditor::Range* createRange(AST* node, RangeEdge = OuterEdge);

  /**
   * Create a new start range encompassing the given AST \a token.
   * The returned range will become the new currentRange().
   *
   * If the current document is loaded, and it supports creating smart ranges,
   * this will be a smart range, otherwise it will be a DocumentRange.
   *
   * \returns the newly created smart range.
   * \overload
   */
  KTextEditor::Range* createRange(const Token& token);

  /**
   * Create a new start range encompassing the given AST \a token.
   * The returned range will become the new currentRange().
   *
   * If the current document is loaded, and it supports creating smart ranges,
   * this will be a smart range, otherwise it will be a DocumentRange.
   *
   * \returns the newly created smart range.
   * \overload
   */
  KTextEditor::Range* createRange(std::size_t token);

  /**
   * Returns the current text range.
   */
  KTextEditor::Range* currentRange() const;

  /**
   * Sets the current range to \a range.
   */
  void setCurrentRange(KTextEditor::Range* range);

public slots:
  static void removeTextSource(KTextEditor::Document* document);

private:
  static QHash<TokenStream*, Lexer*> s_parsedSources;
  static QHash<KUrl, KTextEditor::Document*> s_documents;
  static QHash<KUrl, QVector<KTextEditor::Range*> > s_topRanges;

  Lexer* m_lexer;
  TokenStream* m_tokenStream;
  KUrl m_currentUrl;
  KTextEditor::Range* m_currentRange;
  KTextEditor::Range m_newRangeMarker;
};

#endif // EDITORINTEGRATOR_H

// kate: indent-width 2;
