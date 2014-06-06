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

#include "controlflowgraphbuilder.h"
#include <language/checks/controlflownode.h>
#include <language/checks/controlflowgraph.h>
#include <language/duchain/topducontext.h>
#include <language/duchain/duchainlock.h>
#include <parsesession.h>
#include <lexer.h>
#include <tokens.h>
#include <util/pushvalue.h>

using namespace KDevelop;
QString nodeToString(const ParseSession* s, AST* node);

ControlFlowGraphBuilder::ControlFlowGraphBuilder(const KDevelop::ReferencedTopDUContext& top, const ParseSession* session, ControlFlowGraph* graph)
  : m_session(session)
  , m_graph(graph)
  , m_currentNode(0)
  , m_returnNode(0)
  , m_breakNode(0)
  , m_continueNode(0)
  , m_defaultNode(0)
  , m_top(top)
{}

ControlFlowGraphBuilder::~ControlFlowGraphBuilder()
{}

void ControlFlowGraphBuilder::run(AST* node)
{
  Q_ASSERT(!m_currentNode);
  visit(node);
}

CursorInRevision ControlFlowGraphBuilder::cursorForToken(uint token)
{
  return m_session->positionAt(m_session->token_stream->position(token));
}

RangeInRevision ControlFlowGraphBuilder::nodeRange(AST* node)
{
  RangeInRevision ret = node ? RangeInRevision(cursorForToken(node->start_token), cursorForToken(node->end_token)) : RangeInRevision::invalid();
  if(ret.start>=ret.end) { //I don't understand why I need this
    CursorInRevision start=ret.end;
    ret.start=ret.end;
    ret.end=start;
  }
  return ret;
}

// RangeInRevision rangeBetween(uint start_token, uint end_token);

ControlFlowNode* ControlFlowGraphBuilder::createCompoundStatement(AST* node, ControlFlowNode* next)
{
  ControlFlowNode* startNode = new ControlFlowNode;
  if(node)
    createCompoundStatementFrom(startNode, node, next);
  else {
    startNode->setNext(next);
    m_currentNode = startNode;
  }
  return startNode;
}

void ControlFlowGraphBuilder::createCompoundStatementFrom(ControlFlowNode* startNode, AST* node, ControlFlowNode* next)
{
  Q_ASSERT(node && startNode);
  CursorInRevision startcursor = cursorForToken(node->start_token);
  startNode->setStartCursor(startcursor);
  m_currentNode = startNode;
  visit(node);
  
  if(!m_currentNode->next()) {
    m_currentNode->setNext(next);
    m_currentNode->setEndCursor(cursorForToken(node->end_token));
  }
}

void ControlFlowGraphBuilder::visitFunctionDefinition(FunctionDefinitionAST* node)
{
  if(!node->function_body || !node->function_body->ducontext) //means its ducontext hasn't been built, we probably don't need the flow diagram
    return;
  
  PushValue<ControlFlowNode*> currentNode(m_currentNode);
  m_returnNode = new ControlFlowNode;
  
  Declaration* d;
  {
    KDevelop::DUChainReadLocker lock;
    d=node->function_body->ducontext->owner();
//     qDebug() << "lalala" << nodeToString(m_session, node) << node->function_body->ducontext->owner() << node->function_body;
  }
  
  if(d)
    m_graph->addEntry(d, createCompoundStatement(node->function_body, m_returnNode));
  else
    m_graph->addEntry(createCompoundStatement(node->function_body, m_returnNode));
}

void ControlFlowGraphBuilder::visitEnumerator(EnumeratorAST* node)
{
    bool create=!m_currentNode;
    if(create && node->expression)
      m_graph->addEntry(createCompoundStatement(node->expression, 0));
    else
      DefaultVisitor::visitEnumerator(node);
}


void ControlFlowGraphBuilder::visitIfStatement(IfStatementAST* node)
{
  ControlFlowNode* previous = m_currentNode;
  m_currentNode->setEndCursor(cursorForToken(node->condition->end_token));
  visit(node->condition);
  
  ControlFlowNode* nextNode = new ControlFlowNode;
  
  previous->setConditionRange(nodeRange(node->condition));
  previous->setNext(createCompoundStatement(node->statement, nextNode));
  previous->setAlternative(node->else_statement ? createCompoundStatement(node->else_statement, nextNode) : nextNode);
  
  nextNode->setStartCursor(cursorForToken(node->end_token));
  m_currentNode = nextNode;
}

void ControlFlowGraphBuilder::visitWhileStatement(WhileStatementAST* node)
{
  //TODO:   m_currentNode->m_conditionRange = rangeBetween(node->condition->start_token, node->condition->end_token);
  m_currentNode->setEndCursor(cursorForToken(node->start_token));
  ControlFlowNode* previous = m_currentNode;
  
  ControlFlowNode* nextNode = new ControlFlowNode;
  ControlFlowNode* conditionNode = createCompoundStatement(node->condition, 0);
  
  PushValue<ControlFlowNode*> pushBreak(m_breakNode, nextNode);
  PushValue<ControlFlowNode*> pushContinue(m_continueNode, conditionNode);
  ControlFlowNode* bodyNode = createCompoundStatement(node->statement, conditionNode);
  
  previous->setNext(conditionNode);
  conditionNode->setConditionRange(nodeRange(node->condition));
  conditionNode->setNext(bodyNode);
  conditionNode->setAlternative(nextNode);
  
  nextNode->setStartCursor(cursorForToken(node->end_token));
  m_currentNode = nextNode;
}

void ControlFlowGraphBuilder::visitForStatement(ForStatementAST* node)
{
  AST* flownode = node->condition ? (AST*) node->condition : (AST*) node->range_declaration;
  visit(node->init_statement);
  m_currentNode->setEndCursor(cursorForToken(flownode ? flownode->start_token : node->init_statement->end_token));
  ControlFlowNode* previous = m_currentNode;
  
  ControlFlowNode* nextNode = new ControlFlowNode;
  ControlFlowNode* conditionNode = createCompoundStatement(flownode, nextNode);
  ControlFlowNode* endCondition = m_currentNode;
  ControlFlowNode* incNode = createCompoundStatement(node->expression, conditionNode);
  
  PushValue<ControlFlowNode*> pushBreak(m_breakNode, nextNode);
  PushValue<ControlFlowNode*> pushContinue(m_continueNode, incNode);
  ControlFlowNode* bodyNode = createCompoundStatement(node->statement, incNode);
  
  endCondition->setAlternative(bodyNode);
  endCondition->setConditionRange(nodeRange(flownode));
  
  previous->setNext(conditionNode);
  nextNode->setStartCursor(cursorForToken(node->end_token));
  m_currentNode = nextNode;
}

void ControlFlowGraphBuilder::visitConditionalExpression(ConditionalExpressionAST* node)
{
  visit(node->condition);
  m_currentNode->setEndCursor(cursorForToken(node->condition->end_token));
  ControlFlowNode* previous = m_currentNode;
  
  ControlFlowNode* nextNode = new ControlFlowNode;
  ControlFlowNode* trueNode = createCompoundStatement(node->left_expression, nextNode);
  ControlFlowNode* elseNode = createCompoundStatement(node->right_expression, nextNode);
  
  previous->setNext(trueNode);
  previous->setAlternative(elseNode);
  previous->setConditionRange(nodeRange(node->condition));
  
  nextNode->setStartCursor(cursorForToken(node->end_token));
  m_currentNode = nextNode;
}

void ControlFlowGraphBuilder::visitDoStatement(DoStatementAST* node)
{
  m_currentNode->setEndCursor(cursorForToken(node->start_token));
  ControlFlowNode* previous = m_currentNode;
  
  ControlFlowNode* nextNode = new ControlFlowNode;
  ControlFlowNode* condNode = createCompoundStatement(node->expression, nextNode);
  
  PushValue<ControlFlowNode*> pushBreak(m_breakNode, nextNode);
  PushValue<ControlFlowNode*> pushContinue(m_continueNode, condNode);
  ControlFlowNode* bodyNode = createCompoundStatement(node->statement, condNode);
  
  previous->setNext(bodyNode);
  condNode->setAlternative(bodyNode);
  condNode->setConditionRange(nodeRange(node->expression));
  
  nextNode->setStartCursor(cursorForToken(node->end_token));
  m_currentNode = nextNode;
}

void ControlFlowGraphBuilder::visitReturnStatement(ReturnStatementAST* node)
{
  DefaultVisitor::visitReturnStatement(node);
  m_currentNode->setEndCursor(cursorForToken(node->end_token));
  m_currentNode->setNext(m_returnNode);
}

void ControlFlowGraphBuilder::visitJumpStatement(JumpStatementAST* node)
{
  m_currentNode->setEndCursor(cursorForToken(node->end_token));
  switch(m_session->token_stream->token(node->start_token).kind) {
    case Token_continue:
//       if(!m_continueNode) addproblem(!!!);
      m_currentNode->setNext(m_continueNode);
      break;
    case Token_break:
//       if(!m_breakNode) addproblem(!!!);
      m_currentNode->setNext(m_breakNode);
      break;
    case Token_goto: {
//       qDebug() << "goto!";
      IndexedString tag = m_session->token_stream->symbol(node->identifier);
      QMap< IndexedString, ControlFlowNode* >::const_iterator tagIt = m_taggedNodes.constFind(tag);
      if(tagIt!=m_taggedNodes.constEnd())
        m_currentNode->setNext(*tagIt);
      else {
        m_pendingGotoNodes[tag] += m_currentNode;
        m_currentNode->setNext(0); //we set null waiting to find a proper node to jump to
      }
    } break;
  }
  
  //here we create a node with the dead code
  ControlFlowNode* deadNode = new ControlFlowNode;
  deadNode->setStartCursor(m_currentNode->nodeRange().end);
//   deadNode->setEndCursor(cursorForToken(node->end_token));
  m_currentNode=deadNode;
  m_graph->addDeadNode(deadNode);
}

void ControlFlowGraphBuilder::visitSwitchStatement(SwitchStatementAST* node)
{
  visit(node->condition);
  m_currentNode->setEndCursor(cursorForToken(node->condition->end_token));
//   ControlFlowNode* previous = m_currentNode;
  
  ControlFlowNode* nextNode = new ControlFlowNode;
  PushValue<ControlFlowNode*> pushBreak(m_breakNode, nextNode);
  PushValue<ControlFlowNode*> pushDefault(m_defaultNode, nextNode);
  
  ControlFlowNode* switchNode = m_currentNode;
  switchNode->setNext(nextNode);
  PushValue<QList< QPair<ControlFlowNode*, ControlFlowNode*> > > pushCases(m_caseNodes, QList<QPair<ControlFlowNode*,ControlFlowNode*> > ());
  visit(node->statement);
  switchNode->setNext(m_defaultNode);
  switchNode->setAlternative(m_caseNodes.isEmpty() ? 0 : m_caseNodes.first().first);
  switchNode->setConditionRange(nodeRange(node->condition));
  nextNode->setStartCursor(cursorForToken(node->end_token));
  m_currentNode = nextNode;
}

void ControlFlowGraphBuilder::visitLabeledStatement(LabeledStatementAST* node)
{
  visit(node->expression);
  
  int token = m_session->token_stream->token(node->start_token).kind;
  
  if(token==Token_default || token==Token_case) {
    ControlFlowNode* condNode = new ControlFlowNode;
    condNode->setStartCursor(cursorForToken(node->start_token));
    condNode->setEndCursor(cursorForToken(node->end_token));
    
    condNode->setNext(createCompoundStatement(node->statement, 0));
    
    if(!m_caseNodes.isEmpty()) {
      m_caseNodes.last().first->setAlternative(condNode);
      if(!m_caseNodes.last().second->next())
        m_caseNodes.last().second->setNext(condNode->next());
    }
    
    m_caseNodes+=qMakePair(condNode, m_currentNode);
    
    if(token==Token_default)
      m_defaultNode = condNode;
    
  } else { //it is a goto tag
    m_currentNode->setEndCursor(cursorForToken(node->start_token));
    
    ControlFlowNode* nextNode = new ControlFlowNode;
    nextNode->setStartCursor(cursorForToken(node->start_token));
    if(!m_currentNode->next()) m_currentNode->setNext(nextNode);
    
    IndexedString tag = m_session->token_stream->symbol(node->label);
    m_taggedNodes.insert(tag, nextNode);
    QList< ControlFlowNode* > pendingNodes = m_pendingGotoNodes.take(tag);
    foreach(ControlFlowNode* pending, pendingNodes)
      pending->setNext(nextNode);
    
    m_currentNode = nextNode;
    visit(node->statement);
  }
}
