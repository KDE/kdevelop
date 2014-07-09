/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
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

#include "dumpchain.h"

#include <QtCore/QString>
#include <QTextStream>

#include <kdebug.h>
#include <ktexteditor/range.h>

#include <language/duchain/types/identifiedtype.h>
#include "cppeditorintegrator.h"
#include <language/duchain/ducontext.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/declaration.h>
#include <language/duchain/duchainpointer.h>
#include <language/duchain/parsingenvironment.h>
#include <language/duchain/identifier.h>
#include <language/duchain/use.h>
#include "parser/parsesession.h"
#include "parser/rpp/chartools.h"
#include "parser/dumptree.h"

using namespace Cpp;
using namespace KDevelop;


DumpChain::DumpChain()
  : m_editor(0)
  , indent(0)
{
}

void DumpChain::dump( AST * node, ParseSession* session)
{
  delete m_editor;
  m_editor = 0;

  if (session)
    m_editor = new CppEditorIntegrator(session);

  visit(node);
}

void DumpChain::visit(AST *node)
{
  QString indentation;
  for( int a = 0; a < indent; a++ )
    indentation += "| ";

  if (node) {
    if (m_editor) {
      QString nodeText = m_editor->parseSession()->stringForNode(node);
      if( !nodeText.isEmpty() ) nodeText = "\"" + nodeText + "\"";


      kDebug(9007) << indentation <<  "\\" << names[node->kind]
              << "[(" << node->start_token << ")" << m_editor->findPosition(node->start_token, CppEditorIntegrator::FrontEdge).castToSimpleCursor() << /*", "
              << m_editor->findPosition(node->end_token, CppEditorIntegrator::FrontEdge) <<*/ "]" << nodeText << endl;
    } else
      kDebug(9007) << indentation << "\\" << names[node->kind]
              << "[" << node->start_token << "," << node->end_token << "]" << endl;
  }

  ++indent;
  DefaultVisitor::visit(node);
  --indent;

  if (node)
  {
    if (m_editor)
    {
      kDebug(9007) << indentation << "/" << names[node->kind]
              << "[("  << node->end_token << ") "/*<< m_editor->findPosition(node->start_token, CppEditorIntegrator::FrontEdge) << ", "*/
              << m_editor->findPosition(node->end_token, CppEditorIntegrator::FrontEdge).castToSimpleCursor() << "]" << endl;
    }
    else
    {
      kDebug(9007) << indentation << "/" << names[node->kind]
              << "[" << node->start_token << "," << node->end_token << ']' << endl;
    }
  }
}

DumpChain::~ DumpChain( )
{
  delete m_editor;
}


