/* This file is part of KDevelop
    Copyright 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright 2007 David Nolden <david.nolden.kdevelop@art-master.de>

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

//krazy:excludeall=cpp

#include "name_visitor.h"
#include "name_compiler.h"
#include "type_visitor.h"
#include "lexer.h"
#include "symbol.h"
#include "parsesession.h"
#include "tokens.h"
#include "expressionparser.h"
#include "expressionvisitor.h"
#include "debug.h"
#include <language/duchain/duchainlock.h>


#define LOCKDUCHAIN     DUChainReadLocker lock(DUChain::lock())

using namespace KDevelop;
using namespace Cpp;

NameASTVisitor::NameASTVisitor(ParseSession* session, Cpp::ExpressionVisitor* visitor, const KDevelop::DUContext* context, const KDevelop::TopDUContext* source, const KDevelop::DUContext* localVisibilityContext, const CursorInRevision& position, KDevelop::DUContext::SearchFlags localSearchFlags, bool debug )
: m_session(session), m_visitor(visitor), m_context(context), m_source(source), m_localContext(localVisibilityContext), m_find(m_context, m_source, localSearchFlags, CursorInRevision(position) ), m_debug(debug), m_finalName(0)
{
  m_flags = localSearchFlags;
  m_stopSearch = false;
}

void NameASTVisitor::visitUnqualifiedName(UnqualifiedNameAST *node)
{
  if(m_stopSearch)
    return;
  IndexedString tmp_name;

  if (node->id)
    tmp_name = m_session->token_stream->symbol(node->id);
  if (node->tilde)
    tmp_name = IndexedString(QLatin1String("~") + tmp_name.str());
  if (OperatorFunctionIdAST *op_id = node->operator_id) {
    QString tmpString = QLatin1String("operator");

    if (op_id->op && op_id->op->op)
      tmpString +=  m_session->stringForNode(op_id->op, true);
    else
      tmpString += QLatin1String("{...cast...}");

    tmp_name = IndexedString(tmpString);

    m_typeSpecifier = op_id->type_specifier;
  }
  m_currentIdentifier = Identifier(tmp_name);
  m_find.openIdentifier(m_currentIdentifier);

  if (node->template_arguments) {
    visitNodes(this, node->template_arguments);
  } else if(node->end_token == node->start_token + 3 && node->id == node->start_token
            && m_session->token_stream->kind(node->id+1) == '<')
  {
    ///@todo Represent this nicer in the AST
    ///It's probably a type-specifier with instantiation of the default-parameter, like "Bla<>".
    m_find.openQualifiedIdentifier( ExpressionEvaluationResult() );
    m_find.closeQualifiedIdentifier();
  }

  {
    LOCKDUCHAIN;
    m_find.closeIdentifier(node == m_finalName);
  }
  if( (node->id || node->operator_id) && !m_find.lastDeclarations().isEmpty() ) {
    bool had = false;
    uint start_token;
    uint end_token;
    if ( node->id ) {
      start_token = node->id;
      end_token = node->id + 1;
    } else {
      start_token = node->start_token;
      end_token = node->end_token;
    }
    foreach(const DeclarationPointer &decl, m_find.lastDeclarations()) {
      if(decl && !decl->isForwardDeclaration()) {
        //Prefer registering non forward-declaration uses
        if(m_visitor)
          m_visitor->newUse( node, start_token, end_token, decl );
        had = true;
        break;
      }
    }

    if(!had) //only forward-declarations, register to any.
      if(m_visitor)
        m_visitor->newUse( node, start_token,end_token, m_find.lastDeclarations()[0] );
  } else {
    if(node == m_finalName) {
      if(m_visitor) { //Create a zero use, which will be highlighted as an error
        bool createUse = false;
        {
          LOCKDUCHAIN;
          createUse = !m_foundSomething || !Cpp::isTemplateDependent(m_foundSomething.data());
        }
        if(createUse)
          m_visitor->newUse(node, node->id, node->id+1, DeclarationPointer());
      }

      if( m_debug )
        qCDebug(CPPDUCHAIN) << "failed to find " << m_currentIdentifier << " as part of " << m_session->stringForNode(node) << ", searched in " << m_find.describeLastContext();
    }
  }

  _M_name.push(m_currentIdentifier);

  if(!m_find.lastDeclarations().isEmpty()) {
    m_foundSomething = m_find.lastDeclarations().first();
  }
}

TypeSpecifierAST* NameASTVisitor::lastTypeSpecifier() const {
  if(m_stopSearch)
    return 0;
  return m_typeSpecifier;
}

void NameASTVisitor::visitTemplateArgument(TemplateArgumentAST *node)
{
  if(m_stopSearch)
    return;
  processTemplateArgument(node);
}

ExpressionEvaluationResult NameASTVisitor::processTemplateArgument(TemplateArgumentAST *node)
{
  if(m_stopSearch)
    return ExpressionEvaluationResult();

  ExpressionEvaluationResult res;
  bool opened = false;
  if( node->expression ) {
    bool ownVisitor = false;
    if(!m_visitor) {
      m_visitor = new ExpressionVisitor(m_session, m_source);
      ownVisitor = true;
    }
    if (!node->expression->ducontext) {
      node->expression->ducontext = const_cast<DUContext*>(m_localContext);
    }
    m_visitor->parse( node->expression );

    if( m_visitor->lastType() ) {
      LOCKDUCHAIN;

      res.type = m_visitor->lastType()->indexed();
      foreach(const DeclarationPointer &decl, m_visitor->lastDeclarations())
        if(decl)
          res.allDeclarationsList().append(decl->id());

      res.isInstance = m_visitor->lastInstance().isInstance;
      if(m_visitor->lastInstance().declaration)
        res.instanceDeclaration = m_visitor->lastInstance().declaration->id();

      m_find.openQualifiedIdentifier(res);
      opened = true;

    }else if( m_debug ) {
      qCDebug(CPPDUCHAIN) << "Failed to resolve template-argument " << m_session->stringForNode(node->expression);
    }

    if(ownVisitor) {
      delete m_visitor;
      m_visitor = 0;
    }

  } else if( node->type_id )
  {
    TypeASTVisitor v( m_session, m_visitor, m_localContext, m_source, m_localContext, m_debug );
    v.run( node->type_id );

    if(v.stoppedSearch()) {
      m_stopSearch = true;
      return ExpressionEvaluationResult();
    }

    res.type = v.type()->indexed();

    if( res.type ) {
      LOCKDUCHAIN;
      foreach(const DeclarationPointer &decl, v.declarations())
        if(decl)
          res.allDeclarationsList().append(decl->id());

      m_find.openQualifiedIdentifier(res);
      opened = true;
    }

  }else{
    LOCKDUCHAIN;
    m_find.openQualifiedIdentifier(false);
    m_find.openIdentifier(Identifier(m_session->stringForNode(node)));
    m_find.closeIdentifier(false);
    opened = true;
  }
  LOCKDUCHAIN;
  if(opened)
    m_find.closeQualifiedIdentifier();

  m_currentIdentifier.appendTemplateIdentifier( typeIdentifierFromTemplateArgument(m_session, node) );
  return res;
}

const QualifiedIdentifier& NameASTVisitor::identifier() const
{
  if(m_stopSearch)
  {
    static const QualifiedIdentifier tmpQI;
    return tmpQI;
  }
  return _M_name;
}

void NameASTVisitor::run(UnqualifiedNameAST *node, bool skipThisName)
{
  m_finalName = node;

  m_find.openQualifiedIdentifier(false);
  m_typeSpecifier = 0;
  _M_name.clear();

  if(skipThisName)
    DefaultVisitor::visitUnqualifiedName(node);
  else
    visit(node);

  if(m_stopSearch)
    return;

  LOCKDUCHAIN;
  m_find.closeQualifiedIdentifier();

  if(m_find.lastDeclarations().isEmpty() && (m_flags & DUContext::NoUndefinedTemplateParams)) {
    m_stopSearch = true;
    return;
  }
}

QList<KDevelop::DeclarationPointer> NameASTVisitor::declarations() const
{
  if(m_stopSearch)
    return QList<KDevelop::DeclarationPointer>();
  return m_find.lastDeclarations();
}


void NameASTVisitor::run(NameAST *node, bool skipLastNamePart)
{
  m_find.openQualifiedIdentifier(node->global);
  m_typeSpecifier = 0;

  _M_name.clear();

  m_finalName = node->unqualified_name;

  if(skipLastNamePart)
    visitNodes(this, node->qualified_names); //Skip the unqualified name
  else
    visit(node);

  if(m_stopSearch)
    return;

  _M_name.setExplicitlyGlobal( node->global );
  LOCKDUCHAIN;
  m_find.closeQualifiedIdentifier();
}


DeclarationPointer NameASTVisitor::foundSomething() const
{
  return m_foundSomething;
}
