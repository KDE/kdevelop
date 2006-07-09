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

#include <ktexteditor/range.h>

#include "lexer.h"

class AST;

namespace KTextEditor { class SmartRange; class SmartCursor; class SmartInterface; }

/**
 * Provides facilities for easy integration of a text editor component with
 * the information parsed from a source file.
 *
 * Uses a disguised singleton + stateful design.
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
   * Returns the currently active document for operations.
   */
  KTextEditor::Document* activeDocument() const;

  /**
   * Sets a document as the currently active document for operations.
   */
  void setActiveDocument(KTextEditor::Document* document);

  KTextEditor::SmartInterface* smart() const;

  enum TopRangeType {
    Highlighting,
    DefinitionUseChain
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
  KTextEditor::SmartRange* topRange(TopRangeType type);

  enum Edge {
    FrontEdge,
    BackEdge
  };

  /**
   * Finds the location and \a file where the given \a token was parsed from.  This function
   * does not change any of the EditorIntegrator's state.
   *
   * \param token token to locate
   * \param fileName provide a QString to this parameter to return the file which the token was found in.
   * \param end set to false to return the start position of the token, true to return the end position.
   *
   * \returns the requested cursor relating to the start or end of the given token.
   */
  KTextEditor::Cursor findPosition(const Token& token, Edge edge = BackEdge, QString* file = 0) const;

  /**
   * Finds the location and \a file where the given \a token was parsed from.  This function
   * does not change any of the EditorIntegrator's state.
   *
   * \param token token to locate
   * \param fileName provide a QString to this parameter to return the file which the token was found in.
   * \param end set to false to return the start position of the token, true to return the end position.
   *
   * \returns the requested cursor relating to the start or end of the given token.
   */
  KTextEditor::Cursor findPosition(std::size_t token, Edge edge = BackEdge, QString* file = 0) const;

  /**
   * Create a new smart cursor from the given \a position.
   */
  KTextEditor::SmartCursor* createCursor(const KTextEditor::Cursor& position);

  /**
   * Create a new smart cursor from the given \a token on the given \a edge.
   */
  KTextEditor::SmartCursor* createCursor(std::size_t token, Edge edge);

  // Set defaults for creation of ranges
  void setNewRange(const KTextEditor::Range& range);
  void setNewStart(const KTextEditor::Cursor& position);
  void setNewEnd(const KTextEditor::Cursor& position);

  /**
   * Create a smart range over \a range with the given \a parent.
   * The returned range will become the new currentRange().
   *
   * \param range Range of text to cover.  If this is outside the parent's range, the
   * parent will be adjusted (standard behaviour of SmartRange%s).
   * \returns the newly created smart range.
   */
  KTextEditor::SmartRange* createRange(const KTextEditor::Range& range);

  /**
   * Create a smart range over \a range with the given \a parent.
   * The returned range will become the new currentRange().
   *
   * \param range Range of text to cover.  If this is outside the parent's range, the
   * parent will be adjusted (standard behaviour of SmartRange%s).
   * \param parent Parent range to the range to be created.  If none is set, the current range will
   * be used as the parent.
   * \returns the newly created smart range.
   * \overload
   */
  KTextEditor::SmartRange* createRange(const KTextEditor::Cursor& start, const KTextEditor::Cursor& end);

  /**
   * Create a smart range over the marked range with the given \a parent.
   * The returned range will become the new currentRange().
   *
   * \param parent Parent range to the range to be created.  If none is set, the current range will
   * be used as the parent.
   * \returns the newly created smart range.
   * \overload
   */
  KTextEditor::SmartRange* createRange();

  enum RangeEdge {
    InnerEdge,
    OuterEdge
  };

  /**
   * Create a new start range encompassing the given AST \a node.
   * The returned range will become the new currentRange().
   *
   * \returns the newly created smart range.
   * \overload
   */
  KTextEditor::SmartRange* createRange(AST* node, RangeEdge = OuterEdge);

  /**
   * Create a new start range encompassing the given AST \a token.
   * The returned range will become the new currentRange().
   *
   * \returns the newly created smart range.
   * \overload
   */
  KTextEditor::SmartRange* createRange(const Token& token);

  /**
   * Create a new start range encompassing the given AST \a token.
   * The returned range will become the new currentRange().
   *
   * \returns the newly created smart range.
   * \overload
   */
  KTextEditor::SmartRange* createRange(std::size_t token);

  /**
   * Returns the current smart text range.
   */
  KTextEditor::SmartRange* currentRange() const;

  /**
   * Sets the current range to \a range.
   */
  void setCurrentRange(KTextEditor::SmartRange* range);

public slots:
  static void removeTextSource(KTextEditor::Document* document);

private:
  static QHash<TokenStream*, Lexer*> s_parsedSources;
  static QList<KTextEditor::Document*> s_documents;
  static QHash<KTextEditor::Document*, QVector<KTextEditor::SmartRange*> > s_topRanges;

  Lexer* m_lexer;
  TokenStream* m_tokenStream;
  KTextEditor::Document* m_activeDocument;
  KTextEditor::SmartRange* m_currentRange;
  KTextEditor::Range m_newRangeMarker;
};

#endif // EDITORINTEGRATOR_H

// kate: indent-width 2;
