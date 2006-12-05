// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef csharp_CODEMODEL_H
#define csharp_CODEMODEL_H

#include "kdevcodemodel.h"

#include "csharp_codemodel_fwd.h"

#include <QtCore/QTime>
#include <QtCore/QHash>
#include <QtCore/QList>

#include <ktexteditor/cursor.h>


#include "csharp_ast.h"

#include "codedisplay.h"

#include <QtCore/QString>
#include <QtCore/QStringList>

namespace csharp
  {

#define DECLARE_MODEL_NODE(k) \
enum { __node_kind = Kind_##k }; \
typedef KDevSharedPtr<k##ModelItem> Pointer;

#define ITEM(item) item##ModelItem
#define LIST(item) item##List

  template  <class _Target,  class _Source>
  _Target model_static_cast(_Source item)
  {
    typedef typename _Target::Type * _Target_pointer;

    _Target ptr =  static_cast<_Target_pointer>(item.data());
    return  ptr;
  }

  class CodeModel :  public KDevCodeModel
    {

    public:
      CodeModel( QObject *parent =  0 );
      virtual ~CodeModel();

      template  <class _Target>
      _Target create()
      {
        typedef typename _Target::Type _Target_type;

        _Target result =  _Target_type::create(this);
        return  result;
      }

      void addCodeItem(CodeModelItem item);
      void removeCodeItem(CodeModelItem item);

      void wipeout();

    public:
      GlobalNamespaceDeclarationModelItem globalNamespace() const;
      KDevItemCollection *root() const;

    private:
      GlobalNamespaceDeclarationModelItem _M_globalNamespace;

    private:
      CodeModel(const CodeModel &other);
      void operator=(const CodeModel &other);
    };

  class _CodeModelItem :  public KDevCodeItem
    {

    public:
      enum Kind
      {
        /* These are bit-flags resembling inheritance */
        Kind_Scope =  1,
        Kind_NamespaceDeclaration =  1 << 1 /*| Kind_Scope*/,
        Kind_TypeDeclaration =  1 << 2 /*| Kind_Scope*/,
        Kind_ClassLikeDeclaration =  1 << 3 /*| Kind_TypeDeclaration*/,
        Kind_MethodDeclaration =  1 << 4 /*| Kind_Scope*/,
        Kind_TypeParameterConstraint =  1 << 5,
        KindMask =  (1 << 6) -  1,

        /* These are for classes that are not inherited from */
        FirstKind =  1 << 6,
        Kind_GlobalNamespaceDeclaration =  1 << 7 /*| Kind_NamespaceDeclaration*/,
        Kind_ExternAliasDirective =  1 << 8,
        Kind_UsingAliasDirective =  1 << 9,
        Kind_UsingNamespaceDirective =  1 << 10,
        Kind_ClassDeclaration =  1 << 11 /*| Kind_ClassLikeDeclaration*/,
        Kind_StructDeclaration =  1 << 12 /*| Kind_ClassLikeDeclaration*/,
        Kind_InterfaceDeclaration =  1 << 13 /*| Kind_ClassLikeDeclaration*/,
        Kind_DelegateDeclaration =  1 << 14 /*| Kind_TypeDeclaration*/,
        Kind_EnumDeclaration =  1 << 15 /*| Kind_TypeDeclaration*/,
        Kind_EnumValue =  1 << 16,
        Kind_EventDeclaration =  1 << 17 /*| Kind_Scope*/,
        Kind_EventAccessorDeclaration =  1 << 18,
        Kind_IndexerDeclaration =  1 << 19 /*| Kind_Scope*/,
        Kind_PropertyDeclaration =  1 << 20 /*| Kind_Scope*/,
        Kind_AccessorDeclaration =  1 << 21,
        Kind_ConstructorDeclaration =  1 << 22 /*| Kind_MethodDeclaration*/,
        Kind_FinalizerDeclaration =  1 << 23 /*| Kind_MethodDeclaration*/,
        Kind_VariableDeclaration =  1 << 24,
        Kind_TypePart =  1 << 25,
        Kind_Type =  1 << 26,
        Kind_Parameter =  1 << 27,
        Kind_TypeParameter =  1 << 28,
        Kind_PrimaryOrSecondaryConstraint =  1 << 29 /*| Kind_TypeParameterConstraint*/,
        Kind_ConstructorConstraint =  1 << 30 /*| Kind_TypeParameterConstraint*/
      };

    public:
      virtual ~_CodeModelItem();

      virtual _CodeModelItem *itemAt(int index) const;

      int kind() const;

      KTextEditor::Cursor startPosition() const;
      void setStartPosition(const KTextEditor::Cursor& cursor);

      KTextEditor::Cursor endPosition() const;
      void setEndPosition(const KTextEditor::Cursor& cursor);

      QTime timestamp() const
        {
          return  QTime();
        }

      inline CodeModel *model() const
        {
          return  _M_model;
        }

      CodeModelItem toItem() const;

    private:
      CodeModel *_M_model;
      int _M_kind;
      int _M_startLine;
      int _M_startColumn;
      int _M_endLine;
      int _M_endColumn;

    public:
      QStringList qualifiedName() const;

      // TODO: These methods should be cached upon initialization
      QString display() const;
      QIcon decoration() const;
      QString toolTip() const;
      QString whatsThis() const;

    public:
      QString name() const;
      void setName(QString name);

      QStringList scope() const;
      void setScope(QStringList scope);

      QString fileName() const;
      void setFileName(QString fileName);

    private:
      QString _M_name;
      QStringList _M_scope;
      QString _M_fileName;

    protected:
      _CodeModelItem(CodeModel *model,  int kind);
      void setKind(int kind);

    private:
      _CodeModelItem(const _CodeModelItem &other);
      void operator=(const _CodeModelItem &other);
    };

  class _ScopeModelItem :  public _CodeModelItem
    {

    public:
      DECLARE_MODEL_NODE(Scope)

      static ScopeModelItem create(CodeModel *model);
      virtual ~_ScopeModelItem();

    public:
      QStringList context()
      {
        if  (name().isNull())
          return  scope();
        else
          return  scope() <<  name();
      }

    public:

    private:

    protected:
      _ScopeModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _ScopeModelItem(const _ScopeModelItem &other);
      void operator=(const _ScopeModelItem &other);
    };

  class _NamespaceDeclarationModelItem :  public _ScopeModelItem
    {

    public:
      DECLARE_MODEL_NODE(NamespaceDeclaration)

      static NamespaceDeclarationModelItem create(CodeModel *model);
      virtual ~_NamespaceDeclarationModelItem();

    public:
      NamespaceDeclarationList namespaces() const;
      void addNamespace(NamespaceDeclarationModelItem item);
      void removeNamespace(NamespaceDeclarationModelItem item);
      NamespaceDeclarationModelItem findNamespace(const QString &name) const;

      ExternAliasDirectiveList externAliases() const;
      void addExternAlias(ExternAliasDirectiveModelItem item);
      void removeExternAlias(ExternAliasDirectiveModelItem item);
      ExternAliasDirectiveModelItem findExternAlias(const QString &name) const;

      UsingAliasDirectiveList usingAliases() const;
      void addUsingAlias(UsingAliasDirectiveModelItem item);
      void removeUsingAlias(UsingAliasDirectiveModelItem item);
      UsingAliasDirectiveModelItem findUsingAlias(const QString &name) const;

      UsingNamespaceDirectiveList usingNamespaces() const;
      void addUsingNamespace(UsingNamespaceDirectiveModelItem item);
      void removeUsingNamespace(UsingNamespaceDirectiveModelItem item);

      ClassDeclarationList classes() const;
      void addClass(ClassDeclarationModelItem item);
      void removeClass(ClassDeclarationModelItem item);
      ClassDeclarationModelItem findClass(const QString &name) const;

      StructDeclarationList structs() const;
      void addStruct(StructDeclarationModelItem item);
      void removeStruct(StructDeclarationModelItem item);
      StructDeclarationModelItem findStruct(const QString &name) const;

      InterfaceDeclarationList interfaces() const;
      void addInterface(InterfaceDeclarationModelItem item);
      void removeInterface(InterfaceDeclarationModelItem item);
      InterfaceDeclarationModelItem findInterface(const QString &name) const;

      EnumDeclarationList enums() const;
      void addEnum(EnumDeclarationModelItem item);
      void removeEnum(EnumDeclarationModelItem item);
      EnumDeclarationModelItem findEnum(const QString &name) const;

      DelegateDeclarationList delegates() const;
      void addDelegate(DelegateDeclarationModelItem item);
      void removeDelegate(DelegateDeclarationModelItem item);
      DelegateDeclarationModelItem findDelegate(const QString &name) const;

      inline QHash<QString,  NamespaceDeclarationModelItem> namespaceMap() const
        {
          return  _M_namespaces;
        }

      inline QHash<QString,  ExternAliasDirectiveModelItem> externAliasMap() const
        {
          return  _M_externAliases;
        }

      inline QHash<QString,  UsingAliasDirectiveModelItem> usingAliasMap() const
        {
          return  _M_usingAliases;
        }

      inline QHash<QString,  ClassDeclarationModelItem> classMap() const
        {
          return  _M_classes;
        }

      inline QHash<QString,  StructDeclarationModelItem> structMap() const
        {
          return  _M_structs;
        }

      inline QHash<QString,  InterfaceDeclarationModelItem> interfaceMap() const
        {
          return  _M_interfaces;
        }

      inline QHash<QString,  EnumDeclarationModelItem> enumMap() const
        {
          return  _M_enums;
        }

      inline QHash<QString,  DelegateDeclarationModelItem> delegateMap() const
        {
          return  _M_delegates;
        }

    private:
      QHash<QString,  NamespaceDeclarationModelItem> _M_namespaces;
      QHash<QString,  ExternAliasDirectiveModelItem> _M_externAliases;
      QHash<QString,  UsingAliasDirectiveModelItem> _M_usingAliases;
      UsingNamespaceDirectiveList _M_usingNamespaces;
      QHash<QString,  ClassDeclarationModelItem> _M_classes;
      QHash<QString,  StructDeclarationModelItem> _M_structs;
      QHash<QString,  InterfaceDeclarationModelItem> _M_interfaces;
      QHash<QString,  EnumDeclarationModelItem> _M_enums;
      QHash<QString,  DelegateDeclarationModelItem> _M_delegates;

    protected:
      _NamespaceDeclarationModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _NamespaceDeclarationModelItem(const _NamespaceDeclarationModelItem &other);
      void operator=(const _NamespaceDeclarationModelItem &other);
    };

  class _GlobalNamespaceDeclarationModelItem :  public _NamespaceDeclarationModelItem
    {

    public:
      DECLARE_MODEL_NODE(GlobalNamespaceDeclaration)

      static GlobalNamespaceDeclarationModelItem create(CodeModel *model);
      virtual ~_GlobalNamespaceDeclarationModelItem();

    public:

    private:

    protected:
      _GlobalNamespaceDeclarationModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _GlobalNamespaceDeclarationModelItem(const _GlobalNamespaceDeclarationModelItem &other);
      void operator=(const _GlobalNamespaceDeclarationModelItem &other);
    };

  class _ExternAliasDirectiveModelItem :  public _CodeModelItem
    {

    public:
      DECLARE_MODEL_NODE(ExternAliasDirective)

      static ExternAliasDirectiveModelItem create(CodeModel *model);
      virtual ~_ExternAliasDirectiveModelItem();

    public:

    private:

    protected:
      _ExternAliasDirectiveModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _ExternAliasDirectiveModelItem(const _ExternAliasDirectiveModelItem &other);
      void operator=(const _ExternAliasDirectiveModelItem &other);
    };

  class _UsingAliasDirectiveModelItem :  public _CodeModelItem
    {

    public:
      DECLARE_MODEL_NODE(UsingAliasDirective)

      static UsingAliasDirectiveModelItem create(CodeModel *model);
      virtual ~_UsingAliasDirectiveModelItem();

    public:
      TypeModelItem namespaceOrType() const;
      void setNamespaceOrType(TypeModelItem namespaceOrType);

    private:
      TypeModelItem _M_namespaceOrType;

    protected:
      _UsingAliasDirectiveModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _UsingAliasDirectiveModelItem(const _UsingAliasDirectiveModelItem &other);
      void operator=(const _UsingAliasDirectiveModelItem &other);
    };

  class _UsingNamespaceDirectiveModelItem :  public _CodeModelItem
    {

    public:
      DECLARE_MODEL_NODE(UsingNamespaceDirective)

      static UsingNamespaceDirectiveModelItem create(CodeModel *model);
      virtual ~_UsingNamespaceDirectiveModelItem();

    public:
      QStringList namespaceUsed() const;
      void setNamespaceUsed(QStringList namespaceUsed);

    private:
      QStringList _M_namespaceUsed;

    protected:
      _UsingNamespaceDirectiveModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _UsingNamespaceDirectiveModelItem(const _UsingNamespaceDirectiveModelItem &other);
      void operator=(const _UsingNamespaceDirectiveModelItem &other);
    };

  class _TypeDeclarationModelItem :  public _ScopeModelItem
    {

    public:
      DECLARE_MODEL_NODE(TypeDeclaration)

      static TypeDeclarationModelItem create(CodeModel *model);
      virtual ~_TypeDeclarationModelItem();

    public:
      access_policy::access_policy_enum accessPolicy() const;
      void setAccessPolicy(access_policy::access_policy_enum accessPolicy);

      bool isNew() const;
      void setNew(bool isNew);

    private:
      access_policy::access_policy_enum _M_accessPolicy;
      bool _M_isNew;

    protected:
      _TypeDeclarationModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _TypeDeclarationModelItem(const _TypeDeclarationModelItem &other);
      void operator=(const _TypeDeclarationModelItem &other);
    };

  class _ClassLikeDeclarationModelItem :  public _TypeDeclarationModelItem
    {

    public:
      DECLARE_MODEL_NODE(ClassLikeDeclaration)

      static ClassLikeDeclarationModelItem create(CodeModel *model);
      virtual ~_ClassLikeDeclarationModelItem();

    public:
      ITEM(MethodDeclaration) declaredMethod(ITEM(MethodDeclaration) item);

    public:
      TypeModelItem baseType() const;
      void setBaseType(TypeModelItem baseType);

      EventDeclarationList events() const;
      void addEvent(EventDeclarationModelItem item);
      void removeEvent(EventDeclarationModelItem item);
      EventDeclarationModelItem findEvent(const QString &name) const;

      IndexerDeclarationList indexers() const;
      void addIndexer(IndexerDeclarationModelItem item);
      void removeIndexer(IndexerDeclarationModelItem item);

      PropertyDeclarationList propertys() const;
      void addProperty(PropertyDeclarationModelItem item);
      void removeProperty(PropertyDeclarationModelItem item);
      PropertyDeclarationModelItem findProperty(const QString &name) const;

      MethodDeclarationList methods() const;
      void addMethod(MethodDeclarationModelItem item);
      void removeMethod(MethodDeclarationModelItem item);
      MethodDeclarationList findMethods(const QString &name) const;

      TypeParameterList typeParameters() const;
      void addTypeParameter(TypeParameterModelItem item);
      void removeTypeParameter(TypeParameterModelItem item);

      TypeParameterConstraintList typeParameterConstraints() const;
      void addTypeParameterConstraint(TypeParameterConstraintModelItem item);
      void removeTypeParameterConstraint(TypeParameterConstraintModelItem item);

      bool isUnsafe() const;
      void setUnsafe(bool isUnsafe);

      ClassDeclarationList classes() const;
      void addClass(ClassDeclarationModelItem item);
      void removeClass(ClassDeclarationModelItem item);
      ClassDeclarationModelItem findClass(const QString &name) const;

      StructDeclarationList structs() const;
      void addStruct(StructDeclarationModelItem item);
      void removeStruct(StructDeclarationModelItem item);
      StructDeclarationModelItem findStruct(const QString &name) const;

      InterfaceDeclarationList interfaces() const;
      void addInterface(InterfaceDeclarationModelItem item);
      void removeInterface(InterfaceDeclarationModelItem item);
      InterfaceDeclarationModelItem findInterface(const QString &name) const;

      EnumDeclarationList enums() const;
      void addEnum(EnumDeclarationModelItem item);
      void removeEnum(EnumDeclarationModelItem item);
      EnumDeclarationModelItem findEnum(const QString &name) const;

      DelegateDeclarationList delegates() const;
      void addDelegate(DelegateDeclarationModelItem item);
      void removeDelegate(DelegateDeclarationModelItem item);
      DelegateDeclarationModelItem findDelegate(const QString &name) const;

      VariableDeclarationList variables() const;
      void addVariable(VariableDeclarationModelItem item);
      void removeVariable(VariableDeclarationModelItem item);
      VariableDeclarationModelItem findVariable(const QString &name) const;

      inline QHash<QString,  EventDeclarationModelItem> eventMap() const
        {
          return  _M_events;
        }

      inline QHash<QString,  PropertyDeclarationModelItem> propertyMap() const
        {
          return  _M_propertys;
        }

      inline QMultiHash<QString,  MethodDeclarationModelItem> methodMap() const
        {
          return  _M_methods;
        }

      inline QHash<QString,  ClassDeclarationModelItem> classMap() const
        {
          return  _M_classes;
        }

      inline QHash<QString,  StructDeclarationModelItem> structMap() const
        {
          return  _M_structs;
        }

      inline QHash<QString,  InterfaceDeclarationModelItem> interfaceMap() const
        {
          return  _M_interfaces;
        }

      inline QHash<QString,  EnumDeclarationModelItem> enumMap() const
        {
          return  _M_enums;
        }

      inline QHash<QString,  DelegateDeclarationModelItem> delegateMap() const
        {
          return  _M_delegates;
        }

      inline QHash<QString,  VariableDeclarationModelItem> variableMap() const
        {
          return  _M_variables;
        }

    private:
      TypeModelItem _M_baseType;
      QHash<QString,  EventDeclarationModelItem> _M_events;
      IndexerDeclarationList _M_indexers;
      QHash<QString,  PropertyDeclarationModelItem> _M_propertys;
      QMultiHash<QString,  MethodDeclarationModelItem> _M_methods;
      TypeParameterList _M_typeParameters;
      TypeParameterConstraintList _M_typeParameterConstraints;
      bool _M_isUnsafe;
      QHash<QString,  ClassDeclarationModelItem> _M_classes;
      QHash<QString,  StructDeclarationModelItem> _M_structs;
      QHash<QString,  InterfaceDeclarationModelItem> _M_interfaces;
      QHash<QString,  EnumDeclarationModelItem> _M_enums;
      QHash<QString,  DelegateDeclarationModelItem> _M_delegates;
      QHash<QString,  VariableDeclarationModelItem> _M_variables;

    protected:
      _ClassLikeDeclarationModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _ClassLikeDeclarationModelItem(const _ClassLikeDeclarationModelItem &other);
      void operator=(const _ClassLikeDeclarationModelItem &other);
    };

  class _ClassDeclarationModelItem :  public _ClassLikeDeclarationModelItem
    {

    public:
      DECLARE_MODEL_NODE(ClassDeclaration)

      static ClassDeclarationModelItem create(CodeModel *model);
      virtual ~_ClassDeclarationModelItem();

    public:
      bool isSealed() const;
      void setSealed(bool isSealed);

      bool isAbstract() const;
      void setAbstract(bool isAbstract);

      bool isStatic() const;
      void setStatic(bool isStatic);

    private:
      bool _M_isSealed;
      bool _M_isAbstract;
      bool _M_isStatic;

    protected:
      _ClassDeclarationModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _ClassDeclarationModelItem(const _ClassDeclarationModelItem &other);
      void operator=(const _ClassDeclarationModelItem &other);
    };

  class _StructDeclarationModelItem :  public _ClassLikeDeclarationModelItem
    {

    public:
      DECLARE_MODEL_NODE(StructDeclaration)

      static StructDeclarationModelItem create(CodeModel *model);
      virtual ~_StructDeclarationModelItem();

    public:

    private:

    protected:
      _StructDeclarationModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _StructDeclarationModelItem(const _StructDeclarationModelItem &other);
      void operator=(const _StructDeclarationModelItem &other);
    };

  class _InterfaceDeclarationModelItem :  public _ClassLikeDeclarationModelItem
    {

    public:
      DECLARE_MODEL_NODE(InterfaceDeclaration)

      static InterfaceDeclarationModelItem create(CodeModel *model);
      virtual ~_InterfaceDeclarationModelItem();

    public:

    private:

    protected:
      _InterfaceDeclarationModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _InterfaceDeclarationModelItem(const _InterfaceDeclarationModelItem &other);
      void operator=(const _InterfaceDeclarationModelItem &other);
    };

  class _DelegateDeclarationModelItem :  public _TypeDeclarationModelItem
    {

    public:
      DECLARE_MODEL_NODE(DelegateDeclaration)

      static DelegateDeclarationModelItem create(CodeModel *model);
      virtual ~_DelegateDeclarationModelItem();

    public:
      TypeModelItem returnType() const;
      void setReturnType(TypeModelItem returnType);

      ParameterList parameters() const;
      void addParameter(ParameterModelItem item);
      void removeParameter(ParameterModelItem item);

      TypeParameterList typeParameters() const;
      void addTypeParameter(TypeParameterModelItem item);
      void removeTypeParameter(TypeParameterModelItem item);

      TypeParameterConstraintList typeParameterConstraints() const;
      void addTypeParameterConstraint(TypeParameterConstraintModelItem item);
      void removeTypeParameterConstraint(TypeParameterConstraintModelItem item);

      bool isUnsafe() const;
      void setUnsafe(bool isUnsafe);

    private:
      TypeModelItem _M_returnType;
      ParameterList _M_parameters;
      TypeParameterList _M_typeParameters;
      TypeParameterConstraintList _M_typeParameterConstraints;
      bool _M_isUnsafe;

    protected:
      _DelegateDeclarationModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _DelegateDeclarationModelItem(const _DelegateDeclarationModelItem &other);
      void operator=(const _DelegateDeclarationModelItem &other);
    };

  class _EnumDeclarationModelItem :  public _TypeDeclarationModelItem
    {

    public:
      DECLARE_MODEL_NODE(EnumDeclaration)

      static EnumDeclarationModelItem create(CodeModel *model);
      virtual ~_EnumDeclarationModelItem();

    public:
      EnumValueList enumValues() const;
      void addEnumValue(EnumValueModelItem item);
      void removeEnumValue(EnumValueModelItem item);

      TypeModelItem baseIntegralType() const;
      void setBaseIntegralType(TypeModelItem baseIntegralType);

    private:
      EnumValueList _M_enumValues;
      TypeModelItem _M_baseIntegralType;

    protected:
      _EnumDeclarationModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _EnumDeclarationModelItem(const _EnumDeclarationModelItem &other);
      void operator=(const _EnumDeclarationModelItem &other);
    };

  class _EnumValueModelItem :  public _CodeModelItem
    {

    public:
      DECLARE_MODEL_NODE(EnumValue)

      static EnumValueModelItem create(CodeModel *model);
      virtual ~_EnumValueModelItem();

    public:
      QString value() const;
      void setValue(QString value);

    private:
      QString _M_value;

    protected:
      _EnumValueModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _EnumValueModelItem(const _EnumValueModelItem &other);
      void operator=(const _EnumValueModelItem &other);
    };

  class _EventDeclarationModelItem :  public _ScopeModelItem
    {

    public:
      DECLARE_MODEL_NODE(EventDeclaration)

      static EventDeclarationModelItem create(CodeModel *model);
      virtual ~_EventDeclarationModelItem();

    public:
      TypeModelItem type() const;
      void setType(TypeModelItem type);

      TypeModelItem fromInterface() const;
      void setFromInterface(TypeModelItem fromInterface);

      EventAccessorDeclarationModelItem addAccessor() const;
      void setAddAccessor(EventAccessorDeclarationModelItem addAccessor);

      EventAccessorDeclarationModelItem removeAccessor() const;
      void setRemoveAccessor(EventAccessorDeclarationModelItem removeAccessor);

      access_policy::access_policy_enum accessPolicy() const;
      void setAccessPolicy(access_policy::access_policy_enum accessPolicy);

      bool isNew() const;
      void setNew(bool isNew);

      bool isStatic() const;
      void setStatic(bool isStatic);

      bool isVirtual() const;
      void setVirtual(bool isVirtual);

      bool isSealed() const;
      void setSealed(bool isSealed);

      bool isOverride() const;
      void setOverride(bool isOverride);

      bool isAbstract() const;
      void setAbstract(bool isAbstract);

      bool isExtern() const;
      void setExtern(bool isExtern);

      bool isUnsafe() const;
      void setUnsafe(bool isUnsafe);

    private:
      TypeModelItem _M_type;
      TypeModelItem _M_fromInterface;
      EventAccessorDeclarationModelItem _M_addAccessor;
      EventAccessorDeclarationModelItem _M_removeAccessor;
      access_policy::access_policy_enum _M_accessPolicy;
      bool _M_isNew;
      bool _M_isStatic;
      bool _M_isVirtual;
      bool _M_isSealed;
      bool _M_isOverride;
      bool _M_isAbstract;
      bool _M_isExtern;
      bool _M_isUnsafe;

    protected:
      _EventDeclarationModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _EventDeclarationModelItem(const _EventDeclarationModelItem &other);
      void operator=(const _EventDeclarationModelItem &other);
    };

  class _EventAccessorDeclarationModelItem :  public _CodeModelItem
    {

    public:
      DECLARE_MODEL_NODE(EventAccessorDeclaration)

      static EventAccessorDeclarationModelItem create(CodeModel *model);
      virtual ~_EventAccessorDeclarationModelItem();

    public:

    private:

    protected:
      _EventAccessorDeclarationModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _EventAccessorDeclarationModelItem(const _EventAccessorDeclarationModelItem &other);
      void operator=(const _EventAccessorDeclarationModelItem &other);
    };

  class _IndexerDeclarationModelItem :  public _ScopeModelItem
    {

    public:
      DECLARE_MODEL_NODE(IndexerDeclaration)

      static IndexerDeclarationModelItem create(CodeModel *model);
      virtual ~_IndexerDeclarationModelItem();

    public:
      TypeModelItem type() const;
      void setType(TypeModelItem type);

      TypeModelItem fromInterface() const;
      void setFromInterface(TypeModelItem fromInterface);

      ParameterList parameters() const;
      void addParameter(ParameterModelItem item);
      void removeParameter(ParameterModelItem item);

      AccessorDeclarationList accessors() const;
      void addAccessor(AccessorDeclarationModelItem item);
      void removeAccessor(AccessorDeclarationModelItem item);

      access_policy::access_policy_enum accessPolicy() const;
      void setAccessPolicy(access_policy::access_policy_enum accessPolicy);

      bool isNew() const;
      void setNew(bool isNew);

      bool isVirtual() const;
      void setVirtual(bool isVirtual);

      bool isSealed() const;
      void setSealed(bool isSealed);

      bool isOverride() const;
      void setOverride(bool isOverride);

      bool isAbstract() const;
      void setAbstract(bool isAbstract);

      bool isExtern() const;
      void setExtern(bool isExtern);

      bool isUnsafe() const;
      void setUnsafe(bool isUnsafe);

    private:
      TypeModelItem _M_type;
      TypeModelItem _M_fromInterface;
      ParameterList _M_parameters;
      AccessorDeclarationList _M_accessors;
      access_policy::access_policy_enum _M_accessPolicy;
      bool _M_isNew;
      bool _M_isVirtual;
      bool _M_isSealed;
      bool _M_isOverride;
      bool _M_isAbstract;
      bool _M_isExtern;
      bool _M_isUnsafe;

    protected:
      _IndexerDeclarationModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _IndexerDeclarationModelItem(const _IndexerDeclarationModelItem &other);
      void operator=(const _IndexerDeclarationModelItem &other);
    };

  class _PropertyDeclarationModelItem :  public _ScopeModelItem
    {

    public:
      DECLARE_MODEL_NODE(PropertyDeclaration)

      static PropertyDeclarationModelItem create(CodeModel *model);
      virtual ~_PropertyDeclarationModelItem();

    public:
      TypeModelItem type() const;
      void setType(TypeModelItem type);

      TypeModelItem fromInterface() const;
      void setFromInterface(TypeModelItem fromInterface);

      AccessorDeclarationList accessors() const;
      void addAccessor(AccessorDeclarationModelItem item);
      void removeAccessor(AccessorDeclarationModelItem item);

      access_policy::access_policy_enum accessPolicy() const;
      void setAccessPolicy(access_policy::access_policy_enum accessPolicy);

      bool isNew() const;
      void setNew(bool isNew);

      bool isStatic() const;
      void setStatic(bool isStatic);

      bool isVirtual() const;
      void setVirtual(bool isVirtual);

      bool isSealed() const;
      void setSealed(bool isSealed);

      bool isOverride() const;
      void setOverride(bool isOverride);

      bool isAbstract() const;
      void setAbstract(bool isAbstract);

      bool isExtern() const;
      void setExtern(bool isExtern);

      bool isUnsafe() const;
      void setUnsafe(bool isUnsafe);

    private:
      TypeModelItem _M_type;
      TypeModelItem _M_fromInterface;
      AccessorDeclarationList _M_accessors;
      access_policy::access_policy_enum _M_accessPolicy;
      bool _M_isNew;
      bool _M_isStatic;
      bool _M_isVirtual;
      bool _M_isSealed;
      bool _M_isOverride;
      bool _M_isAbstract;
      bool _M_isExtern;
      bool _M_isUnsafe;

    protected:
      _PropertyDeclarationModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _PropertyDeclarationModelItem(const _PropertyDeclarationModelItem &other);
      void operator=(const _PropertyDeclarationModelItem &other);
    };

  class _AccessorDeclarationModelItem :  public _CodeModelItem
    {

    public:
      DECLARE_MODEL_NODE(AccessorDeclaration)

      static AccessorDeclarationModelItem create(CodeModel *model);
      virtual ~_AccessorDeclarationModelItem();

    public:
      accessor_declarations::accessor_type_enum type() const;
      void setType(accessor_declarations::accessor_type_enum type);

      access_policy::access_policy_enum accessPolicy() const;
      void setAccessPolicy(access_policy::access_policy_enum accessPolicy);

      bool hasAccessPolicy() const;
      void setHasAccessPolicy(bool hasAccessPolicy);

    private:
      accessor_declarations::accessor_type_enum _M_type;
      access_policy::access_policy_enum _M_accessPolicy;
      bool _M_hasAccessPolicy;

    protected:
      _AccessorDeclarationModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _AccessorDeclarationModelItem(const _AccessorDeclarationModelItem &other);
      void operator=(const _AccessorDeclarationModelItem &other);
    };

  class _MethodDeclarationModelItem :  public _ScopeModelItem
    {

    public:
      DECLARE_MODEL_NODE(MethodDeclaration)

      static MethodDeclarationModelItem create(CodeModel *model);
      virtual ~_MethodDeclarationModelItem();

    public:
      TypeModelItem returnType() const;
      void setReturnType(TypeModelItem returnType);

      TypeModelItem fromInterface() const;
      void setFromInterface(TypeModelItem fromInterface);

      TypeParameterList typeParameters() const;
      void addTypeParameter(TypeParameterModelItem item);
      void removeTypeParameter(TypeParameterModelItem item);

      TypeParameterConstraintList typeParameterConstraints() const;
      void addTypeParameterConstraint(TypeParameterConstraintModelItem item);
      void removeTypeParameterConstraint(TypeParameterConstraintModelItem item);

      ParameterList parameters() const;
      void addParameter(ParameterModelItem item);
      void removeParameter(ParameterModelItem item);

      bool isInterfaceMethodDeclaration() const;
      void setInterfaceMethodDeclaration(bool isInterfaceMethodDeclaration);

      access_policy::access_policy_enum accessPolicy() const;
      void setAccessPolicy(access_policy::access_policy_enum accessPolicy);

      bool isNew() const;
      void setNew(bool isNew);

      bool isStatic() const;
      void setStatic(bool isStatic);

      bool isVirtual() const;
      void setVirtual(bool isVirtual);

      bool isSealed() const;
      void setSealed(bool isSealed);

      bool isOverride() const;
      void setOverride(bool isOverride);

      bool isAbstract() const;
      void setAbstract(bool isAbstract);

      bool isExtern() const;
      void setExtern(bool isExtern);

      bool isUnsafe() const;
      void setUnsafe(bool isUnsafe);

      VariableDeclarationList localVariables() const;
      void addLocalVariable(VariableDeclarationModelItem item);
      void removeLocalVariable(VariableDeclarationModelItem item);

    private:
      TypeModelItem _M_returnType;
      TypeModelItem _M_fromInterface;
      TypeParameterList _M_typeParameters;
      TypeParameterConstraintList _M_typeParameterConstraints;
      ParameterList _M_parameters;
      bool _M_isInterfaceMethodDeclaration;
      access_policy::access_policy_enum _M_accessPolicy;
      bool _M_isNew;
      bool _M_isStatic;
      bool _M_isVirtual;
      bool _M_isSealed;
      bool _M_isOverride;
      bool _M_isAbstract;
      bool _M_isExtern;
      bool _M_isUnsafe;
      VariableDeclarationList _M_localVariables;

    protected:
      _MethodDeclarationModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _MethodDeclarationModelItem(const _MethodDeclarationModelItem &other);
      void operator=(const _MethodDeclarationModelItem &other);
    };

  class _ConstructorDeclarationModelItem :  public _MethodDeclarationModelItem
    {

    public:
      DECLARE_MODEL_NODE(ConstructorDeclaration)

      static ConstructorDeclarationModelItem create(CodeModel *model);
      virtual ~_ConstructorDeclarationModelItem();

    public:

    private:

    protected:
      _ConstructorDeclarationModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _ConstructorDeclarationModelItem(const _ConstructorDeclarationModelItem &other);
      void operator=(const _ConstructorDeclarationModelItem &other);
    };

  class _FinalizerDeclarationModelItem :  public _MethodDeclarationModelItem
    {

    public:
      DECLARE_MODEL_NODE(FinalizerDeclaration)

      static FinalizerDeclarationModelItem create(CodeModel *model);
      virtual ~_FinalizerDeclarationModelItem();

    public:

    private:

    protected:
      _FinalizerDeclarationModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _FinalizerDeclarationModelItem(const _FinalizerDeclarationModelItem &other);
      void operator=(const _FinalizerDeclarationModelItem &other);
    };

  class _VariableDeclarationModelItem :  public _CodeModelItem
    {

    public:
      DECLARE_MODEL_NODE(VariableDeclaration)

      static VariableDeclarationModelItem create(CodeModel *model);
      virtual ~_VariableDeclarationModelItem();

    public:
      TypeModelItem type() const;
      void setType(TypeModelItem type);

      access_policy::access_policy_enum accessPolicy() const;
      void setAccessPolicy(access_policy::access_policy_enum accessPolicy);

      bool isConstant() const;
      void setConstant(bool isConstant);

      bool isNew() const;
      void setNew(bool isNew);

      bool isStatic() const;
      void setStatic(bool isStatic);

      bool isReadonly() const;
      void setReadonly(bool isReadonly);

      bool isVolatile() const;
      void setVolatile(bool isVolatile);

      bool isUnsafe() const;
      void setUnsafe(bool isUnsafe);

    private:
      TypeModelItem _M_type;
      access_policy::access_policy_enum _M_accessPolicy;
      bool _M_isConstant;
      bool _M_isNew;
      bool _M_isStatic;
      bool _M_isReadonly;
      bool _M_isVolatile;
      bool _M_isUnsafe;

    protected:
      _VariableDeclarationModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _VariableDeclarationModelItem(const _VariableDeclarationModelItem &other);
      void operator=(const _VariableDeclarationModelItem &other);
    };

  class _TypePartModelItem :  public _CodeModelItem
    {

    public:
      DECLARE_MODEL_NODE(TypePart)

      static TypePartModelItem create(CodeModel *model);
      virtual ~_TypePartModelItem();

    public:
      bool operator==( const _TypePartModelItem &other );
      bool operator!=( const _TypePartModelItem &other )
      {
        return  !(*this ==  other);
      }

      QString toString() const;

    public:
      TypeList typeArguments() const;
      void addTypeArgument(TypeModelItem item);
      void removeTypeArgument(TypeModelItem item);

    private:
      TypeList _M_typeArguments;

    protected:
      _TypePartModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _TypePartModelItem(const _TypePartModelItem &other);
      void operator=(const _TypePartModelItem &other);
    };

  class _TypeModelItem :  public _CodeModelItem
    {

    public:
      DECLARE_MODEL_NODE(Type)

      static TypeModelItem create(CodeModel *model);
      virtual ~_TypeModelItem();

    public:
      bool operator==( const _TypeModelItem &other );
      bool operator!=( const _TypeModelItem &other )
      {
        return  !(*this ==  other);
      }

      QString toString() const;

    public:
      QString qualifiedAliasLabel() const;
      void setQualifiedAliasLabel(QString qualifiedAliasLabel);

      TypePartList typeParts() const;
      void addTypePart(TypePartModelItem item);
      void removeTypePart(TypePartModelItem item);

    private:
      QString _M_qualifiedAliasLabel;
      TypePartList _M_typeParts;

    protected:
      _TypeModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _TypeModelItem(const _TypeModelItem &other);
      void operator=(const _TypeModelItem &other);
    };

  class _ParameterModelItem :  public _CodeModelItem
    {

    public:
      DECLARE_MODEL_NODE(Parameter)

      static ParameterModelItem create(CodeModel *model);
      virtual ~_ParameterModelItem();

    public:
      QString toString() const
        {
          Q_ASSERT( type() );
          return  type()->toString() +  " " +  name();
        }

    public:
      TypeModelItem type() const;
      void setType(TypeModelItem type);

      bool isArray() const;
      void setArray(bool isArray);

      parameter::parameter_type_enum parameterType() const;
      void setParameterType(parameter::parameter_type_enum parameterType);

    private:
      TypeModelItem _M_type;
      bool _M_isArray;
      parameter::parameter_type_enum _M_parameterType;

    protected:
      _ParameterModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _ParameterModelItem(const _ParameterModelItem &other);
      void operator=(const _ParameterModelItem &other);
    };

  class _TypeParameterModelItem :  public _CodeModelItem
    {

    public:
      DECLARE_MODEL_NODE(TypeParameter)

      static TypeParameterModelItem create(CodeModel *model);
      virtual ~_TypeParameterModelItem();

    public:

    private:

    protected:
      _TypeParameterModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _TypeParameterModelItem(const _TypeParameterModelItem &other);
      void operator=(const _TypeParameterModelItem &other);
    };

  class _TypeParameterConstraintModelItem :  public _CodeModelItem
    {

    public:
      DECLARE_MODEL_NODE(TypeParameterConstraint)

      static TypeParameterConstraintModelItem create(CodeModel *model);
      virtual ~_TypeParameterConstraintModelItem();

    public:

    private:

    protected:
      _TypeParameterConstraintModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _TypeParameterConstraintModelItem(const _TypeParameterConstraintModelItem &other);
      void operator=(const _TypeParameterConstraintModelItem &other);
    };

  class _PrimaryOrSecondaryConstraintModelItem :  public _TypeParameterConstraintModelItem
    {

    public:
      DECLARE_MODEL_NODE(PrimaryOrSecondaryConstraint)

      static PrimaryOrSecondaryConstraintModelItem create(CodeModel *model);
      virtual ~_PrimaryOrSecondaryConstraintModelItem();

    public:
      TypeModelItem typeOrParameterName() const;
      void setTypeOrParameterName(TypeModelItem typeOrParameterName);

      primary_or_secondary_constraint::primary_or_secondary_constraint_enum constraint_type() const;
      void setConstraint_type(primary_or_secondary_constraint::primary_or_secondary_constraint_enum constraint_type);

    private:
      TypeModelItem _M_typeOrParameterName;
      primary_or_secondary_constraint::primary_or_secondary_constraint_enum _M_constraint_type;

    protected:
      _PrimaryOrSecondaryConstraintModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _PrimaryOrSecondaryConstraintModelItem(const _PrimaryOrSecondaryConstraintModelItem &other);
      void operator=(const _PrimaryOrSecondaryConstraintModelItem &other);
    };

  class _ConstructorConstraintModelItem :  public _TypeParameterConstraintModelItem
    {

    public:
      DECLARE_MODEL_NODE(ConstructorConstraint)

      static ConstructorConstraintModelItem create(CodeModel *model);
      virtual ~_ConstructorConstraintModelItem();

    public:

    private:

    protected:
      _ConstructorConstraintModelItem(CodeModel *model,  int kind =  __node_kind);

    private:
      _ConstructorConstraintModelItem(const _ConstructorConstraintModelItem &other);
      void operator=(const _ConstructorConstraintModelItem &other);
    };

  template  <class _Target,  class _Source>
  _Target model_safe_cast(_Source item)
  {
    typedef typename _Target::Type * _Target_pointer;
    typedef typename _Source::Type * _Source_pointer;

    _Source_pointer source =  item.data();

    if  (source &&  source->kind() ==  _Target_pointer(0)->__node_kind)
      {
        _Target ptr(static_cast<_Target_pointer>(source));
        return  ptr;
      }

    return  _Target();
  }

  template  <typename _Target,  typename _Source>
  _Target model_dynamic_cast(_Source item)
  {
    typedef typename _Target::Type * _Target_pointer;
    typedef typename _Source::Type * _Source_pointer;

    _Source_pointer source =  item.data();

    if  (source &&  (source->kind() ==  _Target_pointer(0)->__node_kind
                     ||  (_Target_pointer(0)->__node_kind <=  int(_CodeModelItem::KindMask)
                          &&  ((source->kind() & _Target_pointer(0)->__node_kind)
                               ==  _Target_pointer(0)->__node_kind))))
      {
        _Target ptr(static_cast<_Target_pointer>(source));
        return  ptr;
      }

    return  _Target();
  }


} // end of namespace csharp

#endif


