// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#include "csharp_codemodel.h"

namespace csharp
  {


  // ---------------------------------------------------------------------------
#define CLASS CodeModel
#define BASECLASS KDevCodeModel

  CodeModel::CodeModel( QObject *parent )
      :  KDevCodeModel( parent )
  {

    _M_globalNamespace =  create < ITEM(GlobalNamespaceDeclaration) > ();
  }

  CodeModel::~CodeModel()
  {

    // FIXME jpetso: don't we need "delete _M_globalNamespace;" here?
  }

  void CodeModel::wipeout()
  {}

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

  GlobalNamespaceDeclarationModelItem CodeModel::globalNamespace() const
    {
      return  _M_globalNamespace;
    }

  KDevItemCollection *CodeModel::root() const
    {
      return  _M_globalNamespace;
    }

#undef CLASS
#undef BASECLASS

  // ---------------------------------------------------------------------------
#define CLASS TypeInfo
#define BASECLASS has_no_base_class!

  QStringList TypeInfo::qualifiedName() const
    {
      return  _M_qualifiedName;
    }

  void TypeInfo::setQualifiedName(QStringList qualifiedName)
  {
    _M_qualifiedName =  qualifiedName;
  }


  QString CLASS::toString() const
    {
      return  qualifiedName().join(".");
    }

  bool CLASS::operator==(const CLASS &other)
  {
    return  qualifiedName() ==  other.qualifiedName();
  }

#undef CLASS
#undef BASECLASS

  // ---------------------------------------------------------------------------
#define CLASS _CodeModelItem
#define BASECLASS has_no_base_class!

  _CodeModelItem::_CodeModelItem(CodeModel *model,  int kind)
      :  KDevCodeItem( QString::null,  0 )
      ,  _M_model(model)
      ,  _M_kind(kind)
      ,  _M_startLine( -1)
      ,  _M_startColumn( -1)
      ,  _M_endLine( -1)
      ,  _M_endColumn( -1)
  {}

  _CodeModelItem::~_CodeModelItem()
  {}

  _CodeModelItem *_CodeModelItem::itemAt(int index) const
    {
      return  static_cast<_CodeModelItem*>(KDevItemCollection::itemAt(index));
    }

  CodeModelItem _CodeModelItem::toItem() const
    {
      return  CodeModelItem(const_cast<_CodeModelItem*>(this));
    }

  int _CodeModelItem::kind() const
    {
      return  _M_kind;
    }

  void _CodeModelItem::setKind(int kind)
  {
    _M_kind =  kind;
  }

  KTextEditor::Cursor _CodeModelItem::startPosition() const
    {
      return  KTextEditor::Cursor(_M_startLine,  _M_startColumn);
    }

  void _CodeModelItem::setStartPosition(const KTextEditor::Cursor& cursor)
  {
    _M_startLine =  cursor.line();
    _M_startColumn =  cursor.column();
  }

  KTextEditor::Cursor _CodeModelItem::endPosition() const
    {
      return  KTextEditor::Cursor(_M_endLine,  _M_endColumn);
    }

  void _CodeModelItem::setEndPosition(const KTextEditor::Cursor& cursor)
  {
    _M_endLine =  cursor.line();
    _M_endColumn =  cursor.column();
  }

  QString _CodeModelItem::name() const
    {
      return  _M_name;
    }

  void _CodeModelItem::setName(QString name)
  {
    _M_name =  name;
  }

  QStringList _CodeModelItem::scope() const
    {
      return  _M_scope;
    }

  void _CodeModelItem::setScope(QStringList scope)
  {
    _M_scope =  scope;
  }

  QString _CodeModelItem::fileName() const
    {
      return  _M_fileName;
    }

  void _CodeModelItem::setFileName(QString fileName)
  {
    _M_fileName =  fileName;
  }


  QStringList CLASS::qualifiedName() const
    {
      QStringList q =  scope();

      if  (!name().isEmpty())
        q +=  name();

      return  q;
    }

  // TODO: These methods should be cached upon initialization
  QString CLASS::display() const
    {
      return  CodeDisplay::display( const_cast<const CLASS*>( this ) );
    }

  QIcon CLASS::decoration() const
    {
      return  CodeDisplay::decoration( const_cast<const CLASS*>( this ) );
    }

  QString CLASS::toolTip() const
    {
      return  CodeDisplay::toolTip( const_cast<const CLASS*>( this ) );
    }

  QString CLASS::whatsThis() const
    {
      return  CodeDisplay::whatsThis( const_cast<const CLASS*>( this ) );
    }

#undef CLASS
#undef BASECLASS

  // ---------------------------------------------------------------------------
#define CLASS _ScopeModelItem
#define BASECLASS _CodeModelItem

  _ScopeModelItem::_ScopeModelItem(CodeModel *model,  int kind)
      :  _CodeModelItem(model,  kind)
  {}

  _ScopeModelItem::~_ScopeModelItem()
  {}

  ScopeModelItem _ScopeModelItem::create(CodeModel *model)
  {
    ScopeModelItem item(new _ScopeModelItem(model));
    return  item;
  }

#undef CLASS
#undef BASECLASS

  // ---------------------------------------------------------------------------
#define CLASS _NamespaceDeclarationModelItem
#define BASECLASS _ScopeModelItem

  _NamespaceDeclarationModelItem::_NamespaceDeclarationModelItem(CodeModel *model,  int kind)
      :  _ScopeModelItem(model,  kind)
  {}

  _NamespaceDeclarationModelItem::~_NamespaceDeclarationModelItem()
  {}

  NamespaceDeclarationModelItem _NamespaceDeclarationModelItem::create(CodeModel *model)
  {
    NamespaceDeclarationModelItem item(new _NamespaceDeclarationModelItem(model));
    return  item;
  }

  NamespaceDeclarationList _NamespaceDeclarationModelItem::namespaces() const
    {
      return  _M_namespaces.values();
    }

  void _NamespaceDeclarationModelItem::addNamespace(NamespaceDeclarationModelItem item)
  {
    if  ( _M_namespaces.contains(item->name()) )
      removeNamespace(_M_namespaces[item->name()]);

    model()->beginAppendItem(item,  this);
    _M_namespaces.insertMulti(item->name(),  item);
    model()->endAppendItem();
  }

  void _NamespaceDeclarationModelItem::removeNamespace(NamespaceDeclarationModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_namespaces.remove(item->name());
    model()->endRemoveItem();
  }

  NamespaceDeclarationModelItem _NamespaceDeclarationModelItem::findNamespace(const QString &name) const
    {
      return  _M_namespaces.value(name);
    }

  ExternAliasDirectiveList _NamespaceDeclarationModelItem::externAliases() const
    {
      return  _M_externAliases.values();
    }

  void _NamespaceDeclarationModelItem::addExternAlias(ExternAliasDirectiveModelItem item)
  {
    if  ( _M_externAliases.contains(item->name()) )
      removeExternAlias(_M_externAliases[item->name()]);

    model()->beginAppendItem(item,  this);
    _M_externAliases.insertMulti(item->name(),  item);
    model()->endAppendItem();
  }

  void _NamespaceDeclarationModelItem::removeExternAlias(ExternAliasDirectiveModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_externAliases.remove(item->name());
    model()->endRemoveItem();
  }

  ExternAliasDirectiveModelItem _NamespaceDeclarationModelItem::findExternAlias(const QString &name) const
    {
      return  _M_externAliases.value(name);
    }

  UsingAliasDirectiveList _NamespaceDeclarationModelItem::usingAliases() const
    {
      return  _M_usingAliases.values();
    }

  void _NamespaceDeclarationModelItem::addUsingAlias(UsingAliasDirectiveModelItem item)
  {
    if  ( _M_usingAliases.contains(item->name()) )
      removeUsingAlias(_M_usingAliases[item->name()]);

    model()->beginAppendItem(item,  this);
    _M_usingAliases.insertMulti(item->name(),  item);
    model()->endAppendItem();
  }

  void _NamespaceDeclarationModelItem::removeUsingAlias(UsingAliasDirectiveModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_usingAliases.remove(item->name());
    model()->endRemoveItem();
  }

  UsingAliasDirectiveModelItem _NamespaceDeclarationModelItem::findUsingAlias(const QString &name) const
    {
      return  _M_usingAliases.value(name);
    }

  UsingNamespaceDirectiveList _NamespaceDeclarationModelItem::usingNamespaces() const
    {
      return  _M_usingNamespaces;
    }

  void _NamespaceDeclarationModelItem::addUsingNamespace(UsingNamespaceDirectiveModelItem item)
  {
    model()->beginAppendItem(item,  this);
    _M_usingNamespaces.append(item);
    model()->endAppendItem();
  }

  void _NamespaceDeclarationModelItem::removeUsingNamespace(UsingNamespaceDirectiveModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_usingNamespaces.removeAt(_M_usingNamespaces.indexOf(item));
    model()->endRemoveItem();
  }

  ClassDeclarationList _NamespaceDeclarationModelItem::classes() const
    {
      return  _M_classes.values();
    }

  void _NamespaceDeclarationModelItem::addClass(ClassDeclarationModelItem item)
  {
    if  ( _M_classes.contains(item->name()) )
      removeClass(_M_classes[item->name()]);

    model()->beginAppendItem(item,  this);
    _M_classes.insertMulti(item->name(),  item);
    model()->endAppendItem();
  }

  void _NamespaceDeclarationModelItem::removeClass(ClassDeclarationModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_classes.remove(item->name());
    model()->endRemoveItem();
  }

  ClassDeclarationModelItem _NamespaceDeclarationModelItem::findClass(const QString &name) const
    {
      return  _M_classes.value(name);
    }

  StructDeclarationList _NamespaceDeclarationModelItem::structs() const
    {
      return  _M_structs.values();
    }

  void _NamespaceDeclarationModelItem::addStruct(StructDeclarationModelItem item)
  {
    if  ( _M_structs.contains(item->name()) )
      removeStruct(_M_structs[item->name()]);

    model()->beginAppendItem(item,  this);
    _M_structs.insertMulti(item->name(),  item);
    model()->endAppendItem();
  }

  void _NamespaceDeclarationModelItem::removeStruct(StructDeclarationModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_structs.remove(item->name());
    model()->endRemoveItem();
  }

  StructDeclarationModelItem _NamespaceDeclarationModelItem::findStruct(const QString &name) const
    {
      return  _M_structs.value(name);
    }

  InterfaceDeclarationList _NamespaceDeclarationModelItem::interfaces() const
    {
      return  _M_interfaces.values();
    }

  void _NamespaceDeclarationModelItem::addInterface(InterfaceDeclarationModelItem item)
  {
    if  ( _M_interfaces.contains(item->name()) )
      removeInterface(_M_interfaces[item->name()]);

    model()->beginAppendItem(item,  this);
    _M_interfaces.insertMulti(item->name(),  item);
    model()->endAppendItem();
  }

  void _NamespaceDeclarationModelItem::removeInterface(InterfaceDeclarationModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_interfaces.remove(item->name());
    model()->endRemoveItem();
  }

  InterfaceDeclarationModelItem _NamespaceDeclarationModelItem::findInterface(const QString &name) const
    {
      return  _M_interfaces.value(name);
    }

  EnumDeclarationList _NamespaceDeclarationModelItem::enums() const
    {
      return  _M_enums.values();
    }

  void _NamespaceDeclarationModelItem::addEnum(EnumDeclarationModelItem item)
  {
    if  ( _M_enums.contains(item->name()) )
      removeEnum(_M_enums[item->name()]);

    model()->beginAppendItem(item,  this);
    _M_enums.insertMulti(item->name(),  item);
    model()->endAppendItem();
  }

  void _NamespaceDeclarationModelItem::removeEnum(EnumDeclarationModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_enums.remove(item->name());
    model()->endRemoveItem();
  }

  EnumDeclarationModelItem _NamespaceDeclarationModelItem::findEnum(const QString &name) const
    {
      return  _M_enums.value(name);
    }

  DelegateDeclarationList _NamespaceDeclarationModelItem::delegates() const
    {
      return  _M_delegates.values();
    }

  void _NamespaceDeclarationModelItem::addDelegate(DelegateDeclarationModelItem item)
  {
    if  ( _M_delegates.contains(item->name()) )
      removeDelegate(_M_delegates[item->name()]);

    model()->beginAppendItem(item,  this);
    _M_delegates.insertMulti(item->name(),  item);
    model()->endAppendItem();
  }

  void _NamespaceDeclarationModelItem::removeDelegate(DelegateDeclarationModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_delegates.remove(item->name());
    model()->endRemoveItem();
  }

  DelegateDeclarationModelItem _NamespaceDeclarationModelItem::findDelegate(const QString &name) const
    {
      return  _M_delegates.value(name);
    }

#undef CLASS
#undef BASECLASS

  // ---------------------------------------------------------------------------
#define CLASS _GlobalNamespaceDeclarationModelItem
#define BASECLASS _NamespaceDeclarationModelItem

  _GlobalNamespaceDeclarationModelItem::_GlobalNamespaceDeclarationModelItem(CodeModel *model,  int kind)
      :  _NamespaceDeclarationModelItem(model,  kind)
  {
    setName(QString::null);
  }

  _GlobalNamespaceDeclarationModelItem::~_GlobalNamespaceDeclarationModelItem()
  {}

  GlobalNamespaceDeclarationModelItem _GlobalNamespaceDeclarationModelItem::create(CodeModel *model)
  {
    GlobalNamespaceDeclarationModelItem item(new _GlobalNamespaceDeclarationModelItem(model));
    return  item;
  }

  AttributeSectionList _GlobalNamespaceDeclarationModelItem::globalAttributes() const
    {
      return  _M_globalAttributes;
    }

  void _GlobalNamespaceDeclarationModelItem::addGlobalAttribute(AttributeSectionModelItem item)
  {
    model()->beginAppendItem(item,  this);
    _M_globalAttributes.append(item);
    model()->endAppendItem();
  }

  void _GlobalNamespaceDeclarationModelItem::removeGlobalAttribute(AttributeSectionModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_globalAttributes.removeAt(_M_globalAttributes.indexOf(item));
    model()->endRemoveItem();
  }

#undef CLASS
#undef BASECLASS

  // ---------------------------------------------------------------------------
#define CLASS _ExternAliasDirectiveModelItem
#define BASECLASS _CodeModelItem

  _ExternAliasDirectiveModelItem::_ExternAliasDirectiveModelItem(CodeModel *model,  int kind)
      :  _CodeModelItem(model,  kind)
  {}

  _ExternAliasDirectiveModelItem::~_ExternAliasDirectiveModelItem()
  {}

  ExternAliasDirectiveModelItem _ExternAliasDirectiveModelItem::create(CodeModel *model)
  {
    ExternAliasDirectiveModelItem item(new _ExternAliasDirectiveModelItem(model));
    return  item;
  }

#undef CLASS
#undef BASECLASS

  // ---------------------------------------------------------------------------
#define CLASS _UsingAliasDirectiveModelItem
#define BASECLASS _CodeModelItem

  _UsingAliasDirectiveModelItem::_UsingAliasDirectiveModelItem(CodeModel *model,  int kind)
      :  _CodeModelItem(model,  kind)
  {}

  _UsingAliasDirectiveModelItem::~_UsingAliasDirectiveModelItem()
  {}

  UsingAliasDirectiveModelItem _UsingAliasDirectiveModelItem::create(CodeModel *model)
  {
    UsingAliasDirectiveModelItem item(new _UsingAliasDirectiveModelItem(model));
    return  item;
  }

  TypeInfo _UsingAliasDirectiveModelItem::namespaceOrType() const
    {
      return  _M_namespaceOrType;
    }

  void _UsingAliasDirectiveModelItem::setNamespaceOrType(TypeInfo namespaceOrType)
  {
    _M_namespaceOrType =  namespaceOrType;
  }

#undef CLASS
#undef BASECLASS

  // ---------------------------------------------------------------------------
#define CLASS _UsingNamespaceDirectiveModelItem
#define BASECLASS _CodeModelItem

  _UsingNamespaceDirectiveModelItem::_UsingNamespaceDirectiveModelItem(CodeModel *model,  int kind)
      :  _CodeModelItem(model,  kind)
  {
    setName(QString::null);
  }

  _UsingNamespaceDirectiveModelItem::~_UsingNamespaceDirectiveModelItem()
  {}

  UsingNamespaceDirectiveModelItem _UsingNamespaceDirectiveModelItem::create(CodeModel *model)
  {
    UsingNamespaceDirectiveModelItem item(new _UsingNamespaceDirectiveModelItem(model));
    return  item;
  }

  QStringList _UsingNamespaceDirectiveModelItem::namespaceUsed() const
    {
      return  _M_namespaceUsed;
    }

  void _UsingNamespaceDirectiveModelItem::setNamespaceUsed(QStringList namespaceUsed)
  {
    _M_namespaceUsed =  namespaceUsed;
  }

#undef CLASS
#undef BASECLASS

  // ---------------------------------------------------------------------------
#define CLASS _TypeDeclarationModelItem
#define BASECLASS _ScopeModelItem

  _TypeDeclarationModelItem::_TypeDeclarationModelItem(CodeModel *model,  int kind)
      :  _ScopeModelItem(model,  kind)
      ,  _M_accessPolicy( access_policy::access_private )
      ,  _M_isNew( false )
  {}

  _TypeDeclarationModelItem::~_TypeDeclarationModelItem()
  {}

  TypeDeclarationModelItem _TypeDeclarationModelItem::create(CodeModel *model)
  {
    TypeDeclarationModelItem item(new _TypeDeclarationModelItem(model));
    return  item;
  }

  access_policy::access_policy_enum _TypeDeclarationModelItem::accessPolicy() const
    {
      return  _M_accessPolicy;
    }

  void _TypeDeclarationModelItem::setAccessPolicy(access_policy::access_policy_enum accessPolicy)
  {
    _M_accessPolicy =  accessPolicy;
  }

  bool _TypeDeclarationModelItem::isNew() const
    {
      return  _M_isNew;
    }

  void _TypeDeclarationModelItem::setNew(bool isNew)
  {
    _M_isNew =  isNew;
  }

#undef CLASS
#undef BASECLASS

  // ---------------------------------------------------------------------------
#define CLASS _ClassLikeDeclarationModelItem
#define BASECLASS _TypeDeclarationModelItem

  _ClassLikeDeclarationModelItem::_ClassLikeDeclarationModelItem(CodeModel *model,  int kind)
      :  _TypeDeclarationModelItem(model,  kind)
      ,  _M_isUnsafe( false )
  {}

  _ClassLikeDeclarationModelItem::~_ClassLikeDeclarationModelItem()
  {}

  ClassLikeDeclarationModelItem _ClassLikeDeclarationModelItem::create(CodeModel *model)
  {
    ClassLikeDeclarationModelItem item(new _ClassLikeDeclarationModelItem(model));
    return  item;
  }

  TypeInfo _ClassLikeDeclarationModelItem::baseType() const
    {
      return  _M_baseType;
    }

  void _ClassLikeDeclarationModelItem::setBaseType(TypeInfo baseType)
  {
    _M_baseType =  baseType;
  }

  EventDeclarationList _ClassLikeDeclarationModelItem::events() const
    {
      return  _M_events.values();
    }

  void _ClassLikeDeclarationModelItem::addEvent(EventDeclarationModelItem item)
  {
    if  ( _M_events.contains(item->name()) )
      removeEvent(_M_events[item->name()]);

    model()->beginAppendItem(item,  this);
    _M_events.insertMulti(item->name(),  item);
    model()->endAppendItem();
  }

  void _ClassLikeDeclarationModelItem::removeEvent(EventDeclarationModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_events.remove(item->name());
    model()->endRemoveItem();
  }

  EventDeclarationModelItem _ClassLikeDeclarationModelItem::findEvent(const QString &name) const
    {
      return  _M_events.value(name);
    }

  IndexerDeclarationList _ClassLikeDeclarationModelItem::indexers() const
    {
      return  _M_indexers;
    }

  void _ClassLikeDeclarationModelItem::addIndexer(IndexerDeclarationModelItem item)
  {
    model()->beginAppendItem(item,  this);
    _M_indexers.append(item);
    model()->endAppendItem();
  }

  void _ClassLikeDeclarationModelItem::removeIndexer(IndexerDeclarationModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_indexers.removeAt(_M_indexers.indexOf(item));
    model()->endRemoveItem();
  }

  PropertyDeclarationList _ClassLikeDeclarationModelItem::propertys() const
    {
      return  _M_propertys.values();
    }

  void _ClassLikeDeclarationModelItem::addProperty(PropertyDeclarationModelItem item)
  {
    if  ( _M_propertys.contains(item->name()) )
      removeProperty(_M_propertys[item->name()]);

    model()->beginAppendItem(item,  this);
    _M_propertys.insertMulti(item->name(),  item);
    model()->endAppendItem();
  }

  void _ClassLikeDeclarationModelItem::removeProperty(PropertyDeclarationModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_propertys.remove(item->name());
    model()->endRemoveItem();
  }

  PropertyDeclarationModelItem _ClassLikeDeclarationModelItem::findProperty(const QString &name) const
    {
      return  _M_propertys.value(name);
    }

  MethodDeclarationList _ClassLikeDeclarationModelItem::methods() const
    {
      return  _M_methods.values();
    }

  void _ClassLikeDeclarationModelItem::addMethod(MethodDeclarationModelItem item)
  {
    foreach( _MethodDeclarationModelItem *existingItem,
             _M_methods.values(item->name()) )
    removeMethod(existingItem);
    model()->beginAppendItem(item,  this);
    _M_methods.insert(item->name(),  item);
    model()->endAppendItem();
  }

  void _ClassLikeDeclarationModelItem::removeMethod(MethodDeclarationModelItem item)
  {
    QMultiHash<QString,  MethodDeclarationModelItem>::Iterator it =  _M_methods.find(item->name());

    while  (it !=  _M_methods.end() &&  it.key() ==  item->name()
            &&  it.value() !=  item)
      {
        ++it;
      }

    if  (it !=  _M_methods.end() &&  it.value() ==  item)
      {
        model()->beginRemoveItem(item);
        _M_methods.erase(it);
        model()->endRemoveItem();
      }
  }

  MethodDeclarationList _ClassLikeDeclarationModelItem::findMethods(const QString &name) const
    {
      return  _M_methods.values(name);
    }

  TypeParameterList _ClassLikeDeclarationModelItem::typeParameters() const
    {
      return  _M_typeParameters;
    }

  void _ClassLikeDeclarationModelItem::addTypeParameter(TypeParameterModelItem item)
  {
    model()->beginAppendItem(item,  this);
    _M_typeParameters.append(item);
    model()->endAppendItem();
  }

  void _ClassLikeDeclarationModelItem::removeTypeParameter(TypeParameterModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_typeParameters.removeAt(_M_typeParameters.indexOf(item));
    model()->endRemoveItem();
  }

  TypeParameterConstraintList _ClassLikeDeclarationModelItem::typeParameterConstraints() const
    {
      return  _M_typeParameterConstraints;
    }

  void _ClassLikeDeclarationModelItem::addTypeParameterConstraint(TypeParameterConstraintModelItem item)
  {
    model()->beginAppendItem(item,  this);
    _M_typeParameterConstraints.append(item);
    model()->endAppendItem();
  }

  void _ClassLikeDeclarationModelItem::removeTypeParameterConstraint(TypeParameterConstraintModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_typeParameterConstraints.removeAt(_M_typeParameterConstraints.indexOf(item));
    model()->endRemoveItem();
  }

  bool _ClassLikeDeclarationModelItem::isUnsafe() const
    {
      return  _M_isUnsafe;
    }

  void _ClassLikeDeclarationModelItem::setUnsafe(bool isUnsafe)
  {
    _M_isUnsafe =  isUnsafe;
  }

  ClassDeclarationList _ClassLikeDeclarationModelItem::classes() const
    {
      return  _M_classes.values();
    }

  void _ClassLikeDeclarationModelItem::addClass(ClassDeclarationModelItem item)
  {
    if  ( _M_classes.contains(item->name()) )
      removeClass(_M_classes[item->name()]);

    model()->beginAppendItem(item,  this);
    _M_classes.insertMulti(item->name(),  item);
    model()->endAppendItem();
  }

  void _ClassLikeDeclarationModelItem::removeClass(ClassDeclarationModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_classes.remove(item->name());
    model()->endRemoveItem();
  }

  ClassDeclarationModelItem _ClassLikeDeclarationModelItem::findClass(const QString &name) const
    {
      return  _M_classes.value(name);
    }

  StructDeclarationList _ClassLikeDeclarationModelItem::structs() const
    {
      return  _M_structs.values();
    }

  void _ClassLikeDeclarationModelItem::addStruct(StructDeclarationModelItem item)
  {
    if  ( _M_structs.contains(item->name()) )
      removeStruct(_M_structs[item->name()]);

    model()->beginAppendItem(item,  this);
    _M_structs.insertMulti(item->name(),  item);
    model()->endAppendItem();
  }

  void _ClassLikeDeclarationModelItem::removeStruct(StructDeclarationModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_structs.remove(item->name());
    model()->endRemoveItem();
  }

  StructDeclarationModelItem _ClassLikeDeclarationModelItem::findStruct(const QString &name) const
    {
      return  _M_structs.value(name);
    }

  InterfaceDeclarationList _ClassLikeDeclarationModelItem::interfaces() const
    {
      return  _M_interfaces.values();
    }

  void _ClassLikeDeclarationModelItem::addInterface(InterfaceDeclarationModelItem item)
  {
    if  ( _M_interfaces.contains(item->name()) )
      removeInterface(_M_interfaces[item->name()]);

    model()->beginAppendItem(item,  this);
    _M_interfaces.insertMulti(item->name(),  item);
    model()->endAppendItem();
  }

  void _ClassLikeDeclarationModelItem::removeInterface(InterfaceDeclarationModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_interfaces.remove(item->name());
    model()->endRemoveItem();
  }

  InterfaceDeclarationModelItem _ClassLikeDeclarationModelItem::findInterface(const QString &name) const
    {
      return  _M_interfaces.value(name);
    }

  EnumDeclarationList _ClassLikeDeclarationModelItem::enums() const
    {
      return  _M_enums.values();
    }

  void _ClassLikeDeclarationModelItem::addEnum(EnumDeclarationModelItem item)
  {
    if  ( _M_enums.contains(item->name()) )
      removeEnum(_M_enums[item->name()]);

    model()->beginAppendItem(item,  this);
    _M_enums.insertMulti(item->name(),  item);
    model()->endAppendItem();
  }

  void _ClassLikeDeclarationModelItem::removeEnum(EnumDeclarationModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_enums.remove(item->name());
    model()->endRemoveItem();
  }

  EnumDeclarationModelItem _ClassLikeDeclarationModelItem::findEnum(const QString &name) const
    {
      return  _M_enums.value(name);
    }

  DelegateDeclarationList _ClassLikeDeclarationModelItem::delegates() const
    {
      return  _M_delegates.values();
    }

  void _ClassLikeDeclarationModelItem::addDelegate(DelegateDeclarationModelItem item)
  {
    if  ( _M_delegates.contains(item->name()) )
      removeDelegate(_M_delegates[item->name()]);

    model()->beginAppendItem(item,  this);
    _M_delegates.insertMulti(item->name(),  item);
    model()->endAppendItem();
  }

  void _ClassLikeDeclarationModelItem::removeDelegate(DelegateDeclarationModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_delegates.remove(item->name());
    model()->endRemoveItem();
  }

  DelegateDeclarationModelItem _ClassLikeDeclarationModelItem::findDelegate(const QString &name) const
    {
      return  _M_delegates.value(name);
    }

  VariableDeclarationList _ClassLikeDeclarationModelItem::variables() const
    {
      return  _M_variables.values();
    }

  void _ClassLikeDeclarationModelItem::addVariable(VariableDeclarationModelItem item)
  {
    if  ( _M_variables.contains(item->name()) )
      removeVariable(_M_variables[item->name()]);

    model()->beginAppendItem(item,  this);
    _M_variables.insertMulti(item->name(),  item);
    model()->endAppendItem();
  }

  void _ClassLikeDeclarationModelItem::removeVariable(VariableDeclarationModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_variables.remove(item->name());
    model()->endRemoveItem();
  }

  VariableDeclarationModelItem _ClassLikeDeclarationModelItem::findVariable(const QString &name) const
    {
      return  _M_variables.value(name);
    }


  ITEM(MethodDeclaration) CLASS::declaredMethod(ITEM(MethodDeclaration) item)
  {
    LIST(MethodDeclaration) methodList =  findMethods(item->name());

    foreach (ITEM(MethodDeclaration) method,  methodList)
    {
      if  (method->isSimilar(model_static_cast < ITEM(Code) > (item), false))
        return  method;
    }

    return  ITEM(MethodDeclaration)();
  }

#undef CLASS
#undef BASECLASS

  // ---------------------------------------------------------------------------
#define CLASS _ClassDeclarationModelItem
#define BASECLASS _ClassLikeDeclarationModelItem

  _ClassDeclarationModelItem::_ClassDeclarationModelItem(CodeModel *model,  int kind)
      :  _ClassLikeDeclarationModelItem(model,  kind)
      ,  _M_isSealed( false )
      ,  _M_isAbstract( false )
      ,  _M_isStatic( false )
{}

  _ClassDeclarationModelItem::~_ClassDeclarationModelItem()
  {}

  ClassDeclarationModelItem _ClassDeclarationModelItem::create(CodeModel *model)
  {
    ClassDeclarationModelItem item(new _ClassDeclarationModelItem(model));
    return  item;
  }

  bool _ClassDeclarationModelItem::isSealed() const
    {
      return  _M_isSealed;
    }

  void _ClassDeclarationModelItem::setSealed(bool isSealed)
  {
    _M_isSealed =  isSealed;
  }

  bool _ClassDeclarationModelItem::isAbstract() const
    {
      return  _M_isAbstract;
    }

  void _ClassDeclarationModelItem::setAbstract(bool isAbstract)
  {
    _M_isAbstract =  isAbstract;
  }

  bool _ClassDeclarationModelItem::isStatic() const
    {
      return  _M_isStatic;
    }

  void _ClassDeclarationModelItem::setStatic(bool isStatic)
  {
    _M_isStatic =  isStatic;
  }

#undef CLASS
#undef BASECLASS

  // ---------------------------------------------------------------------------
#define CLASS _StructDeclarationModelItem
#define BASECLASS _ClassLikeDeclarationModelItem

  _StructDeclarationModelItem::_StructDeclarationModelItem(CodeModel *model,  int kind)
      :  _ClassLikeDeclarationModelItem(model,  kind)
  {}

  _StructDeclarationModelItem::~_StructDeclarationModelItem()
  {}

  StructDeclarationModelItem _StructDeclarationModelItem::create(CodeModel *model)
  {
    StructDeclarationModelItem item(new _StructDeclarationModelItem(model));
    return  item;
  }

#undef CLASS
#undef BASECLASS

  // ---------------------------------------------------------------------------
#define CLASS _InterfaceDeclarationModelItem
#define BASECLASS _ClassLikeDeclarationModelItem

  _InterfaceDeclarationModelItem::_InterfaceDeclarationModelItem(CodeModel *model,  int kind)
      :  _ClassLikeDeclarationModelItem(model,  kind)
  {}

  _InterfaceDeclarationModelItem::~_InterfaceDeclarationModelItem()
  {}

  InterfaceDeclarationModelItem _InterfaceDeclarationModelItem::create(CodeModel *model)
  {
    InterfaceDeclarationModelItem item(new _InterfaceDeclarationModelItem(model));
    return  item;
  }

#undef CLASS
#undef BASECLASS

  // ---------------------------------------------------------------------------
#define CLASS _DelegateDeclarationModelItem
#define BASECLASS _TypeDeclarationModelItem

  _DelegateDeclarationModelItem::_DelegateDeclarationModelItem(CodeModel *model,  int kind)
      :  _TypeDeclarationModelItem(model,  kind)
      ,  _M_isUnsafe( false )
  {}

  _DelegateDeclarationModelItem::~_DelegateDeclarationModelItem()
  {}

  DelegateDeclarationModelItem _DelegateDeclarationModelItem::create(CodeModel *model)
  {
    DelegateDeclarationModelItem item(new _DelegateDeclarationModelItem(model));
    return  item;
  }

  AttributeSectionList _DelegateDeclarationModelItem::attributes() const
    {
      return  _M_attributes;
    }

  void _DelegateDeclarationModelItem::addAttribute(AttributeSectionModelItem item)
  {
    model()->beginAppendItem(item,  this);
    _M_attributes.append(item);
    model()->endAppendItem();
  }

  void _DelegateDeclarationModelItem::removeAttribute(AttributeSectionModelItem item)
  {
    model()->beginRemoveItem(item);
    _M_attributes.removeAt(_M_attributes.indexOf(item));
    model()->endRemoveItem();
  }

  TypeInfo _DelegateDeclarationModelItem::returnType() const
        {
          return  _M_returnType;
        }

      void _DelegateDeclarationModelItem::setReturnType(TypeInfo returnType)
      {
        _M_returnType =  returnType;
      }

      ParameterList _DelegateDeclarationModelItem::parameters() const
        {
          return  _M_parameters;
        }

      void _DelegateDeclarationModelItem::addParameter(ParameterModelItem item)
      {
        model()->beginAppendItem(item,  this);
        _M_parameters.append(item);
        model()->endAppendItem();
      }

      void _DelegateDeclarationModelItem::removeParameter(ParameterModelItem item)
      {
        model()->beginRemoveItem(item);
        _M_parameters.removeAt(_M_parameters.indexOf(item));
        model()->endRemoveItem();
      }

      TypeParameterList _DelegateDeclarationModelItem::typeParameters() const
        {
          return  _M_typeParameters;
        }

      void _DelegateDeclarationModelItem::addTypeParameter(TypeParameterModelItem item)
      {
        model()->beginAppendItem(item,  this);
        _M_typeParameters.append(item);
        model()->endAppendItem();
      }

      void _DelegateDeclarationModelItem::removeTypeParameter(TypeParameterModelItem item)
      {
        model()->beginRemoveItem(item);
        _M_typeParameters.removeAt(_M_typeParameters.indexOf(item));
        model()->endRemoveItem();
      }

      TypeParameterConstraintList _DelegateDeclarationModelItem::typeParameterConstraints() const
        {
          return  _M_typeParameterConstraints;
        }

      void _DelegateDeclarationModelItem::addTypeParameterConstraint(TypeParameterConstraintModelItem item)
      {
        model()->beginAppendItem(item,  this);
        _M_typeParameterConstraints.append(item);
        model()->endAppendItem();
      }

      void _DelegateDeclarationModelItem::removeTypeParameterConstraint(TypeParameterConstraintModelItem item)
      {
        model()->beginRemoveItem(item);
        _M_typeParameterConstraints.removeAt(_M_typeParameterConstraints.indexOf(item));
        model()->endRemoveItem();
      }

      bool _DelegateDeclarationModelItem::isUnsafe() const
        {
          return  _M_isUnsafe;
        }

      void _DelegateDeclarationModelItem::setUnsafe(bool isUnsafe)
      {
        _M_isUnsafe =  isUnsafe;
      }

#undef CLASS
#undef BASECLASS

      // ---------------------------------------------------------------------------
#define CLASS _EnumDeclarationModelItem
#define BASECLASS _TypeDeclarationModelItem

      _EnumDeclarationModelItem::_EnumDeclarationModelItem(CodeModel *model,  int kind)
          :  _TypeDeclarationModelItem(model,  kind)
      {}

      _EnumDeclarationModelItem::~_EnumDeclarationModelItem()
      {}

      EnumDeclarationModelItem _EnumDeclarationModelItem::create(CodeModel *model)
      {
        EnumDeclarationModelItem item(new _EnumDeclarationModelItem(model));
        return  item;
      }

      EnumValueList _EnumDeclarationModelItem::enumValues() const
        {
          return  _M_enumValues;
        }

      void _EnumDeclarationModelItem::addEnumValue(EnumValueModelItem item)
      {
        model()->beginAppendItem(item,  this);
        _M_enumValues.append(item);
        model()->endAppendItem();
      }

      void _EnumDeclarationModelItem::removeEnumValue(EnumValueModelItem item)
      {
        model()->beginRemoveItem(item);
        _M_enumValues.removeAt(_M_enumValues.indexOf(item));
        model()->endRemoveItem();
      }

      TypeInfo _EnumDeclarationModelItem::baseIntegralType() const
        {
          return  _M_baseIntegralType;
        }

      void _EnumDeclarationModelItem::setBaseIntegralType(TypeInfo baseIntegralType)
      {
        _M_baseIntegralType =  baseIntegralType;
      }

#undef CLASS
#undef BASECLASS

      // ---------------------------------------------------------------------------
#define CLASS _EnumValueModelItem
#define BASECLASS _CodeModelItem

      _EnumValueModelItem::_EnumValueModelItem(CodeModel *model,  int kind)
          :  _CodeModelItem(model,  kind)
      {}

      _EnumValueModelItem::~_EnumValueModelItem()
      {}

      EnumValueModelItem _EnumValueModelItem::create(CodeModel *model)
      {
        EnumValueModelItem item(new _EnumValueModelItem(model));
        return  item;
      }

      AttributeSectionList _EnumValueModelItem::attributes() const
        {
          return  _M_attributes;
        }

      void _EnumValueModelItem::addAttribute(AttributeSectionModelItem item)
      {
        model()->beginAppendItem(item,  this);
        _M_attributes.append(item);
        model()->endAppendItem();
      }

      void _EnumValueModelItem::removeAttribute(AttributeSectionModelItem item)
      {
        model()->beginRemoveItem(item);
        _M_attributes.removeAt(_M_attributes.indexOf(item));
        model()->endRemoveItem();
      }

      QString _EnumValueModelItem::value() const
        {
          return  _M_value;
        }

      void _EnumValueModelItem::setValue(QString value)
      {
        _M_value =  value;
      }

#undef CLASS
#undef BASECLASS

      // ---------------------------------------------------------------------------
#define CLASS _EventDeclarationModelItem
#define BASECLASS _ScopeModelItem

      _EventDeclarationModelItem::_EventDeclarationModelItem(CodeModel *model,  int kind)
          :  _ScopeModelItem(model,  kind)
          ,  _M_accessPolicy( access_policy::access_private )
          ,  _M_isNew( false )
          ,  _M_isStatic( false )
          ,  _M_isVirtual( false )
          ,  _M_isSealed( false )
          ,  _M_isOverride( false )
          ,  _M_isAbstract( false )
          ,  _M_isExtern( false )
          ,  _M_isUnsafe( false )
      {}

      _EventDeclarationModelItem::~_EventDeclarationModelItem()
      {}

      EventDeclarationModelItem _EventDeclarationModelItem::create(CodeModel *model)
      {
        EventDeclarationModelItem item(new _EventDeclarationModelItem(model));
        return  item;
      }

      AttributeSectionList _EventDeclarationModelItem::attributes() const
        {
          return  _M_attributes;
        }

      void _EventDeclarationModelItem::addAttribute(AttributeSectionModelItem item)
      {
        model()->beginAppendItem(item,  this);
        _M_attributes.append(item);
        model()->endAppendItem();
      }

      void _EventDeclarationModelItem::removeAttribute(AttributeSectionModelItem item)
      {
        model()->beginRemoveItem(item);
        _M_attributes.removeAt(_M_attributes.indexOf(item));
        model()->endRemoveItem();
      }

      TypeInfo _EventDeclarationModelItem::type() const
        {
          return  _M_type;
        }

      void _EventDeclarationModelItem::setType(TypeInfo type)
      {
        _M_type =  type;
      }

      TypeInfo _EventDeclarationModelItem::fromInterface() const
        {
          return  _M_fromInterface;
        }

      void _EventDeclarationModelItem::setFromInterface(TypeInfo fromInterface)
      {
        _M_fromInterface =  fromInterface;
      }

      EventAccessorDeclarationModelItem _EventDeclarationModelItem::addAccessor() const
        {
          return  _M_addAccessor;
        }

      void _EventDeclarationModelItem::setAddAccessor(EventAccessorDeclarationModelItem addAccessor)
      {
        _M_addAccessor =  addAccessor;
      }

      EventAccessorDeclarationModelItem _EventDeclarationModelItem::removeAccessor() const
        {
          return  _M_removeAccessor;
        }

      void _EventDeclarationModelItem::setRemoveAccessor(EventAccessorDeclarationModelItem removeAccessor)
      {
        _M_removeAccessor =  removeAccessor;
      }

      access_policy::access_policy_enum _EventDeclarationModelItem::accessPolicy() const
        {
          return  _M_accessPolicy;
        }

      void _EventDeclarationModelItem::setAccessPolicy(access_policy::access_policy_enum accessPolicy)
      {
        _M_accessPolicy =  accessPolicy;
      }

      bool _EventDeclarationModelItem::isNew() const
        {
          return  _M_isNew;
        }

      void _EventDeclarationModelItem::setNew(bool isNew)
      {
        _M_isNew =  isNew;
      }

      bool _EventDeclarationModelItem::isStatic() const
        {
          return  _M_isStatic;
        }

      void _EventDeclarationModelItem::setStatic(bool isStatic)
      {
        _M_isStatic =  isStatic;
      }

      bool _EventDeclarationModelItem::isVirtual() const
        {
          return  _M_isVirtual;
        }

      void _EventDeclarationModelItem::setVirtual(bool isVirtual)
      {
        _M_isVirtual =  isVirtual;
      }

      bool _EventDeclarationModelItem::isSealed() const
        {
          return  _M_isSealed;
        }

      void _EventDeclarationModelItem::setSealed(bool isSealed)
      {
        _M_isSealed =  isSealed;
      }

      bool _EventDeclarationModelItem::isOverride() const
        {
          return  _M_isOverride;
        }

      void _EventDeclarationModelItem::setOverride(bool isOverride)
      {
        _M_isOverride =  isOverride;
      }

      bool _EventDeclarationModelItem::isAbstract() const
        {
          return  _M_isAbstract;
        }

      void _EventDeclarationModelItem::setAbstract(bool isAbstract)
      {
        _M_isAbstract =  isAbstract;
      }

      bool _EventDeclarationModelItem::isExtern() const
        {
          return  _M_isExtern;
        }

      void _EventDeclarationModelItem::setExtern(bool isExtern)
      {
        _M_isExtern =  isExtern;
      }

      bool _EventDeclarationModelItem::isUnsafe() const
        {
          return  _M_isUnsafe;
        }

      void _EventDeclarationModelItem::setUnsafe(bool isUnsafe)
      {
        _M_isUnsafe =  isUnsafe;
      }

#undef CLASS
#undef BASECLASS

      // ---------------------------------------------------------------------------
#define CLASS _EventAccessorDeclarationModelItem
#define BASECLASS _CodeModelItem

      _EventAccessorDeclarationModelItem::_EventAccessorDeclarationModelItem(CodeModel *model,  int kind)
          :  _CodeModelItem(model,  kind)
      {}

      _EventAccessorDeclarationModelItem::~_EventAccessorDeclarationModelItem()
      {}

      EventAccessorDeclarationModelItem _EventAccessorDeclarationModelItem::create(CodeModel *model)
      {
        EventAccessorDeclarationModelItem item(new _EventAccessorDeclarationModelItem(model));
        return  item;
      }

      AttributeSectionList _EventAccessorDeclarationModelItem::attributes() const
        {
          return  _M_attributes;
        }

      void _EventAccessorDeclarationModelItem::addAttribute(AttributeSectionModelItem item)
      {
        model()->beginAppendItem(item,  this);
        _M_attributes.append(item);
        model()->endAppendItem();
      }

      void _EventAccessorDeclarationModelItem::removeAttribute(AttributeSectionModelItem item)
      {
        model()->beginRemoveItem(item);
        _M_attributes.removeAt(_M_attributes.indexOf(item));
        model()->endRemoveItem();
      }

#undef CLASS
#undef BASECLASS

      // ---------------------------------------------------------------------------
#define CLASS _IndexerDeclarationModelItem
#define BASECLASS _ScopeModelItem

      _IndexerDeclarationModelItem::_IndexerDeclarationModelItem(CodeModel *model,  int kind)
          :  _ScopeModelItem(model,  kind)
          ,  _M_accessPolicy( access_policy::access_private )
          ,  _M_isNew( false )
          ,  _M_isVirtual( false )
          ,  _M_isSealed( false )
          ,  _M_isOverride( false )
          ,  _M_isAbstract( false )
          ,  _M_isExtern( false )
          ,  _M_isUnsafe( false )
      {
        setName(QString::null);
      }

      _IndexerDeclarationModelItem::~_IndexerDeclarationModelItem()
      {}

      IndexerDeclarationModelItem _IndexerDeclarationModelItem::create(CodeModel *model)
      {
        IndexerDeclarationModelItem item(new _IndexerDeclarationModelItem(model));
        return  item;
      }

      AttributeSectionList _IndexerDeclarationModelItem::attributes() const
        {
          return  _M_attributes;
        }

      void _IndexerDeclarationModelItem::addAttribute(AttributeSectionModelItem item)
      {
        model()->beginAppendItem(item,  this);
        _M_attributes.append(item);
        model()->endAppendItem();
      }

      void _IndexerDeclarationModelItem::removeAttribute(AttributeSectionModelItem item)
      {
        model()->beginRemoveItem(item);
        _M_attributes.removeAt(_M_attributes.indexOf(item));
        model()->endRemoveItem();
      }

      TypeInfo _IndexerDeclarationModelItem::type() const
        {
          return  _M_type;
        }

      void _IndexerDeclarationModelItem::setType(TypeInfo type)
      {
        _M_type =  type;
      }

      TypeInfo _IndexerDeclarationModelItem::fromInterface() const
        {
          return  _M_fromInterface;
        }

      void _IndexerDeclarationModelItem::setFromInterface(TypeInfo fromInterface)
      {
        _M_fromInterface =  fromInterface;
      }

      ParameterList _IndexerDeclarationModelItem::parameters() const
        {
          return  _M_parameters;
        }

      void _IndexerDeclarationModelItem::addParameter(ParameterModelItem item)
      {
        model()->beginAppendItem(item,  this);
        _M_parameters.append(item);
        model()->endAppendItem();
      }

      void _IndexerDeclarationModelItem::removeParameter(ParameterModelItem item)
      {
        model()->beginRemoveItem(item);
        _M_parameters.removeAt(_M_parameters.indexOf(item));
        model()->endRemoveItem();
      }

      AccessorDeclarationList _IndexerDeclarationModelItem::accessors() const
        {
          return  _M_accessors;
        }

      void _IndexerDeclarationModelItem::addAccessor(AccessorDeclarationModelItem item)
      {
        model()->beginAppendItem(item,  this);
        _M_accessors.append(item);
        model()->endAppendItem();
      }

      void _IndexerDeclarationModelItem::removeAccessor(AccessorDeclarationModelItem item)
      {
        model()->beginRemoveItem(item);
        _M_accessors.removeAt(_M_accessors.indexOf(item));
        model()->endRemoveItem();
      }

      access_policy::access_policy_enum _IndexerDeclarationModelItem::accessPolicy() const
        {
          return  _M_accessPolicy;
        }

      void _IndexerDeclarationModelItem::setAccessPolicy(access_policy::access_policy_enum accessPolicy)
      {
        _M_accessPolicy =  accessPolicy;
      }

      bool _IndexerDeclarationModelItem::isNew() const
        {
          return  _M_isNew;
        }

      void _IndexerDeclarationModelItem::setNew(bool isNew)
      {
        _M_isNew =  isNew;
      }

      bool _IndexerDeclarationModelItem::isVirtual() const
        {
          return  _M_isVirtual;
        }

      void _IndexerDeclarationModelItem::setVirtual(bool isVirtual)
      {
        _M_isVirtual =  isVirtual;
      }

      bool _IndexerDeclarationModelItem::isSealed() const
        {
          return  _M_isSealed;
        }

      void _IndexerDeclarationModelItem::setSealed(bool isSealed)
      {
        _M_isSealed =  isSealed;
      }

      bool _IndexerDeclarationModelItem::isOverride() const
        {
          return  _M_isOverride;
        }

      void _IndexerDeclarationModelItem::setOverride(bool isOverride)
      {
        _M_isOverride =  isOverride;
      }

      bool _IndexerDeclarationModelItem::isAbstract() const
        {
          return  _M_isAbstract;
        }

      void _IndexerDeclarationModelItem::setAbstract(bool isAbstract)
      {
        _M_isAbstract =  isAbstract;
      }

      bool _IndexerDeclarationModelItem::isExtern() const
        {
          return  _M_isExtern;
        }

      void _IndexerDeclarationModelItem::setExtern(bool isExtern)
      {
        _M_isExtern =  isExtern;
      }

      bool _IndexerDeclarationModelItem::isUnsafe() const
        {
          return  _M_isUnsafe;
        }

      void _IndexerDeclarationModelItem::setUnsafe(bool isUnsafe)
      {
        _M_isUnsafe =  isUnsafe;
      }

#undef CLASS
#undef BASECLASS

      // ---------------------------------------------------------------------------
#define CLASS _PropertyDeclarationModelItem
#define BASECLASS _ScopeModelItem

      _PropertyDeclarationModelItem::_PropertyDeclarationModelItem(CodeModel *model,  int kind)
          :  _ScopeModelItem(model,  kind)
          ,  _M_accessPolicy( access_policy::access_private )
          ,  _M_isNew( false )
          ,  _M_isStatic( false )
          ,  _M_isVirtual( false )
          ,  _M_isSealed( false )
          ,  _M_isOverride( false )
          ,  _M_isAbstract( false )
          ,  _M_isExtern( false )
          ,  _M_isUnsafe( false )
      {}

      _PropertyDeclarationModelItem::~_PropertyDeclarationModelItem()
      {}

      PropertyDeclarationModelItem _PropertyDeclarationModelItem::create(CodeModel *model)
      {
        PropertyDeclarationModelItem item(new _PropertyDeclarationModelItem(model));
        return  item;
      }

      AttributeSectionList _PropertyDeclarationModelItem::attributes() const
        {
          return  _M_attributes;
        }

      void _PropertyDeclarationModelItem::addAttribute(AttributeSectionModelItem item)
      {
        model()->beginAppendItem(item,  this);
        _M_attributes.append(item);
        model()->endAppendItem();
      }

      void _PropertyDeclarationModelItem::removeAttribute(AttributeSectionModelItem item)
      {
        model()->beginRemoveItem(item);
        _M_attributes.removeAt(_M_attributes.indexOf(item));
        model()->endRemoveItem();
      }

      TypeInfo _PropertyDeclarationModelItem::type() const
        {
          return  _M_type;
        }

      void _PropertyDeclarationModelItem::setType(TypeInfo type)
      {
        _M_type =  type;
      }

      TypeInfo _PropertyDeclarationModelItem::fromInterface() const
        {
          return  _M_fromInterface;
        }

      void _PropertyDeclarationModelItem::setFromInterface(TypeInfo fromInterface)
      {
        _M_fromInterface =  fromInterface;
      }

      AccessorDeclarationList _PropertyDeclarationModelItem::accessors() const
        {
          return  _M_accessors;
        }

      void _PropertyDeclarationModelItem::addAccessor(AccessorDeclarationModelItem item)
      {
        model()->beginAppendItem(item,  this);
        _M_accessors.append(item);
        model()->endAppendItem();
      }

      void _PropertyDeclarationModelItem::removeAccessor(AccessorDeclarationModelItem item)
      {
        model()->beginRemoveItem(item);
        _M_accessors.removeAt(_M_accessors.indexOf(item));
        model()->endRemoveItem();
      }

      access_policy::access_policy_enum _PropertyDeclarationModelItem::accessPolicy() const
        {
          return  _M_accessPolicy;
        }

      void _PropertyDeclarationModelItem::setAccessPolicy(access_policy::access_policy_enum accessPolicy)
      {
        _M_accessPolicy =  accessPolicy;
      }

      bool _PropertyDeclarationModelItem::isNew() const
        {
          return  _M_isNew;
        }

      void _PropertyDeclarationModelItem::setNew(bool isNew)
      {
        _M_isNew =  isNew;
      }

      bool _PropertyDeclarationModelItem::isStatic() const
        {
          return  _M_isStatic;
        }

      void _PropertyDeclarationModelItem::setStatic(bool isStatic)
      {
        _M_isStatic =  isStatic;
      }

      bool _PropertyDeclarationModelItem::isVirtual() const
        {
          return  _M_isVirtual;
        }

      void _PropertyDeclarationModelItem::setVirtual(bool isVirtual)
      {
        _M_isVirtual =  isVirtual;
      }

      bool _PropertyDeclarationModelItem::isSealed() const
        {
          return  _M_isSealed;
        }

      void _PropertyDeclarationModelItem::setSealed(bool isSealed)
      {
        _M_isSealed =  isSealed;
      }

      bool _PropertyDeclarationModelItem::isOverride() const
        {
          return  _M_isOverride;
        }

      void _PropertyDeclarationModelItem::setOverride(bool isOverride)
      {
        _M_isOverride =  isOverride;
      }

      bool _PropertyDeclarationModelItem::isAbstract() const
        {
          return  _M_isAbstract;
        }

      void _PropertyDeclarationModelItem::setAbstract(bool isAbstract)
      {
        _M_isAbstract =  isAbstract;
      }

      bool _PropertyDeclarationModelItem::isExtern() const
        {
          return  _M_isExtern;
        }

      void _PropertyDeclarationModelItem::setExtern(bool isExtern)
      {
        _M_isExtern =  isExtern;
      }

      bool _PropertyDeclarationModelItem::isUnsafe() const
        {
          return  _M_isUnsafe;
        }

      void _PropertyDeclarationModelItem::setUnsafe(bool isUnsafe)
      {
        _M_isUnsafe =  isUnsafe;
      }

#undef CLASS
#undef BASECLASS

      // ---------------------------------------------------------------------------
#define CLASS _AccessorDeclarationModelItem
#define BASECLASS _CodeModelItem

      _AccessorDeclarationModelItem::_AccessorDeclarationModelItem(CodeModel *model,  int kind)
          :  _CodeModelItem(model,  kind)
          ,  _M_accessPolicy( access_policy::access_private )
          ,  _M_hasAccessPolicy( false )
      {}

      _AccessorDeclarationModelItem::~_AccessorDeclarationModelItem()
      {}

      AccessorDeclarationModelItem _AccessorDeclarationModelItem::create(CodeModel *model)
      {
        AccessorDeclarationModelItem item(new _AccessorDeclarationModelItem(model));
        return  item;
      }

      AttributeSectionList _AccessorDeclarationModelItem::attributes() const
        {
          return  _M_attributes;
        }

      void _AccessorDeclarationModelItem::addAttribute(AttributeSectionModelItem item)
      {
        model()->beginAppendItem(item,  this);
        _M_attributes.append(item);
        model()->endAppendItem();
      }

      void _AccessorDeclarationModelItem::removeAttribute(AttributeSectionModelItem item)
      {
        model()->beginRemoveItem(item);
        _M_attributes.removeAt(_M_attributes.indexOf(item));
        model()->endRemoveItem();
      }

      accessor_declarations::accessor_type_enum _AccessorDeclarationModelItem::type() const
        {
          return  _M_type;
        }

      void _AccessorDeclarationModelItem::setType(accessor_declarations::accessor_type_enum type)
      {
        _M_type =  type;
      }

      access_policy::access_policy_enum _AccessorDeclarationModelItem::accessPolicy() const
        {
          return  _M_accessPolicy;
        }

      void _AccessorDeclarationModelItem::setAccessPolicy(access_policy::access_policy_enum accessPolicy)
      {
        _M_accessPolicy =  accessPolicy;
      }

      bool _AccessorDeclarationModelItem::hasAccessPolicy() const
        {
          return  _M_hasAccessPolicy;
        }

      void _AccessorDeclarationModelItem::setHasAccessPolicy(bool hasAccessPolicy)
      {
        _M_hasAccessPolicy =  hasAccessPolicy;
      }

#undef CLASS
#undef BASECLASS

      // ---------------------------------------------------------------------------
#define CLASS _MethodDeclarationModelItem
#define BASECLASS _ScopeModelItem

      _MethodDeclarationModelItem::_MethodDeclarationModelItem(CodeModel *model,  int kind)
          :  _ScopeModelItem(model,  kind)
          ,  _M_isConstructor( false )
          ,  _M_isFinalizer( false )
          ,  _M_accessPolicy( access_policy::access_private )
          ,  _M_isNew( false )
          ,  _M_isStatic( false )
          ,  _M_isVirtual( false )
          ,  _M_isSealed( false )
          ,  _M_isOverride( false )
          ,  _M_isAbstract( false )
          ,  _M_isExtern( false )
          ,  _M_isUnsafe( false )
      {}

      _MethodDeclarationModelItem::~_MethodDeclarationModelItem()
      {}

      MethodDeclarationModelItem _MethodDeclarationModelItem::create(CodeModel *model)
      {
        MethodDeclarationModelItem item(new _MethodDeclarationModelItem(model));
        return  item;
      }

      AttributeSectionList _MethodDeclarationModelItem::attributes() const
        {
          return  _M_attributes;
        }

      void _MethodDeclarationModelItem::addAttribute(AttributeSectionModelItem item)
      {
        model()->beginAppendItem(item,  this);
        _M_attributes.append(item);
        model()->endAppendItem();
      }

      void _MethodDeclarationModelItem::removeAttribute(AttributeSectionModelItem item)
      {
        model()->beginRemoveItem(item);
        _M_attributes.removeAt(_M_attributes.indexOf(item));
        model()->endRemoveItem();
      }

      TypeInfo _MethodDeclarationModelItem::returnType() const
            {
              return  _M_returnType;
            }

          void _MethodDeclarationModelItem::setReturnType(TypeInfo returnType)
          {
            _M_returnType =  returnType;
          }

          TypeInfo _MethodDeclarationModelItem::fromInterface() const
            {
              return  _M_fromInterface;
            }

          void _MethodDeclarationModelItem::setFromInterface(TypeInfo fromInterface)
          {
            _M_fromInterface =  fromInterface;
          }

          TypeParameterList _MethodDeclarationModelItem::typeParameters() const
            {
              return  _M_typeParameters;
            }

          void _MethodDeclarationModelItem::addTypeParameter(TypeParameterModelItem item)
          {
            model()->beginAppendItem(item,  this);
            _M_typeParameters.append(item);
            model()->endAppendItem();
          }

          void _MethodDeclarationModelItem::removeTypeParameter(TypeParameterModelItem item)
          {
            model()->beginRemoveItem(item);
            _M_typeParameters.removeAt(_M_typeParameters.indexOf(item));
            model()->endRemoveItem();
          }

          TypeParameterConstraintList _MethodDeclarationModelItem::typeParameterConstraints() const
            {
              return  _M_typeParameterConstraints;
            }

          void _MethodDeclarationModelItem::addTypeParameterConstraint(TypeParameterConstraintModelItem item)
          {
            model()->beginAppendItem(item,  this);
            _M_typeParameterConstraints.append(item);
            model()->endAppendItem();
          }

          void _MethodDeclarationModelItem::removeTypeParameterConstraint(TypeParameterConstraintModelItem item)
          {
            model()->beginRemoveItem(item);
            _M_typeParameterConstraints.removeAt(_M_typeParameterConstraints.indexOf(item));
            model()->endRemoveItem();
          }

          ParameterList _MethodDeclarationModelItem::parameters() const
            {
              return  _M_parameters;
            }

          void _MethodDeclarationModelItem::addParameter(ParameterModelItem item)
          {
            model()->beginAppendItem(item,  this);
            _M_parameters.append(item);
            model()->endAppendItem();
          }

          void _MethodDeclarationModelItem::removeParameter(ParameterModelItem item)
          {
            model()->beginRemoveItem(item);
            _M_parameters.removeAt(_M_parameters.indexOf(item));
            model()->endRemoveItem();
          }

          bool _MethodDeclarationModelItem::isConstructor() const
            {
              return  _M_isConstructor;
            }

          void _MethodDeclarationModelItem::setConstructor(bool isConstructor)
          {
            _M_isConstructor =  isConstructor;
          }

          bool _MethodDeclarationModelItem::isFinalizer() const
            {
              return  _M_isFinalizer;
            }

          void _MethodDeclarationModelItem::setFinalizer(bool isFinalizer)
          {
            _M_isFinalizer =  isFinalizer;
          }

          access_policy::access_policy_enum _MethodDeclarationModelItem::accessPolicy() const
            {
              return  _M_accessPolicy;
            }

          void _MethodDeclarationModelItem::setAccessPolicy(access_policy::access_policy_enum accessPolicy)
          {
            _M_accessPolicy =  accessPolicy;
          }

          bool _MethodDeclarationModelItem::isNew() const
            {
              return  _M_isNew;
            }

          void _MethodDeclarationModelItem::setNew(bool isNew)
          {
            _M_isNew =  isNew;
          }

          bool _MethodDeclarationModelItem::isStatic() const
            {
              return  _M_isStatic;
            }

          void _MethodDeclarationModelItem::setStatic(bool isStatic)
          {
            _M_isStatic =  isStatic;
          }

          bool _MethodDeclarationModelItem::isVirtual() const
            {
              return  _M_isVirtual;
            }

          void _MethodDeclarationModelItem::setVirtual(bool isVirtual)
          {
            _M_isVirtual =  isVirtual;
          }

          bool _MethodDeclarationModelItem::isSealed() const
            {
              return  _M_isSealed;
            }

          void _MethodDeclarationModelItem::setSealed(bool isSealed)
          {
            _M_isSealed =  isSealed;
          }

          bool _MethodDeclarationModelItem::isOverride() const
            {
              return  _M_isOverride;
            }

          void _MethodDeclarationModelItem::setOverride(bool isOverride)
          {
            _M_isOverride =  isOverride;
          }

          bool _MethodDeclarationModelItem::isAbstract() const
            {
              return  _M_isAbstract;
            }

          void _MethodDeclarationModelItem::setAbstract(bool isAbstract)
          {
            _M_isAbstract =  isAbstract;
          }

          bool _MethodDeclarationModelItem::isExtern() const
            {
              return  _M_isExtern;
            }

          void _MethodDeclarationModelItem::setExtern(bool isExtern)
          {
            _M_isExtern =  isExtern;
          }

          bool _MethodDeclarationModelItem::isUnsafe() const
            {
              return  _M_isUnsafe;
            }

          void _MethodDeclarationModelItem::setUnsafe(bool isUnsafe)
          {
            _M_isUnsafe =  isUnsafe;
          }

#undef CLASS
#undef BASECLASS

          // ---------------------------------------------------------------------------
#define CLASS _VariableDeclarationModelItem
#define BASECLASS _CodeModelItem

          _VariableDeclarationModelItem::_VariableDeclarationModelItem(CodeModel *model,  int kind)
              :  _CodeModelItem(model,  kind)
              ,  _M_accessPolicy( access_policy::access_private )
              ,  _M_isConstant( false )
              ,  _M_isNew( false )
              ,  _M_isStatic( false )
              ,  _M_isReadonly( false )
              ,  _M_isVolatile( false )
              ,  _M_isUnsafe( false )
          {}

          _VariableDeclarationModelItem::~_VariableDeclarationModelItem()
          {}

          VariableDeclarationModelItem _VariableDeclarationModelItem::create(CodeModel *model)
          {
            VariableDeclarationModelItem item(new _VariableDeclarationModelItem(model));
            return  item;
          }

          AttributeSectionList _VariableDeclarationModelItem::attributes() const
            {
              return  _M_attributes;
            }

          void _VariableDeclarationModelItem::addAttribute(AttributeSectionModelItem item)
          {
            model()->beginAppendItem(item,  this);
            _M_attributes.append(item);
            model()->endAppendItem();
          }

          void _VariableDeclarationModelItem::removeAttribute(AttributeSectionModelItem item)
          {
            model()->beginRemoveItem(item);
            _M_attributes.removeAt(_M_attributes.indexOf(item));
            model()->endRemoveItem();
          }

          TypeInfo _VariableDeclarationModelItem::type() const
            {
              return  _M_type;
            }

          void _VariableDeclarationModelItem::setType(TypeInfo type)
          {
            _M_type =  type;
          }

          access_policy::access_policy_enum _VariableDeclarationModelItem::accessPolicy() const
            {
              return  _M_accessPolicy;
            }

          void _VariableDeclarationModelItem::setAccessPolicy(access_policy::access_policy_enum accessPolicy)
          {
            _M_accessPolicy =  accessPolicy;
          }

          bool _VariableDeclarationModelItem::isConstant() const
            {
              return  _M_isConstant;
            }

          void _VariableDeclarationModelItem::setConstant(bool isConstant)
          {
            _M_isConstant =  isConstant;
          }

          bool _VariableDeclarationModelItem::isNew() const
            {
              return  _M_isNew;
            }

          void _VariableDeclarationModelItem::setNew(bool isNew)
          {
            _M_isNew =  isNew;
          }

          bool _VariableDeclarationModelItem::isStatic() const
            {
              return  _M_isStatic;
            }

          void _VariableDeclarationModelItem::setStatic(bool isStatic)
          {
            _M_isStatic =  isStatic;
          }

          bool _VariableDeclarationModelItem::isReadonly() const
            {
              return  _M_isReadonly;
            }

          void _VariableDeclarationModelItem::setReadonly(bool isReadonly)
          {
            _M_isReadonly =  isReadonly;
          }

          bool _VariableDeclarationModelItem::isVolatile() const
            {
              return  _M_isVolatile;
            }

          void _VariableDeclarationModelItem::setVolatile(bool isVolatile)
          {
            _M_isVolatile =  isVolatile;
          }

          bool _VariableDeclarationModelItem::isUnsafe() const
            {
              return  _M_isUnsafe;
            }

          void _VariableDeclarationModelItem::setUnsafe(bool isUnsafe)
          {
            _M_isUnsafe =  isUnsafe;
          }

#undef CLASS
#undef BASECLASS

          // ---------------------------------------------------------------------------
#define CLASS _ParameterModelItem
#define BASECLASS _CodeModelItem

          _ParameterModelItem::_ParameterModelItem(CodeModel *model,  int kind)
              :  _CodeModelItem(model,  kind)
              ,  _M_isArray( false )
              ,  _M_parameterType( parameter::value_parameter )
          {}

          _ParameterModelItem::~_ParameterModelItem()
          {}

          ParameterModelItem _ParameterModelItem::create(CodeModel *model)
          {
            ParameterModelItem item(new _ParameterModelItem(model));
            return  item;
          }

          AttributeSectionList _ParameterModelItem::attributes() const
            {
              return  _M_attributes;
            }

          void _ParameterModelItem::addAttribute(AttributeSectionModelItem item)
          {
            model()->beginAppendItem(item,  this);
            _M_attributes.append(item);
            model()->endAppendItem();
          }

          void _ParameterModelItem::removeAttribute(AttributeSectionModelItem item)
          {
            model()->beginRemoveItem(item);
            _M_attributes.removeAt(_M_attributes.indexOf(item));
            model()->endRemoveItem();
          }

          TypeInfo _ParameterModelItem::type() const
            {
              return  _M_type;
            }

          void _ParameterModelItem::setType(TypeInfo type)
          {
            _M_type =  type;
          }

          bool _ParameterModelItem::isArray() const
            {
              return  _M_isArray;
            }

          void _ParameterModelItem::setArray(bool isArray)
          {
            _M_isArray =  isArray;
          }

          parameter::parameter_type_enum _ParameterModelItem::parameterType() const
            {
              return  _M_parameterType;
            }

          void _ParameterModelItem::setParameterType(parameter::parameter_type_enum parameterType)
          {
            _M_parameterType =  parameterType;
          }

#undef CLASS
#undef BASECLASS

          // ---------------------------------------------------------------------------
#define CLASS _TypeParameterModelItem
#define BASECLASS _CodeModelItem

          _TypeParameterModelItem::_TypeParameterModelItem(CodeModel *model,  int kind)
              :  _CodeModelItem(model,  kind)
          {}

          _TypeParameterModelItem::~_TypeParameterModelItem()
          {}

          TypeParameterModelItem _TypeParameterModelItem::create(CodeModel *model)
          {
            TypeParameterModelItem item(new _TypeParameterModelItem(model));
            return  item;
          }

          AttributeSectionList _TypeParameterModelItem::attributes() const
            {
              return  _M_attributes;
            }

          void _TypeParameterModelItem::addAttribute(AttributeSectionModelItem item)
          {
            model()->beginAppendItem(item,  this);
            _M_attributes.append(item);
            model()->endAppendItem();
          }

          void _TypeParameterModelItem::removeAttribute(AttributeSectionModelItem item)
          {
            model()->beginRemoveItem(item);
            _M_attributes.removeAt(_M_attributes.indexOf(item));
            model()->endRemoveItem();
          }

#undef CLASS
#undef BASECLASS

          // ---------------------------------------------------------------------------
#define CLASS _TypeParameterConstraintModelItem
#define BASECLASS _CodeModelItem

          _TypeParameterConstraintModelItem::_TypeParameterConstraintModelItem(CodeModel *model,  int kind)
              :  _CodeModelItem(model,  kind)
          {}

          _TypeParameterConstraintModelItem::~_TypeParameterConstraintModelItem()
          {}

          TypeParameterConstraintModelItem _TypeParameterConstraintModelItem::create(CodeModel *model)
          {
            TypeParameterConstraintModelItem item(new _TypeParameterConstraintModelItem(model));
            return  item;
          }

#undef CLASS
#undef BASECLASS

          // ---------------------------------------------------------------------------
#define CLASS _PrimaryOrSecondaryConstraintModelItem
#define BASECLASS _TypeParameterConstraintModelItem

          _PrimaryOrSecondaryConstraintModelItem::_PrimaryOrSecondaryConstraintModelItem(CodeModel *model,  int kind)
              :  _TypeParameterConstraintModelItem(model,  kind)
              ,  _M_constraint_type( primary_or_secondary_constraint::type_type )
          {}

          _PrimaryOrSecondaryConstraintModelItem::~_PrimaryOrSecondaryConstraintModelItem()
          {}

          PrimaryOrSecondaryConstraintModelItem _PrimaryOrSecondaryConstraintModelItem::create(CodeModel *model)
          {
            PrimaryOrSecondaryConstraintModelItem item(new _PrimaryOrSecondaryConstraintModelItem(model));
            return  item;
          }

          TypeInfo _PrimaryOrSecondaryConstraintModelItem::typeOrParameterName() const
            {
              return  _M_typeOrParameterName;
            }

          void _PrimaryOrSecondaryConstraintModelItem::setTypeOrParameterName(TypeInfo typeOrParameterName)
          {
            _M_typeOrParameterName =  typeOrParameterName;
          }

          primary_or_secondary_constraint::primary_or_secondary_constraint_enum _PrimaryOrSecondaryConstraintModelItem::constraint_type() const
            {
              return  _M_constraint_type;
            }

          void _PrimaryOrSecondaryConstraintModelItem::setConstraint_type(primary_or_secondary_constraint::primary_or_secondary_constraint_enum constraint_type)
          {
            _M_constraint_type =  constraint_type;
          }

#undef CLASS
#undef BASECLASS

          // ---------------------------------------------------------------------------
#define CLASS _ConstructorConstraintModelItem
#define BASECLASS _TypeParameterConstraintModelItem

          _ConstructorConstraintModelItem::_ConstructorConstraintModelItem(CodeModel *model,  int kind)
              :  _TypeParameterConstraintModelItem(model,  kind)
          {}

          _ConstructorConstraintModelItem::~_ConstructorConstraintModelItem()
          {}

          ConstructorConstraintModelItem _ConstructorConstraintModelItem::create(CodeModel *model)
          {
            ConstructorConstraintModelItem item(new _ConstructorConstraintModelItem(model));
            return  item;
          }

#undef CLASS
#undef BASECLASS

          // ---------------------------------------------------------------------------
#define CLASS _AttributeSectionModelItem
#define BASECLASS _CodeModelItem

          _AttributeSectionModelItem::_AttributeSectionModelItem(CodeModel *model,  int kind)
              :  _CodeModelItem(model,  kind)
          {
            setName(QString::null);
          }

          _AttributeSectionModelItem::~_AttributeSectionModelItem()
          {}

          AttributeSectionModelItem _AttributeSectionModelItem::create(CodeModel *model)
          {
            AttributeSectionModelItem item(new _AttributeSectionModelItem(model));
            return  item;
          }

          QString _AttributeSectionModelItem::target() const
            {
              return  _M_target;
            }

          void _AttributeSectionModelItem::setTarget(QString target)
          {
            _M_target =  target;
          }

          QString _AttributeSectionModelItem::attribute() const
            {
              return  _M_attribute;
            }

          void _AttributeSectionModelItem::setAttribute(QString attribute)
          {
            _M_attribute =  attribute;
          }

#undef CLASS
#undef BASECLASS


        } // end of namespace csharp


