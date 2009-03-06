/*
   Copyright 2008 Hamish Rodda <rodda@kde.org>

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

#ifndef ASTCHANGE_H
#define ASTCHANGE_H

#include <language/codegen/astchangeset.h>

#include "ast.h"

class AccessSpecifierChange;
class AsmDefinitionChange;
class BaseClauseChange;
class BaseSpecifierChange;
class BinaryExpressionChange;
class CastExpressionChange;
class ClassMemberAccessChange;
class ClassSpecifierChange;
class CompoundStatementChange;
class ConditionChange;
class ConditionalExpressionChange;
class CppCastExpressionChange;
class CtorInitializerChange;
class DeclarationChange;
class DeclarationStatementChange;
class DeclaratorChange;
class DeleteExpressionChange;
class DoStatementChange;
class ElaboratedTypeSpecifierChange;
class EnumSpecifierChange;
class EnumeratorChange;
class ExceptionSpecificationChange;
class ExpressionChange;
class ExpressionOrDeclarationStatementChange;
class ExpressionStatementChange;
class ForStatementChange;
class FunctionCallChange;
class FunctionDefinitionChange;
class IfStatementChange;
class IncrDecrExpressionChange;
class InitDeclaratorChange;
class InitializerChange;
class InitializerClauseChange;
class JumpStatementChange;
class LabeledStatementChange;
class LinkageBodyChange;
class LinkageSpecificationChange;
class MemInitializerChange;
class NameChange;
class NamespaceChange;
class NamespaceAliasDefinitionChange;
class NewDeclaratorChange;
class NewExpressionChange;
class NewInitializerChange;
class NewTypeIdChange;
class OperatorChange;
class OperatorFunctionIdChange;
class ParameterDeclarationChange;
class ParameterDeclarationClauseChange;
class PostfixExpressionChange;
class PrimaryExpressionChange;
class PtrOperatorChange;
class PtrToMemberChange;
class ReturnStatementChange;
class SimpleDeclarationChange;
class SimpleTypeSpecifierChange;
class SizeofExpressionChange;
class StatementChange;
class StringLiteralChange;
class SubscriptExpressionChange;
class SwitchStatementChange;
class SignalSlotExpressionChange;
class TemplateArgumentChange;
class TemplateDeclarationChange;
class TemplateParameterChange;
class ThrowExpressionChange;
class TranslationUnitChange;
class TryBlockStatementChange;
class CatchStatementChange;
class TypeIdChange;
class TypeIdentificationChange;
class TypeParameterChange;
class TypeSpecifierChange;
class TypedefChange;
class UnaryExpressionChange;
class UnqualifiedNameChange;
class UsingChange;
class UsingDirectiveChange;
class WhileStatementChange;
class WinDeclSpecChange;

class Change
{
};

template <typename BaseChange, typename AstNode, typename AstChange>
class ListChange
{
public:
  ListChange(BaseChange* change);

  bool hasChanges();

  int count() const;
  AstNode* nodeAt(int index) const;
  AstChange* changeAt(int index) const;

  void clear();
  void insert(AstChange* change, int index);
  void append(AstChange* change);
  AstChange* change(int index);
  void remove(AstNode* node);
  void remove(AstChange* change);

private:
  BaseChange* m_change;
};

class UnqualifiedNameChange : public Change, public KDevelop::AstNodeRef<UnqualifiedNameAST>
{
  void setTilde(bool hasTilde);
  void setId(bool hasId);
  //OperatorFunctionIdChange* operatorId();
  //ListChange<UnqualifiedNameChange, TemplateArgumentAST, TemplateArgumentChange> templateArguments;
};

class NameChange : public Change, public KDevelop::AstNodeRef<NameAST>
{
  void setGlobal(bool global);
  ListChange<NameChange, UnqualifiedNameAST, UnqualifiedNameChange> qualifiedNames;
  UnqualifiedNameChange* changeUnqualifiedName();
};

class BaseClauseChange : public Change, public KDevelop::AstNodeRef<BaseClauseAST>
{
public:
  ListChange<BaseClauseChange, BaseSpecifierAST, BaseSpecifierChange> baseSpecifiers;
};

class BaseSpecifierChange : public Change, public KDevelop::AstNodeRef<BaseSpecifierAST>
{
public:
  bool isVirtual() const;
  void setVirtual(bool isVirtual);

  int accessSpecifier() const;
  void setAccessSpecifier(int token);

  NameChange* changeName();
};

class DeclarationChange
{
public:
  DeclarationChange();

  /*AccessSpecifierChange* accessSpecifier();
  AsmDefinitionChange* asmDefinition();
  FunctionDefinitionChange* functionDefinition();
  LinkageSpecificationChange* linkageSpecification();
  NamespaceChange* namespaceChange();
  NamespaceAliasDefinitionChange* namespaceAliasDefinition();
  SimpleDeclarationChange* simpleDeclaration();
  TemplateDeclarationChange* templateDeclaration();
  TypedefChange* typedefChange();
  UsingChange* usingChange();*/
};

class ClassSpecifierChange : public Change, public KDevelop::AstNodeRef<ClassSpecifierAST>
{
public:
  //WinDeclSpecChange* changeWinDeclSpec();
  void newClassKey(std::size_t class_key);

  NameChange* changeName();
  BaseClauseChange* changeBaseClause();

  ListChange<ClassSpecifierChange, DeclarationAST, DeclarationChange> members;
};

#endif // ASTCHANGE_H
