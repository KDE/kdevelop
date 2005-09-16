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
   the Free Software Foundation, Inc., 51 Franklin Steet, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "codemodel.h"

// ---------------------------------------------------------------------------
CodeModel::CodeModel()
  : _M_creation_id(0)
{
  _M_globalNamespace = create<NamespaceModelItem>();
}

CodeModel::~CodeModel()
{
}

void CodeModel::wipeout()
{
  _M_globalNamespace = create<NamespaceModelItem>();
  _M_files.clear();
}

FileList CodeModel::files() const
{
  return _M_files.values();
}

NamespaceModelItem CodeModel::globalNamespace() const
{
  return _M_globalNamespace;
}

void CodeModel::addFile(FileModelItem item)
{
  _M_creation_id = 0; // reset the creation id
  _M_files.insert(item->name(), item);
}

void CodeModel::removeFile(FileModelItem item)
{
  QHash<QString, FileModelItem>::Iterator it = _M_files.find(item->name());

  if (it != _M_files.end() && it.value() == item)
    _M_files.erase(it);
}

FileModelItem CodeModel::findFile(const QString &name) const
{
  return _M_files.value(name);
}

QHash<QString, FileModelItem> CodeModel::fileMap() const
{
  return _M_files;
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
  : _M_model(model),
    _M_kind(kind),
    _M_startLine(0),
    _M_startColumn(0),
    _M_endLine(0),
    _M_endColumn(0),
    _M_creation_id(0)
{
}

_CodeModelItem::~_CodeModelItem()
{
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

FileModelItem _CodeModelItem::file() const
{
  return model()->findFile(fileName());
}

void _CodeModelItem::getStartPosition(int *line, int *column)
{
  *line = _M_startLine;
  *column = _M_startColumn;
}

void _CodeModelItem::setStartPosition(int line, int column)
{
  _M_startLine = line;
  _M_startColumn = column;
}

void _CodeModelItem::getEndPosition(int *line, int *column)
{
  *line = _M_endLine;
  *column = _M_endColumn;
}

void _CodeModelItem::setEndPosition(int line, int column)
{
  _M_endLine = line;
  _M_endColumn = column;
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
      if (fun->isSimilar(item))
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
  _M_classes.insert(item->name(), item);
}

void _ScopeModelItem::addFunction(FunctionModelItem item)
{
  _M_functions.insert(item->name(), item);
}

void _ScopeModelItem::addFunctionDefinition(FunctionDefinitionModelItem item)
{
  _M_functionDefinitions.insert(item->name(), item);
}

void _ScopeModelItem::addVariable(VariableModelItem item)
{
  _M_variables.insert(item->name(), item);
}

void _ScopeModelItem::addTypeAlias(TypeAliasModelItem item)
{
  _M_typeAliases.insert(item->name(), item);
}

void _ScopeModelItem::addEnum(EnumModelItem item)
{
  _M_enums.insert(item->name(), item);
}

void _ScopeModelItem::addTemplate(TemplateModelItem item)
{
  _M_templates.insert(item->name(), item);
}

void _ScopeModelItem::removeClass(ClassModelItem item)
{
  QHash<QString, ClassModelItem>::Iterator it = _M_classes.find(item->name());

  if (it != _M_classes.end() && it.value() == item)
    _M_classes.erase(it);
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
      _M_functions.erase(it);
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
      _M_functionDefinitions.erase(it);
    }
}

void _ScopeModelItem::removeVariable(VariableModelItem item)
{
  QHash<QString, VariableModelItem>::Iterator it = _M_variables.find(item->name());

  if (it != _M_variables.end() && it.value() == item)
    _M_variables.erase(it);
}

void _ScopeModelItem::removeTypeAlias(TypeAliasModelItem item)
{
  QHash<QString, TypeAliasModelItem>::Iterator it = _M_typeAliases.find(item->name());

  if (it != _M_typeAliases.end() && it.value() == item)
    _M_typeAliases.erase(it);
}

void _ScopeModelItem::removeEnum(EnumModelItem item)
{
  QHash<QString, EnumModelItem>::Iterator it = _M_enums.find(item->name());

  if (it != _M_enums.end() && it.value() == item)
    _M_enums.erase(it);
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
      _M_templates.erase(it);
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
  _M_namespaces.insert(item->name(), item);
}
void _NamespaceModelItem::removeNamespace(NamespaceModelItem item)
{
  QHash<QString, NamespaceModelItem>::Iterator it = _M_namespaces.find(item->name());

  if (it != _M_namespaces.end() && it.value() == item)
    _M_namespaces.erase(it);
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
bool _FunctionModelItem::isSimilar(FunctionModelItem other) const
{
  if (name() != other->name())
    return false;

  if (isConstant() != other->isConstant())
    return false;

  if (arguments().count() != other->arguments().count())
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
  _M_arguments.append(item);
}

void _FunctionModelItem::removeArgument(ArgumentModelItem item)
{
  _M_arguments.removeAt(_M_arguments.indexOf(item));
}

CodeModel::FunctionType _FunctionModelItem::functionType() const
{
  return _M_functionType;
}

void _FunctionModelItem::setFunctionType(CodeModel::FunctionType functionType)
{
  _M_functionType = functionType;
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
  _M_enumerators.append(item);
}

void _EnumModelItem::removeEnumerator(EnumeratorModelItem item)
{
  _M_enumerators.removeAt(_M_enumerators.indexOf(item));
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
  _M_parameters.append(item);
}

void _TemplateModelItem::removeParameter(TemplateParameterModelItem item)
{
  _M_parameters.removeAt(_M_parameters.indexOf(item));
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

