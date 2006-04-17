/* This file is part of KDevelop
    Copyright (C) 2002-2005 Roberto Raggi <roberto@kdevelop.org>
    Copyright (C) 2006 Adam Treat <treat@kde.org>

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

#include "codemodel.h"

// ---------------------------------------------------------------------------
CodeModel::CodeModel( QObject *parent )
  : KDevCodeModel( parent )
{
  _M_globalNamespace = create<NamespaceModelItem>();
  _M_globalNamespace->setName(QString::null);
}

CodeModel::~CodeModel()
{
}

void CodeModel::wipeout()
{
}

NamespaceModelItem CodeModel::globalNamespace() const
{
  return _M_globalNamespace;
}

KDevItemCollection *CodeModel::root() const
{
  return _M_globalNamespace;
}

void CodeModel::addCodeItem(CodeModelItem item)
{
  beginAppendItem(item);
  endAppendItem();
}

void CodeModel::removeCodeItem(CodeModelItem item)
{
    beginRemoveItem(item);
    endRemoveItem();
}

CodeModelItem CodeModel::findItem(const QStringList &qualifiedName, CodeModelItem scope) const
{
  for (int i=0; i<qualifiedName.size(); ++i) {
    // ### Extend to look for members etc too.
    const QString &name = qualifiedName.at(i);
    if (NamespaceModelItem ns = model_dynamic_cast<NamespaceModelItem>(scope))
      {
        NamespaceModelItem tmp_ns = ns->findNamespace(name);
        if (tmp_ns) {
          scope = tmp_ns;
          continue;
        }
      }
    if (ScopeModelItem ss = model_dynamic_cast<ScopeModelItem>(scope))
      {
        if (ClassModelItem cs = ss->findClass(name))
          {
            scope = cs;
          }
        else if (EnumModelItem es = ss->findEnum(name))
          {
            Q_ASSERT(i == qualifiedName.size() - 1);
            return es->toItem();
          }
      }
  }
  return scope;
}

QString TypeInfo::toString() const
{
  QString tmp;
  if (isConstant())
    tmp += "const ";
  tmp += m_qualified_name.join("::");
  if (indirections())
    tmp += QString(indirections(), QLatin1Char('*'));
  if (isReference())
    tmp += QLatin1Char('&');
  return tmp;
}

bool TypeInfo::operator==(const TypeInfo &other)
{
  return m_constant == other.m_constant
    && m_reference == other.m_reference
    && m_indirections == other.m_indirections
    && m_qualified_name == other.m_qualified_name;
}

// ---------------------------------------------------------------------------
_CodeModelItem::_CodeModelItem(CodeModel *model, int kind)
  : KDevCodeItem( QString::null, 0 ),
    _M_model(model),
    _M_kind(kind),
    _M_startLine(-1),
    _M_startColumn(-1),
    _M_endLine(-1),
    _M_endColumn(-1)
{
}

_CodeModelItem::~_CodeModelItem()
{
}

_CodeModelItem *_CodeModelItem::itemAt(int index) const
{
  return static_cast<_CodeModelItem*>(KDevItemCollection::itemAt(index));
}

CodeModelItem _CodeModelItem::toItem() const
{
  return CodeModelItem(const_cast<_CodeModelItem*>(this));
}

int _CodeModelItem::kind() const
{
  return _M_kind;
}

void _CodeModelItem::setKind(int kind)
{
  _M_kind = kind;
}

QStringList _CodeModelItem::qualifiedName() const
{
  QStringList q = scope();

  if (!name().isEmpty())
    q += name();

  return q;
}

QString _CodeModelItem::name() const
{
  return _M_name;
}

void _CodeModelItem::setName(const QString &name)
{
  _M_name = name;
}

QStringList _CodeModelItem::scope() const
{
  return _M_scope;
}

void _CodeModelItem::setScope(const QStringList &scope)
{
  _M_scope = scope;
}

QString _CodeModelItem::fileName() const
{
  return _M_fileName;
}

void _CodeModelItem::setFileName(const QString &fileName)
{
  _M_fileName = fileName;
}

KTextEditor::Cursor _CodeModelItem::startPosition() const
{
  return KTextEditor::Cursor(_M_startLine, _M_startColumn);
}

void _CodeModelItem::setStartPosition(const KTextEditor::Cursor& cursor)
{
  _M_startLine = cursor.line();
  _M_startColumn = cursor.column();
}

KTextEditor::Cursor _CodeModelItem::endPosition() const
{
  return KTextEditor::Cursor(_M_endLine, _M_endColumn);
}

void _CodeModelItem::setEndPosition(const KTextEditor::Cursor& cursor)
{
  _M_endLine = cursor.line();
  _M_endColumn = cursor.column();
}

// ---------------------------------------------------------------------------
QStringList _ClassModelItem::baseClasses() const
{
  return _M_baseClasses;
}

void _ClassModelItem::setBaseClasses(const QStringList &baseClasses)
{
  _M_baseClasses = baseClasses;
}

void _ClassModelItem::addBaseClass(const QString &baseClass)
{
  _M_baseClasses.append(baseClass);
}

void _ClassModelItem::removeBaseClass(const QString &baseClass)
{
  _M_baseClasses.removeAt(_M_baseClasses.indexOf(baseClass));
}

bool _ClassModelItem::extendsClass(const QString &name) const
{
  return _M_baseClasses.contains(name);
}

void _ClassModelItem::setClassType(CodeModel::ClassType type)
{
  _M_classType = type;
}

CodeModel::ClassType _ClassModelItem::classType() const
{
  return _M_classType;
}

// ---------------------------------------------------------------------------
FunctionModelItem _ScopeModelItem::declaredFunction(FunctionModelItem item)
{
  FunctionList function_list = findFunctions(item->name());

  foreach (FunctionModelItem fun, function_list)
    {
      if (fun->isSimilar(model_static_cast<CodeModelItem>(item),false))
        return fun;
    }

  return FunctionModelItem();
}

ClassList _ScopeModelItem::classes() const
{
  return _M_classes.values();
}

TypeAliasList _ScopeModelItem::typeAliases() const
{
  return _M_typeAliases.values();
}

VariableList _ScopeModelItem::variables() const
{
  return _M_variables.values();
}

FunctionList _ScopeModelItem::functions() const
{
  return _M_functions.values();
}

FunctionDefinitionList _ScopeModelItem::functionDefinitions() const
{
  return _M_functionDefinitions.values();
}

EnumList _ScopeModelItem::enums() const
{
  return _M_enums.values();
}

void _ScopeModelItem::addClass(ClassModelItem item)
{
  if (_M_classes.contains(item->name()))
    removeClass(_M_classes[item->name()]);
  model()->beginAppendItem(item, this);
  _M_classes.insertMulti(item->name(), item);
  model()->endAppendItem();
}

void _ScopeModelItem::addFunction(FunctionModelItem item)
{
  foreach( _FunctionModelItem *i,
           _M_functions.values(item->name()) )
    if ( i->isSimilar( model_static_cast<CodeModelItem>(item) ) )
      removeFunction(i);
  model()->beginAppendItem(item, this);
  _M_functions.insert(item->name(), item);
  model()->endAppendItem();
}

void _ScopeModelItem::addFunctionDefinition(FunctionDefinitionModelItem item)
{
  foreach( _FunctionDefinitionModelItem *i,
           _M_functionDefinitions.values(item->name()) )
    if ( i->isSimilar( model_static_cast<CodeModelItem>(item) ) )
      removeFunctionDefinition(i);
  model()->beginAppendItem(item, this);
  _M_functionDefinitions.insert(item->name(), item);
  model()->endAppendItem();
}

void _ScopeModelItem::addVariable(VariableModelItem item)
{
  if (_M_variables.contains(item->name()))
    removeVariable(_M_variables[item->name()]);
  model()->beginAppendItem(item, this);
  _M_variables.insertMulti(item->name(), item);
  model()->endAppendItem();
}

void _ScopeModelItem::addTypeAlias(TypeAliasModelItem item)
{
  if (_M_typeAliases.contains(item->name()))
    removeTypeAlias(_M_typeAliases[item->name()]);
  model()->beginAppendItem(item, this);
  _M_typeAliases.insertMulti(item->name(), item);
  model()->endAppendItem();
}

void _ScopeModelItem::addEnum(EnumModelItem item)
{
  if (_M_enums.contains(item->name()))
    removeEnum(_M_enums[item->name()]);
  model()->beginAppendItem(item, this);
  _M_enums.insertMulti(item->name(), item);
  model()->endAppendItem();
}

void _ScopeModelItem::addTemplate(TemplateModelItem item)
{
  model()->beginAppendItem(item, this);
  _M_templates.insert(item->name(), item);
  model()->endAppendItem();
}

void _ScopeModelItem::removeClass(ClassModelItem item)
{
  model()->beginRemoveItem(item);
  _M_classes.remove(item->name());
  model()->endRemoveItem();
}

void _ScopeModelItem::removeFunction(FunctionModelItem item)
{
  QMultiHash<QString, FunctionModelItem>::Iterator it = _M_functions.find(item->name());

  while (it != _M_functions.end() && it.key() == item->name()
         && it.value() != item)
    {
      ++it;
    }

  if (it != _M_functions.end() && it.value() == item)
    {
      model()->beginRemoveItem(item);
      _M_functions.erase(it);
      model()->endRemoveItem();
    }
}

void _ScopeModelItem::removeFunctionDefinition(FunctionDefinitionModelItem item)
{
  QMultiHash<QString, FunctionDefinitionModelItem>::Iterator it = _M_functionDefinitions.find(item->name());

  while (it != _M_functionDefinitions.end() && it.key() == item->name()
         && it.value() != item)
    {
      ++it;
    }

  if (it != _M_functionDefinitions.end() && it.value() == item)
    {
      model()->beginRemoveItem(item);
      _M_functionDefinitions.erase(it);
      model()->endRemoveItem();
    }
}

void _ScopeModelItem::removeVariable(VariableModelItem item)
{
  model()->beginRemoveItem(item);
  _M_variables.remove(item->name());
  model()->endRemoveItem();
}

void _ScopeModelItem::removeTypeAlias(TypeAliasModelItem item)
{
  model()->beginRemoveItem(item);
  _M_typeAliases.remove(item->name());
  model()->endRemoveItem();
}

void _ScopeModelItem::removeEnum(EnumModelItem item)
{
  model()->beginRemoveItem(item);
  _M_enums.remove(item->name());
  model()->endRemoveItem();
}

void _ScopeModelItem::removeTemplate(TemplateModelItem item)
{
  QMultiHash<QString, TemplateModelItem>::Iterator
    it = _M_templates.find(item->name());

  while (it != _M_templates.end() && it.key() == item->name()
         && it.value() != item)
    {
      ++it;
    }

  if (it != _M_templates.end() && it.value() == item)
    {
      model()->beginRemoveItem(item);
      _M_templates.erase(it);
      model()->endRemoveItem();
    }
}

ClassModelItem _ScopeModelItem::findClass(const QString &name) const
{
  return _M_classes.value(name);
}

VariableModelItem _ScopeModelItem::findVariable(const QString &name) const
{
  return _M_variables.value(name);
}

TypeAliasModelItem _ScopeModelItem::findTypeAlias(const QString &name) const
{
  return _M_typeAliases.value(name);
}

EnumModelItem _ScopeModelItem::findEnum(const QString &name) const
{
  return _M_enums.value(name);
}

FunctionList _ScopeModelItem::findFunctions(const QString &name) const
{
  return _M_functions.values(name);
}

FunctionDefinitionList _ScopeModelItem::findFunctionDefinitions(const QString &name) const
{
  return _M_functionDefinitions.values(name);
}

TemplateList _ScopeModelItem::findTemplates(const QString &name) const
{
  return _M_templates.values(name);
}

// ---------------------------------------------------------------------------
NamespaceList _NamespaceModelItem::namespaces() const
{
  return _M_namespaces.values();
}
void _NamespaceModelItem::addNamespace(NamespaceModelItem item)
{
  if (_M_namespaces.contains(item->name()))
    removeNamespace(_M_namespaces[item->name()]);
  model()->beginAppendItem(item, this);
  _M_namespaces.insertMulti(item->name(), item);
  model()->endAppendItem();
}
void _NamespaceModelItem::removeNamespace(NamespaceModelItem item)
{
  model()->beginRemoveItem(item);
  _M_namespaces.remove(item->name());
  model()->endRemoveItem();
}

NamespaceModelItem _NamespaceModelItem::findNamespace(const QString &name) const
{
  return _M_namespaces.value(name);
}

// ---------------------------------------------------------------------------
TypeInfo _ArgumentModelItem::type() const
{
  return _M_type;
}

void _ArgumentModelItem::setType(const TypeInfo &type)
{
  _M_type = type;
}

bool _ArgumentModelItem::defaultValue() const
{
  return _M_defaultValue;
}

void _ArgumentModelItem::setDefaultValue(bool defaultValue)
{
  _M_defaultValue = defaultValue;
}

// ---------------------------------------------------------------------------
bool _FunctionModelItem::isSimilar(KDevCodeItem *other, bool strict ) const
{
  if (!_MemberModelItem::isSimilar(other,strict))
    return false;

  FunctionModelItem func = dynamic_cast<_FunctionModelItem*>(other);

  if (isConstant() != func->isConstant())
    return false;

  if (arguments().count() != func->arguments().count())
    return false;

  for (int i=0; i<arguments().count(); ++i)
    {
      ArgumentModelItem arg1 = arguments().at(i);
      ArgumentModelItem arg2 = arguments().at(i);

      if (arg1->type() != arg2->type())
        return false;
    }

  return true;
}

ArgumentList _FunctionModelItem::arguments() const
{
  return _M_arguments;
}

void _FunctionModelItem::addArgument(ArgumentModelItem item)
{
  model()->beginAppendItem(item, this);
  _M_arguments.append(item);
  model()->endAppendItem();
}

void _FunctionModelItem::removeArgument(ArgumentModelItem item)
{
  model()->beginRemoveItem(item);
  _M_arguments.removeAt(_M_arguments.indexOf(item));
  model()->endRemoveItem();
}

CodeModel::FunctionType _FunctionModelItem::functionType() const
{
  return _M_functionType;
}

void _FunctionModelItem::setFunctionType(CodeModel::FunctionType functionType)
{
  _M_functionType = functionType;
}

bool _FunctionModelItem::isConstructor() const
{
  ///TODO cache this information upon initialization
  return scope().last() == name();
}

bool _FunctionModelItem::isDestructor() const
{
  ///TODO cache this information upon initialization
  return name().startsWith('~');
}

bool _FunctionModelItem::isVirtual() const
{
  return _M_isVirtual;
}

void _FunctionModelItem::setVirtual(bool isVirtual)
{
  _M_isVirtual = isVirtual;
}

bool _FunctionModelItem::isInline() const
{
  return _M_isInline;
}

void _FunctionModelItem::setInline(bool isInline)
{
  _M_isInline = isInline;
}

bool _FunctionModelItem::isExplicit() const
{
  return _M_isExplicit;
}

void _FunctionModelItem::setExplicit(bool isExplicit)
{
  _M_isExplicit = isExplicit;
}

bool _FunctionModelItem::isAbstract() const
{
  return _M_isAbstract;
}

void _FunctionModelItem::setAbstract(bool isAbstract)
{
  _M_isAbstract = isAbstract;
}

// ---------------------------------------------------------------------------
TypeInfo _TypeAliasModelItem::type() const
{
  return _M_type;
}

void _TypeAliasModelItem::setType(const TypeInfo &type)
{
  _M_type = type;
}

// ---------------------------------------------------------------------------
CodeModel::AccessPolicy _EnumModelItem::accessPolicy() const
{
  return _M_accessPolicy;
}

void _EnumModelItem::setAccessPolicy(CodeModel::AccessPolicy accessPolicy)
{
  _M_accessPolicy = accessPolicy;
}

EnumeratorList _EnumModelItem::enumerators() const
{
  return _M_enumerators;
}

void _EnumModelItem::addEnumerator(EnumeratorModelItem item)
{
  model()->beginAppendItem(item, this);
  _M_enumerators.append(item);
  model()->endAppendItem();
}

void _EnumModelItem::removeEnumerator(EnumeratorModelItem item)
{
  model()->beginRemoveItem(item);
  _M_enumerators.removeAt(_M_enumerators.indexOf(item));
  model()->endRemoveItem();
}

// ---------------------------------------------------------------------------
QString _EnumeratorModelItem::value() const
{
  return _M_value;
}

void _EnumeratorModelItem::setValue(const QString &value)
{
  _M_value = value;
}

// ---------------------------------------------------------------------------
TypeInfo _TemplateParameterModelItem::type() const
{
  return _M_type;
}

void _TemplateParameterModelItem::setType(const TypeInfo &type)
{
  _M_type = type;
}

bool _TemplateParameterModelItem::defaultValue() const
{
  return _M_defaultValue;
}

void _TemplateParameterModelItem::setDefaultValue(bool defaultValue)
{
  _M_defaultValue = defaultValue;
}

// ---------------------------------------------------------------------------
TemplateParameterList _TemplateModelItem::parameters() const
{
  return _M_parameters;
}

void _TemplateModelItem::addParameter(TemplateParameterModelItem item)
{
  model()->beginAppendItem(item, this);
  _M_parameters.append(item);
  model()->endAppendItem();
}

void _TemplateModelItem::removeParameter(TemplateParameterModelItem item)
{
  model()->beginRemoveItem(item);
  _M_parameters.removeAt(_M_parameters.indexOf(item));
  model()->endRemoveItem();
}

CodeModelItem _TemplateModelItem::declaration() const
{
  return _M_declaration;
}

void _TemplateModelItem::setDeclaration(CodeModelItem declaration)
{
  _M_declaration = declaration;
}

// ---------------------------------------------------------------------------
ScopeModelItem _ScopeModelItem::create(CodeModel *model)
{
  ScopeModelItem item(new _ScopeModelItem(model));
  return item;
}

ClassModelItem _ClassModelItem::create(CodeModel *model)
{
  ClassModelItem item(new _ClassModelItem(model));
  return item;
}

NamespaceModelItem _NamespaceModelItem::create(CodeModel *model)
{
  NamespaceModelItem item(new _NamespaceModelItem(model));
  return item;
}

FileModelItem _FileModelItem::create(CodeModel *model)
{
  FileModelItem item(new _FileModelItem(model));
  return item;
}

ArgumentModelItem _ArgumentModelItem::create(CodeModel *model)
{
  ArgumentModelItem item(new _ArgumentModelItem(model));
  return item;
}

FunctionModelItem _FunctionModelItem::create(CodeModel *model)
{
  FunctionModelItem item(new _FunctionModelItem(model));
  return item;
}

FunctionDefinitionModelItem _FunctionDefinitionModelItem::create(CodeModel *model)
{
  FunctionDefinitionModelItem item(new _FunctionDefinitionModelItem(model));
  return item;
}

VariableModelItem _VariableModelItem::create(CodeModel *model)
{
  VariableModelItem item(new _VariableModelItem(model));
  return item;
}

TypeAliasModelItem _TypeAliasModelItem::create(CodeModel *model)
{
  TypeAliasModelItem item(new _TypeAliasModelItem(model));
  return item;
}

EnumModelItem _EnumModelItem::create(CodeModel *model)
{
  EnumModelItem item(new _EnumModelItem(model));
  return item;
}

EnumeratorModelItem _EnumeratorModelItem::create(CodeModel *model)
{
  EnumeratorModelItem item(new _EnumeratorModelItem(model));
  return item;
}

TemplateParameterModelItem _TemplateParameterModelItem::create(CodeModel *model)
{
  TemplateParameterModelItem item(new _TemplateParameterModelItem(model));
  return item;
}

TemplateModelItem _TemplateModelItem::create(CodeModel *model)
{
  TemplateModelItem item(new _TemplateModelItem(model));
  return item;
}

// ---------------------------------------------------------------------------
TypeInfo _MemberModelItem::type() const
{
  return _M_type;
}

void _MemberModelItem::setType(const TypeInfo &type)
{
  _M_type = type;
}

CodeModel::AccessPolicy _MemberModelItem::accessPolicy() const
{
  return _M_accessPolicy;
}

void _MemberModelItem::setAccessPolicy(CodeModel::AccessPolicy accessPolicy)
{
  _M_accessPolicy = accessPolicy;
}

bool _MemberModelItem::isStatic() const
{
  return _M_isStatic;
}

void _MemberModelItem::setStatic(bool isStatic)
{
  _M_isStatic = isStatic;
}

bool _MemberModelItem::isConstant() const
{
  return _M_isConstant;
}

void _MemberModelItem::setConstant(bool isConstant)
{
  _M_isConstant = isConstant;
}

bool _MemberModelItem::isVolatile() const
{
  return _M_isVolatile;
}

void _MemberModelItem::setVolatile(bool isVolatile)
{
  _M_isVolatile = isVolatile;
}

bool _MemberModelItem::isAuto() const
{
  return _M_isAuto;
}

void _MemberModelItem::setAuto(bool isAuto)
{
  _M_isAuto = isAuto;
}

bool _MemberModelItem::isFriend() const
{
  return _M_isFriend;
}

void _MemberModelItem::setFriend(bool isFriend)
{
  _M_isFriend = isFriend;
}

bool _MemberModelItem::isRegister() const
{
  return _M_isRegister;
}

void _MemberModelItem::setRegister(bool isRegister)
{
  _M_isRegister = isRegister;
}

bool _MemberModelItem::isExtern() const
{
  return _M_isExtern;
}

void _MemberModelItem::setExtern(bool isExtern)
{
  _M_isExtern = isExtern;
}

bool _MemberModelItem::isMutable() const
{
  return _M_isMutable;
}

void _MemberModelItem::setMutable(bool isMutable)
{
  _M_isMutable = isMutable;
}

// kate: space-indent on; indent-width 2; replace-tabs on;

