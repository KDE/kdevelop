/***************************************************************************
   copyright            : (C) 2006 by David Nolden
   email                : david.nolden.kdevelop@art-master.de
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef SIMPLETYPEFUNCTION_H
#define SIMPLETYPEFUNCTION_H

#include "simpletype.h"
#include "simpletypecachebinder.h"
#include "simpletypecatalog.h"

HashedStringSet getIncludeFiles( const ItemDom& item );


///Interface that functions should implement
class SimpleTypeFunctionInterface {
  public:
  
    SimpleTypeFunctionInterface() {
    }
  
    SimpleTypeFunctionInterface( SimpleTypeFunctionInterface* rhs ) {
      m_nextFunction = rhs->m_nextFunction;
    }
  
    void clearNextFunctions() {
      m_nextFunction = SimpleType();
    }
  
    void appendNextFunction( SimpleType func );
  
    SimpleType nextFunction() {
      return m_nextFunction;
    }
  
    ///Returns the totally unresolved return-type
    virtual TypeDesc getReturnType() = 0;
  
    virtual QValueList<TypeDesc> getArgumentTypes() = 0;
  
    virtual QStringList getArgumentDefaults() = 0;
  
    virtual QStringList getArgumentNames() = 0;
  
    virtual bool isConst() = 0;
  
    virtual QString signature();
  
///TODO: This function should locate the correct overloaded method in the chain, fitting the parameters
    ///should also moved into another class then
    SimpleTypeImpl* match( const QValueList<LocateResult>& /*params*/ ) {
        //      QValueList<TypeDesc> args = getArgumentTypes();
      return dynamic_cast<SimpleTypeImpl*>( this );
    }

  private:
        ///Since functions can be overloaded, many functions with the same name can exist. Other functions that belong to this one
    ///should be appended to this.
    SimpleType m_nextFunction;
   
  
  protected:
  
    bool containsUndefinedTemplateParam( TypeDesc& desc, SimpleTypeImpl::TemplateParamInfo& paramInfo );
  
    ///Tries to match the types, filling implicit template-params into paramInfo
    void resolveImplicitTypes( TypeDesc& argType, TypeDesc& gottenArgType, SimpleTypeImpl::TemplateParamInfo& paramInfo );
  
    ///Tries to match the types, filling implicit template-params into paramInfo
    void resolveImplicitTypes( TypeDesc::TemplateParams& argTypes, TypeDesc::TemplateParams& gottenArgTypes, SimpleTypeImpl::TemplateParamInfo& paramInfo );
  
    ///Tries to match the types, filling implicit template-params into paramInfo
    void resolveImplicitTypes( QValueList<TypeDesc>& argTypes, QValueList<TypeDesc>& gottenArgTypes, SimpleTypeImpl::TemplateParamInfo& paramInfo );
};

template <class Base=SimpleTypeImpl>
class SimpleTypeFunction : public Base, public SimpleTypeFunctionInterface {
public:
  SimpleTypeFunction() : Base() {
  }
    
  SimpleTypeFunction( SimpleTypeFunction<Base>* rhs ) : Base( rhs ), SimpleTypeFunctionInterface( rhs ) {
  }
  
  template <class Type>
    SimpleTypeFunction( Type t ) : Base( t ) {
    }
  
  virtual ~SimpleTypeFunction() {
  };
  
  virtual SimpleTypeImpl::TypePointer clone() = 0;
  
  virtual SimpleTypeImpl::MemberInfo findMember( TypeDesc name, SimpleTypeImpl::MemberInfo::MemberType type ){
    SimpleTypeImpl::MemberInfo ret;
    if( type & SimpleTypeImpl::MemberInfo::Template ) {
      TypeDesc s = Base::findTemplateParam( name.name() );
      if( s ) {
        ret.memberType = SimpleTypeImpl::MemberInfo::Template;
        ret.type = s;
      }
    }
    
    return ret;
  }
  
  virtual LocateResult applyOperator( typename Base::Operator op , QValueList<LocateResult> params ) {
    Debug d("#apply#");
    if( !d )
      return LocateResult();
    
    if( op == SimpleTypeImpl::ParenOp ) {
            ///First, try to find an overloaded function matching the parameter-types.
      SimpleTypeImpl* f = match( params );
      if( f && f->asFunction() ) {
        ifVerbose( dbg() << "applying Operator " << this->operatorToString( op ) << " to \"" << f->desc().fullNameChain() << "\"" <<  endl );
        
        TypeDesc rt = f->asFunction()->getReturnType();
        SimpleTypeImpl::TemplateParamInfo paramInfo = f->getTemplateParamInfo();
        if( containsUndefinedTemplateParam( rt, paramInfo ) ) {
                    /** This is the place where implicit template-function-instatiation takes place.
                     *  Match the given param-types with the argument-types to resolve new template-params.
                    */
          QValueList<TypeDesc> args = getArgumentTypes();
          QValueList<TypeDesc> paramDescs;
          for( QValueList<LocateResult>::iterator it = params.begin(); it != params.end(); ++it )
            paramDescs << (TypeDesc)(*it);
          resolveImplicitTypes( args, paramDescs, paramInfo );
                    ///paramInfo now contains the information for all implicit types
        }
        
        return this->parent()->locateDecType( f->replaceTemplateParams( rt, paramInfo ) );
      } else {
        ifVerbose( dbg() << "failed to find a fitting overloaded method" << endl );
      }
    }
    return Base::applyOperator( op, params );
  }
  
};

/**
The SimpleTypeCodeModel and SimpleTypeCatalog can represent namespaces too,
but they only represent a REAL(physical) namespace and only within either the catalog
or the code-model. This is a proxy-class that handles namespace-aliases, imports, and splits requests to both, the catalog, and the code-model.
There can be multiple namespaces aliased to the same one
*/


class SimpleTypeCodeModel : public SimpleTypeImpl {
public:
  
  SimpleTypeCodeModel( SimpleTypeCodeModel* rhs ) : SimpleTypeImpl( rhs ), m_item( rhs->m_item ) {
  }
  
  SimpleTypeCodeModel( SimpleTypeImpl* rhs  ) : SimpleTypeImpl( rhs ) {
    init();
  }
  
  SimpleTypeCodeModel( const QStringList& scope ) : SimpleTypeImpl( scope ) {
    init();
  }
  
  virtual bool hasNode() const {
    return (bool)m_item;
  };
  
  virtual QString comment() const {
    if( m_item ) {
      return m_item->comment();
    } else {
      return "";
    }
  };
  
  virtual DeclarationInfo getDeclarationInfo();
  
  SimpleTypeCodeModel( ItemDom& item );
  
  virtual TypePointer clone() {
    return new SimpleTypeCachedCodeModel( this );
  }
  
  virtual Repository rep() {
    return CodeModel;
  }
  
  virtual bool isNamespace() const {
    if( m_item ) {
      return m_item->isNamespace();
    } else {
      return false;
    }
  };
  
  inline ItemDom& item() {
    return m_item;
  }
  
  virtual TemplateParamInfo getTemplateParamInfo();
  
  virtual const LocateResult findTemplateParam( const QString& name );
  
    /** In case of a class, returns all base-types */
  virtual QValueList<LocateResult> getBases();
  
  ItemDom locateModelContainer( class CodeModel* m, TypeDesc t, ClassDom cnt = ClassDom() );

  virtual QString specialization() const;

  virtual void addAliasesTo( SimpleTypeNamespace* ns );
  
private:
  ItemDom m_item;
  
  bool findItem();
  
  void init();

    //ClassDom pickMostRelated( ClassList lst, QString fn );
  
protected:
  SimpleTypeCodeModel() : SimpleTypeImpl() {
  };
  
  
  struct CodeModelBuildInfo : public TypeBuildInfo {
    ItemDom m_item;
    TypeDesc m_desc;
    TypePointer m_parent;
    
    CodeModelBuildInfo( ItemDom item, const TypeDesc& desc, TypePointer parent ) : m_item( item ), m_desc( desc ), m_parent( parent ) {
    }
    
    virtual TypePointer build();
  };
  
  virtual MemberInfo findMember( TypeDesc name , MemberInfo::MemberType type = MemberInfo::AllTypes) ;

  virtual QValueList<TypePointer> getMemberClasses( const TypeDesc& name ) ;
};


class SimpleTypeCodeModelFunction : public SimpleTypeFunction<SimpleTypeCodeModel> {
public:
  SimpleTypeCodeModelFunction() : SimpleTypeFunction<SimpleTypeCodeModel>() {
  }
    
  SimpleTypeCodeModelFunction( SimpleTypeCodeModelFunction* rhs ) : SimpleTypeFunction<SimpleTypeCodeModel> ( rhs ) {
  }
  
  virtual SimpleTypeImpl::TypePointer clone() {
    return new SimpleTypeCodeModelFunction( this );
  }
  
  SimpleTypeCodeModelFunction( ItemDom item ) : SimpleTypeFunction<SimpleTypeCodeModel>( item ) {
  }
  
  virtual TypeDesc getReturnType();
  
  virtual bool isConst();
  
  virtual QValueList<TypeDesc> getArgumentTypes();
  
  virtual QStringList getArgumentNames();
  
  virtual QStringList getArgumentDefaults();
  
  struct CodeModelFunctionBuildInfo : public TypeBuildInfo {
    FunctionList m_items;
    TypeDesc m_desc;
    TypePointer m_parent;
    
    CodeModelFunctionBuildInfo( FunctionList items, TypeDesc& desc, TypePointer parent ) : m_items( items ), m_desc( desc ), m_parent( parent ) {
    }
    CodeModelFunctionBuildInfo( FunctionDefinitionList items, TypeDesc& desc, TypePointer parent );
    
    virtual TypePointer build();
  };

private:
  FunctionModel* asFunctionModel() {
    if( ! &(*item() ) ) return 0;
    return dynamic_cast<FunctionModel*>( &(*item() ) );
  }
  
};


class SimpleTypeCatalogFunction : public SimpleTypeFunction<SimpleTypeCatalog> {
public:
  SimpleTypeCatalogFunction() : SimpleTypeFunction<SimpleTypeCatalog>() {
  }
    
  SimpleTypeCatalogFunction( Tag tag ) : SimpleTypeFunction<SimpleTypeCatalog>( tag ) {
  }
    
  SimpleTypeCatalogFunction( SimpleTypeCatalogFunction* rhs ) : SimpleTypeFunction<SimpleTypeCatalog>( rhs )  {
  }
  
  virtual SimpleTypeImpl::TypePointer clone() {
    return new SimpleTypeCatalogFunction( this );
  }
  
  struct CatalogFunctionBuildInfo : public TypeBuildInfo {
    QValueList<Tag> m_tags;
    TypeDesc m_desc;
    TypePointer m_parent;
    
    CatalogFunctionBuildInfo( QValueList<Tag> tags, TypeDesc& desc, TypePointer parent ) : m_tags( tags ), m_desc( desc ), m_parent( parent ) {
    }
    
    virtual TypePointer build();
  };
  
  virtual TypeDesc getReturnType();
  
  virtual bool isConst ();
  
  virtual QStringList getArgumentDefaults() {
    return QStringList();
  }
  
  virtual QStringList getArgumentNames();
  
  virtual QValueList<TypeDesc> getArgumentTypes();
};


#endif
// kate: indent-mode csands; tab-width 4;
