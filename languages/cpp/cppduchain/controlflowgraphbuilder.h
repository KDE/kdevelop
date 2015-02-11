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

#ifndef CONTROLFLOWGRAPHBUILDER_H
#define CONTROLFLOWGRAPHBUILDER_H

#include <cppduchainexport.h>
#include <parser/default_visitor.h>
#include <language/duchain/topducontext.h>

namespace KDevelop
{
class ControlFlowGraph;
class ControlFlowNode;
class CursorInRevision;
class IndexedString;
}

class KDEVCPPDUCHAIN_EXPORT ControlFlowGraphBuilder : public DefaultVisitor
{
  public:
    ControlFlowGraphBuilder(const KDevelop::ReferencedTopDUContext& top, const ParseSession* session, KDevelop::ControlFlowGraph* graph);
    virtual ~ControlFlowGraphBuilder();
    
    void run(AST* node);
    
  protected:
    virtual void visitFunctionDefinition(FunctionDefinitionAST* node) override;
    virtual void visitEnumerator(EnumeratorAST* node) override;
    
    virtual void visitIfStatement(IfStatementAST* node) override;
    virtual void visitWhileStatement(WhileStatementAST* node) override;
    virtual void visitForStatement(ForStatementAST* node) override;
    virtual void visitConditionalExpression(ConditionalExpressionAST* node) override;
    virtual void visitDoStatement(DoStatementAST* node) override;
    
    virtual void visitReturnStatement(ReturnStatementAST* node) override;
    virtual void visitJumpStatement(JumpStatementAST* node) override;
    virtual void visitSwitchStatement(SwitchStatementAST* node) override;
    virtual void visitLabeledStatement(LabeledStatementAST* node) override;
    
  private:
    KDevelop::ControlFlowNode* createCompoundStatement(AST* node, KDevelop::ControlFlowNode* next);
    void createCompoundStatementFrom(KDevelop::ControlFlowNode* curr, AST* node, KDevelop::ControlFlowNode* next);
    KDevelop::CursorInRevision cursorForToken(uint token);
    KDevelop::RangeInRevision nodeRange(AST* node);
    
    const ParseSession* m_session;
    KDevelop::ControlFlowGraph* m_graph;
    KDevelop::ControlFlowNode* m_currentNode;
    
    KDevelop::ControlFlowNode* m_returnNode;
    KDevelop::ControlFlowNode* m_breakNode;
    KDevelop::ControlFlowNode* m_continueNode;
    
    KDevelop::ControlFlowNode* m_defaultNode;
    QList<QPair<KDevelop::ControlFlowNode*, KDevelop::ControlFlowNode*> > m_caseNodes;
    QMap<KDevelop::IndexedString, KDevelop::ControlFlowNode*> m_taggedNodes;
    QMap<KDevelop::IndexedString, QList<KDevelop::ControlFlowNode*> > m_pendingGotoNodes;
    
    KDevelop::ReferencedTopDUContext m_top;
};

#endif // CONTROLFLOWGRAPHBUILDER_H
