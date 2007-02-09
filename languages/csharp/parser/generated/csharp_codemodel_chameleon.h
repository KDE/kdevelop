// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#ifndef csharp_CODEMODEL_CHAMELEON_H
#define csharp_CODEMODEL_CHAMELEON_H

#include "csharp_codemodel.h"

namespace csharp
  {

  template  <class T>

  struct Nullable
    {
      bool isNull;
      T item;
      Nullable() :  isNull(true)
      {}

      Nullable(T _item) :  isNull(false),  item(_item)
      {}

    }

  ;

  /** Don't instantiate this class directly.
   * Instead, use the wrapper named ModelItemChameleon
   * which provides shared pointer abilities.
   */

  class _ModelItemChameleon :  public KDevelop::Shared
    {
      CodeModelItem _M_item;

    public:
      CodeModelItem item()
      {
        return  _M_item;
      }

      _ModelItemChameleon(CodeModelItem item) :  _M_item(item)
      {}

      ~_ModelItemChameleon()
      {}

      Nullable<QString > name( ) const;
      void setName(QString name);

      Nullable<QStringList > scope( ) const;
      void setScope(QStringList scope);

      Nullable<QString > fileName( ) const;
      void setFileName(QString fileName);

      Nullable<NamespaceDeclarationList > namespaces( ) const;
      void addNamespace(NamespaceDeclarationModelItem item);
      void removeNamespace(NamespaceDeclarationModelItem item);
      Nullable<NamespaceDeclarationModelItem > findNamespace(const QString & name) const;

      Nullable<ExternAliasDirectiveList > externAliases( ) const;
      void addExternAlias(ExternAliasDirectiveModelItem item);
      void removeExternAlias(ExternAliasDirectiveModelItem item);
      Nullable<ExternAliasDirectiveModelItem > findExternAlias(const QString & name) const;

      Nullable<UsingAliasDirectiveList > usingAliases( ) const;
      void addUsingAlias(UsingAliasDirectiveModelItem item);
      void removeUsingAlias(UsingAliasDirectiveModelItem item);
      Nullable<UsingAliasDirectiveModelItem > findUsingAlias(const QString & name) const;

      Nullable<UsingNamespaceDirectiveList > usingNamespaces( ) const;
      void addUsingNamespace(UsingNamespaceDirectiveModelItem item);
      void removeUsingNamespace(UsingNamespaceDirectiveModelItem item);

      Nullable<ClassDeclarationList > classes( ) const;
      void addClass(ClassDeclarationModelItem item);
      void removeClass(ClassDeclarationModelItem item);
      Nullable<ClassDeclarationModelItem > findClass(const QString & name) const;

      Nullable<StructDeclarationList > structs( ) const;
      void addStruct(StructDeclarationModelItem item);
      void removeStruct(StructDeclarationModelItem item);
      Nullable<StructDeclarationModelItem > findStruct(const QString & name) const;

      Nullable<InterfaceDeclarationList > interfaces( ) const;
      void addInterface(InterfaceDeclarationModelItem item);
      void removeInterface(InterfaceDeclarationModelItem item);
      Nullable<InterfaceDeclarationModelItem > findInterface(const QString & name) const;

      Nullable<EnumDeclarationList > enums( ) const;
      void addEnum(EnumDeclarationModelItem item);
      void removeEnum(EnumDeclarationModelItem item);
      Nullable<EnumDeclarationModelItem > findEnum(const QString & name) const;

      Nullable<DelegateDeclarationList > delegates( ) const;
      void addDelegate(DelegateDeclarationModelItem item);
      void removeDelegate(DelegateDeclarationModelItem item);
      Nullable<DelegateDeclarationModelItem > findDelegate(const QString & name) const;

      Nullable<TypeModelItem > namespaceOrType( ) const;
      void setNamespaceOrType(TypeModelItem namespaceOrType);

      Nullable<QStringList > namespaceUsed( ) const;
      void setNamespaceUsed(QStringList namespaceUsed);

      Nullable<access_policy::access_policy_enum > accessPolicy( ) const;
      void setAccessPolicy(access_policy::access_policy_enum accessPolicy);

      Nullable<bool > isNew( ) const;
      void setNew(bool isNew);

      Nullable<TypeModelItem > baseType( ) const;
      void setBaseType(TypeModelItem baseType);

      Nullable<EventDeclarationList > events( ) const;
      void addEvent(EventDeclarationModelItem item);
      void removeEvent(EventDeclarationModelItem item);
      Nullable<EventDeclarationModelItem > findEvent(const QString & name) const;

      Nullable<IndexerDeclarationList > indexers( ) const;
      void addIndexer(IndexerDeclarationModelItem item);
      void removeIndexer(IndexerDeclarationModelItem item);

      Nullable<PropertyDeclarationList > propertys( ) const;
      void addProperty(PropertyDeclarationModelItem item);
      void removeProperty(PropertyDeclarationModelItem item);
      Nullable<PropertyDeclarationModelItem > findProperty(const QString & name) const;

      Nullable<MethodDeclarationList > methods( ) const;
      void addMethod(MethodDeclarationModelItem item);
      void removeMethod(MethodDeclarationModelItem item);
      Nullable<MethodDeclarationList > findMethods(const QString & name) const;

      Nullable<TypeParameterList > typeParameters( ) const;
      void addTypeParameter(TypeParameterModelItem item);
      void removeTypeParameter(TypeParameterModelItem item);

      Nullable<TypeParameterConstraintList > typeParameterConstraints( ) const;
      void addTypeParameterConstraint(TypeParameterConstraintModelItem item);
      void removeTypeParameterConstraint(TypeParameterConstraintModelItem item);

      Nullable<bool > isUnsafe( ) const;
      void setUnsafe(bool isUnsafe);

      Nullable<VariableDeclarationList > variables( ) const;
      void addVariable(VariableDeclarationModelItem item);
      void removeVariable(VariableDeclarationModelItem item);
      Nullable<VariableDeclarationModelItem > findVariable(const QString & name) const;

      Nullable<bool > isSealed( ) const;
      void setSealed(bool isSealed);

      Nullable<bool > isAbstract( ) const;
      void setAbstract(bool isAbstract);

      Nullable<bool > isStatic( ) const;
      void setStatic(bool isStatic);

      Nullable<TypeModelItem > returnType( ) const;
      void setReturnType(TypeModelItem returnType);

      Nullable<ParameterList > parameters( ) const;
      void addParameter(ParameterModelItem item);
      void removeParameter(ParameterModelItem item);

      Nullable<EnumValueList > enumValues( ) const;
      void addEnumValue(EnumValueModelItem item);
      void removeEnumValue(EnumValueModelItem item);

      Nullable<TypeModelItem > baseIntegralType( ) const;
      void setBaseIntegralType(TypeModelItem baseIntegralType);

      Nullable<QString > value( ) const;
      void setValue(QString value);

      Nullable<TypeModelItem > type( ) const;
      void setType(TypeModelItem type);

      Nullable<TypeModelItem > fromInterface( ) const;
      void setFromInterface(TypeModelItem fromInterface);

      Nullable<EventAccessorDeclarationModelItem > addAccessor( ) const;
      void setAddAccessor(EventAccessorDeclarationModelItem addAccessor);

      Nullable<EventAccessorDeclarationModelItem > removeAccessor( ) const;
      void setRemoveAccessor(EventAccessorDeclarationModelItem removeAccessor);

      Nullable<bool > isVirtual( ) const;
      void setVirtual(bool isVirtual);

      Nullable<bool > isOverride( ) const;
      void setOverride(bool isOverride);

      Nullable<bool > isExtern( ) const;
      void setExtern(bool isExtern);

      Nullable<AccessorDeclarationList > accessors( ) const;
      void addAccessor(AccessorDeclarationModelItem item);
      void removeAccessor(AccessorDeclarationModelItem item);

      void setType(accessor_declarations::accessor_type_enum type);

      Nullable<bool > hasAccessPolicy( ) const;
      void setHasAccessPolicy(bool hasAccessPolicy);

      Nullable<bool > isInterfaceMethodDeclaration( ) const;
      void setInterfaceMethodDeclaration(bool isInterfaceMethodDeclaration);

      Nullable<VariableDeclarationList > localVariables( ) const;
      void addLocalVariable(VariableDeclarationModelItem item);
      void removeLocalVariable(VariableDeclarationModelItem item);

      Nullable<bool > isConstant( ) const;
      void setConstant(bool isConstant);

      Nullable<bool > isReadonly( ) const;
      void setReadonly(bool isReadonly);

      Nullable<bool > isVolatile( ) const;
      void setVolatile(bool isVolatile);

      Nullable<TypeList > typeArguments( ) const;
      void addTypeArgument(TypeModelItem item);
      void removeTypeArgument(TypeModelItem item);

      Nullable<QString > qualifiedAliasLabel( ) const;
      void setQualifiedAliasLabel(QString qualifiedAliasLabel);

      Nullable<TypePartList > typeParts( ) const;
      void addTypePart(TypePartModelItem item);
      void removeTypePart(TypePartModelItem item);

      Nullable<bool > isArray( ) const;
      void setArray(bool isArray);

      Nullable<parameter::parameter_type_enum > parameterType( ) const;
      void setParameterType(parameter::parameter_type_enum parameterType);

      Nullable<TypeModelItem > typeOrParameterName( ) const;
      void setTypeOrParameterName(TypeModelItem typeOrParameterName);

      Nullable<primary_or_secondary_constraint::primary_or_secondary_constraint_enum > constraint_type( ) const;
      void setConstraint_type(primary_or_secondary_constraint::primary_or_secondary_constraint_enum constraint_type);

    private:
      _ModelItemChameleon(const _ModelItemChameleon &other);
      void operator=(const _ModelItemChameleon &other);
    };

  class ModelItemChameleon :  public KDevelop::SharedPtr<_ModelItemChameleon>
    {

    public:
      ModelItemChameleon(CodeModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(new _ModelItemChameleon(item))
      {}

      ModelItemChameleon(ScopeModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(NamespaceDeclarationModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(GlobalNamespaceDeclarationModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(ExternAliasDirectiveModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(UsingAliasDirectiveModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(UsingNamespaceDirectiveModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(TypeDeclarationModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(ClassLikeDeclarationModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(ClassDeclarationModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(StructDeclarationModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(InterfaceDeclarationModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(DelegateDeclarationModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(EnumDeclarationModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(EnumValueModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(EventDeclarationModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(EventAccessorDeclarationModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(IndexerDeclarationModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(PropertyDeclarationModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(AccessorDeclarationModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(MethodDeclarationModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(ConstructorDeclarationModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(FinalizerDeclarationModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(VariableDeclarationModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(TypePartModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(TypeModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(ParameterModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(TypeParameterModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(TypeParameterConstraintModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(PrimaryOrSecondaryConstraintModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

      ModelItemChameleon(ConstructorConstraintModelItem item)
          :  KDevelop::SharedPtr<_ModelItemChameleon>(
            new _ModelItemChameleon(model_static_cast<CodeModelItem>(item)) )
      {}

    }

  ;


} // end of namespace csharp

#endif


