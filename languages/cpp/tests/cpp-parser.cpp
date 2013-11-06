/***************************************************************************
 *   This file is part of KDevelop                                         *
 *   Copyright 2008 Niko Sams <niko.sams@gmail.com>                        *
 *   Copyright 2009 Milian Wolff <mail@milianw.de>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by the Free Software Foundation; either version 2 of the    *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU Library General Public     *
 *   License along with this program; if not, write to the                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include "parsesession.h"
#include "parser.h"
#include "dumptree.h"
#include "ast.h"
#include "tokens.h"

#include <KAboutData>

#include <language/util/debuglanguageparserhelper.h>

#include "rpp/pp-location.h"
#include "rpp/preprocessor.h"
#include "rpp/pp-engine.h"

#include "contextbuilder.h"
#include "cpputils.h"
#include "control.h"
#include <memorypool.h>

using namespace Cpp;
using namespace KDevelopUtils;

class MemSizeVisitor : public DefaultVisitor
{
public:
  MemSizeVisitor()
  : m_size(0)
  {
  }
  virtual ~MemSizeVisitor()
  {
  }
  virtual void visitAccessSpecifier(AccessSpecifierAST* node) { m_size += sizeof(AccessSpecifierAST); DefaultVisitor::visitAccessSpecifier(node); }
  virtual void visitAliasDeclaration(AliasDeclarationAST* node) { m_size += sizeof(AliasDeclarationAST); DefaultVisitor::visitAliasDeclaration(node); }
  virtual void visitAsmDefinition(AsmDefinitionAST* node) { m_size += sizeof(AsmDefinitionAST); DefaultVisitor::visitAsmDefinition(node); }
  virtual void visitBaseClause(BaseClauseAST* node) { m_size += sizeof(BaseClauseAST); DefaultVisitor::visitBaseClause(node); }
  virtual void visitBaseSpecifier(BaseSpecifierAST* node) { m_size += sizeof(BaseSpecifierAST); DefaultVisitor::visitBaseSpecifier(node); }
  virtual void visitBinaryExpression(BinaryExpressionAST* node) { m_size += sizeof(BinaryExpressionAST); DefaultVisitor::visitBinaryExpression(node); }
  virtual void visitBracedInitList(BracedInitListAST* node) { m_size += sizeof(BracedInitListAST); DefaultVisitor::visitBracedInitList(node); }
  virtual void visitCastExpression(CastExpressionAST* node) { m_size += sizeof(CastExpressionAST); DefaultVisitor::visitCastExpression(node); }
  virtual void visitCatchStatement(CatchStatementAST* node) { m_size += sizeof(CatchStatementAST); DefaultVisitor::visitCatchStatement(node); }
  virtual void visitClassMemberAccess(ClassMemberAccessAST* node) { m_size += sizeof(ClassMemberAccessAST); DefaultVisitor::visitClassMemberAccess(node); }
  virtual void visitClassSpecifier(ClassSpecifierAST* node) { m_size += sizeof(ClassSpecifierAST); DefaultVisitor::visitClassSpecifier(node); }
  virtual void visitCompoundStatement(CompoundStatementAST* node) { m_size += sizeof(CompoundStatementAST); DefaultVisitor::visitCompoundStatement(node); }
  virtual void visitCondition(ConditionAST* node) { m_size += sizeof(ConditionAST); DefaultVisitor::visitCondition(node); }
  virtual void visitConditionalExpression(ConditionalExpressionAST* node) { m_size += sizeof(ConditionalExpressionAST); DefaultVisitor::visitConditionalExpression(node); }
  virtual void visitCppCastExpression(CppCastExpressionAST* node) { m_size += sizeof(CppCastExpressionAST); DefaultVisitor::visitCppCastExpression(node); }
  virtual void visitCtorInitializer(CtorInitializerAST* node) { m_size += sizeof(CtorInitializerAST); DefaultVisitor::visitCtorInitializer(node); }
  virtual void visitDeclarationStatement(DeclarationStatementAST* node) { m_size += sizeof(DeclarationStatementAST); DefaultVisitor::visitDeclarationStatement(node); }
  virtual void visitDeclarator(DeclaratorAST* node) { m_size += sizeof(DeclaratorAST); DefaultVisitor::visitDeclarator(node); }
  virtual void visitDeleteExpression(DeleteExpressionAST* node) { m_size += sizeof(DeleteExpressionAST); DefaultVisitor::visitDeleteExpression(node); }
  virtual void visitDoStatement(DoStatementAST* node) { m_size += sizeof(DoStatementAST); DefaultVisitor::visitDoStatement(node); }
  virtual void visitElaboratedTypeSpecifier(ElaboratedTypeSpecifierAST* node) { m_size += sizeof(ElaboratedTypeSpecifierAST); DefaultVisitor::visitElaboratedTypeSpecifier(node); }
  virtual void visitEnumerator(EnumeratorAST* node) { m_size += sizeof(EnumeratorAST); DefaultVisitor::visitEnumerator(node); }
  virtual void visitEnumSpecifier(EnumSpecifierAST* node) { m_size += sizeof(EnumSpecifierAST); DefaultVisitor::visitEnumSpecifier(node); }
  virtual void visitExceptionSpecification(ExceptionSpecificationAST* node) { m_size += sizeof(ExceptionSpecificationAST); DefaultVisitor::visitExceptionSpecification(node); }
  virtual void visitExpressionOrDeclarationStatement(ExpressionOrDeclarationStatementAST* node) { m_size += sizeof(ExpressionOrDeclarationStatementAST); DefaultVisitor::visitExpressionOrDeclarationStatement(node); }
  virtual void visitExpressionStatement(ExpressionStatementAST* node) { m_size += sizeof(ExpressionStatementAST); DefaultVisitor::visitExpressionStatement(node); }
  virtual void visitForRangeDeclaration(ForRangeDeclarationAst* node) { m_size += sizeof(ForRangeDeclarationAst); DefaultVisitor::visitForRangeDeclaration(node); }
  virtual void visitForStatement(ForStatementAST* node) { m_size += sizeof(ForStatementAST); DefaultVisitor::visitForStatement(node); }
  virtual void visitFunctionCall(FunctionCallAST* node) { m_size += sizeof(FunctionCallAST); DefaultVisitor::visitFunctionCall(node); }
  virtual void visitFunctionDefinition(FunctionDefinitionAST* node) { m_size += sizeof(FunctionDefinitionAST); DefaultVisitor::visitFunctionDefinition(node); }
  virtual void visitIfStatement(IfStatementAST* node) { m_size += sizeof(IfStatementAST); DefaultVisitor::visitIfStatement(node); }
  virtual void visitIncrDecrExpression(IncrDecrExpressionAST* node) { m_size += sizeof(IncrDecrExpressionAST); DefaultVisitor::visitIncrDecrExpression(node); }
  virtual void visitInitDeclarator(InitDeclaratorAST* node) { m_size += sizeof(InitDeclaratorAST); DefaultVisitor::visitInitDeclarator(node); }
  virtual void visitInitializer(InitializerAST* node) { m_size += sizeof(InitializerAST); DefaultVisitor::visitInitializer(node); }
  virtual void visitInitializerClause(InitializerClauseAST* node) { m_size += sizeof(InitializerClauseAST); DefaultVisitor::visitInitializerClause(node); }
  virtual void visitInitializerList(InitializerListAST* node) { m_size += sizeof(InitializerListAST); DefaultVisitor::visitInitializerList(node); }
  virtual void visitJumpStatement(JumpStatementAST* node) { m_size += sizeof(JumpStatementAST); DefaultVisitor::visitJumpStatement(node); }
  virtual void visitLabeledStatement(LabeledStatementAST* node) { m_size += sizeof(LabeledStatementAST); DefaultVisitor::visitLabeledStatement(node); }
  virtual void visitLambdaCapture(LambdaCaptureAST* node) { m_size += sizeof(LambdaCaptureAST); DefaultVisitor::visitLambdaCapture(node); }
  virtual void visitLambdaDeclarator(LambdaDeclaratorAST* node) { m_size += sizeof(LambdaDeclaratorAST); DefaultVisitor::visitLambdaDeclarator(node); }
  virtual void visitLambdaExpression(LambdaExpressionAST* node) { m_size += sizeof(LambdaExpressionAST); DefaultVisitor::visitLambdaExpression(node); }
  virtual void visitLinkageBody(LinkageBodyAST* node) { m_size += sizeof(LinkageBodyAST); DefaultVisitor::visitLinkageBody(node); }
  virtual void visitLinkageSpecification(LinkageSpecificationAST* node) { m_size += sizeof(LinkageSpecificationAST); DefaultVisitor::visitLinkageSpecification(node); }
  virtual void visitMemInitializer(MemInitializerAST* node) { m_size += sizeof(MemInitializerAST); DefaultVisitor::visitMemInitializer(node); }
  virtual void visitName(NameAST* node) { m_size += sizeof(NameAST); DefaultVisitor::visitName(node); }
  virtual void visitNamespace(NamespaceAST* node) { m_size += sizeof(NamespaceAST); DefaultVisitor::visitNamespace(node); }
  virtual void visitNamespaceAliasDefinition(NamespaceAliasDefinitionAST* node) { m_size += sizeof(NamespaceAliasDefinitionAST); DefaultVisitor::visitNamespaceAliasDefinition(node); }
  virtual void visitNewDeclarator(NewDeclaratorAST* node) { m_size += sizeof(NewDeclaratorAST); DefaultVisitor::visitNewDeclarator(node); }
  virtual void visitNewExpression(NewExpressionAST* node) { m_size += sizeof(NewExpressionAST); DefaultVisitor::visitNewExpression(node); }
  virtual void visitNewInitializer(NewInitializerAST* node) { m_size += sizeof(NewInitializerAST); DefaultVisitor::visitNewInitializer(node); }
  virtual void visitNewTypeId(NewTypeIdAST* node) { m_size += sizeof(NewTypeIdAST); DefaultVisitor::visitNewTypeId(node); }
  virtual void visitOperator(OperatorAST* node) { m_size += sizeof(OperatorAST); DefaultVisitor::visitOperator(node); }
  virtual void visitParameterDeclaration(ParameterDeclarationAST* node) { m_size += sizeof(ParameterDeclarationAST); DefaultVisitor::visitParameterDeclaration(node); }
  virtual void visitOperatorFunctionId(OperatorFunctionIdAST* node) { m_size += sizeof(OperatorFunctionIdAST); DefaultVisitor::visitOperatorFunctionId(node); }
  virtual void visitParameterDeclarationClause(ParameterDeclarationClauseAST* node) { m_size += sizeof(ParameterDeclarationClauseAST); DefaultVisitor::visitParameterDeclarationClause(node); }
  virtual void visitPostfixExpression(PostfixExpressionAST* node) { m_size += sizeof(PostfixExpressionAST); DefaultVisitor::visitPostfixExpression(node); }
  virtual void visitPrimaryExpression(PrimaryExpressionAST* node) { m_size += sizeof(PrimaryExpressionAST); DefaultVisitor::visitPrimaryExpression(node); }
  virtual void visitPtrOperator(PtrOperatorAST* node) { m_size += sizeof(PtrOperatorAST); DefaultVisitor::visitPtrOperator(node); }
  virtual void visitPtrToMember(PtrToMemberAST* node) { m_size += sizeof(PtrToMemberAST); DefaultVisitor::visitPtrToMember(node); }
  virtual void visitQPropertyDeclaration(QPropertyDeclarationAST* node) { m_size += sizeof(QPropertyDeclarationAST); DefaultVisitor::visitQPropertyDeclaration(node); }
  virtual void visitReturnStatement(ReturnStatementAST* node) { m_size += sizeof(ReturnStatementAST); DefaultVisitor::visitReturnStatement(node); }
  virtual void visitSignalSlotExpression(SignalSlotExpressionAST* node) { m_size += sizeof(SignalSlotExpressionAST); DefaultVisitor::visitSignalSlotExpression(node); }
  virtual void visitSimpleDeclaration(SimpleDeclarationAST* node) { m_size += sizeof(SimpleDeclarationAST); DefaultVisitor::visitSimpleDeclaration(node); }
  virtual void visitSimpleTypeSpecifier(SimpleTypeSpecifierAST* node) { m_size += sizeof(SimpleTypeSpecifierAST); DefaultVisitor::visitSimpleTypeSpecifier(node); }
  virtual void visitSizeofExpression(SizeofExpressionAST* node) { m_size += sizeof(SizeofExpressionAST); DefaultVisitor::visitSizeofExpression(node); }
  virtual void visitStaticAssert(StaticAssertAST* node) { m_size += sizeof(StaticAssertAST); DefaultVisitor::visitStaticAssert(node); }
  virtual void visitStringLiteral(StringLiteralAST* node) { m_size += sizeof(StringLiteralAST); DefaultVisitor::visitStringLiteral(node); }
  virtual void visitSubscriptExpression(SubscriptExpressionAST* node) { m_size += sizeof(SubscriptExpressionAST); DefaultVisitor::visitSubscriptExpression(node); }
  virtual void visitSwitchStatement(SwitchStatementAST* node) { m_size += sizeof(SwitchStatementAST); DefaultVisitor::visitSwitchStatement(node); }
  virtual void visitTemplateArgument(TemplateArgumentAST* node) { m_size += sizeof(TemplateArgumentAST); DefaultVisitor::visitTemplateArgument(node); }
  virtual void visitTemplateDeclaration(TemplateDeclarationAST* node) { m_size += sizeof(TemplateDeclarationAST); DefaultVisitor::visitTemplateDeclaration(node); }
  virtual void visitTemplateParameter(TemplateParameterAST* node) { m_size += sizeof(TemplateParameterAST); DefaultVisitor::visitTemplateParameter(node); }
  virtual void visitThrowExpression(ThrowExpressionAST* node) { m_size += sizeof(ThrowExpressionAST); DefaultVisitor::visitThrowExpression(node); }
  virtual void visitTrailingReturnType(TrailingReturnTypeAST* node) { m_size += sizeof(TrailingReturnTypeAST); DefaultVisitor::visitTrailingReturnType(node); }
  virtual void visitTranslationUnit(TranslationUnitAST* node) { m_size += sizeof(TranslationUnitAST); DefaultVisitor::visitTranslationUnit(node); }
  virtual void visitTryBlockStatement(TryBlockStatementAST* node) { m_size += sizeof(TryBlockStatementAST); DefaultVisitor::visitTryBlockStatement(node); }
  virtual void visitTypedef(TypedefAST* node) { m_size += sizeof(TypedefAST); DefaultVisitor::visitTypedef(node); }
  virtual void visitTypeIdentification(TypeIdentificationAST* node) { m_size += sizeof(TypeIdentificationAST); DefaultVisitor::visitTypeIdentification(node); }
  virtual void visitTypeId(TypeIdAST* node) { m_size += sizeof(TypeIdAST); DefaultVisitor::visitTypeId(node); }
  virtual void visitTypeIDOperator(TypeIDOperatorAST* node) { m_size += sizeof(TypeIDOperatorAST); DefaultVisitor::visitTypeIDOperator(node); }
  virtual void visitTypeParameter(TypeParameterAST* node) { m_size += sizeof(TypeParameterAST); DefaultVisitor::visitTypeParameter(node); }
  virtual void visitUnaryExpression(UnaryExpressionAST* node) { m_size += sizeof(UnaryExpressionAST); DefaultVisitor::visitUnaryExpression(node); }
  virtual void visitUnqualifiedName(UnqualifiedNameAST* node) { m_size += sizeof(UnqualifiedNameAST); DefaultVisitor::visitUnqualifiedName(node); }
  virtual void visitUsing(UsingAST* node) { m_size += sizeof(UsingAST); DefaultVisitor::visitUsing(node); }
  virtual void visitUsingDirective(UsingDirectiveAST* node) { m_size += sizeof(UsingDirectiveAST); DefaultVisitor::visitUsingDirective(node); }
  virtual void visitWhileStatement(WhileStatementAST* node) { m_size += sizeof(WhileStatementAST); DefaultVisitor::visitWhileStatement(node); }
  virtual void visitWinDeclSpec(WinDeclSpecAST* node) { m_size += sizeof(WinDeclSpecAST); DefaultVisitor::visitWinDeclSpec(node); }

  quint64 size() const
  {
    return m_size;
  }
private:
  quint64 m_size;
};

class CppParser {
public:
    CppParser(const bool printAst, const bool printTokens)
      : m_printAst(printAst), m_printTokens(printTokens)
    {
    }

    /// parse contents of a file
    void parseFile( const QString &fileName )
    {
      m_session.setUrl(IndexedString(fileName));
      rpp::Preprocessor preprocessor;
      rpp::pp pp(&preprocessor);
      m_session.setContentsAndGenerateLocationTable(pp.processFile(fileName));
      runSession();
    }

    /// parse code directly
    void parseCode( const QString &code )
    {
      qout << "Parsing input" << endl;
      m_session.setUrl(IndexedString("-"));

      rpp::Preprocessor preprocessor;
      rpp::pp pp(&preprocessor);
      m_session.setContentsAndGenerateLocationTable(pp.processFile("anonymous", code.toUtf8()));
      runSession();
    }

private:
    /**
     * actually run the parse session
     */
    void runSession()
    {
      Control control;
      Parser parser(&control);
      TranslationUnitAST* ast = parser.parse(&m_session);

      if (m_printTokens) {
        if ( !m_session.token_stream || m_session.token_stream->isEmpty() ) {
          qout << "empty token stream";
        } else {
          qout << "token stream:" << endl;
          for(int i = 0; i < m_session.token_stream->count(); ++i) {
            const Token& t = m_session.token_stream->at(i);
            const QString str = m_session.token_stream->symbolString(t);
            Q_ASSERT(t.size || str.isEmpty());
            qout << token_name(t.kind) << ": " << str << endl;
          }
        }
      }

      if (!ast) {
        qerr << "no AST tree could be generated" << endl;
      } else {
        ast->session = &m_session;
        qout << "AST tree successfully generated" << endl;
        if (m_printAst) {
          DumpTree dumper;
          dumper.dump(ast, m_session.token_stream, true);
        }
      }
      if (!control.problems().isEmpty()) {
        qout << endl << "problems encountered during parsing:" << endl;
        foreach(KDevelop::ProblemPointer p, control.problems()) {
          qout << p->toString() << endl;
        }
      } else {
        qout << "no problems encountered during parsing" << endl;
      }

      qout << "contents vector size: " << m_session.contentsVector().size() << endl;
      qout << "mempool size: " << m_session.mempool->size() << endl;
      MemSizeVisitor visitor;
      if (ast) {
        visitor.visit(ast);
        qout << "actual AST size: " << visitor.size() << endl;
      }

      if (!ast) {
        exit(255);
      }
    }

    ParseSession m_session;
    const bool m_printAst;
    const bool m_printTokens;
};

int main(int argc, char* argv[])
{
    KAboutData aboutData( "cpp-parser", 0, ki18n( "cpp-parser" ),
                          "1", ki18n("KDevelop CPP parser debugging utility"), KAboutData::License_GPL,
                          ki18n( "2011 Milian Wolff" ), KLocalizedString(), "http://www.kdevelop.org" );

    return KDevelopUtils::initAndRunParser<CppParser>(aboutData, argc, argv);
}
