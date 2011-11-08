/* This file is part of KDevelop
    Copyright 2010 Aleix Pol Gonzalez <aleixpol@kde.org>

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

#include "usedecoratorvisitor.h"
#include <parsesession.h>
#include <lexer.h>
#include <tokens.h>
#include <language/duchain/indexedstring.h>
#include <language/duchain/ducontext.h>
#include <language/duchain/use.h>
#include <language/duchain/duchain.h>
#include <language/duchain/duchainlock.h>
#include <language/duchain/types/delayedtype.h>
#include <language/duchain/types/referencetype.h>
#include <util/pushvalue.h>
#include <language/checks/dataaccessrepository.h>
#include <language/duchain/declaration.h>

using namespace KDevelop;
QString UseDecoratorVisitor::nodeToString(AST* node)
{
  QString ret;
  if(!node) { return "<null>";}
  
  for(uint i=node->start_token; i<node->end_token; i++) {
    ret += ' '+m_session->token_stream->token(i).symbolString();
  }
  return ret;
}

AbstractType::Ptr constructReadOnlyType()
{
  static AbstractType::Ptr ntype(new DelayedType);
  return ntype;
}

ReferenceType::Ptr constructReferenceType()
{
  static AbstractType::Ptr ntype(new DelayedType);
  static ReferenceType::Ptr reftype(new ReferenceType);
  reftype->setBaseType(ntype);
  
  return reftype;
}

UseDecoratorVisitor::UseDecoratorVisitor(const ParseSession* session, DataAccessRepository* repo)
  : m_session(session), m_defaultFlags(DataAccess::Read), m_mods(repo)
{
  Q_ASSERT(repo);
}

CursorInRevision UseDecoratorVisitor::cursorForToken(uint token)
{
  return m_session->positionAt(m_session->token_stream->position(token));
}

RangeInRevision UseDecoratorVisitor::rangeForNode(AST* ast)
{
  if(ast)
    return RangeInRevision(cursorForToken(ast->start_token), cursorForToken(ast->end_token));
  else
    return RangeInRevision::invalid();
}

void UseDecoratorVisitor::run(AST* node)
{
  visit(node);
}

void UseDecoratorVisitor::visitUnqualifiedName(UnqualifiedNameAST* node)
{
  //Type exctraction
  AbstractType::Ptr type;
  if(!m_argStack.isEmpty()) {
    int arg = m_argStack.top();
    if(arg<m_callStack.top().size())
      type = m_callStack.top()[arg];
  }
//   qDebug() << "found name" << nodeToString(node) << (type ? type->toString() : "no type");
  
  if(type) {
    //Use extraction
    CursorInRevision cursor = cursorForToken(node->start_token);
    
    //use rw computation
    KDevelop::DataAccess::DataAccessFlags f = m_defaultFlags;
    
    TypePtr< ReferenceType > reftype=type.cast<ReferenceType>();
    if(reftype && reftype->baseType() && !reftype->baseType()->modifiers() & AbstractType::ConstModifier) {
      f |= DataAccess::Write;
    }
//     qDebug() << "adding..." << f << nodeToString(node);
    m_mods->addModification(cursor, f);
  }
  
  //further visit
  visit(node->operator_id);
  visitNodes(this, node->template_arguments);
}

void UseDecoratorVisitor::visitFunctionCall(FunctionCallAST* node)
{
  
  FunctionType::Ptr type = m_session->typeFromCallAst(node);
//   qDebug() << "function call" << (type ? type->toString() : "<null>") << nodeToString(node);
  
  if(type) {
    m_callStack.push(type->arguments());
    m_argStack.push(0);
    DefaultVisitor::visitFunctionCall(node);
    m_argStack.pop();
    m_callStack.pop();
  } else {
    kDebug() << "couldn't find the type for " << nodeToString(node);
  }
}

void UseDecoratorVisitor::visitNewExpression(NewExpressionAST* node)
{
    FunctionType::Ptr type = m_session->typeFromCallAst(node);
//     qDebug() << "new constructor call" << nodeToString(node) << (type ? type->toString() : "<null>") << node;
    QList<AbstractType::Ptr> args;
    if(type)
      args = type->arguments();
    else {
      kDebug() << "couldn't find the type for " << node << nodeToString(node);
      
      args.append(constructReadOnlyType());
    }
    
    m_callStack.push(args);
    m_argStack.push(0);
    visit(node->expression);
    visit(node->type_id);
    visit(node->new_initializer);
//     visit(node->new_type_id);
    m_argStack.pop();
    m_callStack.pop();
}

void UseDecoratorVisitor::visitBinaryExpression(BinaryExpressionAST* node)
{
  //we have two use cases here: the , parameter where we only want to advance in case we're in a function call argument list
  //or it's an operator expression and we want to visit the two sides of the expression.
  
//   qDebug() << "BinaryExpression" << m_session->token_stream->token(node->op).symbolString()
//                 << nodeToString(node)
//                 << m_session->positionAt( m_session->token_stream->position(node->start_token) );
  
  FunctionType::Ptr optype = m_session->typeFromCallAst(node);
  Token optoken = m_session->token_stream->token(node->op);
  bool isFunctionArguments = optoken.kind==',';
  
  QList< AbstractType::Ptr > args;
  PushValue<KDevelop::DataAccess::DataAccessFlags> v(m_defaultFlags, DataAccess::Read);
  PrimaryExpressionAST* primary = static_cast<PrimaryExpressionAST*>(node->left_expression);
  if(!optype && optoken.kind=='=' && node->left_expression->kind==AST::Kind_PrimaryExpression && primary->name) {
//     qDebug() << "lalala" << node->left_expression->kind << nodeToString(primary) << nodeToString(primary->name) << nodeToString(node->right_expression);
    m_mods->addModification(cursorForToken(primary->name->start_token), DataAccess::Write, rangeForNode(node->right_expression));
    
    m_callStack.push(QList< AbstractType::Ptr >() << constructReadOnlyType());
    m_argStack.push(0);
    visit(node->right_expression);
    m_argStack.pop();
    m_callStack.pop();
  } else {
    if(optype) {
      args = optype->arguments();
      if(args.size()==1) { //if there's just the last argument, create a delayed type of the first with the proper flags
        AbstractType::Ptr ntype(new DelayedType);
        ntype->setModifiers(optype->modifiers());
        args.prepend(ntype);
      }
    } else if(!isFunctionArguments) {
      AbstractType::Ptr ntype(constructReadOnlyType());
      
      args += ntype;
      args += ntype;
    }
    
    if(!args.isEmpty()) {
      m_callStack.push(args);
      m_argStack.push(0);
    }
    
    visit(node->left_expression);
  
    //argstack can be empty in cases like "int a,b,c;"
    if(!m_argStack.isEmpty() && (optype || isFunctionArguments)) {
      ++m_argStack.top();
//       qDebug() << "advancing parameter" << m_argStack.top() << "over" << m_callStack.top().size();
//       qDebug() << "xxx " << nodeToString(node) << "\n";
//       Q_ASSERT(m_callStack.top().size()>m_argStack.top());
    }
    m_defaultFlags = DataAccess::Read;
    
    visit(node->right_expression);
    
    if(!args.isEmpty()) {
      m_argStack.pop();
      m_callStack.pop();
    }
  }
}

void UseDecoratorVisitor::visitInitializerList(InitializerListAST* node)
{
  const ListNode< InitializerClauseAST* >* nodes=node->clauses;

  if (!nodes || m_argStack.isEmpty())
    return;

  const ListNode<InitializerClauseAST*>
    *it = nodes->toFront(),
    *end = it;

  do
    {
      visit(it->element);
      ++m_argStack.top();
      it = it->next;
    }
  while (it != end);
}


void UseDecoratorVisitor::visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST* node)
{
    if(node->expressionChosen)
      visit(node->expression);
    else
      visit(node->declaration);
}

void UseDecoratorVisitor::visitUnaryExpression(UnaryExpressionAST* node)
{
  FunctionType::Ptr optype = m_session->typeFromCallAst(node);
  Token optoken = m_session->token_stream->token(node->op);
  
  QList< AbstractType::Ptr > args;
  if(optype) {
    args = optype->arguments();
    if(args.isEmpty()) { //if there's no argument, create a delayed type of the first with the proper flags
      AbstractType::Ptr ntype(new DelayedType);
      ntype->setModifiers(optype->modifiers());
      if(optype->modifiers()&AbstractType::ConstModifier) {
        ReferenceType::Ptr reftype(new ReferenceType);
        reftype->setBaseType(ntype);
        
        args.append(reftype.cast<AbstractType>());
      } else
        args.append(ntype);
    }
  } else {
    if(optoken.kind==Token_incr || optoken.kind==Token_decr) {
      
      args.append(constructReferenceType().cast<AbstractType>());
    } else {
      args.append(constructReadOnlyType());
    }
  }
  
  m_callStack.push(args);
  m_argStack.push(0);
  
  PushValue<KDevelop::DataAccess::DataAccessFlags> v(m_defaultFlags, DataAccess::Read);
  visit(node->expression);
  
  m_argStack.pop();
  m_callStack.pop();
}

void UseDecoratorVisitor::visitMemInitializer(MemInitializerAST* node)
{
  m_mods->addModification(cursorForToken(node->initializer_id->unqualified_name->start_token), DataAccess::Write, rangeForNode(node->expression));
  
  PushValue<KDevelop::DataAccess::DataAccessFlags> v(m_defaultFlags, DataAccess::Read);
  QList<AbstractType::Ptr> args;
  
  FunctionType::Ptr optype = m_session->typeFromCallAst(node);
  if(optype)
    args = optype->arguments();
  else
    args << constructReadOnlyType();
  m_callStack.push(args);
  m_argStack.push(0);
  
  visit(node->expression);
  
  m_argStack.pop();
  m_callStack.pop();
}

void UseDecoratorVisitor::visitConditionalExpression(ConditionalExpressionAST* node)
{
  PushValue<KDevelop::DataAccess::DataAccessFlags> v(m_defaultFlags, DataAccess::Read);
  
  m_callStack.push(QList< AbstractType::Ptr >() << constructReadOnlyType());
  m_argStack.push(0);
  
  visit(node->condition);
  visit(node->left_expression);
  visit(node->right_expression);
  
  m_argStack.pop();
  m_callStack.pop();
}

template <class _Tp>
void visitNodesBackwardsRec(Visitor *v, const ListNode<_Tp> *it,  const ListNode<_Tp>* end)
{
  if(it != end)
    visitNodesBackwardsRec(v, it->next, end);
  v->visit(it->element);
}

template <class _Tp>
void visitNodesBackwards(Visitor *v, const ListNode<_Tp> *nodes)
{
  if (!nodes)
    return;

  const ListNode<_Tp>
    *it = nodes->toFront(),
    *end = it;
  
  visitNodesBackwardsRec(v, it->next, end);
}


void UseDecoratorVisitor::visitPostfixExpression(PostfixExpressionAST* node)
{
//   qDebug() << "visit: PostfixExpression" << nodeToString(node) << nodeToString(node->expression);
  PushValue<KDevelop::DataAccess::DataAccessFlags> v(m_defaultFlags, DataAccess::Read);
  
  m_callStack.push(QList< AbstractType::Ptr >() << constructReadOnlyType());
  m_argStack.push(0);
  
  visit(node->type_specifier); //visit specifiers
  visitNodesBackwards(this, node->sub_expressions); //visit subexpressions, like ++ in (e++) and .b in (e.b)
  visit(node->expression);
  
  m_argStack.pop();
  m_callStack.pop();
}

void UseDecoratorVisitor::visitCppCastExpression(CppCastExpressionAST* node)
{
  PushValue<KDevelop::DataAccess::DataAccessFlags> v(m_defaultFlags, DataAccess::Read);
  
  m_callStack.push(QList< AbstractType::Ptr >() << constructReadOnlyType());
  m_argStack.push(0);
  
  visit(node->type_id); //visit specifiers
  visitNodesBackwards(this, node->sub_expressions); //visit subexpressions, like ++ in (e++) and .b in (e.b)
  visit(node->expression);
  
  m_argStack.pop();
  m_callStack.pop();
}

void UseDecoratorVisitor::visitIncrDecrExpression(IncrDecrExpressionAST* node)
{
  PushValue<KDevelop::DataAccess::DataAccessFlags> v(m_defaultFlags, DataAccess::Read);
  FunctionType::Ptr optype = m_session->typeFromCallAst(node);
  
  if(optype)
    m_callStack.top()=optype->arguments();
  else
    m_callStack.top()=(QList< AbstractType::Ptr >() << constructReferenceType().cast<AbstractType>());
  
  m_argStack.top() = 0;
}

void UseDecoratorVisitor::visitClassMemberAccess(ClassMemberAccessAST* node)
{
  PushValue<KDevelop::DataAccess::DataAccessFlags> v(m_defaultFlags, DataAccess::Read);
  FunctionType::Ptr optype = m_session->typeFromCallAst(node);
  bool modif = optype ? optype->modifiers()&FunctionType::ConstModifier : m_session->token_stream->token(node->op).kind!=Token_arrow;
  
  DataAccess::DataAccessFlags flags(DataAccess::Call | DataAccess::Read | (modif ? DataAccess::Write : DataAccess::None));
  m_mods->addModification(cursorForToken(node->name->start_token), flags);
//   qDebug() << "class member access" << nodeToString(node->name) << modif << (optype ? optype->toString() : "null") << (node->kind);
  m_callStack.top()=(QList< AbstractType::Ptr >() << (modif ? constructReferenceType().cast<AbstractType>() : constructReadOnlyType()));
}

void UseDecoratorVisitor::visitInitDeclarator(InitDeclaratorAST* node)
{
  PushValue<KDevelop::DataAccess::DataAccessFlags> v(m_defaultFlags, DataAccess::Read);
  
  if(node->initializer && node->declarator) {
//     qDebug() << "lalala" << nodeToString(node->declarator->id);
    uint start = node->declarator->id ? node->declarator->id->start_token : node->declarator->start_token;
    m_mods->addModification(cursorForToken(start), DataAccess::Write, rangeForNode(node->initializer));
    
    FunctionType::Ptr type=m_session->typeFromCallAst(node);
    QList<AbstractType::Ptr> args;
    if(type)
      args=type->arguments();
    else
      args += constructReadOnlyType();
    
    m_callStack.push(args);
    m_argStack.push(0);
    visit(node->initializer);
    m_argStack.pop();
    m_callStack.pop();
  }
}

# define IMPL_DEFAULT_VISIT_WITH_FLAGS(a, b) \
void UseDecoratorVisitor::visit##a(a##AST* node)\
{\
  PushValue<KDevelop::DataAccess::DataAccessFlags> v(m_defaultFlags, b);\
/*   qDebug() << "visit: " #a << nodeToString(node);*/\
  m_callStack.push(QList< AbstractType::Ptr >() << constructReadOnlyType());\
  m_argStack.push(0);\
  \
  DefaultVisitor::visit##a(node);\
  \
  m_argStack.pop();\
  m_callStack.pop();\
}

IMPL_DEFAULT_VISIT_WITH_FLAGS(Condition, DataAccess::Read)
IMPL_DEFAULT_VISIT_WITH_FLAGS(DeleteExpression, DataAccess::Read)
IMPL_DEFAULT_VISIT_WITH_FLAGS(ReturnStatement, DataAccess::Read)
