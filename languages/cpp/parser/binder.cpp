/* This file is part of KDevelop
    Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>

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

#include "binder.h"
#include "symbol.h"
#include "codemodel_finder.h"
#include "class_compiler.h"
#include "compiler_utils.h"
#include "tokens.h"

#include <iostream>

#include <qdebug.h>

#include <ktexteditor/smartrange.h>
#include <ktexteditor/smartinterface.h>
#include <ktexteditor/document.h>

Binder::Binder(CodeModel *model,
               TokenStream *token_stream,
               Lexer *lexer,
               KTextEditor::SmartRange* highlight)
  : _M_model(model),
    _M_token_stream(token_stream),
    _M_lexer(lexer),
    _M_highlight(highlight),
    type_cc(token_stream),
    name_cc(token_stream),
    decl_cc(token_stream)
{
  // generalize me
  _M_qualified_types.insert(QString::fromUtf8("bool"));
  _M_qualified_types.insert(QString::fromUtf8("char"));
  _M_qualified_types.insert(QString::fromUtf8("double"));
  _M_qualified_types.insert(QString::fromUtf8("float"));
  _M_qualified_types.insert(QString::fromUtf8("int"));
  _M_qualified_types.insert(QString::fromUtf8("long"));
  _M_qualified_types.insert(QString::fromUtf8("short"));
  _M_qualified_types.insert(QString::fromUtf8("void"));
}

Binder::~Binder()
{
}

void Binder::run( const KUrl &url, AST *node)
{
  _M_currentFile = url.url();
  _M_global_namespace = model()->globalNamespace();
  _M_current_access = CodeModel::Public;
  visit(node);
}

ScopeModelItem Binder::currentScope()
{
  if (_M_current_class)
    return model_static_cast<ScopeModelItem>(_M_current_class);
  else if (_M_current_namespace)
    return model_static_cast<ScopeModelItem>(_M_current_namespace);

  return model_static_cast<ScopeModelItem>(_M_global_namespace);
}

CodeModel::AccessPolicy Binder::changeCurrentAccess(CodeModel::AccessPolicy accessPolicy)
{
  CodeModel::AccessPolicy old = accessPolicy;
  _M_current_access = accessPolicy;
  return old;
}

NamespaceModelItem Binder::changeCurrentNamespace(NamespaceModelItem item)
{
  NamespaceModelItem old = _M_current_namespace;
  _M_current_namespace = item;
  return old;
}

ClassModelItem Binder::changeCurrentClass(ClassModelItem item)
{
  ClassModelItem old = _M_current_class;
  _M_current_class = item;
  return old;
}

TemplateModelItem Binder::changeCurrentTemplate(TemplateModelItem item)
{
  TemplateModelItem old = _M_current_template;
  _M_current_template = item;
  return old;
}

FunctionDefinitionModelItem Binder::changeCurrentFunction(FunctionDefinitionModelItem item)
{
  FunctionDefinitionModelItem old = _M_current_function;
  _M_current_function = item;
  return old;
}

int Binder::decode_token(std::size_t index) const
{
  return _M_token_stream->kind(index);
}

CodeModel::AccessPolicy Binder::decode_access_policy(std::size_t index) const
{
  switch (decode_token(index))
    {
      case Token_class:
        return CodeModel::Private;

      case Token_struct:
      case Token_union:
        return CodeModel::Public;

      default:
        return CodeModel::Public;
    }
}

CodeModel::ClassType Binder::decode_class_type(std::size_t index) const
{
  switch (decode_token(index))
    {
      case Token_class:
        return CodeModel::Class;
      case Token_struct:
        return CodeModel::Struct;
      case Token_union:
        return CodeModel::Union;
      default:
//         std::cerr << "** WARNING unrecognized class type" << std::endl;
        break;
    }
    return CodeModel::Class;
}

const NameSymbol *Binder::decode_symbol(std::size_t index) const
{
  return _M_token_stream->symbol(index);
}

void Binder::visitAccessSpecifier(AccessSpecifierAST *node)
{
  const ListNode<std::size_t> *it =  node->specs;
  if (it == 0)
    return;

  it = it->toFront();
  const ListNode<std::size_t> *end = it;

  do
    {
      switch (decode_token(it->element))
        {
          default:
            break;

          case Token_public:
            changeCurrentAccess(CodeModel::Public);
            break;
          case Token_protected:
            changeCurrentAccess(CodeModel::Protected);
            break;
          case Token_private:
            changeCurrentAccess(CodeModel::Private);
            break;
        }
      it = it->next;
    }
  while (it != end);
}

void Binder::visitSimpleDeclaration(SimpleDeclarationAST *node)
{
  visit(node->type_specifier);

  if (const ListNode<InitDeclaratorAST*> *it = node->init_declarators)
    {
      it = it->toFront();
      const ListNode<InitDeclaratorAST*> *end = it;
      do
        {
          InitDeclaratorAST *init_declarator = it->element;
          DeclaratorAST *declarator = init_declarator->declarator;

          if (!declarator || (declarator && declarator->sub_declarator) || !declarator->id)
            {
#if defined(__GNUC__)
#warning "Binder::visitSimpleDeclaration() -- implement me"
#endif
//               std::cerr << "** WARNING skip sub declarators or anonymous symbol" << std::endl;
            }
          else
            {
              declare_symbol(node, init_declarator);
            }

          it = it->next;
        }
      while (it != end);
    }
}

void Binder::declare_symbol(SimpleDeclarationAST *node, InitDeclaratorAST *init_declarator)
{
  DeclaratorAST *declarator = init_declarator->declarator;
  NameAST *id = declarator->id;

  CodeModelFinder finder(model(), _M_token_stream);
  ScopeModelItem symbolScope = finder.resolveScope(id, currentScope());
  if (! symbolScope)
    {
      name_cc.run(id);
//           std::cerr << "** WARNING scope not found for symbol:"
//         << qPrintable(name_cc.name()) << std::endl;
      return;
    }

  decl_cc.run(declarator);

  if (decl_cc.isFunction())
    {
      name_cc.run(id->unqualified_name);

      FunctionModelItem fun = model()->create<FunctionModelItem>();
      fun->setAccessPolicy(_M_current_access);
      fun->setName(name_cc.name());

      setPositionAt( fun, id->unqualified_name );

      fun->setAbstract(init_declarator->initializer != 0);
      fun->setConstant(declarator->fun_cv != 0);
      applyStorageSpecifiers(node->storage_specifiers, model_static_cast<MemberModelItem>(fun));
      applyFunctionSpecifiers(node->function_specifiers, fun);

      // build the type
      TypeInfo typeInfo = CompilerUtils::typeDescription(node->type_specifier,
                                                         declarator,
                                                         _M_token_stream);
      fun->setType(typeInfo);

      // ... and the signature
      foreach (DeclaratorCompiler::Parameter p, decl_cc.parameters())
        {
          ArgumentModelItem arg = model()->create<ArgumentModelItem>();
          arg->setType(qualifyType(p.type, _M_context));
          arg->setName(p.name);
          arg->setDefaultValue(p.defaultValue);
          fun->addArgument(arg);
        }

      fun->setScope(symbolScope->qualifiedName());
      symbolScope->addFunction(fun);
    }
  else
    {
      VariableModelItem var = model()->create<VariableModelItem>();
      var->setAccessPolicy(_M_current_access);
      name_cc.run(id->unqualified_name);
      var->setName(name_cc.name());
      TypeInfo typeInfo = CompilerUtils::typeDescription(node->type_specifier,
                                                         declarator,
                                                         _M_token_stream);
      var->setType(qualifyType(typeInfo, _M_context));
      applyStorageSpecifiers(node->storage_specifiers, model_static_cast<MemberModelItem>(var));

      var->setScope(symbolScope->qualifiedName());
      setPositionAt( var, id->unqualified_name );
      symbolScope->addVariable(var);
    }
}

void Binder::visitFunctionDefinition(FunctionDefinitionAST *node)
{
  Q_ASSERT(node->init_declarator != 0);

  ScopeModelItem scope = currentScope();

  InitDeclaratorAST *init_declarator = node->init_declarator;
  DeclaratorAST *declarator = init_declarator->declarator;

  CodeModelFinder finder(model(), _M_token_stream);

  ScopeModelItem functionScope = finder.resolveScope(declarator->id, scope);
  if (! functionScope)
    {
      name_cc.run(declarator->id);
//       std::cerr << "** WARNING scope not found for function definition:"
//                 << qPrintable(name_cc.name()) << std::endl
//                 << "\tdefinition *ignored*"
//                 << std::endl;
      return;
    }

  decl_cc.run(declarator);

//   Q_ASSERT(! decl_cc.id().isEmpty());

  FunctionDefinitionModelItem
    old = changeCurrentFunction(model()->create<FunctionDefinitionModelItem>());
  _M_current_function->setScope(functionScope->qualifiedName());

  Q_ASSERT(declarator->id->unqualified_name != 0);
  name_cc.run(declarator->id->unqualified_name);
  QString unqualified_name = name_cc.name();

  _M_current_function->setName(unqualified_name);
  TypeInfo tmp_type = CompilerUtils::typeDescription(node->type_specifier,
                                                     declarator, _M_token_stream);
  _M_current_function->setType(qualifyType(tmp_type, _M_context));
  _M_current_function->setAccessPolicy(_M_current_access);
  _M_current_function->setConstant(declarator->fun_cv != 0);

  applyFunctionSpecifiers(node->function_specifiers,
                          model_static_cast<FunctionModelItem>(_M_current_function));

  foreach (DeclaratorCompiler::Parameter p, decl_cc.parameters())
    {
      ArgumentModelItem arg = model()->create<ArgumentModelItem>();
      arg->setType(qualifyType(p.type, functionScope->qualifiedName()));
      arg->setName(p.name);
      arg->setDefaultValue(p.defaultValue);
      _M_current_function->addArgument(arg);
    }

  setPositionAt( _M_current_function, declarator->id->unqualified_name );
  functionScope->addFunctionDefinition(_M_current_function);

  FunctionModelItem prototype = model_static_cast<FunctionModelItem>(_M_current_function);

  // try to find a function declaration for this definition..
  if (! functionScope->declaredFunction(prototype))
    {
      //functionScope->addFunction(prototype);
    }

  changeCurrentFunction(old);
}
#if defined(__GNUC__)

#endif
void Binder::visitTemplateDeclaration(TemplateDeclarationAST * /* node */)
{
#if defined(__GNUC__)
#warning "Binder::visitTemplateDeclaration() -- implement me"
#endif

#if 0 // ### skip the template declarations
  TemplateModelItem
    old = changeCurrentTemplate(model()->create<TemplateModelItem>());
  DefaultVisitor::visitTemplateDeclaration(node);
  changeCurrentTemplate(old);
#endif
}

void Binder::visitTypedef(TypedefAST *node)
{
  DefaultVisitor::visitTypedef(node);
}

void Binder::visitNamespace(NamespaceAST *node)
{
  bool anonymous = (node->namespace_name == 0);

  ScopeModelItem scope = currentScope();
  NamespaceModelItem old /*= changeCurrentNamespace(_M_global_namespace)*/;
  if (! anonymous)
    {
      QString name = decode_symbol(node->namespace_name)->as_string();

      QStringList qualified_name = scope->qualifiedName();
      qualified_name += name;
      NamespaceModelItem ns =
        model_safe_cast<NamespaceModelItem>(model()->findItem(qualified_name,
          _M_global_namespace->toItem()));
      if (!ns || ns == _M_global_namespace )
        {
          ns = model()->create<NamespaceModelItem>();
          ns->setName(name);
          ns->setScope(scope->qualifiedName());
        }
      old = changeCurrentNamespace(ns);

      _M_context.append(name);
    }

  DefaultVisitor::visitNamespace(node);

  // Mark namespace name
  /*if (KTextEditor::SmartRange* range = newRange(node->namespace_name))
    _M_current_namespace->addReference(range);*/

  if (! anonymous)
    {
      Q_ASSERT(scope->kind() == _CodeModelItem::Kind_Namespace
               || scope->kind() == _CodeModelItem::Kind_File);

      _M_context.removeLast();

      if (NamespaceModelItem ns = model_static_cast<NamespaceModelItem>(scope))
        {
          if ( ns != _M_current_namespace )
          {
            setPositionAt( _M_current_namespace, node );
            ns->addNamespace(_M_current_namespace);
          }
        }

      changeCurrentNamespace(old);
    }
}

void Binder::visitClassSpecifier(ClassSpecifierAST *node)
{
  ClassCompiler class_cc(_M_token_stream);
  class_cc.run(node);

  if (class_cc.name().isEmpty())
    {
      // anonymous not supported
      return;
    }

  Q_ASSERT(node->name != 0 && node->name->unqualified_name != 0);

  ScopeModelItem scope = currentScope();

  ClassModelItem old = changeCurrentClass(model()->create<ClassModelItem>());
  _M_current_class->setName(class_cc.name());
  _M_current_class->setBaseClasses(class_cc.baseClasses());
  _M_current_class->setClassType(decode_class_type(node->class_key));

  CodeModel::AccessPolicy oldAccessPolicy = changeCurrentAccess(decode_access_policy(node->class_key));

  _M_current_class->setScope(scope->qualifiedName());
  setPositionAt( _M_current_class, node->name->unqualified_name );
  scope->addClass(_M_current_class);

  name_cc.run(node->name->unqualified_name);
  _M_context.append(name_cc.name());
  visitNodes(this, node->member_specs);
  _M_context.removeLast();

  changeCurrentClass(old);
  changeCurrentAccess(oldAccessPolicy);
}

void Binder::visitLinkageSpecification(LinkageSpecificationAST *node)
{
  DefaultVisitor::visitLinkageSpecification(node);
}

void Binder::visitUsing(UsingAST *node)
{
  DefaultVisitor::visitUsing(node);
}

void Binder::visitEnumSpecifier(EnumSpecifierAST *node)
{
  CodeModelFinder finder(model(), _M_token_stream);
  ScopeModelItem scope = currentScope();
  ScopeModelItem enumScope = finder.resolveScope(node->name, scope);

  name_cc.run(node->name);
  QString name = name_cc.name();

  if (name.isEmpty())
    {
      // anonymous enum
      static int N = 0;
      name = QLatin1String("$$enum_");
      name += QString::number(++N);
    }

  _M_current_enum = model()->create<EnumModelItem>();
  _M_current_enum->setName(name);
  _M_current_enum->setScope(enumScope->qualifiedName());
  _M_qualified_types.insert(_M_current_enum->qualifiedName().join("."));
  if (node->name)
    setPositionAt( _M_current_enum, node->name );
  enumScope->addEnum(_M_current_enum);

  DefaultVisitor::visitEnumSpecifier(node);

  _M_current_enum = 0;
}

void Binder::visitEnumerator(EnumeratorAST *node)
{
  Q_ASSERT(_M_current_enum != 0);
  EnumeratorModelItem e = model()->create<EnumeratorModelItem>();
  e->setName(decode_symbol(node->id)->as_string());

  if (ExpressionAST *expr = node->expression)
    {
      const Token &start_token = _M_token_stream->token(expr->start_token);
      const Token &end_token = _M_token_stream->token(expr->end_token);

      e->setValue(QString::fromUtf8(&start_token.text[start_token.position],
                                    end_token.position - start_token.position).trimmed());
    }

  setPositionAt( _M_current_enum, node );
  _M_current_enum->addEnumerator(e);
}

void Binder::visitUsingDirective(UsingDirectiveAST *node)
{
  DefaultVisitor::visitUsingDirective(node);
}

void Binder::applyStorageSpecifiers(const ListNode<std::size_t> *it, MemberModelItem item)
{
  if (it == 0)
    return;

  it = it->toFront();
  const ListNode<std::size_t> *end = it;

  do
    {
      switch (decode_token(it->element))
        {
          default:
            break;

          case Token_friend:
            item->setFriend(true);
            break;
          case Token_auto:
            item->setAuto(true);
            break;
          case Token_register:
            item->setRegister(true);
            break;
          case Token_static:
            item->setStatic(true);
            break;
          case Token_extern:
            item->setExtern(true);
            break;
          case Token_mutable:
            item->setMutable(true);
            break;
        }
      it = it->next;
    }
  while (it != end);
}

void Binder::applyFunctionSpecifiers(const ListNode<std::size_t> *it, FunctionModelItem item)
{
  if (it == 0)
    return;

  it = it->toFront();
  const ListNode<std::size_t> *end = it;

  do
    {
      switch (decode_token(it->element))
        {
          default:
            break;

          case Token_inline:
            item->setInline(true);
            break;

          case Token_virtual:
            item->setVirtual(true);
            break;

          case Token_explicit:
            item->setExplicit(true);
            break;
        }
      it = it->next;
    }
  while (it != end);
}

TypeInfo Binder::qualifyType(const TypeInfo &type, const QStringList &context) const
{
  // ### Potentially improve to use string list in the name table to
  // avoid all those splits/joins

  QString type_name = type.qualifiedName().join(".");

  if (_M_qualified_types.contains(type_name))
    return type;

  // ### will break when using keyword is used.
  for (int i=context.size() - 1; i >= 0; --i)
    {
      QString tmp_key;
      for (int j=0; j<=i; ++j)
        tmp_key += context.at(j) + QLatin1String(".");
      tmp_key += type_name;
      if (_M_qualified_types.contains(tmp_key)) {
        TypeInfo tmp_type_info = type;
        tmp_type_info.setQualifiedName(tmp_key.split(QLatin1Char('.')));
        return tmp_type_info;
      }
    }

//   std::cerr << "** WARNING unable to qualify type: "
//             << "'" << qPrintable(type.toString()) << "'" << std::endl;
  return type;
}

void Binder::setPositionAt(_CodeModelItem *item, AST *ast)
{
  QString fileName;
  int startLine, startColumn;
  int endLine, endColumn;

  const Token &start_token =
    _M_token_stream->token(ast->start_token);
  const Token &end_token =
    _M_token_stream->token(ast->end_token);

  _M_lexer->positionAt(start_token.position,
                       &startLine, &startColumn, &fileName);
  _M_lexer->positionAt(end_token.position,
                       &endLine, &endColumn, &fileName);

  item->setFileName(fileName.isEmpty() ? _M_currentFile : fileName);
  item->setStartPosition(KTextEditor::Cursor(startLine, startColumn));
  item->setEndPosition(KTextEditor::Cursor(endLine, endColumn));
}

// kate: space-indent on; indent-width 2; replace-tabs on;
