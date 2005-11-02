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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "arbitraryhighlighttest.h"

#include <ktexteditor/document.h>
#include <ktexteditor/smartinterface.h>
#include <ktexteditor/rangefeedback.h>
#include <ktexteditor/attribute.h>

#include <QTimer>

#include "cool.h"
#include "editmodelbuilder.h"

using namespace KTextEditor;

ArbitraryHighlightTest::ArbitraryHighlightTest(Document* parent)
  : QObject(parent)
  , m_topRange(0L)
{
  QTimer::singleShot(0, this, SLOT(slotCreateTopRange()));
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
  /*SmartRange* currentRange = mostSpecificChild;
  currentRange->deleteChildRanges();

  text = currentRange->document()->textLines(currentRange);*/

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

  // Initialise lexer globals
  _M_token_begin = _M_token_end = 0;
  _G_current_offset = 0;
  _G_newLineLocations.clear();
  _G_newLineLocations.append(0);

  // 1) tokenize
  tokenize(parser);

  // 2) parse
  program_ast *ast = 0;
  if (parser.parse_program(&ast)) {
    EditModelBuilder builder(range, token_stream);
    builder.visit_node(ast);

  } else {
    kdDebug() << "** ERROR expected a declaration: token position:" << _M_token_begin << endl;
  }

  //outputRange(range, mostSpecificChild);
}

void ArbitraryHighlightTest::outputRange( KTextEditor::SmartRange * range, KTextEditor::SmartRange * mostSpecific )
{
  kdDebug() << (mostSpecific == range ? "==> " : "       ") << QString(range->depth(), ' ') << *range << endl;
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
  m_topRange = smart()->newSmartRange(static_cast<Document*>(parent())->documentRange());
  smart()->addHighlightToDocument(m_topRange, true);
  m_topRange->setInsertBehaviour(SmartRange::ExpandRight);
  connect(m_topRange->notifier(), SIGNAL(contentsChanged(KTextEditor::SmartRange*, KTextEditor::SmartRange*)), SLOT(slotRangeChanged(KTextEditor::SmartRange*, KTextEditor::SmartRange*)));
  connect(m_topRange->notifier(), SIGNAL(deleted(KTextEditor::SmartRange*)), SLOT(slotRangeDeleted(KTextEditor::SmartRange*)));

  slotRangeChanged(m_topRange, m_topRange);
}

#include "arbitraryhighlighttest.moc"
