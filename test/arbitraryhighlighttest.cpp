/* This file is part of the KDE libraries
   Copyright (C) 2005 Hamish Rodda <rodda@kde.org>

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

#include "arbitraryhighlighttest.h"

#include <ktexteditor/document.h>
#include <ktexteditor/smartinterface.h>
#include <ktexteditor/rangefeedback.h>
#include <ktexteditor/attribute.h>

#include <QTimer>

#include "editmodelbuilder.h"

using namespace KTextEditor;

ArbitraryHighlightTest::ArbitraryHighlightTest(Document* parent)
  : QObject(parent)
  , m_topRange(0L)
{
  //QTimer::singleShot(0, this, SLOT(slotCreateTopRange()));
  connect(parent, SIGNAL(completed()), SLOT(slotCreateTopRange()));
}

ArbitraryHighlightTest::~ArbitraryHighlightTest()
{
}

Document * ArbitraryHighlightTest::doc( ) const
{
  return qobject_cast<Document*>(const_cast<QObject*>(parent()));
}

KTextEditor::SmartInterface * ArbitraryHighlightTest::smart( ) const
{
  return dynamic_cast<SmartInterface*>(doc());
}

/// Cool stuff...
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>

#define MAX_BUFF (10 * 1024)

char *_G_contents;
std::size_t _M_token_begin, _M_token_end;
std::size_t _G_current_offset;
extern QVector<int> _G_newLineLocations;

int yylex();

static void tokenize(cool &m)
{
  // tokenize
  int kind = cool::Token_EOF;
  do
    {
      kind = ::yylex();
      if (!kind)
        kind = cool::Token_EOF;

      cool::token_type &t = m.token_stream->next();
      t.kind = kind;
      t.begin = _M_token_begin;
      t.end = _M_token_end;
      t.text = _G_contents;
    }
  while (kind != cool::Token_EOF);

  m.yylex(); // produce the look ahead token
}

void ArbitraryHighlightTest::slotRangeChanged(SmartRange* range, SmartRange* mostSpecificChild)
{
  //static bool switchEachTime = false;

  kDebug() << k_funcinfo << *range << " mostSpecific " << *mostSpecificChild << endl;

  // Initialise lexer globals
  _M_token_begin = _M_token_end = 0;
  _G_current_offset = 0;
  _G_newLineLocations.clear();
  _G_newLineLocations.append(0);

  // Incremental parser... hmmm
  SmartRange* recoveryPoint = mostSpecificChild;

#if 0
  forever {
    // Only necessary if not every AST node is valid for recovery
    while (recoveryPoint && !m_recoveryPoints.contains(recoveryPoint))
      recoveryPoint = recoveryPoint->parentRange();

    if (!recoveryPoint)
      break;

    // We've decided on a recovery point to try
    kDebug() << "Trying to incrementally parse range " << *recoveryPoint << endl;

    // Nuke current children -- to be replaced
    recoveryPoint->deleteChildRanges();

    // Get text
    QByteArray documentContents = recoveryPoint->document()->text(*recoveryPoint).toLatin1();
    _G_contents = documentContents.data();

    cool::token_stream_type token_stream;
    cool::memory_pool_type memory_pool;

    // 0) setup
    cool parser;
    parser.set_token_stream(&token_stream);
    parser.set_memory_pool(&memory_pool);

    // 1) tokenize
    tokenize(parser);

    cool_ast_node* node = 0L;

    if (parseAST(parser, &node, m_recoveryPoints[recoveryPoint])) {
      EditModelBuilder builder(recoveryPoint, token_stream, m_recoveryPoints, recoveryPoint->start(), true);
      //switchEachTime = !switchEachTime;
      builder.visit_node(node);
      kDebug() << "Succeeded partially parsing " << *recoveryPoint << endl;
      return;
    }

    m_recoveryPoints.remove(recoveryPoint);
    recoveryPoint = recoveryPoint->parentRange();
  }
#endif

  // Nuke current children -- to be replaced
  range->deleteChildRanges();

  // Get text
  QByteArray documentContents = doc()->text().toLatin1();
  _G_contents = documentContents.data();

  cool::token_stream_type token_stream;
  cool::memory_pool_type memory_pool;

  // 0) setup
  cool parser;
  parser.set_token_stream(&token_stream);
  parser.set_memory_pool(&memory_pool);

  // 1) tokenize
  tokenize(parser);

  // 2) parse
  program_ast *ast = 0;
  if (parser.parse_program(&ast)) {
    EditModelBuilder builder(range, token_stream, m_recoveryPoints, Cursor(0,0), true);
    //switchEachTime = !switchEachTime;
    builder.visit_node(ast);

  } else {
    kDebug() << "** ERROR expected a declaration: token position:" << _M_token_begin << endl;
  }

  //outputRange(range, mostSpecificChild);
}

bool ArbitraryHighlightTest::parseAST(cool& parser, cool_ast_node** node, int type)
{
  switch (type) {
    case cool_ast_node::Kind_additive_expression:
      return parser.parse_additive_expression(reinterpret_cast<additive_expression_ast**>(node));

    case cool_ast_node::Kind_block_expression:
      return parser.parse_block_expression(reinterpret_cast<block_expression_ast**>(node));

    case cool_ast_node::Kind_case_condition:
      return parser.parse_case_condition(reinterpret_cast<case_condition_ast**>(node));

    case cool_ast_node::Kind_case_expression:
      return parser.parse_case_expression(reinterpret_cast<case_expression_ast**>(node));

    case cool_ast_node::Kind_class:
      return parser.parse_class(reinterpret_cast<class_ast**>(node));

    case cool_ast_node::Kind_expression:
      return parser.parse_expression(reinterpret_cast<expression_ast**>(node));

    case cool_ast_node::Kind_feature:
      return parser.parse_feature(reinterpret_cast<feature_ast**>(node));

    case cool_ast_node::Kind_formal:
      return parser.parse_formal(reinterpret_cast<formal_ast**>(node));

    case cool_ast_node::Kind_if_expression:
      return parser.parse_if_expression(reinterpret_cast<if_expression_ast**>(node));

    case cool_ast_node::Kind_let_declaration:
      return parser.parse_let_declaration(reinterpret_cast<let_declaration_ast**>(node));

    case cool_ast_node::Kind_let_expression:
      return parser.parse_let_expression(reinterpret_cast<let_expression_ast**>(node));

    case cool_ast_node::Kind_multiplicative_expression:
      return parser.parse_multiplicative_expression(reinterpret_cast<multiplicative_expression_ast**>(node));

    case cool_ast_node::Kind_postfix_expression:
      return parser.parse_postfix_expression(reinterpret_cast<postfix_expression_ast**>(node));

    case cool_ast_node::Kind_primary_expression:
      return parser.parse_primary_expression(reinterpret_cast<primary_expression_ast**>(node));

    case cool_ast_node::Kind_program:
      return parser.parse_program(reinterpret_cast<program_ast**>(node));

    case cool_ast_node::Kind_relational_expression:
      return parser.parse_relational_expression(reinterpret_cast<relational_expression_ast**>(node));

    case cool_ast_node::Kind_unary_expression:
      return parser.parse_unary_expression(reinterpret_cast<unary_expression_ast**>(node));

    case cool_ast_node::Kind_while_expression:
      return parser.parse_while_expression(reinterpret_cast<while_expression_ast**>(node));

    default:
      return false;
  }
}

void ArbitraryHighlightTest::outputRange( KTextEditor::SmartRange * range, KTextEditor::SmartRange * mostSpecific )
{
  kDebug() << (mostSpecific == range ? "==> " : "       ") << QString(range->depth(), ' ') << *range << endl;
  foreach (SmartRange* child, range->childRanges())
    outputRange(child, mostSpecific);
}

void ArbitraryHighlightTest::slotRangeDeleted( KTextEditor::SmartRange * )
{
  m_topRange = 0L;
  QTimer::singleShot(0, this, SLOT(slotCreateTopRange()));
}

void ArbitraryHighlightTest::slotCreateTopRange( )
{
  if (m_topRange) {
    smart()->removeHighlightFromDocument(m_topRange);
    delete m_topRange;
  }

  m_topRange = smart()->newSmartRange(static_cast<Document*>(parent())->documentRange());
  smart()->addHighlightToDocument(m_topRange, true);
  m_topRange->setInsertBehaviour(SmartRange::ExpandRight);
  connect(m_topRange->notifier(), SIGNAL(contentsChanged(KTextEditor::SmartRange*, KTextEditor::SmartRange*)), SLOT(slotRangeChanged(KTextEditor::SmartRange*, KTextEditor::SmartRange*)));
  connect(m_topRange->notifier(), SIGNAL(deleted(KTextEditor::SmartRange*)), SLOT(slotRangeDeleted(KTextEditor::SmartRange*)));

  slotRangeChanged(m_topRange, m_topRange);
}

#include "arbitraryhighlighttest.moc"
