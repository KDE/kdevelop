// THIS FILE IS GENERATED
// WARNING! All changes made in this file will be lost!

#include "csharp_codemodel_chameleon.h"

namespace csharp
  {


  Nullable<QString > _ModelItemChameleon::name( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_Scope:
          {
            return  Nullable<QString >(model_static_cast<ScopeModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_NamespaceDeclaration:
          {
            return  Nullable<QString >(model_static_cast<NamespaceDeclarationModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
          {
            return  Nullable<QString >(model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_ExternAliasDirective:
          {
            return  Nullable<QString >(model_static_cast<ExternAliasDirectiveModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_UsingAliasDirective:
          {
            return  Nullable<QString >(model_static_cast<UsingAliasDirectiveModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_UsingNamespaceDirective:
          {
            return  Nullable<QString >(model_static_cast<UsingNamespaceDirectiveModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_TypeDeclaration:
          {
            return  Nullable<QString >(model_static_cast<TypeDeclarationModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<QString >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<QString >(model_static_cast<ClassDeclarationModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<QString >(model_static_cast<StructDeclarationModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<QString >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_DelegateDeclaration:
          {
            return  Nullable<QString >(model_static_cast<DelegateDeclarationModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_EnumDeclaration:
          {
            return  Nullable<QString >(model_static_cast<EnumDeclarationModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_EnumValue:
          {
            return  Nullable<QString >(model_static_cast<EnumValueModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_EventDeclaration:
          {
            return  Nullable<QString >(model_static_cast<EventDeclarationModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_EventAccessorDeclaration:
          {
            return  Nullable<QString >(model_static_cast<EventAccessorDeclarationModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_IndexerDeclaration:
          {
            return  Nullable<QString >(model_static_cast<IndexerDeclarationModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_PropertyDeclaration:
          {
            return  Nullable<QString >(model_static_cast<PropertyDeclarationModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_AccessorDeclaration:
          {
            return  Nullable<QString >(model_static_cast<AccessorDeclarationModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_MethodDeclaration:
          {
            return  Nullable<QString >(model_static_cast<MethodDeclarationModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_ConstructorDeclaration:
          {
            return  Nullable<QString >(model_static_cast<ConstructorDeclarationModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_FinalizerDeclaration:
          {
            return  Nullable<QString >(model_static_cast<FinalizerDeclarationModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_VariableDeclaration:
          {
            return  Nullable<QString >(model_static_cast<VariableDeclarationModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_TypePart:
          {
            return  Nullable<QString >(model_static_cast<TypePartModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_Type:
          {
            return  Nullable<QString >(model_static_cast<TypeModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_Parameter:
          {
            return  Nullable<QString >(model_static_cast<ParameterModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_TypeParameter:
          {
            return  Nullable<QString >(model_static_cast<TypeParameterModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_TypeParameterConstraint:
          {
            return  Nullable<QString >(model_static_cast<TypeParameterConstraintModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_PrimaryOrSecondaryConstraint:
          {
            return  Nullable<QString >(model_static_cast<PrimaryOrSecondaryConstraintModelItem>(_M_item)->name());
          }

        case _CodeModelItem::Kind_ConstructorConstraint:
          {
            return  Nullable<QString >(model_static_cast<ConstructorConstraintModelItem>(_M_item)->name());
          }

        default:
          break;
        }

      return  Nullable<QString>();
    }

  void _ModelItemChameleon::setName(QString name)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_Scope:
        {
          model_static_cast<ScopeModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_NamespaceDeclaration:
        {
          model_static_cast<NamespaceDeclarationModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
        {
          model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_ExternAliasDirective:
        {
          model_static_cast<ExternAliasDirectiveModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_UsingAliasDirective:
        {
          model_static_cast<UsingAliasDirectiveModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_UsingNamespaceDirective:
        {
          model_static_cast<UsingNamespaceDirectiveModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_TypeDeclaration:
        {
          model_static_cast<TypeDeclarationModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_DelegateDeclaration:
        {
          model_static_cast<DelegateDeclarationModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_EnumDeclaration:
        {
          model_static_cast<EnumDeclarationModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_EnumValue:
        {
          model_static_cast<EnumValueModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_EventDeclaration:
        {
          model_static_cast<EventDeclarationModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_EventAccessorDeclaration:
        {
          model_static_cast<EventAccessorDeclarationModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_IndexerDeclaration:
        {
          model_static_cast<IndexerDeclarationModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_PropertyDeclaration:
        {
          model_static_cast<PropertyDeclarationModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_AccessorDeclaration:
        {
          model_static_cast<AccessorDeclarationModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_MethodDeclaration:
        {
          model_static_cast<MethodDeclarationModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_ConstructorDeclaration:
        {
          model_static_cast<ConstructorDeclarationModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_FinalizerDeclaration:
        {
          model_static_cast<FinalizerDeclarationModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_VariableDeclaration:
        {
          model_static_cast<VariableDeclarationModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_TypePart:
        {
          model_static_cast<TypePartModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_Type:
        {
          model_static_cast<TypeModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_Parameter:
        {
          model_static_cast<ParameterModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_TypeParameter:
        {
          model_static_cast<TypeParameterModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_TypeParameterConstraint:
        {
          model_static_cast<TypeParameterConstraintModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_PrimaryOrSecondaryConstraint:
        {
          model_static_cast<PrimaryOrSecondaryConstraintModelItem>(_M_item)->setName(name);
          return ;
        }

      case _CodeModelItem::Kind_ConstructorConstraint:
        {
          model_static_cast<ConstructorConstraintModelItem>(_M_item)->setName(name);
          return ;
        }

      default:
        break;
      }

    return ;
  }


  Nullable<QStringList > _ModelItemChameleon::scope( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_Scope:
          {
            return  Nullable<QStringList >(model_static_cast<ScopeModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_NamespaceDeclaration:
          {
            return  Nullable<QStringList >(model_static_cast<NamespaceDeclarationModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
          {
            return  Nullable<QStringList >(model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_ExternAliasDirective:
          {
            return  Nullable<QStringList >(model_static_cast<ExternAliasDirectiveModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_UsingAliasDirective:
          {
            return  Nullable<QStringList >(model_static_cast<UsingAliasDirectiveModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_UsingNamespaceDirective:
          {
            return  Nullable<QStringList >(model_static_cast<UsingNamespaceDirectiveModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_TypeDeclaration:
          {
            return  Nullable<QStringList >(model_static_cast<TypeDeclarationModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<QStringList >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<QStringList >(model_static_cast<ClassDeclarationModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<QStringList >(model_static_cast<StructDeclarationModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<QStringList >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_DelegateDeclaration:
          {
            return  Nullable<QStringList >(model_static_cast<DelegateDeclarationModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_EnumDeclaration:
          {
            return  Nullable<QStringList >(model_static_cast<EnumDeclarationModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_EnumValue:
          {
            return  Nullable<QStringList >(model_static_cast<EnumValueModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_EventDeclaration:
          {
            return  Nullable<QStringList >(model_static_cast<EventDeclarationModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_EventAccessorDeclaration:
          {
            return  Nullable<QStringList >(model_static_cast<EventAccessorDeclarationModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_IndexerDeclaration:
          {
            return  Nullable<QStringList >(model_static_cast<IndexerDeclarationModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_PropertyDeclaration:
          {
            return  Nullable<QStringList >(model_static_cast<PropertyDeclarationModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_AccessorDeclaration:
          {
            return  Nullable<QStringList >(model_static_cast<AccessorDeclarationModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_MethodDeclaration:
          {
            return  Nullable<QStringList >(model_static_cast<MethodDeclarationModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_ConstructorDeclaration:
          {
            return  Nullable<QStringList >(model_static_cast<ConstructorDeclarationModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_FinalizerDeclaration:
          {
            return  Nullable<QStringList >(model_static_cast<FinalizerDeclarationModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_VariableDeclaration:
          {
            return  Nullable<QStringList >(model_static_cast<VariableDeclarationModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_TypePart:
          {
            return  Nullable<QStringList >(model_static_cast<TypePartModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_Type:
          {
            return  Nullable<QStringList >(model_static_cast<TypeModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_Parameter:
          {
            return  Nullable<QStringList >(model_static_cast<ParameterModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_TypeParameter:
          {
            return  Nullable<QStringList >(model_static_cast<TypeParameterModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_TypeParameterConstraint:
          {
            return  Nullable<QStringList >(model_static_cast<TypeParameterConstraintModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_PrimaryOrSecondaryConstraint:
          {
            return  Nullable<QStringList >(model_static_cast<PrimaryOrSecondaryConstraintModelItem>(_M_item)->scope());
          }

        case _CodeModelItem::Kind_ConstructorConstraint:
          {
            return  Nullable<QStringList >(model_static_cast<ConstructorConstraintModelItem>(_M_item)->scope());
          }

        default:
          break;
        }

      return  Nullable<QStringList>();
    }

  void _ModelItemChameleon::setScope(QStringList scope)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_Scope:
        {
          model_static_cast<ScopeModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_NamespaceDeclaration:
        {
          model_static_cast<NamespaceDeclarationModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
        {
          model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_ExternAliasDirective:
        {
          model_static_cast<ExternAliasDirectiveModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_UsingAliasDirective:
        {
          model_static_cast<UsingAliasDirectiveModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_UsingNamespaceDirective:
        {
          model_static_cast<UsingNamespaceDirectiveModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_TypeDeclaration:
        {
          model_static_cast<TypeDeclarationModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_DelegateDeclaration:
        {
          model_static_cast<DelegateDeclarationModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_EnumDeclaration:
        {
          model_static_cast<EnumDeclarationModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_EnumValue:
        {
          model_static_cast<EnumValueModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_EventDeclaration:
        {
          model_static_cast<EventDeclarationModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_EventAccessorDeclaration:
        {
          model_static_cast<EventAccessorDeclarationModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_IndexerDeclaration:
        {
          model_static_cast<IndexerDeclarationModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_PropertyDeclaration:
        {
          model_static_cast<PropertyDeclarationModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_AccessorDeclaration:
        {
          model_static_cast<AccessorDeclarationModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_MethodDeclaration:
        {
          model_static_cast<MethodDeclarationModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_ConstructorDeclaration:
        {
          model_static_cast<ConstructorDeclarationModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_FinalizerDeclaration:
        {
          model_static_cast<FinalizerDeclarationModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_VariableDeclaration:
        {
          model_static_cast<VariableDeclarationModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_TypePart:
        {
          model_static_cast<TypePartModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_Type:
        {
          model_static_cast<TypeModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_Parameter:
        {
          model_static_cast<ParameterModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_TypeParameter:
        {
          model_static_cast<TypeParameterModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_TypeParameterConstraint:
        {
          model_static_cast<TypeParameterConstraintModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_PrimaryOrSecondaryConstraint:
        {
          model_static_cast<PrimaryOrSecondaryConstraintModelItem>(_M_item)->setScope(scope);
          return ;
        }

      case _CodeModelItem::Kind_ConstructorConstraint:
        {
          model_static_cast<ConstructorConstraintModelItem>(_M_item)->setScope(scope);
          return ;
        }

      default:
        break;
      }

    return ;
  }


  Nullable<QString > _ModelItemChameleon::fileName( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_Scope:
          {
            return  Nullable<QString >(model_static_cast<ScopeModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_NamespaceDeclaration:
          {
            return  Nullable<QString >(model_static_cast<NamespaceDeclarationModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
          {
            return  Nullable<QString >(model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_ExternAliasDirective:
          {
            return  Nullable<QString >(model_static_cast<ExternAliasDirectiveModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_UsingAliasDirective:
          {
            return  Nullable<QString >(model_static_cast<UsingAliasDirectiveModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_UsingNamespaceDirective:
          {
            return  Nullable<QString >(model_static_cast<UsingNamespaceDirectiveModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_TypeDeclaration:
          {
            return  Nullable<QString >(model_static_cast<TypeDeclarationModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<QString >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<QString >(model_static_cast<ClassDeclarationModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<QString >(model_static_cast<StructDeclarationModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<QString >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_DelegateDeclaration:
          {
            return  Nullable<QString >(model_static_cast<DelegateDeclarationModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_EnumDeclaration:
          {
            return  Nullable<QString >(model_static_cast<EnumDeclarationModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_EnumValue:
          {
            return  Nullable<QString >(model_static_cast<EnumValueModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_EventDeclaration:
          {
            return  Nullable<QString >(model_static_cast<EventDeclarationModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_EventAccessorDeclaration:
          {
            return  Nullable<QString >(model_static_cast<EventAccessorDeclarationModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_IndexerDeclaration:
          {
            return  Nullable<QString >(model_static_cast<IndexerDeclarationModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_PropertyDeclaration:
          {
            return  Nullable<QString >(model_static_cast<PropertyDeclarationModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_AccessorDeclaration:
          {
            return  Nullable<QString >(model_static_cast<AccessorDeclarationModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_MethodDeclaration:
          {
            return  Nullable<QString >(model_static_cast<MethodDeclarationModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_ConstructorDeclaration:
          {
            return  Nullable<QString >(model_static_cast<ConstructorDeclarationModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_FinalizerDeclaration:
          {
            return  Nullable<QString >(model_static_cast<FinalizerDeclarationModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_VariableDeclaration:
          {
            return  Nullable<QString >(model_static_cast<VariableDeclarationModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_TypePart:
          {
            return  Nullable<QString >(model_static_cast<TypePartModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_Type:
          {
            return  Nullable<QString >(model_static_cast<TypeModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_Parameter:
          {
            return  Nullable<QString >(model_static_cast<ParameterModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_TypeParameter:
          {
            return  Nullable<QString >(model_static_cast<TypeParameterModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_TypeParameterConstraint:
          {
            return  Nullable<QString >(model_static_cast<TypeParameterConstraintModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_PrimaryOrSecondaryConstraint:
          {
            return  Nullable<QString >(model_static_cast<PrimaryOrSecondaryConstraintModelItem>(_M_item)->fileName());
          }

        case _CodeModelItem::Kind_ConstructorConstraint:
          {
            return  Nullable<QString >(model_static_cast<ConstructorConstraintModelItem>(_M_item)->fileName());
          }

        default:
          break;
        }

      return  Nullable<QString>();
    }

  void _ModelItemChameleon::setFileName(QString fileName)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_Scope:
        {
          model_static_cast<ScopeModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_NamespaceDeclaration:
        {
          model_static_cast<NamespaceDeclarationModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
        {
          model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_ExternAliasDirective:
        {
          model_static_cast<ExternAliasDirectiveModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_UsingAliasDirective:
        {
          model_static_cast<UsingAliasDirectiveModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_UsingNamespaceDirective:
        {
          model_static_cast<UsingNamespaceDirectiveModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_TypeDeclaration:
        {
          model_static_cast<TypeDeclarationModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_DelegateDeclaration:
        {
          model_static_cast<DelegateDeclarationModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_EnumDeclaration:
        {
          model_static_cast<EnumDeclarationModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_EnumValue:
        {
          model_static_cast<EnumValueModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_EventDeclaration:
        {
          model_static_cast<EventDeclarationModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_EventAccessorDeclaration:
        {
          model_static_cast<EventAccessorDeclarationModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_IndexerDeclaration:
        {
          model_static_cast<IndexerDeclarationModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_PropertyDeclaration:
        {
          model_static_cast<PropertyDeclarationModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_AccessorDeclaration:
        {
          model_static_cast<AccessorDeclarationModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_MethodDeclaration:
        {
          model_static_cast<MethodDeclarationModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_ConstructorDeclaration:
        {
          model_static_cast<ConstructorDeclarationModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_FinalizerDeclaration:
        {
          model_static_cast<FinalizerDeclarationModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_VariableDeclaration:
        {
          model_static_cast<VariableDeclarationModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_TypePart:
        {
          model_static_cast<TypePartModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_Type:
        {
          model_static_cast<TypeModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_Parameter:
        {
          model_static_cast<ParameterModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_TypeParameter:
        {
          model_static_cast<TypeParameterModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_TypeParameterConstraint:
        {
          model_static_cast<TypeParameterConstraintModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_PrimaryOrSecondaryConstraint:
        {
          model_static_cast<PrimaryOrSecondaryConstraintModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      case _CodeModelItem::Kind_ConstructorConstraint:
        {
          model_static_cast<ConstructorConstraintModelItem>(_M_item)->setFileName(fileName);
          return ;
        }

      default:
        break;
      }

    return ;
  }


  Nullable<NamespaceDeclarationList > _ModelItemChameleon::namespaces( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_NamespaceDeclaration:
          {
            return  Nullable<NamespaceDeclarationList >(model_static_cast<NamespaceDeclarationModelItem>(_M_item)->namespaces());
          }

        case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
          {
            return  Nullable<NamespaceDeclarationList >(model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->namespaces());
          }

        default:
          break;
        }

      return  Nullable<NamespaceDeclarationList>();
    }

  void _ModelItemChameleon::addNamespace(NamespaceDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_NamespaceDeclaration:
        {
          model_static_cast<NamespaceDeclarationModelItem>(_M_item)->addNamespace(item);
          return ;
        }

      case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
        {
          model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->addNamespace(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  void _ModelItemChameleon::removeNamespace(NamespaceDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_NamespaceDeclaration:
        {
          model_static_cast<NamespaceDeclarationModelItem>(_M_item)->removeNamespace(item);
          return ;
        }

      case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
        {
          model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->removeNamespace(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  Nullable<NamespaceDeclarationModelItem > _ModelItemChameleon::findNamespace(const QString & name) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_NamespaceDeclaration:
          {
            return  Nullable<NamespaceDeclarationModelItem >(model_static_cast<NamespaceDeclarationModelItem>(_M_item)->findNamespace(name));
          }

        case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
          {
            return  Nullable<NamespaceDeclarationModelItem >(model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->findNamespace(name));
          }

        default:
          break;
        }

      return  Nullable<NamespaceDeclarationModelItem>();
    }


  Nullable<ExternAliasDirectiveList > _ModelItemChameleon::externAliases( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_NamespaceDeclaration:
          {
            return  Nullable<ExternAliasDirectiveList >(model_static_cast<NamespaceDeclarationModelItem>(_M_item)->externAliases());
          }

        case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
          {
            return  Nullable<ExternAliasDirectiveList >(model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->externAliases());
          }

        default:
          break;
        }

      return  Nullable<ExternAliasDirectiveList>();
    }

  void _ModelItemChameleon::addExternAlias(ExternAliasDirectiveModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_NamespaceDeclaration:
        {
          model_static_cast<NamespaceDeclarationModelItem>(_M_item)->addExternAlias(item);
          return ;
        }

      case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
        {
          model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->addExternAlias(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  void _ModelItemChameleon::removeExternAlias(ExternAliasDirectiveModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_NamespaceDeclaration:
        {
          model_static_cast<NamespaceDeclarationModelItem>(_M_item)->removeExternAlias(item);
          return ;
        }

      case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
        {
          model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->removeExternAlias(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  Nullable<ExternAliasDirectiveModelItem > _ModelItemChameleon::findExternAlias(const QString & name) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_NamespaceDeclaration:
          {
            return  Nullable<ExternAliasDirectiveModelItem >(model_static_cast<NamespaceDeclarationModelItem>(_M_item)->findExternAlias(name));
          }

        case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
          {
            return  Nullable<ExternAliasDirectiveModelItem >(model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->findExternAlias(name));
          }

        default:
          break;
        }

      return  Nullable<ExternAliasDirectiveModelItem>();
    }


  Nullable<UsingAliasDirectiveList > _ModelItemChameleon::usingAliases( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_NamespaceDeclaration:
          {
            return  Nullable<UsingAliasDirectiveList >(model_static_cast<NamespaceDeclarationModelItem>(_M_item)->usingAliases());
          }

        case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
          {
            return  Nullable<UsingAliasDirectiveList >(model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->usingAliases());
          }

        default:
          break;
        }

      return  Nullable<UsingAliasDirectiveList>();
    }

  void _ModelItemChameleon::addUsingAlias(UsingAliasDirectiveModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_NamespaceDeclaration:
        {
          model_static_cast<NamespaceDeclarationModelItem>(_M_item)->addUsingAlias(item);
          return ;
        }

      case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
        {
          model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->addUsingAlias(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  void _ModelItemChameleon::removeUsingAlias(UsingAliasDirectiveModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_NamespaceDeclaration:
        {
          model_static_cast<NamespaceDeclarationModelItem>(_M_item)->removeUsingAlias(item);
          return ;
        }

      case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
        {
          model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->removeUsingAlias(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  Nullable<UsingAliasDirectiveModelItem > _ModelItemChameleon::findUsingAlias(const QString & name) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_NamespaceDeclaration:
          {
            return  Nullable<UsingAliasDirectiveModelItem >(model_static_cast<NamespaceDeclarationModelItem>(_M_item)->findUsingAlias(name));
          }

        case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
          {
            return  Nullable<UsingAliasDirectiveModelItem >(model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->findUsingAlias(name));
          }

        default:
          break;
        }

      return  Nullable<UsingAliasDirectiveModelItem>();
    }


  Nullable<UsingNamespaceDirectiveList > _ModelItemChameleon::usingNamespaces( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_NamespaceDeclaration:
          {
            return  Nullable<UsingNamespaceDirectiveList >(model_static_cast<NamespaceDeclarationModelItem>(_M_item)->usingNamespaces());
          }

        case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
          {
            return  Nullable<UsingNamespaceDirectiveList >(model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->usingNamespaces());
          }

        default:
          break;
        }

      return  Nullable<UsingNamespaceDirectiveList>();
    }

  void _ModelItemChameleon::addUsingNamespace(UsingNamespaceDirectiveModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_NamespaceDeclaration:
        {
          model_static_cast<NamespaceDeclarationModelItem>(_M_item)->addUsingNamespace(item);
          return ;
        }

      case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
        {
          model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->addUsingNamespace(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  void _ModelItemChameleon::removeUsingNamespace(UsingNamespaceDirectiveModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_NamespaceDeclaration:
        {
          model_static_cast<NamespaceDeclarationModelItem>(_M_item)->removeUsingNamespace(item);
          return ;
        }

      case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
        {
          model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->removeUsingNamespace(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }


  Nullable<ClassDeclarationList > _ModelItemChameleon::classes( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_NamespaceDeclaration:
          {
            return  Nullable<ClassDeclarationList >(model_static_cast<NamespaceDeclarationModelItem>(_M_item)->classes());
          }

        case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
          {
            return  Nullable<ClassDeclarationList >(model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->classes());
          }

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<ClassDeclarationList >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->classes());
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<ClassDeclarationList >(model_static_cast<ClassDeclarationModelItem>(_M_item)->classes());
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<ClassDeclarationList >(model_static_cast<StructDeclarationModelItem>(_M_item)->classes());
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<ClassDeclarationList >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->classes());
          }

        default:
          break;
        }

      return  Nullable<ClassDeclarationList>();
    }

  void _ModelItemChameleon::addClass(ClassDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_NamespaceDeclaration:
        {
          model_static_cast<NamespaceDeclarationModelItem>(_M_item)->addClass(item);
          return ;
        }

      case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
        {
          model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->addClass(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->addClass(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->addClass(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->addClass(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->addClass(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  void _ModelItemChameleon::removeClass(ClassDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_NamespaceDeclaration:
        {
          model_static_cast<NamespaceDeclarationModelItem>(_M_item)->removeClass(item);
          return ;
        }

      case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
        {
          model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->removeClass(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->removeClass(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->removeClass(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->removeClass(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->removeClass(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  Nullable<ClassDeclarationModelItem > _ModelItemChameleon::findClass(const QString & name) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_NamespaceDeclaration:
          {
            return  Nullable<ClassDeclarationModelItem >(model_static_cast<NamespaceDeclarationModelItem>(_M_item)->findClass(name));
          }

        case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
          {
            return  Nullable<ClassDeclarationModelItem >(model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->findClass(name));
          }

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<ClassDeclarationModelItem >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->findClass(name));
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<ClassDeclarationModelItem >(model_static_cast<ClassDeclarationModelItem>(_M_item)->findClass(name));
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<ClassDeclarationModelItem >(model_static_cast<StructDeclarationModelItem>(_M_item)->findClass(name));
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<ClassDeclarationModelItem >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->findClass(name));
          }

        default:
          break;
        }

      return  Nullable<ClassDeclarationModelItem>();
    }


  Nullable<StructDeclarationList > _ModelItemChameleon::structs( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_NamespaceDeclaration:
          {
            return  Nullable<StructDeclarationList >(model_static_cast<NamespaceDeclarationModelItem>(_M_item)->structs());
          }

        case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
          {
            return  Nullable<StructDeclarationList >(model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->structs());
          }

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<StructDeclarationList >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->structs());
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<StructDeclarationList >(model_static_cast<ClassDeclarationModelItem>(_M_item)->structs());
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<StructDeclarationList >(model_static_cast<StructDeclarationModelItem>(_M_item)->structs());
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<StructDeclarationList >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->structs());
          }

        default:
          break;
        }

      return  Nullable<StructDeclarationList>();
    }

  void _ModelItemChameleon::addStruct(StructDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_NamespaceDeclaration:
        {
          model_static_cast<NamespaceDeclarationModelItem>(_M_item)->addStruct(item);
          return ;
        }

      case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
        {
          model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->addStruct(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->addStruct(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->addStruct(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->addStruct(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->addStruct(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  void _ModelItemChameleon::removeStruct(StructDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_NamespaceDeclaration:
        {
          model_static_cast<NamespaceDeclarationModelItem>(_M_item)->removeStruct(item);
          return ;
        }

      case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
        {
          model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->removeStruct(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->removeStruct(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->removeStruct(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->removeStruct(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->removeStruct(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  Nullable<StructDeclarationModelItem > _ModelItemChameleon::findStruct(const QString & name) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_NamespaceDeclaration:
          {
            return  Nullable<StructDeclarationModelItem >(model_static_cast<NamespaceDeclarationModelItem>(_M_item)->findStruct(name));
          }

        case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
          {
            return  Nullable<StructDeclarationModelItem >(model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->findStruct(name));
          }

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<StructDeclarationModelItem >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->findStruct(name));
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<StructDeclarationModelItem >(model_static_cast<ClassDeclarationModelItem>(_M_item)->findStruct(name));
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<StructDeclarationModelItem >(model_static_cast<StructDeclarationModelItem>(_M_item)->findStruct(name));
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<StructDeclarationModelItem >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->findStruct(name));
          }

        default:
          break;
        }

      return  Nullable<StructDeclarationModelItem>();
    }


  Nullable<InterfaceDeclarationList > _ModelItemChameleon::interfaces( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_NamespaceDeclaration:
          {
            return  Nullable<InterfaceDeclarationList >(model_static_cast<NamespaceDeclarationModelItem>(_M_item)->interfaces());
          }

        case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
          {
            return  Nullable<InterfaceDeclarationList >(model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->interfaces());
          }

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<InterfaceDeclarationList >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->interfaces());
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<InterfaceDeclarationList >(model_static_cast<ClassDeclarationModelItem>(_M_item)->interfaces());
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<InterfaceDeclarationList >(model_static_cast<StructDeclarationModelItem>(_M_item)->interfaces());
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<InterfaceDeclarationList >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->interfaces());
          }

        default:
          break;
        }

      return  Nullable<InterfaceDeclarationList>();
    }

  void _ModelItemChameleon::addInterface(InterfaceDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_NamespaceDeclaration:
        {
          model_static_cast<NamespaceDeclarationModelItem>(_M_item)->addInterface(item);
          return ;
        }

      case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
        {
          model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->addInterface(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->addInterface(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->addInterface(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->addInterface(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->addInterface(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  void _ModelItemChameleon::removeInterface(InterfaceDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_NamespaceDeclaration:
        {
          model_static_cast<NamespaceDeclarationModelItem>(_M_item)->removeInterface(item);
          return ;
        }

      case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
        {
          model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->removeInterface(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->removeInterface(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->removeInterface(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->removeInterface(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->removeInterface(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  Nullable<InterfaceDeclarationModelItem > _ModelItemChameleon::findInterface(const QString & name) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_NamespaceDeclaration:
          {
            return  Nullable<InterfaceDeclarationModelItem >(model_static_cast<NamespaceDeclarationModelItem>(_M_item)->findInterface(name));
          }

        case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
          {
            return  Nullable<InterfaceDeclarationModelItem >(model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->findInterface(name));
          }

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<InterfaceDeclarationModelItem >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->findInterface(name));
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<InterfaceDeclarationModelItem >(model_static_cast<ClassDeclarationModelItem>(_M_item)->findInterface(name));
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<InterfaceDeclarationModelItem >(model_static_cast<StructDeclarationModelItem>(_M_item)->findInterface(name));
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<InterfaceDeclarationModelItem >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->findInterface(name));
          }

        default:
          break;
        }

      return  Nullable<InterfaceDeclarationModelItem>();
    }


  Nullable<EnumDeclarationList > _ModelItemChameleon::enums( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_NamespaceDeclaration:
          {
            return  Nullable<EnumDeclarationList >(model_static_cast<NamespaceDeclarationModelItem>(_M_item)->enums());
          }

        case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
          {
            return  Nullable<EnumDeclarationList >(model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->enums());
          }

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<EnumDeclarationList >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->enums());
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<EnumDeclarationList >(model_static_cast<ClassDeclarationModelItem>(_M_item)->enums());
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<EnumDeclarationList >(model_static_cast<StructDeclarationModelItem>(_M_item)->enums());
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<EnumDeclarationList >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->enums());
          }

        default:
          break;
        }

      return  Nullable<EnumDeclarationList>();
    }

  void _ModelItemChameleon::addEnum(EnumDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_NamespaceDeclaration:
        {
          model_static_cast<NamespaceDeclarationModelItem>(_M_item)->addEnum(item);
          return ;
        }

      case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
        {
          model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->addEnum(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->addEnum(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->addEnum(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->addEnum(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->addEnum(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  void _ModelItemChameleon::removeEnum(EnumDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_NamespaceDeclaration:
        {
          model_static_cast<NamespaceDeclarationModelItem>(_M_item)->removeEnum(item);
          return ;
        }

      case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
        {
          model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->removeEnum(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->removeEnum(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->removeEnum(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->removeEnum(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->removeEnum(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  Nullable<EnumDeclarationModelItem > _ModelItemChameleon::findEnum(const QString & name) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_NamespaceDeclaration:
          {
            return  Nullable<EnumDeclarationModelItem >(model_static_cast<NamespaceDeclarationModelItem>(_M_item)->findEnum(name));
          }

        case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
          {
            return  Nullable<EnumDeclarationModelItem >(model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->findEnum(name));
          }

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<EnumDeclarationModelItem >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->findEnum(name));
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<EnumDeclarationModelItem >(model_static_cast<ClassDeclarationModelItem>(_M_item)->findEnum(name));
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<EnumDeclarationModelItem >(model_static_cast<StructDeclarationModelItem>(_M_item)->findEnum(name));
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<EnumDeclarationModelItem >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->findEnum(name));
          }

        default:
          break;
        }

      return  Nullable<EnumDeclarationModelItem>();
    }


  Nullable<DelegateDeclarationList > _ModelItemChameleon::delegates( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_NamespaceDeclaration:
          {
            return  Nullable<DelegateDeclarationList >(model_static_cast<NamespaceDeclarationModelItem>(_M_item)->delegates());
          }

        case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
          {
            return  Nullable<DelegateDeclarationList >(model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->delegates());
          }

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<DelegateDeclarationList >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->delegates());
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<DelegateDeclarationList >(model_static_cast<ClassDeclarationModelItem>(_M_item)->delegates());
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<DelegateDeclarationList >(model_static_cast<StructDeclarationModelItem>(_M_item)->delegates());
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<DelegateDeclarationList >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->delegates());
          }

        default:
          break;
        }

      return  Nullable<DelegateDeclarationList>();
    }

  void _ModelItemChameleon::addDelegate(DelegateDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_NamespaceDeclaration:
        {
          model_static_cast<NamespaceDeclarationModelItem>(_M_item)->addDelegate(item);
          return ;
        }

      case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
        {
          model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->addDelegate(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->addDelegate(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->addDelegate(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->addDelegate(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->addDelegate(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  void _ModelItemChameleon::removeDelegate(DelegateDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_NamespaceDeclaration:
        {
          model_static_cast<NamespaceDeclarationModelItem>(_M_item)->removeDelegate(item);
          return ;
        }

      case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
        {
          model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->removeDelegate(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->removeDelegate(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->removeDelegate(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->removeDelegate(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->removeDelegate(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  Nullable<DelegateDeclarationModelItem > _ModelItemChameleon::findDelegate(const QString & name) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_NamespaceDeclaration:
          {
            return  Nullable<DelegateDeclarationModelItem >(model_static_cast<NamespaceDeclarationModelItem>(_M_item)->findDelegate(name));
          }

        case _CodeModelItem::Kind_GlobalNamespaceDeclaration:
          {
            return  Nullable<DelegateDeclarationModelItem >(model_static_cast<GlobalNamespaceDeclarationModelItem>(_M_item)->findDelegate(name));
          }

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<DelegateDeclarationModelItem >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->findDelegate(name));
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<DelegateDeclarationModelItem >(model_static_cast<ClassDeclarationModelItem>(_M_item)->findDelegate(name));
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<DelegateDeclarationModelItem >(model_static_cast<StructDeclarationModelItem>(_M_item)->findDelegate(name));
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<DelegateDeclarationModelItem >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->findDelegate(name));
          }

        default:
          break;
        }

      return  Nullable<DelegateDeclarationModelItem>();
    }


  Nullable<TypeModelItem > _ModelItemChameleon::namespaceOrType( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_UsingAliasDirective:
          {
            return  Nullable<TypeModelItem >(model_static_cast<UsingAliasDirectiveModelItem>(_M_item)->namespaceOrType());
          }

        default:
          break;
        }

      return  Nullable<TypeModelItem>();
    }

  void _ModelItemChameleon::setNamespaceOrType(TypeModelItem namespaceOrType)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_UsingAliasDirective:
        {
          model_static_cast<UsingAliasDirectiveModelItem>(_M_item)->setNamespaceOrType(namespaceOrType);
          return ;
        }

      default:
        break;
      }

    return ;
  }


  Nullable<QStringList > _ModelItemChameleon::namespaceUsed( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_UsingNamespaceDirective:
          {
            return  Nullable<QStringList >(model_static_cast<UsingNamespaceDirectiveModelItem>(_M_item)->namespaceUsed());
          }

        default:
          break;
        }

      return  Nullable<QStringList>();
    }

  void _ModelItemChameleon::setNamespaceUsed(QStringList namespaceUsed)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_UsingNamespaceDirective:
        {
          model_static_cast<UsingNamespaceDirectiveModelItem>(_M_item)->setNamespaceUsed(namespaceUsed);
          return ;
        }

      default:
        break;
      }

    return ;
  }


  Nullable<access_policy::access_policy_enum > _ModelItemChameleon::accessPolicy( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_TypeDeclaration:
          {
            return  Nullable<access_policy::access_policy_enum >(model_static_cast<TypeDeclarationModelItem>(_M_item)->accessPolicy());
          }

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<access_policy::access_policy_enum >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->accessPolicy());
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<access_policy::access_policy_enum >(model_static_cast<ClassDeclarationModelItem>(_M_item)->accessPolicy());
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<access_policy::access_policy_enum >(model_static_cast<StructDeclarationModelItem>(_M_item)->accessPolicy());
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<access_policy::access_policy_enum >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->accessPolicy());
          }

        case _CodeModelItem::Kind_DelegateDeclaration:
          {
            return  Nullable<access_policy::access_policy_enum >(model_static_cast<DelegateDeclarationModelItem>(_M_item)->accessPolicy());
          }

        case _CodeModelItem::Kind_EnumDeclaration:
          {
            return  Nullable<access_policy::access_policy_enum >(model_static_cast<EnumDeclarationModelItem>(_M_item)->accessPolicy());
          }

        case _CodeModelItem::Kind_EventDeclaration:
          {
            return  Nullable<access_policy::access_policy_enum >(model_static_cast<EventDeclarationModelItem>(_M_item)->accessPolicy());
          }

        case _CodeModelItem::Kind_IndexerDeclaration:
          {
            return  Nullable<access_policy::access_policy_enum >(model_static_cast<IndexerDeclarationModelItem>(_M_item)->accessPolicy());
          }

        case _CodeModelItem::Kind_PropertyDeclaration:
          {
            return  Nullable<access_policy::access_policy_enum >(model_static_cast<PropertyDeclarationModelItem>(_M_item)->accessPolicy());
          }

        case _CodeModelItem::Kind_AccessorDeclaration:
          {
            return  Nullable<access_policy::access_policy_enum >(model_static_cast<AccessorDeclarationModelItem>(_M_item)->accessPolicy());
          }

        case _CodeModelItem::Kind_MethodDeclaration:
          {
            return  Nullable<access_policy::access_policy_enum >(model_static_cast<MethodDeclarationModelItem>(_M_item)->accessPolicy());
          }

        case _CodeModelItem::Kind_ConstructorDeclaration:
          {
            return  Nullable<access_policy::access_policy_enum >(model_static_cast<ConstructorDeclarationModelItem>(_M_item)->accessPolicy());
          }

        case _CodeModelItem::Kind_FinalizerDeclaration:
          {
            return  Nullable<access_policy::access_policy_enum >(model_static_cast<FinalizerDeclarationModelItem>(_M_item)->accessPolicy());
          }

        case _CodeModelItem::Kind_VariableDeclaration:
          {
            return  Nullable<access_policy::access_policy_enum >(model_static_cast<VariableDeclarationModelItem>(_M_item)->accessPolicy());
          }

        default:
          break;
        }

      return  Nullable<access_policy::access_policy_enum>();
    }

  void _ModelItemChameleon::setAccessPolicy(access_policy::access_policy_enum accessPolicy)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_TypeDeclaration:
        {
          model_static_cast<TypeDeclarationModelItem>(_M_item)->setAccessPolicy(accessPolicy);
          return ;
        }

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->setAccessPolicy(accessPolicy);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->setAccessPolicy(accessPolicy);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->setAccessPolicy(accessPolicy);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->setAccessPolicy(accessPolicy);
          return ;
        }

      case _CodeModelItem::Kind_DelegateDeclaration:
        {
          model_static_cast<DelegateDeclarationModelItem>(_M_item)->setAccessPolicy(accessPolicy);
          return ;
        }

      case _CodeModelItem::Kind_EnumDeclaration:
        {
          model_static_cast<EnumDeclarationModelItem>(_M_item)->setAccessPolicy(accessPolicy);
          return ;
        }

      case _CodeModelItem::Kind_EventDeclaration:
        {
          model_static_cast<EventDeclarationModelItem>(_M_item)->setAccessPolicy(accessPolicy);
          return ;
        }

      case _CodeModelItem::Kind_IndexerDeclaration:
        {
          model_static_cast<IndexerDeclarationModelItem>(_M_item)->setAccessPolicy(accessPolicy);
          return ;
        }

      case _CodeModelItem::Kind_PropertyDeclaration:
        {
          model_static_cast<PropertyDeclarationModelItem>(_M_item)->setAccessPolicy(accessPolicy);
          return ;
        }

      case _CodeModelItem::Kind_AccessorDeclaration:
        {
          model_static_cast<AccessorDeclarationModelItem>(_M_item)->setAccessPolicy(accessPolicy);
          return ;
        }

      case _CodeModelItem::Kind_MethodDeclaration:
        {
          model_static_cast<MethodDeclarationModelItem>(_M_item)->setAccessPolicy(accessPolicy);
          return ;
        }

      case _CodeModelItem::Kind_ConstructorDeclaration:
        {
          model_static_cast<ConstructorDeclarationModelItem>(_M_item)->setAccessPolicy(accessPolicy);
          return ;
        }

      case _CodeModelItem::Kind_FinalizerDeclaration:
        {
          model_static_cast<FinalizerDeclarationModelItem>(_M_item)->setAccessPolicy(accessPolicy);
          return ;
        }

      case _CodeModelItem::Kind_VariableDeclaration:
        {
          model_static_cast<VariableDeclarationModelItem>(_M_item)->setAccessPolicy(accessPolicy);
          return ;
        }

      default:
        break;
      }

    return ;
  }


  Nullable<bool > _ModelItemChameleon::isNew( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_TypeDeclaration:
          {
            return  Nullable<bool >(model_static_cast<TypeDeclarationModelItem>(_M_item)->isNew());
          }

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<bool >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->isNew());
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<bool >(model_static_cast<ClassDeclarationModelItem>(_M_item)->isNew());
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<bool >(model_static_cast<StructDeclarationModelItem>(_M_item)->isNew());
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<bool >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->isNew());
          }

        case _CodeModelItem::Kind_DelegateDeclaration:
          {
            return  Nullable<bool >(model_static_cast<DelegateDeclarationModelItem>(_M_item)->isNew());
          }

        case _CodeModelItem::Kind_EnumDeclaration:
          {
            return  Nullable<bool >(model_static_cast<EnumDeclarationModelItem>(_M_item)->isNew());
          }

        case _CodeModelItem::Kind_EventDeclaration:
          {
            return  Nullable<bool >(model_static_cast<EventDeclarationModelItem>(_M_item)->isNew());
          }

        case _CodeModelItem::Kind_IndexerDeclaration:
          {
            return  Nullable<bool >(model_static_cast<IndexerDeclarationModelItem>(_M_item)->isNew());
          }

        case _CodeModelItem::Kind_PropertyDeclaration:
          {
            return  Nullable<bool >(model_static_cast<PropertyDeclarationModelItem>(_M_item)->isNew());
          }

        case _CodeModelItem::Kind_MethodDeclaration:
          {
            return  Nullable<bool >(model_static_cast<MethodDeclarationModelItem>(_M_item)->isNew());
          }

        case _CodeModelItem::Kind_ConstructorDeclaration:
          {
            return  Nullable<bool >(model_static_cast<ConstructorDeclarationModelItem>(_M_item)->isNew());
          }

        case _CodeModelItem::Kind_FinalizerDeclaration:
          {
            return  Nullable<bool >(model_static_cast<FinalizerDeclarationModelItem>(_M_item)->isNew());
          }

        case _CodeModelItem::Kind_VariableDeclaration:
          {
            return  Nullable<bool >(model_static_cast<VariableDeclarationModelItem>(_M_item)->isNew());
          }

        default:
          break;
        }

      return  Nullable<bool>();
    }

  void _ModelItemChameleon::setNew(bool isNew)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_TypeDeclaration:
        {
          model_static_cast<TypeDeclarationModelItem>(_M_item)->setNew(isNew);
          return ;
        }

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->setNew(isNew);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->setNew(isNew);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->setNew(isNew);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->setNew(isNew);
          return ;
        }

      case _CodeModelItem::Kind_DelegateDeclaration:
        {
          model_static_cast<DelegateDeclarationModelItem>(_M_item)->setNew(isNew);
          return ;
        }

      case _CodeModelItem::Kind_EnumDeclaration:
        {
          model_static_cast<EnumDeclarationModelItem>(_M_item)->setNew(isNew);
          return ;
        }

      case _CodeModelItem::Kind_EventDeclaration:
        {
          model_static_cast<EventDeclarationModelItem>(_M_item)->setNew(isNew);
          return ;
        }

      case _CodeModelItem::Kind_IndexerDeclaration:
        {
          model_static_cast<IndexerDeclarationModelItem>(_M_item)->setNew(isNew);
          return ;
        }

      case _CodeModelItem::Kind_PropertyDeclaration:
        {
          model_static_cast<PropertyDeclarationModelItem>(_M_item)->setNew(isNew);
          return ;
        }

      case _CodeModelItem::Kind_MethodDeclaration:
        {
          model_static_cast<MethodDeclarationModelItem>(_M_item)->setNew(isNew);
          return ;
        }

      case _CodeModelItem::Kind_ConstructorDeclaration:
        {
          model_static_cast<ConstructorDeclarationModelItem>(_M_item)->setNew(isNew);
          return ;
        }

      case _CodeModelItem::Kind_FinalizerDeclaration:
        {
          model_static_cast<FinalizerDeclarationModelItem>(_M_item)->setNew(isNew);
          return ;
        }

      case _CodeModelItem::Kind_VariableDeclaration:
        {
          model_static_cast<VariableDeclarationModelItem>(_M_item)->setNew(isNew);
          return ;
        }

      default:
        break;
      }

    return ;
  }


  Nullable<TypeModelItem > _ModelItemChameleon::baseType( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<TypeModelItem >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->baseType());
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<TypeModelItem >(model_static_cast<ClassDeclarationModelItem>(_M_item)->baseType());
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<TypeModelItem >(model_static_cast<StructDeclarationModelItem>(_M_item)->baseType());
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<TypeModelItem >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->baseType());
          }

        default:
          break;
        }

      return  Nullable<TypeModelItem>();
    }

  void _ModelItemChameleon::setBaseType(TypeModelItem baseType)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->setBaseType(baseType);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->setBaseType(baseType);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->setBaseType(baseType);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->setBaseType(baseType);
          return ;
        }

      default:
        break;
      }

    return ;
  }


  Nullable<EventDeclarationList > _ModelItemChameleon::events( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<EventDeclarationList >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->events());
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<EventDeclarationList >(model_static_cast<ClassDeclarationModelItem>(_M_item)->events());
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<EventDeclarationList >(model_static_cast<StructDeclarationModelItem>(_M_item)->events());
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<EventDeclarationList >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->events());
          }

        default:
          break;
        }

      return  Nullable<EventDeclarationList>();
    }

  void _ModelItemChameleon::addEvent(EventDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->addEvent(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->addEvent(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->addEvent(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->addEvent(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  void _ModelItemChameleon::removeEvent(EventDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->removeEvent(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->removeEvent(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->removeEvent(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->removeEvent(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  Nullable<EventDeclarationModelItem > _ModelItemChameleon::findEvent(const QString & name) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<EventDeclarationModelItem >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->findEvent(name));
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<EventDeclarationModelItem >(model_static_cast<ClassDeclarationModelItem>(_M_item)->findEvent(name));
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<EventDeclarationModelItem >(model_static_cast<StructDeclarationModelItem>(_M_item)->findEvent(name));
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<EventDeclarationModelItem >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->findEvent(name));
          }

        default:
          break;
        }

      return  Nullable<EventDeclarationModelItem>();
    }


  Nullable<IndexerDeclarationList > _ModelItemChameleon::indexers( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<IndexerDeclarationList >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->indexers());
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<IndexerDeclarationList >(model_static_cast<ClassDeclarationModelItem>(_M_item)->indexers());
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<IndexerDeclarationList >(model_static_cast<StructDeclarationModelItem>(_M_item)->indexers());
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<IndexerDeclarationList >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->indexers());
          }

        default:
          break;
        }

      return  Nullable<IndexerDeclarationList>();
    }

  void _ModelItemChameleon::addIndexer(IndexerDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->addIndexer(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->addIndexer(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->addIndexer(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->addIndexer(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  void _ModelItemChameleon::removeIndexer(IndexerDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->removeIndexer(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->removeIndexer(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->removeIndexer(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->removeIndexer(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }


  Nullable<PropertyDeclarationList > _ModelItemChameleon::propertys( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<PropertyDeclarationList >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->propertys());
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<PropertyDeclarationList >(model_static_cast<ClassDeclarationModelItem>(_M_item)->propertys());
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<PropertyDeclarationList >(model_static_cast<StructDeclarationModelItem>(_M_item)->propertys());
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<PropertyDeclarationList >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->propertys());
          }

        default:
          break;
        }

      return  Nullable<PropertyDeclarationList>();
    }

  void _ModelItemChameleon::addProperty(PropertyDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->addProperty(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->addProperty(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->addProperty(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->addProperty(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  void _ModelItemChameleon::removeProperty(PropertyDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->removeProperty(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->removeProperty(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->removeProperty(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->removeProperty(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  Nullable<PropertyDeclarationModelItem > _ModelItemChameleon::findProperty(const QString & name) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<PropertyDeclarationModelItem >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->findProperty(name));
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<PropertyDeclarationModelItem >(model_static_cast<ClassDeclarationModelItem>(_M_item)->findProperty(name));
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<PropertyDeclarationModelItem >(model_static_cast<StructDeclarationModelItem>(_M_item)->findProperty(name));
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<PropertyDeclarationModelItem >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->findProperty(name));
          }

        default:
          break;
        }

      return  Nullable<PropertyDeclarationModelItem>();
    }


  Nullable<MethodDeclarationList > _ModelItemChameleon::methods( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<MethodDeclarationList >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->methods());
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<MethodDeclarationList >(model_static_cast<ClassDeclarationModelItem>(_M_item)->methods());
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<MethodDeclarationList >(model_static_cast<StructDeclarationModelItem>(_M_item)->methods());
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<MethodDeclarationList >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->methods());
          }

        default:
          break;
        }

      return  Nullable<MethodDeclarationList>();
    }

  void _ModelItemChameleon::addMethod(MethodDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->addMethod(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->addMethod(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->addMethod(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->addMethod(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  void _ModelItemChameleon::removeMethod(MethodDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->removeMethod(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->removeMethod(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->removeMethod(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->removeMethod(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  Nullable<MethodDeclarationList > _ModelItemChameleon::findMethods(const QString & name) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<MethodDeclarationList >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->findMethods(name));
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<MethodDeclarationList >(model_static_cast<ClassDeclarationModelItem>(_M_item)->findMethods(name));
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<MethodDeclarationList >(model_static_cast<StructDeclarationModelItem>(_M_item)->findMethods(name));
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<MethodDeclarationList >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->findMethods(name));
          }

        default:
          break;
        }

      return  Nullable<MethodDeclarationList>();
    }


  Nullable<TypeParameterList > _ModelItemChameleon::typeParameters( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<TypeParameterList >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->typeParameters());
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<TypeParameterList >(model_static_cast<ClassDeclarationModelItem>(_M_item)->typeParameters());
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<TypeParameterList >(model_static_cast<StructDeclarationModelItem>(_M_item)->typeParameters());
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<TypeParameterList >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->typeParameters());
          }

        case _CodeModelItem::Kind_DelegateDeclaration:
          {
            return  Nullable<TypeParameterList >(model_static_cast<DelegateDeclarationModelItem>(_M_item)->typeParameters());
          }

        case _CodeModelItem::Kind_MethodDeclaration:
          {
            return  Nullable<TypeParameterList >(model_static_cast<MethodDeclarationModelItem>(_M_item)->typeParameters());
          }

        case _CodeModelItem::Kind_ConstructorDeclaration:
          {
            return  Nullable<TypeParameterList >(model_static_cast<ConstructorDeclarationModelItem>(_M_item)->typeParameters());
          }

        case _CodeModelItem::Kind_FinalizerDeclaration:
          {
            return  Nullable<TypeParameterList >(model_static_cast<FinalizerDeclarationModelItem>(_M_item)->typeParameters());
          }

        default:
          break;
        }

      return  Nullable<TypeParameterList>();
    }

  void _ModelItemChameleon::addTypeParameter(TypeParameterModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->addTypeParameter(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->addTypeParameter(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->addTypeParameter(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->addTypeParameter(item);
          return ;
        }

      case _CodeModelItem::Kind_DelegateDeclaration:
        {
          model_static_cast<DelegateDeclarationModelItem>(_M_item)->addTypeParameter(item);
          return ;
        }

      case _CodeModelItem::Kind_MethodDeclaration:
        {
          model_static_cast<MethodDeclarationModelItem>(_M_item)->addTypeParameter(item);
          return ;
        }

      case _CodeModelItem::Kind_ConstructorDeclaration:
        {
          model_static_cast<ConstructorDeclarationModelItem>(_M_item)->addTypeParameter(item);
          return ;
        }

      case _CodeModelItem::Kind_FinalizerDeclaration:
        {
          model_static_cast<FinalizerDeclarationModelItem>(_M_item)->addTypeParameter(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  void _ModelItemChameleon::removeTypeParameter(TypeParameterModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->removeTypeParameter(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->removeTypeParameter(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->removeTypeParameter(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->removeTypeParameter(item);
          return ;
        }

      case _CodeModelItem::Kind_DelegateDeclaration:
        {
          model_static_cast<DelegateDeclarationModelItem>(_M_item)->removeTypeParameter(item);
          return ;
        }

      case _CodeModelItem::Kind_MethodDeclaration:
        {
          model_static_cast<MethodDeclarationModelItem>(_M_item)->removeTypeParameter(item);
          return ;
        }

      case _CodeModelItem::Kind_ConstructorDeclaration:
        {
          model_static_cast<ConstructorDeclarationModelItem>(_M_item)->removeTypeParameter(item);
          return ;
        }

      case _CodeModelItem::Kind_FinalizerDeclaration:
        {
          model_static_cast<FinalizerDeclarationModelItem>(_M_item)->removeTypeParameter(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }


  Nullable<TypeParameterConstraintList > _ModelItemChameleon::typeParameterConstraints( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<TypeParameterConstraintList >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->typeParameterConstraints());
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<TypeParameterConstraintList >(model_static_cast<ClassDeclarationModelItem>(_M_item)->typeParameterConstraints());
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<TypeParameterConstraintList >(model_static_cast<StructDeclarationModelItem>(_M_item)->typeParameterConstraints());
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<TypeParameterConstraintList >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->typeParameterConstraints());
          }

        case _CodeModelItem::Kind_DelegateDeclaration:
          {
            return  Nullable<TypeParameterConstraintList >(model_static_cast<DelegateDeclarationModelItem>(_M_item)->typeParameterConstraints());
          }

        case _CodeModelItem::Kind_MethodDeclaration:
          {
            return  Nullable<TypeParameterConstraintList >(model_static_cast<MethodDeclarationModelItem>(_M_item)->typeParameterConstraints());
          }

        case _CodeModelItem::Kind_ConstructorDeclaration:
          {
            return  Nullable<TypeParameterConstraintList >(model_static_cast<ConstructorDeclarationModelItem>(_M_item)->typeParameterConstraints());
          }

        case _CodeModelItem::Kind_FinalizerDeclaration:
          {
            return  Nullable<TypeParameterConstraintList >(model_static_cast<FinalizerDeclarationModelItem>(_M_item)->typeParameterConstraints());
          }

        default:
          break;
        }

      return  Nullable<TypeParameterConstraintList>();
    }

  void _ModelItemChameleon::addTypeParameterConstraint(TypeParameterConstraintModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->addTypeParameterConstraint(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->addTypeParameterConstraint(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->addTypeParameterConstraint(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->addTypeParameterConstraint(item);
          return ;
        }

      case _CodeModelItem::Kind_DelegateDeclaration:
        {
          model_static_cast<DelegateDeclarationModelItem>(_M_item)->addTypeParameterConstraint(item);
          return ;
        }

      case _CodeModelItem::Kind_MethodDeclaration:
        {
          model_static_cast<MethodDeclarationModelItem>(_M_item)->addTypeParameterConstraint(item);
          return ;
        }

      case _CodeModelItem::Kind_ConstructorDeclaration:
        {
          model_static_cast<ConstructorDeclarationModelItem>(_M_item)->addTypeParameterConstraint(item);
          return ;
        }

      case _CodeModelItem::Kind_FinalizerDeclaration:
        {
          model_static_cast<FinalizerDeclarationModelItem>(_M_item)->addTypeParameterConstraint(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  void _ModelItemChameleon::removeTypeParameterConstraint(TypeParameterConstraintModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->removeTypeParameterConstraint(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->removeTypeParameterConstraint(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->removeTypeParameterConstraint(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->removeTypeParameterConstraint(item);
          return ;
        }

      case _CodeModelItem::Kind_DelegateDeclaration:
        {
          model_static_cast<DelegateDeclarationModelItem>(_M_item)->removeTypeParameterConstraint(item);
          return ;
        }

      case _CodeModelItem::Kind_MethodDeclaration:
        {
          model_static_cast<MethodDeclarationModelItem>(_M_item)->removeTypeParameterConstraint(item);
          return ;
        }

      case _CodeModelItem::Kind_ConstructorDeclaration:
        {
          model_static_cast<ConstructorDeclarationModelItem>(_M_item)->removeTypeParameterConstraint(item);
          return ;
        }

      case _CodeModelItem::Kind_FinalizerDeclaration:
        {
          model_static_cast<FinalizerDeclarationModelItem>(_M_item)->removeTypeParameterConstraint(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }


  Nullable<bool > _ModelItemChameleon::isUnsafe( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<bool >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->isUnsafe());
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<bool >(model_static_cast<ClassDeclarationModelItem>(_M_item)->isUnsafe());
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<bool >(model_static_cast<StructDeclarationModelItem>(_M_item)->isUnsafe());
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<bool >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->isUnsafe());
          }

        case _CodeModelItem::Kind_DelegateDeclaration:
          {
            return  Nullable<bool >(model_static_cast<DelegateDeclarationModelItem>(_M_item)->isUnsafe());
          }

        case _CodeModelItem::Kind_EventDeclaration:
          {
            return  Nullable<bool >(model_static_cast<EventDeclarationModelItem>(_M_item)->isUnsafe());
          }

        case _CodeModelItem::Kind_IndexerDeclaration:
          {
            return  Nullable<bool >(model_static_cast<IndexerDeclarationModelItem>(_M_item)->isUnsafe());
          }

        case _CodeModelItem::Kind_PropertyDeclaration:
          {
            return  Nullable<bool >(model_static_cast<PropertyDeclarationModelItem>(_M_item)->isUnsafe());
          }

        case _CodeModelItem::Kind_MethodDeclaration:
          {
            return  Nullable<bool >(model_static_cast<MethodDeclarationModelItem>(_M_item)->isUnsafe());
          }

        case _CodeModelItem::Kind_ConstructorDeclaration:
          {
            return  Nullable<bool >(model_static_cast<ConstructorDeclarationModelItem>(_M_item)->isUnsafe());
          }

        case _CodeModelItem::Kind_FinalizerDeclaration:
          {
            return  Nullable<bool >(model_static_cast<FinalizerDeclarationModelItem>(_M_item)->isUnsafe());
          }

        case _CodeModelItem::Kind_VariableDeclaration:
          {
            return  Nullable<bool >(model_static_cast<VariableDeclarationModelItem>(_M_item)->isUnsafe());
          }

        default:
          break;
        }

      return  Nullable<bool>();
    }

  void _ModelItemChameleon::setUnsafe(bool isUnsafe)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->setUnsafe(isUnsafe);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->setUnsafe(isUnsafe);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->setUnsafe(isUnsafe);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->setUnsafe(isUnsafe);
          return ;
        }

      case _CodeModelItem::Kind_DelegateDeclaration:
        {
          model_static_cast<DelegateDeclarationModelItem>(_M_item)->setUnsafe(isUnsafe);
          return ;
        }

      case _CodeModelItem::Kind_EventDeclaration:
        {
          model_static_cast<EventDeclarationModelItem>(_M_item)->setUnsafe(isUnsafe);
          return ;
        }

      case _CodeModelItem::Kind_IndexerDeclaration:
        {
          model_static_cast<IndexerDeclarationModelItem>(_M_item)->setUnsafe(isUnsafe);
          return ;
        }

      case _CodeModelItem::Kind_PropertyDeclaration:
        {
          model_static_cast<PropertyDeclarationModelItem>(_M_item)->setUnsafe(isUnsafe);
          return ;
        }

      case _CodeModelItem::Kind_MethodDeclaration:
        {
          model_static_cast<MethodDeclarationModelItem>(_M_item)->setUnsafe(isUnsafe);
          return ;
        }

      case _CodeModelItem::Kind_ConstructorDeclaration:
        {
          model_static_cast<ConstructorDeclarationModelItem>(_M_item)->setUnsafe(isUnsafe);
          return ;
        }

      case _CodeModelItem::Kind_FinalizerDeclaration:
        {
          model_static_cast<FinalizerDeclarationModelItem>(_M_item)->setUnsafe(isUnsafe);
          return ;
        }

      case _CodeModelItem::Kind_VariableDeclaration:
        {
          model_static_cast<VariableDeclarationModelItem>(_M_item)->setUnsafe(isUnsafe);
          return ;
        }

      default:
        break;
      }

    return ;
  }


  Nullable<VariableDeclarationList > _ModelItemChameleon::variables( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<VariableDeclarationList >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->variables());
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<VariableDeclarationList >(model_static_cast<ClassDeclarationModelItem>(_M_item)->variables());
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<VariableDeclarationList >(model_static_cast<StructDeclarationModelItem>(_M_item)->variables());
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<VariableDeclarationList >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->variables());
          }

        default:
          break;
        }

      return  Nullable<VariableDeclarationList>();
    }

  void _ModelItemChameleon::addVariable(VariableDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->addVariable(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->addVariable(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->addVariable(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->addVariable(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  void _ModelItemChameleon::removeVariable(VariableDeclarationModelItem item)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_ClassLikeDeclaration:
        {
          model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->removeVariable(item);
          return ;
        }

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->removeVariable(item);
          return ;
        }

      case _CodeModelItem::Kind_StructDeclaration:
        {
          model_static_cast<StructDeclarationModelItem>(_M_item)->removeVariable(item);
          return ;
        }

      case _CodeModelItem::Kind_InterfaceDeclaration:
        {
          model_static_cast<InterfaceDeclarationModelItem>(_M_item)->removeVariable(item);
          return ;
        }

      default:
        break;
      }

    return ;
  }

  Nullable<VariableDeclarationModelItem > _ModelItemChameleon::findVariable(const QString & name) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_ClassLikeDeclaration:
          {
            return  Nullable<VariableDeclarationModelItem >(model_static_cast<ClassLikeDeclarationModelItem>(_M_item)->findVariable(name));
          }

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<VariableDeclarationModelItem >(model_static_cast<ClassDeclarationModelItem>(_M_item)->findVariable(name));
          }

        case _CodeModelItem::Kind_StructDeclaration:
          {
            return  Nullable<VariableDeclarationModelItem >(model_static_cast<StructDeclarationModelItem>(_M_item)->findVariable(name));
          }

        case _CodeModelItem::Kind_InterfaceDeclaration:
          {
            return  Nullable<VariableDeclarationModelItem >(model_static_cast<InterfaceDeclarationModelItem>(_M_item)->findVariable(name));
          }

        default:
          break;
        }

      return  Nullable<VariableDeclarationModelItem>();
    }


  Nullable<bool > _ModelItemChameleon::isSealed( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<bool >(model_static_cast<ClassDeclarationModelItem>(_M_item)->isSealed());
          }

        case _CodeModelItem::Kind_EventDeclaration:
          {
            return  Nullable<bool >(model_static_cast<EventDeclarationModelItem>(_M_item)->isSealed());
          }

        case _CodeModelItem::Kind_IndexerDeclaration:
          {
            return  Nullable<bool >(model_static_cast<IndexerDeclarationModelItem>(_M_item)->isSealed());
          }

        case _CodeModelItem::Kind_PropertyDeclaration:
          {
            return  Nullable<bool >(model_static_cast<PropertyDeclarationModelItem>(_M_item)->isSealed());
          }

        case _CodeModelItem::Kind_MethodDeclaration:
          {
            return  Nullable<bool >(model_static_cast<MethodDeclarationModelItem>(_M_item)->isSealed());
          }

        case _CodeModelItem::Kind_ConstructorDeclaration:
          {
            return  Nullable<bool >(model_static_cast<ConstructorDeclarationModelItem>(_M_item)->isSealed());
          }

        case _CodeModelItem::Kind_FinalizerDeclaration:
          {
            return  Nullable<bool >(model_static_cast<FinalizerDeclarationModelItem>(_M_item)->isSealed());
          }

        default:
          break;
        }

      return  Nullable<bool>();
    }

  void _ModelItemChameleon::setSealed(bool isSealed)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->setSealed(isSealed);
          return ;
        }

      case _CodeModelItem::Kind_EventDeclaration:
        {
          model_static_cast<EventDeclarationModelItem>(_M_item)->setSealed(isSealed);
          return ;
        }

      case _CodeModelItem::Kind_IndexerDeclaration:
        {
          model_static_cast<IndexerDeclarationModelItem>(_M_item)->setSealed(isSealed);
          return ;
        }

      case _CodeModelItem::Kind_PropertyDeclaration:
        {
          model_static_cast<PropertyDeclarationModelItem>(_M_item)->setSealed(isSealed);
          return ;
        }

      case _CodeModelItem::Kind_MethodDeclaration:
        {
          model_static_cast<MethodDeclarationModelItem>(_M_item)->setSealed(isSealed);
          return ;
        }

      case _CodeModelItem::Kind_ConstructorDeclaration:
        {
          model_static_cast<ConstructorDeclarationModelItem>(_M_item)->setSealed(isSealed);
          return ;
        }

      case _CodeModelItem::Kind_FinalizerDeclaration:
        {
          model_static_cast<FinalizerDeclarationModelItem>(_M_item)->setSealed(isSealed);
          return ;
        }

      default:
        break;
      }

    return ;
  }


  Nullable<bool > _ModelItemChameleon::isAbstract( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<bool >(model_static_cast<ClassDeclarationModelItem>(_M_item)->isAbstract());
          }

        case _CodeModelItem::Kind_EventDeclaration:
          {
            return  Nullable<bool >(model_static_cast<EventDeclarationModelItem>(_M_item)->isAbstract());
          }

        case _CodeModelItem::Kind_IndexerDeclaration:
          {
            return  Nullable<bool >(model_static_cast<IndexerDeclarationModelItem>(_M_item)->isAbstract());
          }

        case _CodeModelItem::Kind_PropertyDeclaration:
          {
            return  Nullable<bool >(model_static_cast<PropertyDeclarationModelItem>(_M_item)->isAbstract());
          }

        case _CodeModelItem::Kind_MethodDeclaration:
          {
            return  Nullable<bool >(model_static_cast<MethodDeclarationModelItem>(_M_item)->isAbstract());
          }

        case _CodeModelItem::Kind_ConstructorDeclaration:
          {
            return  Nullable<bool >(model_static_cast<ConstructorDeclarationModelItem>(_M_item)->isAbstract());
          }

        case _CodeModelItem::Kind_FinalizerDeclaration:
          {
            return  Nullable<bool >(model_static_cast<FinalizerDeclarationModelItem>(_M_item)->isAbstract());
          }

        default:
          break;
        }

      return  Nullable<bool>();
    }

  void _ModelItemChameleon::setAbstract(bool isAbstract)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->setAbstract(isAbstract);
          return ;
        }

      case _CodeModelItem::Kind_EventDeclaration:
        {
          model_static_cast<EventDeclarationModelItem>(_M_item)->setAbstract(isAbstract);
          return ;
        }

      case _CodeModelItem::Kind_IndexerDeclaration:
        {
          model_static_cast<IndexerDeclarationModelItem>(_M_item)->setAbstract(isAbstract);
          return ;
        }

      case _CodeModelItem::Kind_PropertyDeclaration:
        {
          model_static_cast<PropertyDeclarationModelItem>(_M_item)->setAbstract(isAbstract);
          return ;
        }

      case _CodeModelItem::Kind_MethodDeclaration:
        {
          model_static_cast<MethodDeclarationModelItem>(_M_item)->setAbstract(isAbstract);
          return ;
        }

      case _CodeModelItem::Kind_ConstructorDeclaration:
        {
          model_static_cast<ConstructorDeclarationModelItem>(_M_item)->setAbstract(isAbstract);
          return ;
        }

      case _CodeModelItem::Kind_FinalizerDeclaration:
        {
          model_static_cast<FinalizerDeclarationModelItem>(_M_item)->setAbstract(isAbstract);
          return ;
        }

      default:
        break;
      }

    return ;
  }


  Nullable<bool > _ModelItemChameleon::isStatic( ) const
    {
      switch  (_M_item->kind())
        {

        case _CodeModelItem::Kind_ClassDeclaration:
          {
            return  Nullable<bool >(model_static_cast<ClassDeclarationModelItem>(_M_item)->isStatic());
          }

        case _CodeModelItem::Kind_EventDeclaration:
          {
            return  Nullable<bool >(model_static_cast<EventDeclarationModelItem>(_M_item)->isStatic());
          }

        case _CodeModelItem::Kind_PropertyDeclaration:
          {
            return  Nullable<bool >(model_static_cast<PropertyDeclarationModelItem>(_M_item)->isStatic());
          }

        case _CodeModelItem::Kind_MethodDeclaration:
          {
            return  Nullable<bool >(model_static_cast<MethodDeclarationModelItem>(_M_item)->isStatic());
          }

        case _CodeModelItem::Kind_ConstructorDeclaration:
          {
            return  Nullable<bool >(model_static_cast<ConstructorDeclarationModelItem>(_M_item)->isStatic());
          }

        case _CodeModelItem::Kind_FinalizerDeclaration:
          {
            return  Nullable<bool >(model_static_cast<FinalizerDeclarationModelItem>(_M_item)->isStatic());
          }

        case _CodeModelItem::Kind_VariableDeclaration:
          {
            return  Nullable<bool >(model_static_cast<VariableDeclarationModelItem>(_M_item)->isStatic());
          }

        default:
          break;
        }

      return  Nullable<bool>();
    }

  void _ModelItemChameleon::setStatic(bool isStatic)
  {
    switch  (_M_item->kind())
      {

      case _CodeModelItem::Kind_ClassDeclaration:
        {
          model_static_cast<ClassDeclarationModelItem>(_M_item)->setStatic(isStatic);
          return ;
        }

      case _CodeModelItem::Kind_EventDeclaration:
        {
          model_static_cast<EventDeclarationModelItem>(_M_item)->setStatic(isStatic);
          return ;
        }

      case _CodeModelItem::Kind_PropertyDeclaration:
        {
          model_static_cast<PropertyDeclarationModelItem>(_M_item)->setStatic(isStatic);
          return ;
        }

      case _CodeModelItem::Kind_MethodDeclaration:
        {
          model_static_cast<MethodDeclarationModelItem>(_M_item)->setStatic(isStatic);
          return ;
        }

      case _CodeModelItem::Kind_ConstructorDeclaration:
        {
          model_static_cast<ConstructorDeclarationModelItem>(_M_item)->setStatic(isStatic);
          return ;
        }

      case _CodeModelItem::Kind_FinalizerDeclaration:
        {
          model_static_cast<FinalizerDeclarationModelItem>(_M_item)->setStatic(isStatic);
          return ;
        }

      case _CodeModelItem::Kind_VariableDeclaration:
        {
          model_static_cast<VariableDeclarationModelItem>(_M_item)->setStatic(isStatic);
          return ;
        }

      default:
        break;
      }

    return ;
  }


  Nullable<TypeModelItem > _ModelItemChameleon::returnType( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_DelegateDeclaration:
              {
                return  Nullable<TypeModelItem >(model_static_cast<DelegateDeclarationModelItem>(_M_item)->returnType());
              }

            case _CodeModelItem::Kind_MethodDeclaration:
              {
                return  Nullable<TypeModelItem >(model_static_cast<MethodDeclarationModelItem>(_M_item)->returnType());
              }

            case _CodeModelItem::Kind_ConstructorDeclaration:
              {
                return  Nullable<TypeModelItem >(model_static_cast<ConstructorDeclarationModelItem>(_M_item)->returnType());
              }

            case _CodeModelItem::Kind_FinalizerDeclaration:
              {
                return  Nullable<TypeModelItem >(model_static_cast<FinalizerDeclarationModelItem>(_M_item)->returnType());
              }

            default:
              break;
            }

          return  Nullable<TypeModelItem>();
        }

      void _ModelItemChameleon::setReturnType(TypeModelItem returnType)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_DelegateDeclaration:
            {
              model_static_cast<DelegateDeclarationModelItem>(_M_item)->setReturnType(returnType);
              return ;
            }

          case _CodeModelItem::Kind_MethodDeclaration:
            {
              model_static_cast<MethodDeclarationModelItem>(_M_item)->setReturnType(returnType);
              return ;
            }

          case _CodeModelItem::Kind_ConstructorDeclaration:
            {
              model_static_cast<ConstructorDeclarationModelItem>(_M_item)->setReturnType(returnType);
              return ;
            }

          case _CodeModelItem::Kind_FinalizerDeclaration:
            {
              model_static_cast<FinalizerDeclarationModelItem>(_M_item)->setReturnType(returnType);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<ParameterList > _ModelItemChameleon::parameters( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_DelegateDeclaration:
              {
                return  Nullable<ParameterList >(model_static_cast<DelegateDeclarationModelItem>(_M_item)->parameters());
              }

            case _CodeModelItem::Kind_IndexerDeclaration:
              {
                return  Nullable<ParameterList >(model_static_cast<IndexerDeclarationModelItem>(_M_item)->parameters());
              }

            case _CodeModelItem::Kind_MethodDeclaration:
              {
                return  Nullable<ParameterList >(model_static_cast<MethodDeclarationModelItem>(_M_item)->parameters());
              }

            case _CodeModelItem::Kind_ConstructorDeclaration:
              {
                return  Nullable<ParameterList >(model_static_cast<ConstructorDeclarationModelItem>(_M_item)->parameters());
              }

            case _CodeModelItem::Kind_FinalizerDeclaration:
              {
                return  Nullable<ParameterList >(model_static_cast<FinalizerDeclarationModelItem>(_M_item)->parameters());
              }

            default:
              break;
            }

          return  Nullable<ParameterList>();
        }

      void _ModelItemChameleon::addParameter(ParameterModelItem item)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_DelegateDeclaration:
            {
              model_static_cast<DelegateDeclarationModelItem>(_M_item)->addParameter(item);
              return ;
            }

          case _CodeModelItem::Kind_IndexerDeclaration:
            {
              model_static_cast<IndexerDeclarationModelItem>(_M_item)->addParameter(item);
              return ;
            }

          case _CodeModelItem::Kind_MethodDeclaration:
            {
              model_static_cast<MethodDeclarationModelItem>(_M_item)->addParameter(item);
              return ;
            }

          case _CodeModelItem::Kind_ConstructorDeclaration:
            {
              model_static_cast<ConstructorDeclarationModelItem>(_M_item)->addParameter(item);
              return ;
            }

          case _CodeModelItem::Kind_FinalizerDeclaration:
            {
              model_static_cast<FinalizerDeclarationModelItem>(_M_item)->addParameter(item);
              return ;
            }

          default:
            break;
          }

        return ;
      }

      void _ModelItemChameleon::removeParameter(ParameterModelItem item)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_DelegateDeclaration:
            {
              model_static_cast<DelegateDeclarationModelItem>(_M_item)->removeParameter(item);
              return ;
            }

          case _CodeModelItem::Kind_IndexerDeclaration:
            {
              model_static_cast<IndexerDeclarationModelItem>(_M_item)->removeParameter(item);
              return ;
            }

          case _CodeModelItem::Kind_MethodDeclaration:
            {
              model_static_cast<MethodDeclarationModelItem>(_M_item)->removeParameter(item);
              return ;
            }

          case _CodeModelItem::Kind_ConstructorDeclaration:
            {
              model_static_cast<ConstructorDeclarationModelItem>(_M_item)->removeParameter(item);
              return ;
            }

          case _CodeModelItem::Kind_FinalizerDeclaration:
            {
              model_static_cast<FinalizerDeclarationModelItem>(_M_item)->removeParameter(item);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<EnumValueList > _ModelItemChameleon::enumValues( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_EnumDeclaration:
              {
                return  Nullable<EnumValueList >(model_static_cast<EnumDeclarationModelItem>(_M_item)->enumValues());
              }

            default:
              break;
            }

          return  Nullable<EnumValueList>();
        }

      void _ModelItemChameleon::addEnumValue(EnumValueModelItem item)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_EnumDeclaration:
            {
              model_static_cast<EnumDeclarationModelItem>(_M_item)->addEnumValue(item);
              return ;
            }

          default:
            break;
          }

        return ;
      }

      void _ModelItemChameleon::removeEnumValue(EnumValueModelItem item)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_EnumDeclaration:
            {
              model_static_cast<EnumDeclarationModelItem>(_M_item)->removeEnumValue(item);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<TypeModelItem > _ModelItemChameleon::baseIntegralType( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_EnumDeclaration:
              {
                return  Nullable<TypeModelItem >(model_static_cast<EnumDeclarationModelItem>(_M_item)->baseIntegralType());
              }

            default:
              break;
            }

          return  Nullable<TypeModelItem>();
        }

      void _ModelItemChameleon::setBaseIntegralType(TypeModelItem baseIntegralType)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_EnumDeclaration:
            {
              model_static_cast<EnumDeclarationModelItem>(_M_item)->setBaseIntegralType(baseIntegralType);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<QString > _ModelItemChameleon::value( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_EnumValue:
              {
                return  Nullable<QString >(model_static_cast<EnumValueModelItem>(_M_item)->value());
              }

            default:
              break;
            }

          return  Nullable<QString>();
        }

      void _ModelItemChameleon::setValue(QString value)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_EnumValue:
            {
              model_static_cast<EnumValueModelItem>(_M_item)->setValue(value);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<TypeModelItem > _ModelItemChameleon::type( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_EventDeclaration:
              {
                return  Nullable<TypeModelItem >(model_static_cast<EventDeclarationModelItem>(_M_item)->type());
              }

            case _CodeModelItem::Kind_IndexerDeclaration:
              {
                return  Nullable<TypeModelItem >(model_static_cast<IndexerDeclarationModelItem>(_M_item)->type());
              }

            case _CodeModelItem::Kind_PropertyDeclaration:
              {
                return  Nullable<TypeModelItem >(model_static_cast<PropertyDeclarationModelItem>(_M_item)->type());
              }

            case _CodeModelItem::Kind_VariableDeclaration:
              {
                return  Nullable<TypeModelItem >(model_static_cast<VariableDeclarationModelItem>(_M_item)->type());
              }

            case _CodeModelItem::Kind_Parameter:
              {
                return  Nullable<TypeModelItem >(model_static_cast<ParameterModelItem>(_M_item)->type());
              }

            default:
              break;
            }

          return  Nullable<TypeModelItem>();
        }

      void _ModelItemChameleon::setType(TypeModelItem type)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_EventDeclaration:
            {
              model_static_cast<EventDeclarationModelItem>(_M_item)->setType(type);
              return ;
            }

          case _CodeModelItem::Kind_IndexerDeclaration:
            {
              model_static_cast<IndexerDeclarationModelItem>(_M_item)->setType(type);
              return ;
            }

          case _CodeModelItem::Kind_PropertyDeclaration:
            {
              model_static_cast<PropertyDeclarationModelItem>(_M_item)->setType(type);
              return ;
            }

          case _CodeModelItem::Kind_VariableDeclaration:
            {
              model_static_cast<VariableDeclarationModelItem>(_M_item)->setType(type);
              return ;
            }

          case _CodeModelItem::Kind_Parameter:
            {
              model_static_cast<ParameterModelItem>(_M_item)->setType(type);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<TypeModelItem > _ModelItemChameleon::fromInterface( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_EventDeclaration:
              {
                return  Nullable<TypeModelItem >(model_static_cast<EventDeclarationModelItem>(_M_item)->fromInterface());
              }

            case _CodeModelItem::Kind_IndexerDeclaration:
              {
                return  Nullable<TypeModelItem >(model_static_cast<IndexerDeclarationModelItem>(_M_item)->fromInterface());
              }

            case _CodeModelItem::Kind_PropertyDeclaration:
              {
                return  Nullable<TypeModelItem >(model_static_cast<PropertyDeclarationModelItem>(_M_item)->fromInterface());
              }

            case _CodeModelItem::Kind_MethodDeclaration:
              {
                return  Nullable<TypeModelItem >(model_static_cast<MethodDeclarationModelItem>(_M_item)->fromInterface());
              }

            case _CodeModelItem::Kind_ConstructorDeclaration:
              {
                return  Nullable<TypeModelItem >(model_static_cast<ConstructorDeclarationModelItem>(_M_item)->fromInterface());
              }

            case _CodeModelItem::Kind_FinalizerDeclaration:
              {
                return  Nullable<TypeModelItem >(model_static_cast<FinalizerDeclarationModelItem>(_M_item)->fromInterface());
              }

            default:
              break;
            }

          return  Nullable<TypeModelItem>();
        }

      void _ModelItemChameleon::setFromInterface(TypeModelItem fromInterface)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_EventDeclaration:
            {
              model_static_cast<EventDeclarationModelItem>(_M_item)->setFromInterface(fromInterface);
              return ;
            }

          case _CodeModelItem::Kind_IndexerDeclaration:
            {
              model_static_cast<IndexerDeclarationModelItem>(_M_item)->setFromInterface(fromInterface);
              return ;
            }

          case _CodeModelItem::Kind_PropertyDeclaration:
            {
              model_static_cast<PropertyDeclarationModelItem>(_M_item)->setFromInterface(fromInterface);
              return ;
            }

          case _CodeModelItem::Kind_MethodDeclaration:
            {
              model_static_cast<MethodDeclarationModelItem>(_M_item)->setFromInterface(fromInterface);
              return ;
            }

          case _CodeModelItem::Kind_ConstructorDeclaration:
            {
              model_static_cast<ConstructorDeclarationModelItem>(_M_item)->setFromInterface(fromInterface);
              return ;
            }

          case _CodeModelItem::Kind_FinalizerDeclaration:
            {
              model_static_cast<FinalizerDeclarationModelItem>(_M_item)->setFromInterface(fromInterface);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<EventAccessorDeclarationModelItem > _ModelItemChameleon::addAccessor( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_EventDeclaration:
              {
                return  Nullable<EventAccessorDeclarationModelItem >(model_static_cast<EventDeclarationModelItem>(_M_item)->addAccessor());
              }

            default:
              break;
            }

          return  Nullable<EventAccessorDeclarationModelItem>();
        }

      void _ModelItemChameleon::setAddAccessor(EventAccessorDeclarationModelItem addAccessor)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_EventDeclaration:
            {
              model_static_cast<EventDeclarationModelItem>(_M_item)->setAddAccessor(addAccessor);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<EventAccessorDeclarationModelItem > _ModelItemChameleon::removeAccessor( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_EventDeclaration:
              {
                return  Nullable<EventAccessorDeclarationModelItem >(model_static_cast<EventDeclarationModelItem>(_M_item)->removeAccessor());
              }

            default:
              break;
            }

          return  Nullable<EventAccessorDeclarationModelItem>();
        }

      void _ModelItemChameleon::setRemoveAccessor(EventAccessorDeclarationModelItem removeAccessor)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_EventDeclaration:
            {
              model_static_cast<EventDeclarationModelItem>(_M_item)->setRemoveAccessor(removeAccessor);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<bool > _ModelItemChameleon::isVirtual( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_EventDeclaration:
              {
                return  Nullable<bool >(model_static_cast<EventDeclarationModelItem>(_M_item)->isVirtual());
              }

            case _CodeModelItem::Kind_IndexerDeclaration:
              {
                return  Nullable<bool >(model_static_cast<IndexerDeclarationModelItem>(_M_item)->isVirtual());
              }

            case _CodeModelItem::Kind_PropertyDeclaration:
              {
                return  Nullable<bool >(model_static_cast<PropertyDeclarationModelItem>(_M_item)->isVirtual());
              }

            case _CodeModelItem::Kind_MethodDeclaration:
              {
                return  Nullable<bool >(model_static_cast<MethodDeclarationModelItem>(_M_item)->isVirtual());
              }

            case _CodeModelItem::Kind_ConstructorDeclaration:
              {
                return  Nullable<bool >(model_static_cast<ConstructorDeclarationModelItem>(_M_item)->isVirtual());
              }

            case _CodeModelItem::Kind_FinalizerDeclaration:
              {
                return  Nullable<bool >(model_static_cast<FinalizerDeclarationModelItem>(_M_item)->isVirtual());
              }

            default:
              break;
            }

          return  Nullable<bool>();
        }

      void _ModelItemChameleon::setVirtual(bool isVirtual)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_EventDeclaration:
            {
              model_static_cast<EventDeclarationModelItem>(_M_item)->setVirtual(isVirtual);
              return ;
            }

          case _CodeModelItem::Kind_IndexerDeclaration:
            {
              model_static_cast<IndexerDeclarationModelItem>(_M_item)->setVirtual(isVirtual);
              return ;
            }

          case _CodeModelItem::Kind_PropertyDeclaration:
            {
              model_static_cast<PropertyDeclarationModelItem>(_M_item)->setVirtual(isVirtual);
              return ;
            }

          case _CodeModelItem::Kind_MethodDeclaration:
            {
              model_static_cast<MethodDeclarationModelItem>(_M_item)->setVirtual(isVirtual);
              return ;
            }

          case _CodeModelItem::Kind_ConstructorDeclaration:
            {
              model_static_cast<ConstructorDeclarationModelItem>(_M_item)->setVirtual(isVirtual);
              return ;
            }

          case _CodeModelItem::Kind_FinalizerDeclaration:
            {
              model_static_cast<FinalizerDeclarationModelItem>(_M_item)->setVirtual(isVirtual);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<bool > _ModelItemChameleon::isOverride( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_EventDeclaration:
              {
                return  Nullable<bool >(model_static_cast<EventDeclarationModelItem>(_M_item)->isOverride());
              }

            case _CodeModelItem::Kind_IndexerDeclaration:
              {
                return  Nullable<bool >(model_static_cast<IndexerDeclarationModelItem>(_M_item)->isOverride());
              }

            case _CodeModelItem::Kind_PropertyDeclaration:
              {
                return  Nullable<bool >(model_static_cast<PropertyDeclarationModelItem>(_M_item)->isOverride());
              }

            case _CodeModelItem::Kind_MethodDeclaration:
              {
                return  Nullable<bool >(model_static_cast<MethodDeclarationModelItem>(_M_item)->isOverride());
              }

            case _CodeModelItem::Kind_ConstructorDeclaration:
              {
                return  Nullable<bool >(model_static_cast<ConstructorDeclarationModelItem>(_M_item)->isOverride());
              }

            case _CodeModelItem::Kind_FinalizerDeclaration:
              {
                return  Nullable<bool >(model_static_cast<FinalizerDeclarationModelItem>(_M_item)->isOverride());
              }

            default:
              break;
            }

          return  Nullable<bool>();
        }

      void _ModelItemChameleon::setOverride(bool isOverride)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_EventDeclaration:
            {
              model_static_cast<EventDeclarationModelItem>(_M_item)->setOverride(isOverride);
              return ;
            }

          case _CodeModelItem::Kind_IndexerDeclaration:
            {
              model_static_cast<IndexerDeclarationModelItem>(_M_item)->setOverride(isOverride);
              return ;
            }

          case _CodeModelItem::Kind_PropertyDeclaration:
            {
              model_static_cast<PropertyDeclarationModelItem>(_M_item)->setOverride(isOverride);
              return ;
            }

          case _CodeModelItem::Kind_MethodDeclaration:
            {
              model_static_cast<MethodDeclarationModelItem>(_M_item)->setOverride(isOverride);
              return ;
            }

          case _CodeModelItem::Kind_ConstructorDeclaration:
            {
              model_static_cast<ConstructorDeclarationModelItem>(_M_item)->setOverride(isOverride);
              return ;
            }

          case _CodeModelItem::Kind_FinalizerDeclaration:
            {
              model_static_cast<FinalizerDeclarationModelItem>(_M_item)->setOverride(isOverride);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<bool > _ModelItemChameleon::isExtern( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_EventDeclaration:
              {
                return  Nullable<bool >(model_static_cast<EventDeclarationModelItem>(_M_item)->isExtern());
              }

            case _CodeModelItem::Kind_IndexerDeclaration:
              {
                return  Nullable<bool >(model_static_cast<IndexerDeclarationModelItem>(_M_item)->isExtern());
              }

            case _CodeModelItem::Kind_PropertyDeclaration:
              {
                return  Nullable<bool >(model_static_cast<PropertyDeclarationModelItem>(_M_item)->isExtern());
              }

            case _CodeModelItem::Kind_MethodDeclaration:
              {
                return  Nullable<bool >(model_static_cast<MethodDeclarationModelItem>(_M_item)->isExtern());
              }

            case _CodeModelItem::Kind_ConstructorDeclaration:
              {
                return  Nullable<bool >(model_static_cast<ConstructorDeclarationModelItem>(_M_item)->isExtern());
              }

            case _CodeModelItem::Kind_FinalizerDeclaration:
              {
                return  Nullable<bool >(model_static_cast<FinalizerDeclarationModelItem>(_M_item)->isExtern());
              }

            default:
              break;
            }

          return  Nullable<bool>();
        }

      void _ModelItemChameleon::setExtern(bool isExtern)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_EventDeclaration:
            {
              model_static_cast<EventDeclarationModelItem>(_M_item)->setExtern(isExtern);
              return ;
            }

          case _CodeModelItem::Kind_IndexerDeclaration:
            {
              model_static_cast<IndexerDeclarationModelItem>(_M_item)->setExtern(isExtern);
              return ;
            }

          case _CodeModelItem::Kind_PropertyDeclaration:
            {
              model_static_cast<PropertyDeclarationModelItem>(_M_item)->setExtern(isExtern);
              return ;
            }

          case _CodeModelItem::Kind_MethodDeclaration:
            {
              model_static_cast<MethodDeclarationModelItem>(_M_item)->setExtern(isExtern);
              return ;
            }

          case _CodeModelItem::Kind_ConstructorDeclaration:
            {
              model_static_cast<ConstructorDeclarationModelItem>(_M_item)->setExtern(isExtern);
              return ;
            }

          case _CodeModelItem::Kind_FinalizerDeclaration:
            {
              model_static_cast<FinalizerDeclarationModelItem>(_M_item)->setExtern(isExtern);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<AccessorDeclarationList > _ModelItemChameleon::accessors( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_IndexerDeclaration:
              {
                return  Nullable<AccessorDeclarationList >(model_static_cast<IndexerDeclarationModelItem>(_M_item)->accessors());
              }

            case _CodeModelItem::Kind_PropertyDeclaration:
              {
                return  Nullable<AccessorDeclarationList >(model_static_cast<PropertyDeclarationModelItem>(_M_item)->accessors());
              }

            default:
              break;
            }

          return  Nullable<AccessorDeclarationList>();
        }

      void _ModelItemChameleon::addAccessor(AccessorDeclarationModelItem item)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_IndexerDeclaration:
            {
              model_static_cast<IndexerDeclarationModelItem>(_M_item)->addAccessor(item);
              return ;
            }

          case _CodeModelItem::Kind_PropertyDeclaration:
            {
              model_static_cast<PropertyDeclarationModelItem>(_M_item)->addAccessor(item);
              return ;
            }

          default:
            break;
          }

        return ;
      }

      void _ModelItemChameleon::removeAccessor(AccessorDeclarationModelItem item)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_IndexerDeclaration:
            {
              model_static_cast<IndexerDeclarationModelItem>(_M_item)->removeAccessor(item);
              return ;
            }

          case _CodeModelItem::Kind_PropertyDeclaration:
            {
              model_static_cast<PropertyDeclarationModelItem>(_M_item)->removeAccessor(item);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      void _ModelItemChameleon::setType(accessor_declarations::accessor_type_enum type)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_AccessorDeclaration:
            {
              model_static_cast<AccessorDeclarationModelItem>(_M_item)->setType(type);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<bool > _ModelItemChameleon::hasAccessPolicy( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_AccessorDeclaration:
              {
                return  Nullable<bool >(model_static_cast<AccessorDeclarationModelItem>(_M_item)->hasAccessPolicy());
              }

            default:
              break;
            }

          return  Nullable<bool>();
        }

      void _ModelItemChameleon::setHasAccessPolicy(bool hasAccessPolicy)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_AccessorDeclaration:
            {
              model_static_cast<AccessorDeclarationModelItem>(_M_item)->setHasAccessPolicy(hasAccessPolicy);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<bool > _ModelItemChameleon::isInterfaceMethodDeclaration( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_MethodDeclaration:
              {
                return  Nullable<bool >(model_static_cast<MethodDeclarationModelItem>(_M_item)->isInterfaceMethodDeclaration());
              }

            case _CodeModelItem::Kind_ConstructorDeclaration:
              {
                return  Nullable<bool >(model_static_cast<ConstructorDeclarationModelItem>(_M_item)->isInterfaceMethodDeclaration());
              }

            case _CodeModelItem::Kind_FinalizerDeclaration:
              {
                return  Nullable<bool >(model_static_cast<FinalizerDeclarationModelItem>(_M_item)->isInterfaceMethodDeclaration());
              }

            default:
              break;
            }

          return  Nullable<bool>();
        }

      void _ModelItemChameleon::setInterfaceMethodDeclaration(bool isInterfaceMethodDeclaration)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_MethodDeclaration:
            {
              model_static_cast<MethodDeclarationModelItem>(_M_item)->setInterfaceMethodDeclaration(isInterfaceMethodDeclaration);
              return ;
            }

          case _CodeModelItem::Kind_ConstructorDeclaration:
            {
              model_static_cast<ConstructorDeclarationModelItem>(_M_item)->setInterfaceMethodDeclaration(isInterfaceMethodDeclaration);
              return ;
            }

          case _CodeModelItem::Kind_FinalizerDeclaration:
            {
              model_static_cast<FinalizerDeclarationModelItem>(_M_item)->setInterfaceMethodDeclaration(isInterfaceMethodDeclaration);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<VariableDeclarationList > _ModelItemChameleon::localVariables( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_MethodDeclaration:
              {
                return  Nullable<VariableDeclarationList >(model_static_cast<MethodDeclarationModelItem>(_M_item)->localVariables());
              }

            case _CodeModelItem::Kind_ConstructorDeclaration:
              {
                return  Nullable<VariableDeclarationList >(model_static_cast<ConstructorDeclarationModelItem>(_M_item)->localVariables());
              }

            case _CodeModelItem::Kind_FinalizerDeclaration:
              {
                return  Nullable<VariableDeclarationList >(model_static_cast<FinalizerDeclarationModelItem>(_M_item)->localVariables());
              }

            default:
              break;
            }

          return  Nullable<VariableDeclarationList>();
        }

      void _ModelItemChameleon::addLocalVariable(VariableDeclarationModelItem item)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_MethodDeclaration:
            {
              model_static_cast<MethodDeclarationModelItem>(_M_item)->addLocalVariable(item);
              return ;
            }

          case _CodeModelItem::Kind_ConstructorDeclaration:
            {
              model_static_cast<ConstructorDeclarationModelItem>(_M_item)->addLocalVariable(item);
              return ;
            }

          case _CodeModelItem::Kind_FinalizerDeclaration:
            {
              model_static_cast<FinalizerDeclarationModelItem>(_M_item)->addLocalVariable(item);
              return ;
            }

          default:
            break;
          }

        return ;
      }

      void _ModelItemChameleon::removeLocalVariable(VariableDeclarationModelItem item)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_MethodDeclaration:
            {
              model_static_cast<MethodDeclarationModelItem>(_M_item)->removeLocalVariable(item);
              return ;
            }

          case _CodeModelItem::Kind_ConstructorDeclaration:
            {
              model_static_cast<ConstructorDeclarationModelItem>(_M_item)->removeLocalVariable(item);
              return ;
            }

          case _CodeModelItem::Kind_FinalizerDeclaration:
            {
              model_static_cast<FinalizerDeclarationModelItem>(_M_item)->removeLocalVariable(item);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<bool > _ModelItemChameleon::isConstant( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_VariableDeclaration:
              {
                return  Nullable<bool >(model_static_cast<VariableDeclarationModelItem>(_M_item)->isConstant());
              }

            default:
              break;
            }

          return  Nullable<bool>();
        }

      void _ModelItemChameleon::setConstant(bool isConstant)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_VariableDeclaration:
            {
              model_static_cast<VariableDeclarationModelItem>(_M_item)->setConstant(isConstant);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<bool > _ModelItemChameleon::isReadonly( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_VariableDeclaration:
              {
                return  Nullable<bool >(model_static_cast<VariableDeclarationModelItem>(_M_item)->isReadonly());
              }

            default:
              break;
            }

          return  Nullable<bool>();
        }

      void _ModelItemChameleon::setReadonly(bool isReadonly)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_VariableDeclaration:
            {
              model_static_cast<VariableDeclarationModelItem>(_M_item)->setReadonly(isReadonly);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<bool > _ModelItemChameleon::isVolatile( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_VariableDeclaration:
              {
                return  Nullable<bool >(model_static_cast<VariableDeclarationModelItem>(_M_item)->isVolatile());
              }

            default:
              break;
            }

          return  Nullable<bool>();
        }

      void _ModelItemChameleon::setVolatile(bool isVolatile)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_VariableDeclaration:
            {
              model_static_cast<VariableDeclarationModelItem>(_M_item)->setVolatile(isVolatile);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<TypeList > _ModelItemChameleon::typeArguments( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_TypePart:
              {
                return  Nullable<TypeList >(model_static_cast<TypePartModelItem>(_M_item)->typeArguments());
              }

            default:
              break;
            }

          return  Nullable<TypeList>();
        }

      void _ModelItemChameleon::addTypeArgument(TypeModelItem item)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_TypePart:
            {
              model_static_cast<TypePartModelItem>(_M_item)->addTypeArgument(item);
              return ;
            }

          default:
            break;
          }

        return ;
      }

      void _ModelItemChameleon::removeTypeArgument(TypeModelItem item)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_TypePart:
            {
              model_static_cast<TypePartModelItem>(_M_item)->removeTypeArgument(item);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<QString > _ModelItemChameleon::qualifiedAliasLabel( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_Type:
              {
                return  Nullable<QString >(model_static_cast<TypeModelItem>(_M_item)->qualifiedAliasLabel());
              }

            default:
              break;
            }

          return  Nullable<QString>();
        }

      void _ModelItemChameleon::setQualifiedAliasLabel(QString qualifiedAliasLabel)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_Type:
            {
              model_static_cast<TypeModelItem>(_M_item)->setQualifiedAliasLabel(qualifiedAliasLabel);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<TypePartList > _ModelItemChameleon::typeParts( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_Type:
              {
                return  Nullable<TypePartList >(model_static_cast<TypeModelItem>(_M_item)->typeParts());
              }

            default:
              break;
            }

          return  Nullable<TypePartList>();
        }

      void _ModelItemChameleon::addTypePart(TypePartModelItem item)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_Type:
            {
              model_static_cast<TypeModelItem>(_M_item)->addTypePart(item);
              return ;
            }

          default:
            break;
          }

        return ;
      }

      void _ModelItemChameleon::removeTypePart(TypePartModelItem item)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_Type:
            {
              model_static_cast<TypeModelItem>(_M_item)->removeTypePart(item);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<bool > _ModelItemChameleon::isArray( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_Parameter:
              {
                return  Nullable<bool >(model_static_cast<ParameterModelItem>(_M_item)->isArray());
              }

            default:
              break;
            }

          return  Nullable<bool>();
        }

      void _ModelItemChameleon::setArray(bool isArray)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_Parameter:
            {
              model_static_cast<ParameterModelItem>(_M_item)->setArray(isArray);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<parameter::parameter_type_enum > _ModelItemChameleon::parameterType( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_Parameter:
              {
                return  Nullable<parameter::parameter_type_enum >(model_static_cast<ParameterModelItem>(_M_item)->parameterType());
              }

            default:
              break;
            }

          return  Nullable<parameter::parameter_type_enum>();
        }

      void _ModelItemChameleon::setParameterType(parameter::parameter_type_enum parameterType)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_Parameter:
            {
              model_static_cast<ParameterModelItem>(_M_item)->setParameterType(parameterType);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<TypeModelItem > _ModelItemChameleon::typeOrParameterName( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_PrimaryOrSecondaryConstraint:
              {
                return  Nullable<TypeModelItem >(model_static_cast<PrimaryOrSecondaryConstraintModelItem>(_M_item)->typeOrParameterName());
              }

            default:
              break;
            }

          return  Nullable<TypeModelItem>();
        }

      void _ModelItemChameleon::setTypeOrParameterName(TypeModelItem typeOrParameterName)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_PrimaryOrSecondaryConstraint:
            {
              model_static_cast<PrimaryOrSecondaryConstraintModelItem>(_M_item)->setTypeOrParameterName(typeOrParameterName);
              return ;
            }

          default:
            break;
          }

        return ;
      }


      Nullable<primary_or_secondary_constraint::primary_or_secondary_constraint_enum > _ModelItemChameleon::constraint_type( ) const
        {
          switch  (_M_item->kind())
            {

            case _CodeModelItem::Kind_PrimaryOrSecondaryConstraint:
              {
                return  Nullable<primary_or_secondary_constraint::primary_or_secondary_constraint_enum >(model_static_cast<PrimaryOrSecondaryConstraintModelItem>(_M_item)->constraint_type());
              }

            default:
              break;
            }

          return  Nullable<primary_or_secondary_constraint::primary_or_secondary_constraint_enum>();
        }

      void _ModelItemChameleon::setConstraint_type(primary_or_secondary_constraint::primary_or_secondary_constraint_enum constraint_type)
      {
        switch  (_M_item->kind())
          {

          case _CodeModelItem::Kind_PrimaryOrSecondaryConstraint:
            {
              model_static_cast<PrimaryOrSecondaryConstraintModelItem>(_M_item)->setConstraint_type(constraint_type);
              return ;
            }

          default:
            break;
          }

        return ;
      }



    } // end of namespace csharp


