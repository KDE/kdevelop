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

#ifndef CODEMODEL_H
#define CODEMODEL_H

#include "kdevcodemodel.h"

#include "codedisplay.h"
#include "codemodel_fwd.h"

#include <QtCore/QTime>
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVector>

#include <kurl.h>
#include <kdevexport.h>
#include <ktexteditor/cursor.h>
#include <kdevexport.h>

#define DECLARE_MODEL_NODE(k) \
    enum { __node_kind = Kind_##k }; \
    typedef KDevSharedPtr<k##ModelItem> Pointer;

template <class _Target, class _Source>
_Target model_static_cast(_Source item)
{
  typedef typename _Target::Type * _Target_pointer;

  _Target ptr = static_cast<_Target_pointer>(item.data());
  return ptr;
}

class KDEVCPPLANGUAGE_EXPORT CodeModel : public KDevCodeModel
{
public:
  enum AccessPolicy
  {
    Public,
    Protected,
    Private
  };

  enum FunctionType
  {
    Normal,
    Signal,
    Slot
  };

  enum ClassType
  {
    Class,
    Struct,
    Union
  };

public:
  CodeModel( QObject *parent = 0 );
  virtual ~CodeModel();

  template <class _Target> _Target create()
  {
    typedef typename _Target::Type _Target_type;

    _Target result = _Target_type::create(this);
    return result;
  }

  NamespaceModelItem globalNamespace() const;
  KDevItemCollection *root() const;

  void addCodeItem(CodeModelItem item);
  void removeCodeItem(CodeModelItem item);

  CodeModelItem findItem(const QStringList &qualifiedName, CodeModelItem scope) const;

  void wipeout();

private:
  NamespaceModelItem _M_globalNamespace;

private:
  CodeModel(const CodeModel &other);
  void operator = (const CodeModel &other);
};

class TypeInfo
{
public:
    QStringList qualifiedName() const { return m_qualified_name; }
    void setQualifiedName(const QStringList &qualified_name) { m_qualified_name = qualified_name; }

    bool isConstant() const { return m_constant; }
    void setConstant(bool is) { m_constant = is; }

    bool isReference() const { return m_reference; }
    void setReference(bool is) { m_reference = is; }

    int indirections() const { return m_indirections; }
    void setIndirections(int indirections) { m_indirections = indirections; }

    bool operator==(const TypeInfo &other);
    bool operator!=(const TypeInfo &other) { return !(*this==other); }

    // ### arrays and templates??

    QString toString() const;

private:
    QStringList m_qualified_name;
    uint m_constant : 1;
    uint m_reference : 1;
    uint m_indirections : 6;
};

class KDEVCPPLANGUAGE_EXPORT _CodeModelItem: public KDevCodeItem
{
public:
  enum Kind
  {
    /* These are bit-flags resembling inheritance */
    Kind_Scope = 0x1,
    Kind_Namespace = 0x2 /*| Kind_Scope*/,
    Kind_Member = 0x4,
    Kind_Function = 0x8 /*| Kind_Member*/,
    KindMask = 0xf,

    /* These are for classes that are not inherited from */
    FirstKind = 0x10,
    Kind_Argument = 1 << FirstKind,
    Kind_Class = 2 << FirstKind /*| Kind_Scope*/,
    Kind_Enum = 4 << FirstKind,
    Kind_Enumerator = 8 << FirstKind,
    Kind_File = 16 << FirstKind /*| Kind_Namespace*/,
    Kind_FunctionDefinition = 32 << FirstKind /*| Kind_Function*/,
    Kind_Template = 64 << FirstKind,
    Kind_TemplateParameter = 128 << FirstKind,
    Kind_TypeAlias = 256 << FirstKind,
    Kind_Variable = 512 << FirstKind /*| Kind_Member*/
  };

public:
  virtual ~_CodeModelItem();

  virtual _CodeModelItem *itemAt(int index) const;

  ///TODO These functions should be cached upon initialization
  QString display() const
  {
    return CodeDisplay::display( const_cast<const _CodeModelItem*>( this ) );
  }

  QIcon decoration() const
  {
    return CodeDisplay::decoration( const_cast<const _CodeModelItem*>( this ) );
  }

  QString toolTip() const
  {
    return CodeDisplay::toolTip( const_cast<const _CodeModelItem*>( this ) );
  }

  QString whatsThis() const
  {
    return CodeDisplay::whatsThis( const_cast<const _CodeModelItem*>( this ) );
  }

  int kind() const;

  QStringList qualifiedName() const;

  QString name() const;
  void setName(const QString &name);

  QStringList scope() const;
  void setScope(const QStringList &scope);

  QString fileName() const;
  void setFileName(const QString &fileName);

  KTextEditor::Cursor startPosition() const;
  void setStartPosition(const KTextEditor::Cursor& cursor);

  KTextEditor::Cursor endPosition() const;
  void setEndPosition(const KTextEditor::Cursor& cursor);

  QTime timestamp() const { return QTime(); }

  inline CodeModel *model() const { return _M_model; }

  CodeModelItem toItem() const;

protected:
  _CodeModelItem(CodeModel *model, int kind);
  void setKind(int kind);

private:
  CodeModel *_M_model;
  int _M_kind;
  int _M_startLine;
  int _M_startColumn;
  int _M_endLine;
  int _M_endColumn;
  QString _M_name;
  QString _M_fileName;
  QStringList _M_scope;

private:
  _CodeModelItem(const _CodeModelItem &other);
  void operator = (const _CodeModelItem &other);
};

class _ScopeModelItem: public _CodeModelItem
{
public:
  DECLARE_MODEL_NODE(Scope)

  static ScopeModelItem create(CodeModel *model);

public:
  ClassList classes() const;
  EnumList enums() const;
  FunctionDefinitionList functionDefinitions() const;
  FunctionList functions() const;
  TypeAliasList typeAliases() const;
  VariableList variables() const;
  TemplateList templates() const;

  void addClass(ClassModelItem item);
  void addEnum(EnumModelItem item);
  void addFunction(FunctionModelItem item);
  void addFunctionDefinition(FunctionDefinitionModelItem item);
  void addTypeAlias(TypeAliasModelItem item);
  void addVariable(VariableModelItem item);
  void addTemplate(TemplateModelItem item);

  void removeClass(ClassModelItem item);
  void removeEnum(EnumModelItem item);
  void removeFunction(FunctionModelItem item);
  void removeFunctionDefinition(FunctionDefinitionModelItem item);
  void removeTypeAlias(TypeAliasModelItem item);
  void removeVariable(VariableModelItem item);
  void removeTemplate(TemplateModelItem item);

  ClassModelItem findClass(const QString &name) const;
  EnumModelItem findEnum(const QString &name) const;
  FunctionDefinitionList findFunctionDefinitions(const QString &name) const;
  FunctionList findFunctions(const QString &name) const;
  TypeAliasModelItem findTypeAlias(const QString &name) const;
  VariableModelItem findVariable(const QString &name) const;
  TemplateList findTemplates(const QString &name) const;

  inline QHash<QString, ClassModelItem> classMap() const { return _M_classes; }
  inline QHash<QString, EnumModelItem> enumMap() const { return _M_enums; }
  inline QHash<QString, TypeAliasModelItem> typeAliasMap() const { return _M_typeAliases; }
  inline QHash<QString, VariableModelItem> variableMap() const { return _M_variables; }
  inline QMultiHash<QString, FunctionDefinitionModelItem> functionDefinitionMap() const { return _M_functionDefinitions; }
  inline QMultiHash<QString, FunctionModelItem> functionMap() const { return _M_functions; }
  inline QMultiHash<QString, TemplateModelItem> templateMap() const { return _M_templates; }

  FunctionModelItem declaredFunction(FunctionModelItem item);

protected:
  _ScopeModelItem(CodeModel *model, int kind = __node_kind)
    : _CodeModelItem(model, kind) {}

private:
  QHash<QString, ClassModelItem> _M_classes;
  QHash<QString, EnumModelItem> _M_enums;
  QHash<QString, TypeAliasModelItem> _M_typeAliases;
  QHash<QString, VariableModelItem> _M_variables;
  QMultiHash<QString, FunctionDefinitionModelItem> _M_functionDefinitions;
  QMultiHash<QString, FunctionModelItem> _M_functions;
  QMultiHash<QString, TemplateModelItem> _M_templates;

private:
  _ScopeModelItem(const _ScopeModelItem &other);
  void operator = (const _ScopeModelItem &other);
};

class _ClassModelItem: public _ScopeModelItem
{
public:
  DECLARE_MODEL_NODE(Class)

  static ClassModelItem create(CodeModel *model);

public:
  QStringList baseClasses() const;

  void setBaseClasses(const QStringList &baseClasses);
  void addBaseClass(const QString &baseClass);
  void removeBaseClass(const QString &baseClass);

  bool extendsClass(const QString &name) const;

  void setClassType(CodeModel::ClassType type);
  CodeModel::ClassType classType() const;

protected:
  _ClassModelItem(CodeModel *model, int kind = __node_kind)
    : _ScopeModelItem(model, kind), _M_classType(CodeModel::Class) {}

private:
  QStringList _M_baseClasses;
  CodeModel::ClassType _M_classType;

private:
  _ClassModelItem(const _ClassModelItem &other);
  void operator = (const _ClassModelItem &other);
};

class _NamespaceModelItem: public _ScopeModelItem
{
public:
  DECLARE_MODEL_NODE(Namespace)

  static NamespaceModelItem create(CodeModel *model);

public:
  NamespaceList namespaces() const;

  void addNamespace(NamespaceModelItem item);
  void removeNamespace(NamespaceModelItem item);

  NamespaceModelItem findNamespace(const QString &name) const;

  inline QHash<QString, NamespaceModelItem> namespaceMap() const { return _M_namespaces; };

protected:
  _NamespaceModelItem(CodeModel *model, int kind = __node_kind)
    : _ScopeModelItem(model, kind) {}

private:
  QHash<QString, NamespaceModelItem> _M_namespaces;

private:
  _NamespaceModelItem(const _NamespaceModelItem &other);
  void operator = (const _NamespaceModelItem &other);
};

class _FileModelItem: public _NamespaceModelItem
{
public:
  DECLARE_MODEL_NODE(File)

  static FileModelItem create(CodeModel *model);

protected:
  _FileModelItem(CodeModel *model, int kind = __node_kind)
    : _NamespaceModelItem(model, kind) {}

private:
  _FileModelItem(const _FileModelItem &other);
  void operator = (const _FileModelItem &other);
};

class _ArgumentModelItem: public _CodeModelItem
{
public:
  DECLARE_MODEL_NODE(Argument)

  static ArgumentModelItem create(CodeModel *model);

public:
  TypeInfo type() const;
  void setType(const TypeInfo &type);

  bool defaultValue() const;
  void setDefaultValue(bool defaultValue);

protected:
  _ArgumentModelItem(CodeModel *model, int kind = __node_kind)
    : _CodeModelItem(model, kind), _M_defaultValue(false) {}

private:
  TypeInfo _M_type;
  bool _M_defaultValue;

private:
  _ArgumentModelItem(const _ArgumentModelItem &other);
  void operator = (const _ArgumentModelItem &other);
};

class _MemberModelItem: public _CodeModelItem
{
public:
  DECLARE_MODEL_NODE(Member)

  bool isConstant() const;
  void setConstant(bool isConstant);

  bool isVolatile() const;
  void setVolatile(bool isVolatile);

  bool isStatic() const;
  void setStatic(bool isStatic);

  bool isAuto() const;
  void setAuto(bool isAuto);

  bool isFriend() const;
  void setFriend(bool isFriend);

  bool isRegister() const;
  void setRegister(bool isRegister);

  bool isExtern() const;
  void setExtern(bool isExtern);

  bool isMutable() const;
  void setMutable(bool isMutable);

  CodeModel::AccessPolicy accessPolicy() const;
  void setAccessPolicy(CodeModel::AccessPolicy accessPolicy);

  TypeInfo type() const;
  void setType(const TypeInfo &type);

protected:
  _MemberModelItem(CodeModel *model, int kind)
    : _CodeModelItem(model, kind),
      _M_accessPolicy(CodeModel::Public),
      _M_isConstant(0), _M_isVolatile(0),
      _M_isStatic(0), _M_isAuto(0),
      _M_isFriend(0), _M_isRegister(0),
      _M_isExtern(0), _M_isMutable(0)
  {}

private:
  TypeInfo _M_type;
  CodeModel::AccessPolicy _M_accessPolicy;
  uint _M_isConstant: 1;
  uint _M_isVolatile: 1;
  uint _M_isStatic: 1;
  uint _M_isAuto: 1;
  uint _M_isFriend: 1;
  uint _M_isRegister: 1;
  uint _M_isExtern: 1;
  uint _M_isMutable: 1;
};

class KDEVCPPPARSER_EXPORT _FunctionModelItem: public _MemberModelItem
{
public:
  DECLARE_MODEL_NODE(Function)

  static FunctionModelItem create(CodeModel *model);

public:
  ArgumentList arguments() const;

  void addArgument(ArgumentModelItem item);
  void removeArgument(ArgumentModelItem item);

  CodeModel::FunctionType functionType() const;
  void setFunctionType(CodeModel::FunctionType functionType);

  bool isConstructor() const;
  bool isDestructor() const;

  bool isVirtual() const;
  void setVirtual(bool isVirtual);

  bool isInline() const;
  void setInline(bool isInline);

  bool isExplicit() const;
  void setExplicit(bool isExplicit);

  bool isAbstract() const;
  void setAbstract(bool isAbstract);

  bool isSimilar(KDevCodeItem *other, bool strict = true) const;

protected:
  _FunctionModelItem(CodeModel *model, int kind = __node_kind)
    : _MemberModelItem(model, kind),
      _M_functionType(CodeModel::Normal),
      _M_isVirtual(false), _M_isInline(false), _M_isAbstract(false),
      _M_isExplicit(false)
  {}

private:
  mutable bool _M_constructor;
  mutable bool _M_destructor;
  ArgumentList _M_arguments;
  CodeModel::FunctionType _M_functionType;
  bool _M_isVirtual: 1;
  bool _M_isInline: 1;
  bool _M_isAbstract: 1;
  bool _M_isExplicit: 1;

private:
  _FunctionModelItem(const _FunctionModelItem &other);
  void operator = (const _FunctionModelItem &other);
};

class _FunctionDefinitionModelItem: public _FunctionModelItem
{
public:
  DECLARE_MODEL_NODE(FunctionDefinition)

  static FunctionDefinitionModelItem create(CodeModel *model);

protected:
  _FunctionDefinitionModelItem(CodeModel *model, int kind = __node_kind)
    : _FunctionModelItem(model, kind) {}

private:
  _FunctionDefinitionModelItem(const _FunctionDefinitionModelItem &other);
  void operator = (const _FunctionDefinitionModelItem &other);
};

class _VariableModelItem: public _MemberModelItem
{
public:
  DECLARE_MODEL_NODE(Variable)

  static VariableModelItem create(CodeModel *model);

protected:
  _VariableModelItem(CodeModel *model, int kind = __node_kind)
    : _MemberModelItem(model, kind)
  {}

private:
  _VariableModelItem(const _VariableModelItem &other);
  void operator = (const _VariableModelItem &other);
};

class _TypeAliasModelItem: public _CodeModelItem
{
public:
  DECLARE_MODEL_NODE(TypeAlias)

  static TypeAliasModelItem create(CodeModel *model);

public:
  TypeInfo type() const;
  void setType(const TypeInfo &type);

protected:
  _TypeAliasModelItem(CodeModel *model, int kind = __node_kind)
    : _CodeModelItem(model, kind) {}

private:
  TypeInfo _M_type;

private:
  _TypeAliasModelItem(const _TypeAliasModelItem &other);
  void operator = (const _TypeAliasModelItem &other);
};

class _EnumModelItem: public _CodeModelItem
{
public:
  DECLARE_MODEL_NODE(Enum)

  static EnumModelItem create(CodeModel *model);

public:
  CodeModel::AccessPolicy accessPolicy() const;
  void setAccessPolicy(CodeModel::AccessPolicy accessPolicy);

  EnumeratorList enumerators() const;
  void addEnumerator(EnumeratorModelItem item);
  void removeEnumerator(EnumeratorModelItem item);

protected:
  _EnumModelItem(CodeModel *model, int kind = __node_kind)
    : _CodeModelItem(model, kind),
      _M_accessPolicy(CodeModel::Public)
  {}

private:
  CodeModel::AccessPolicy _M_accessPolicy;
  EnumeratorList _M_enumerators;

private:
  _EnumModelItem(const _EnumModelItem &other);
  void operator = (const _EnumModelItem &other);
};

class _EnumeratorModelItem: public _CodeModelItem
{
public:
  DECLARE_MODEL_NODE(Enumerator)

  static EnumeratorModelItem create(CodeModel *model);

public:
  QString value() const;
  void setValue(const QString &value);

protected:
  _EnumeratorModelItem(CodeModel *model, int kind = __node_kind)
    : _CodeModelItem(model, kind) {}

private:
  QString _M_value;

private:
  _EnumeratorModelItem(const _EnumeratorModelItem &other);
  void operator = (const _EnumeratorModelItem &other);
};

class _TemplateParameterModelItem: public _CodeModelItem
{
public:
  DECLARE_MODEL_NODE(TemplateParameter)

  static TemplateParameterModelItem create(CodeModel *model);

public:
  TypeInfo type() const;
  void setType(const TypeInfo &type);

  bool defaultValue() const;
  void setDefaultValue(bool defaultValue);

protected:
  _TemplateParameterModelItem(CodeModel *model, int kind = __node_kind)
    : _CodeModelItem(model, kind), _M_defaultValue(false) {}

private:
  TypeInfo _M_type;
  bool _M_defaultValue;

private:
  _TemplateParameterModelItem(const _TemplateParameterModelItem &other);
  void operator = (const _TemplateParameterModelItem &other);
};

class _TemplateModelItem: public _CodeModelItem
{
public:
  DECLARE_MODEL_NODE(Template)

  static TemplateModelItem create(CodeModel *model);

public:
  TemplateParameterList parameters() const;
  void addParameter(TemplateParameterModelItem item);
  void removeParameter(TemplateParameterModelItem item);

  CodeModelItem declaration() const;
  void setDeclaration(CodeModelItem declaration);

protected:
  _TemplateModelItem(CodeModel *model, int kind = __node_kind)
    : _CodeModelItem(model, kind) {}

private:
  TemplateParameterList _M_parameters;
  CodeModelItem _M_declaration;

private:
  _TemplateModelItem(const _TemplateModelItem &other);
  void operator = (const _TemplateModelItem &other);
};

template <class _Target, class _Source>
_Target model_safe_cast(_Source item)
{
  typedef typename _Target::Type * _Target_pointer;
  typedef typename _Source::Type * _Source_pointer;

  _Source_pointer source = item.data();
  if (source && source->kind() == _Target_pointer(0)->__node_kind)
    {
      _Target ptr(static_cast<_Target_pointer>(source));
      return ptr;
    }

  return _Target();
}

template <typename _Target, typename _Source>
_Target model_dynamic_cast(_Source item)
{
  typedef typename _Target::Type * _Target_pointer;
  typedef typename _Source::Type * _Source_pointer;

  _Source_pointer source = item.data();
  if (source && (source->kind() == _Target_pointer(0)->__node_kind
         || (_Target_pointer(0)->__node_kind <= int(_CodeModelItem::KindMask)
             && ((source->kind() & _Target_pointer(0)->__node_kind)
                  == _Target_pointer(0)->__node_kind))))
    {
      _Target ptr(static_cast<_Target_pointer>(source));
      return ptr;
    }

  return _Target();
}
#endif // CODEMODEL_H

// kate: space-indent on; indent-width 2; replace-tabs on;
