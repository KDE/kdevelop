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

#ifndef CPPEDITORINTEGRATOR_H
#define CPPEDITORINTEGRATOR_H

#include <language/editor/rangeinrevision.h>
#include "cppduchainexport.h"
#include "lexer.h"

class AST;

/**
 * Provides facilities for easy integration of a text editor component with
 * the information parsed from a source file.
 *
 * Uses a disguised singleton + stateful design.
 *
 * \todo introduce stacks for the state?
 */
class KDEVCPPDUCHAIN_EXPORT CppEditorIntegrator
{
public:
  CppEditorIntegrator(ParseSession* session);

  ParseSession* parseSession() const;

  enum Edge {
    FrontEdge,
    BackEdge
  };
  
  enum RangeEdge {
    InnerEdge,
    OuterEdge
  };
  
  /**
   * Finds the location and \a file where the given \a token was parsed from.  This function
   * does not change any of the CppEditorIntegrator's state.
   *
   * \param token token to locate
   * \param edge set to FrontEdge to return the start position of the token, BackEdge to return the end position.
   *
   * \returns the requested cursor relating to the start or end of the given token.
   */
  KDevelop::CursorInRevision findPosition(const Token& token, Edge edge = BackEdge) const;

  /**
   * Finds the location and \a file where the given \a token was parsed from.
   * This function does not change any of the CppEditorIntegrator's state.
   *
   * \param token token to locate
   * \param edge set to FrontEdge to return the start position of the token, BackEdge to return the end position.
   *
   * \returns the requested cursor relating to the start or end of the given token.
   */
  KDevelop::CursorInRevision findPosition(std::size_t token, Edge edge = BackEdge) const;

  /**
   * Create a range encompassing the given AST \a node.
   * This function does not change any of the CppEditorIntegrator's state.
   *
   * \overload
   */
  KDevelop::RangeInRevision findRange(AST* node, RangeEdge = OuterEdge);

  /**
   * Create a range encompassing the given AST \a nodes.
   * This function does not change any of the CppEditorIntegrator's state.
   *
   * \overload
   */
  KDevelop::RangeInRevision findRange(AST* from, AST* to);

  /**
   * Create a range encompassing the given AST \a token.
   * This function does not change any of the CppEditorIntegrator's state.
   *
   * \overload
   */
  KDevelop::RangeInRevision findRange(const Token& token);

  /**
   * Create a range encompassing the given AST \a token.
   * This function does not change any of the CppEditorIntegrator's state.
   *
   * \overload
   */
  KDevelop::RangeInRevision findRange(std::size_t token);

  /**
   * Create a range encompassing the given AST \a tokens.
   * This function does not change any of the CppEditorIntegrator's state.
   *
   * \overload
   */
  KDevelop::RangeInRevision findRange(std::size_t start_token, std::size_t end_token);

  /**
   * Create a range encompassing the given range \a tokens.
   * 
   * The difference to the common functions, this one checks whether the given range has been expanded within
   * one macro-expansion, and if it has, returns a collapsed range at the position from where the expansion was started.
   * \overload
   */
  KDevelop::RangeInRevision findRangeForContext(std::size_t start_token, std::size_t end_token);
  
  /**
   * Retrieve the string represented by a token.
   */
  QString tokenToString(std::size_t token) const;

  /**
   * Retrieve the string represented by a range of tokens.
   */
  QString tokensToString(std::size_t start, std::size_t end) const;

private:
  ParseSession* m_session;
};

#endif // EDITORINTEGRATOR_H

