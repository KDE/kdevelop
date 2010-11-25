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
#include "flownode.h"
#include "flowgraph.h"
#include <parsesession.h>
#include <lexer.h>

using namespace KDevelop;

ControlFlowGraphBuilder::ControlFlowGraphBuilder(ParseSession* session, ControlFlowGraph* graph)
  : m_session(session)
  , m_graph(graph)
  , m_currentNode(0)
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

// RangeInRevision rangeBetween(uint start_token, uint end_token);

ControlFlowNode* ControlFlowGraphBuilder::createCompoundStatement(AST* node, ControlFlowNode* next)
{
  ControlFlowNode* startNode = new ControlFlowNode;
  CursorInRevision startcursor = cursorForToken(node->start_token);
  startNode->setStartCursor(startcursor);
  m_currentNode = startNode;
  visit(node);
  CursorInRevision endcursor = cursorForToken(node->end_token);
  m_currentNode->setEndCursor(endcursor);
  m_currentNode->m_next = next;
  return startNode;
}

void ControlFlowGraphBuilder::visitFunctionDefinition(FunctionDefinitionAST* node)
{
  m_graph->addEntry(createCompoundStatement(node->function_body, 0));
  m_currentNode=0;
}

void ControlFlowGraphBuilder::visitIfStatement(IfStatementAST* node)
{
//TODO:   m_currentNode->m_conditionRange = rangeBetween(node->condition->start_token, node->condition->end_token);
  ControlFlowNode* previous = m_currentNode;
  m_currentNode->setEndCursor(cursorForToken(node->start_token));
  visit(node->condition);
  
  ControlFlowNode* nextNode = new ControlFlowNode;
  
  previous->m_next = createCompoundStatement(node->statement, nextNode);
  if(node->else_statement)
    previous->m_alternative = createCompoundStatement(node->else_statement, nextNode);
  else
    previous->m_alternative = nextNode;
  
  nextNode->setStartCursor(cursorForToken(node->end_token));
  m_currentNode = nextNode;
}

void ControlFlowGraphBuilder::visitWhileStatement(WhileStatementAST* node)
{
  //TODO:   m_currentNode->m_conditionRange = rangeBetween(node->condition->start_token, node->condition->end_token);
  m_currentNode->setEndCursor(cursorForToken(node->start_token));
  ControlFlowNode* previous = m_currentNode;
  
  ControlFlowNode* conditionNode = createCompoundStatement(node->condition, 0);
  ControlFlowNode* bodyNode = createCompoundStatement(node->statement, 0);
  ControlFlowNode* nextNode = new ControlFlowNode;
  
  previous->m_next = conditionNode;
  bodyNode->m_next = conditionNode;
  conditionNode->m_next =bodyNode;
  conditionNode->m_alternative = nextNode;
  
  nextNode->setStartCursor(cursorForToken(node->end_token));
  m_currentNode = nextNode;
}

void ControlFlowGraphBuilder::visitForStatement(ForStatementAST* node)
{
  visit(node->init_statement);
  m_currentNode->setEndCursor(cursorForToken(node->init_statement->end_token));
  ControlFlowNode* previous = m_currentNode;
  
  ControlFlowNode* nextNode = new ControlFlowNode;
  ControlFlowNode* conditionNode = createCompoundStatement(node->condition, 0);
  ControlFlowNode* incNode = createCompoundStatement(node->expression, conditionNode);
  ControlFlowNode* bodyNode = createCompoundStatement(node->statement, incNode);
  
  conditionNode->m_next = bodyNode;
  conditionNode->m_alternative = nextNode;
  
  previous->m_next = conditionNode;
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
  
  previous->m_next = trueNode;
  previous->m_alternative = elseNode;
  
  nextNode->setStartCursor(cursorForToken(node->end_token));
  m_currentNode = nextNode;
}

