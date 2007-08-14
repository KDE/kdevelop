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

#include <identifiedtype.h>
#include "cppeditorintegrator.h"
#include <ducontext.h>
#include <topducontext.h>
#include <declaration.h>
#include <definition.h>
#include <duchainpointer.h>
#include <parsingenvironment.h>
#include <identifier.h>
#include <use.h>
#include "parser/parsesession.h"

using namespace KDevelop;

static char const * const names[] = {
  0,
  "AccessSpecifier",
  "AsmDeclaration",
  "BaseClause",
  "BaseSpecifier",
  "BinaryExpression",
  "CastExpression",
  "ClassMemberAccess",
  "ClassSpecifier",
  "CompoundStatement",
  "Condition",
  "ConditionalExpression",
  "CppCastExpression",
  "CtorInitializer",
  "DeclarationStatement",
  "Declarator",
  "DeleteExpression",
  "DoStatement",
  "ElaboratedTypeSpecifier",
  "EnumSpecifier",
  "Enumerator",
  "ExceptionSpecification",
  "ExpressionOrDeclarationStatement",
  "ExpressionStatement",
  "ForStatement",
  "FunctionCall",
  "FunctionDeclaration",
  "IfStatement",
  "IncrDecrExpression",
  "InitDeclarator",
  "Initializer",
  "InitializerClause",
  "LabeledStatement",
  "LinkageBody",
  "LinkageSpecification",
  "MemInitializer",
  "Name",
  "Namespace",
  "NamespaceAliasDeclaration",
  "NewDeclarator",
  "NewExpression",
  "NewInitializer",
  "NewTypeId",
  "Operator",
  "OperatorFunctionId",
  "ParameterDeclaration",
  "ParameterDeclarationClause",
  "PostfixExpression",
  "PrimaryExpression",
  "PtrOperator",
  "PtrToMember",
  "ReturnStatement",
  "SimpleDeclaration",
  "SimpleTypeSpecifier",
  "SizeofExpression",
  "StringLiteral",
  "SubscriptExpression",
  "SwitchStatement",
  "TemplateArgument",
  "TemplateDeclaration",
  "TemplateParameter",
  "ThrowExpression",
  "TranslationUnit",
  "TryBlockStatement",
  "TypeId",
  "TypeIdentification",
  "TypeParameter",
  "Typedef",
  "UnaryExpression",
  "UnqualifiedName",
  "Using",
  "UsingDirective",
  "WhileStatement",
  "WinDeclSpec"
};

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

  if (node)
    if (m_editor) {
      QString nodeText;
      for( std::size_t a = node->start_token; a != node->end_token; a++ ) {
        const Token& tok( m_editor->parseSession()->token_stream->token((int) a) );
        if( !nodeText.isEmpty() )
          nodeText += ' ';
        nodeText += QByteArray( tok.text+tok.position, tok.size );
      }
      if( !nodeText.isEmpty() ) nodeText = "\"" + nodeText + "\"";


      kDebug(9007) << indentation <<  "\\" << names[node->kind]
              << "[(" << node->start_token << ")" << m_editor->findPosition(node->start_token, CppEditorIntegrator::FrontEdge) << /*", "
              << m_editor->findPosition(node->end_token, CppEditorIntegrator::FrontEdge) <<*/ "]" << nodeText << endl;
    } else
      kDebug(9007) << indentation << "\\" << names[node->kind]
              << "[" << node->start_token << "," << node->end_token << "]" << endl;

  ++indent;
  DefaultVisitor::visit(node);
  --indent;

  if (node)
    if (m_editor)
      kDebug(9007) << indentation << "/" << names[node->kind]
              << "[("  << node->end_token << ") "/*<< m_editor->findPosition(node->start_token, CppEditorIntegrator::FrontEdge) << ", "*/
              << m_editor->findPosition(node->end_token, CppEditorIntegrator::FrontEdge) << "]" << endl;
    else
      kDebug(9007) << indentation << "/" << names[node->kind]
              << "[" << node->start_token << "," << node->end_token << ']' << endl;
}

DumpChain::~ DumpChain( )
{
  delete m_editor;
}

void DumpChain::dump( DUContext * context, bool imported )
{
  kDebug(9007) << QString(indent * 2, ' ') << (imported ? "==import==> Context " : "New Context ") << context << "\"" <<  context->localScopeIdentifier() << "\" [" << context->scopeIdentifier() << "]" << context->textRange() << " " << (dynamic_cast<TopDUContext*>(context) ? "top-context" : "");
  if( !context )
    return;
  if (!imported) {
    foreach (Declaration* dec, context->localDeclarations()) {
      
      //IdentifiedType* idType = dynamic_cast<IdentifiedType*>(dec->abstractType().data());
      kDebug(9007) << QString((indent+1) * 2, ' ') << "Declaration: " << dec->toString() << /*(idType ? (" (type-identity: " + idType->identifier().toString() + ")") : QString()) <<*/ " [" << dec->qualifiedIdentifier() << "]" << dec->textRangePtr() << " " << &dec->textRangePtr()->start() << " " << dec->textRange() << "," << (dec->isDefinition() ? "defined, " : (dec->definition() ? "" : "no definition, ")) << dec->uses().count() << "use(s).";
      if (dec->definition())
        kDebug(9007) << QString((indent+1) * 2 + 1, ' ') << "Definition:" << dec->definition()->textRange();
      foreach (Use* use, dec->uses())
        kDebug(9007) << QString((indent+2) * 2, ' ') << "Use:" << use->textRange();
    }
  }

  ++indent;
  if (!imported) {
    foreach (DUContextPointer parent, context->importedParentContexts()) {
      dump(parent.data(), true);
    }

    foreach (DUContext* child, context->childContexts())
      dump(child);
  }
  --indent;
}

QString shortLabel(KDevelop::DUContext* context) {
  return QString("q%1").arg((uint)context);
}

QString shortLabel(KDevelop::Declaration* declaration) {
  return QString("q%1").arg((uint)declaration);
}

QString shortLabel(KDevelop::Definition* definition) {
  return QString("q%1").arg((uint)definition);
}

QString DumpChain::dotGraph(KDevelop::DUContext* context, bool shortened, bool isMaster) {
  QTextStream stream;
  QString ret;
  stream.setString(&ret, QIODevice::WriteOnly);

  if( isMaster )
    stream << "Digraph chain {\n";
  
  QString shape = "box";
  QString label = "unknown";
  
  if( dynamic_cast<TopDUContext*>(context) ) {
    shape = "parallelogram";
    if( static_cast<TopDUContext*>(context)->parsingEnvironmentFile() ) {
      label = static_cast<TopDUContext*>(context)->parsingEnvironmentFile()->identity().toString();
    } else {
      label = "unknown file";
    }
  }else{
    label = "context " + context->scopeIdentifier().toString();
  }

  stream << shortLabel(context) << "[shape=" << shape << ",label=\"" << label << "\"];\n";
  
  foreach (DUContextPointer parent, context->importedParentContexts()) {
    if( parent ) {
      stream << dotGraph(parent.data(), shortened, false);
      stream << shortLabel(context) << " -> " << shortLabel(parent.data()) << "[style=dotted,label=\"imports\"];\n";
    }
  }

/*  if( context->parentContext() ) {
    stream << dotGraph(context->parentContext(), shortened, false);
    stream << shortLabel(context) << " -- " << shortLabel(context->parentContext()) << ";\n";
  }*/
  
  if( !context->childContexts().isEmpty() ) {
    stream << shortLabel(context) << "C[shape=hexagon, label=\"" << QString("%1 contexts").arg(context->childContexts().count()) << "\"];\n";
    stream << shortLabel(context) << " -> " << shortLabel(context) << "C;\n";
  }

  if( !shortened ) {
    foreach (DUContext* child, context->childContexts()) {
      stream << dotGraph(child, shortened, false);
      stream << shortLabel(context) << "C -> " << shortLabel(child) << "[style=dotted];\n";
    }
  }

  if( !context->localDeclarations().isEmpty() ) {
    stream << shortLabel(context) << "D[shape=hexagon, label=\"" << QString("%1 decls").arg(context->localDeclarations().count()) << "\"];\n";
    stream << shortLabel(context) << " -> " << shortLabel(context) << "D;\n";
  }
  
  if(!shortened ) {
    foreach (Declaration* dec, context->localDeclarations()) {

      stream << shortLabel(dec) <<
          "[shape=distortion,label=\"" <<
          dec->toString() << " [" <<
          dec->qualifiedIdentifier().toString() << "]" /*<<
          dec->textRange().()*/ << "\"];\n";
      stream << shortLabel(context) << "D -> " << shortLabel(dec) << ";\n";
      if( dec->internalContext() )
        stream << shortLabel(dec) << " -> " << shortLabel(dec->internalContext()) << "[label=\"internal\", color=blue];\n";
    }
  }

  if( !context->localDefinitions().isEmpty() ) {
    stream << shortLabel(context) << "Df[shape=hexagon, label=\"" << QString("%1 defs").arg(context->localDefinitions().count()) << "\"];\n";
    stream << shortLabel(context) << " -> " << shortLabel(context) << "Df;\n";
  }
  
  if(!shortened ) {
    foreach (Definition* def, context->localDefinitions()) {

      stream << shortLabel(def) <<  "[shape=regular,label=\"" << (def->declaration() ? def->declaration()->toString() : QString("no declaration")) << /*dec->textRange() <<*/ "\"];\n";
      stream << shortLabel(context) << "Df -> " << shortLabel(def) << ";\n";
      if( def->declaration() )
        stream << shortLabel(def) << " -> " << shortLabel(def->declaration()) << "[label=\"defines\"];\n";
    }
  }

  if( isMaster )
    stream << "}\n";
  return ret;
}

// kate: space-indent on; indent-width 2; replace-tabs on;
