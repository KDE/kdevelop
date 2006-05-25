
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

#ifndef __SIMPLETYPE_H__
#define __SIMPLETYPE_H__

#include <set>

#include "bithelpers.h"
#include "stringhelpers.h"
#include "completiondebug.h"
#include "typedesc.h"
#include "cpp_tags.h"
#include "codemodel.h"

extern QString globalCurrentFile;

using namespace CompletionDebug;
using namespace BitHelpers;
using namespace StringHelpers;

class SimpleTypeImpl;
class SimpleTypeFunctionInterface;
class SimpleTypeNamespace;

typedef KSharedPtr<SimpleTypeImpl> TypePointer;

enum Repository {
  CodeModel,
    Catalog,
    StringList,
    Both,
    Undefined
};

/**
    A Type can be invalid( operator hasNode() returns false ) but still have a desc. In that case, the desc is the
    nearest point reached in the resolution of the type.

    Warning: The types are not copied when they are assigned to each other,
    they manage internal references, so just changing a type may cause bugs.
    Use makePrivate() before changing. */

class SimpleType {
public:
  
  friend class SimpleTypeImpl;

  SimpleType( const SimpleType& rhs ) {
    *this = rhs;
  }
  
  SimpleType( const QStringList& scope, Repository rep = Undefined ) : m_resolved(false) {
    init( scope, rep );
  }
  
  SimpleType( const QString& text, Repository rep = Undefined  ) : m_resolved(false)  {
    init( splitType( text ), rep );
  };
  
  SimpleType( Repository rep = Undefined ) : m_resolved(false) {
    init( QStringList(), rep );
  };
  
  SimpleType( SimpleTypeImpl* ip ) : m_type( TypePointer( ip ) ), m_resolved( true )  {
  }
  
  SimpleType( ItemDom item );

  bool operator < ( SimpleType& rhs );
    
    /*SimpleType( Tag tag );*/
  
  SimpleTypeImpl* operator -> () const {
    resolve();
    return &(*m_type);
  }
  
  TypePointer get() const {
    resolve();
    return m_type;
  }
  
  SimpleTypeImpl& operator * () const {
    resolve();
    return *m_type;
  }
  
  SimpleType& operator = ( const SimpleType& rhs ) {
    m_type = rhs.m_type;
    m_resolved = rhs.m_resolved;
    return *this;
  }
  
    /** Just compares the scope */
  bool operator == ( const SimpleType& rhs ) const {
    return scope() == rhs.scope();
  }
  
  void makePrivate();
  
  operator QString() const {
    return str();
  }
  
    ///lazily returns the scope
  const QStringList& scope() const;
  
  const QString str() const;
  
    ///valid() does not check whether the type was found in some model,
    ///it just checks whether this theoretically represents a type.
  inline operator bool () const {
    return valid();
  }
  
  bool valid() const {
    return !scope().isEmpty();
  }
  
  static void setGlobalNamespace( TypePointer tp ) {
  m_globalNamespace = tp;
  }
  
  static void resetGlobalNamespace() {
  m_globalNamespace = 0;
  }
  
  
  ///Since many cross-references are possible, this function breaks them all so that all SimpleTypeImpls can free themselves.
  static void destroyStore();

private:    
  
  void init( const QStringList& scope , Repository rep );
  
  void resolve( Repository rep = Undefined ) const ;
  
  mutable TypePointer m_type;
  mutable bool m_resolved;
  static TypePointer m_globalNamespace; ///this is bad, but with the current parser we can't clearly determine the correct global-namespace for each class/file
  static bool m_unregistered;
  typedef std::set<SimpleTypeImpl*> TypeStore ;
  static TypeStore m_typeStore; ///This is necessary because TypeDescs ind SimpleTypeImpls can have cross-references, and thereby make themselves unreleasable, so each SimpleTypeImpl is stored in this list and destroyed at once by SimpleTypeConfiguration( it breaks all references ) 
  
  
  static void registerType( SimpleTypeImpl* tp ) {
    if( !tp ) return;
    m_typeStore.insert( tp );
  }
  
  static void unregisterType( SimpleTypeImpl* tp ) {
    m_typeStore.erase( tp );
    m_unregistered = true;
  }
  
};


class SimpleTypeConfiguration {
public:
  SimpleTypeConfiguration( QString currentFileName = "" ) {
    globalCurrentFile = currentFileName;
  }
  
  void setGlobalNamespace( TypePointer globalNamespace ) {
    SimpleType::setGlobalNamespace( globalNamespace );
  }
  
  virtual ~SimpleTypeConfiguration() {
    SimpleType::resetGlobalNamespace();
    SimpleType::destroyStore();
  }
};


class SimpleTypeImpl : public KShared {
  enum ResolutionFlags {
    NoFlag = 0,
    HadTypedef = 1,
    HadTemplate = 2,
    HadAlias = 3
  };

public:
  typedef KSharedPtr<SimpleTypeImpl> TypePointer;
  
  void tracePrepend( const TypeDesc& t ) {
    m_trace.push_front( t );
  }
  
  QValueList<TypeDesc> trace() {
    return m_trace;
  }
  
  class TemplateParamInfo {
  public:
    struct TemplateParam {
      QString name;
      TypeDesc def;
      TypeDesc value;
      int number;
    TemplateParam() : number(0) {
    }
    };
  private:
    QMap<int, TemplateParam> m_paramsByNumber;
    QMap<QString, TemplateParam> m_paramsByName;
    
  public:
    
    TemplateParamInfo( ) {
    }
    
    virtual bool getParam( TemplateParam& target, QString name ) const {
      QMap<QString, TemplateParam>::const_iterator it = m_paramsByName.find( name );
      if( it != m_paramsByName.end() ) {
        target = *it;
        return true;
      }
      return false;
    }
    
    virtual bool getParam( TemplateParam& target, int number ) const {
      QMap<int, TemplateParam>::const_iterator it = m_paramsByNumber.find( number );
      if( it != m_paramsByNumber.end() ) {
        target = *it;
        return true;
      }
      return false;
    }
    
    virtual void removeParam( int number ) {
      QMap<int, TemplateParam>::iterator it = m_paramsByNumber.find( number );
      if( it != m_paramsByNumber.end() ) {
        m_paramsByName.remove( (*it).name );
        m_paramsByNumber.remove( it );
      }
    }
    
    virtual void addParam( const TemplateParam& param ) {
      m_paramsByNumber[param.number] = param;
      m_paramsByName[param.name] = param;
    }
  };
  
  int resolutionCount() {
    return m_resolutionCount;
  }
  
  bool hasResolutionFlag( ResolutionFlags flag ) {
    return m_resolutionFlags & flag;
  }
  
  enum Operator {
    ArrowOp,
      StarOp,
      AddrOp,
      IndexOp,
      ParenOp,
      NoOp
  };
  
  typedef CppCodeCompletion::MemberAccessOp MemberAccessOp;
  static CppCodeCompletion* data;
  TypePointer m_masterProxy; ///If this is only a slave of a namespace-proxy, this holds the proxy.
  
  virtual ~SimpleTypeImpl() {
    unreg();
  }
  
  virtual Repository rep() {
    return StringList;
  }
  
    ///Returns the function-interface if this is a function, else zero
  SimpleTypeFunctionInterface* asFunction();
  
  virtual bool isNamespace() const {
    return false;
  };
  
  virtual QString comment() const {
    return "";
  };
  
  
    ///Sets the parent of the given slave to either this class, or the proxy of this class
  virtual void setSlaveParent( SimpleTypeImpl& slave ) {
    if( ! m_masterProxy ) {
      slave.setParent( this );
    } else {
      slave.setParent( m_masterProxy );
    }
  }
  
  void setMasterProxy( TypePointer t ) {
    m_masterProxy = t;
  }
  
    ///@todo remove this and use getTemplateParamInfo instead
  virtual const TypeDesc findTemplateParam( const QString& /*name*/ ) {
    return TypeDesc();
  }
  
  virtual TemplateParamInfo getTemplateParamInfo() {
    return TemplateParamInfo();
  }
  
  virtual void parseParams( TypeDesc desc ) {
    invalidateCache();
    m_desc = desc;
  }
  
  virtual void takeTemplateParams( TypeDesc desc ) {
    invalidateCache();
    m_desc.templateParams() = desc.templateParams();
  }
  
  SimpleTypeImpl( const QStringList& scope ) :  m_resolutionCount(0), m_resolutionFlags(NoFlag), m_scope(scope) {
    checkTemplateParams();
    reg();
  }
    
  SimpleTypeImpl( const TypeDesc& desc ) :  m_resolutionCount(0), m_resolutionFlags(NoFlag), m_desc(desc) {
    m_scope.push_back( m_desc.name() );
    reg();
  }
    
  SimpleTypeImpl( ) : m_resolutionCount(0), m_resolutionFlags(NoFlag)  {
    reg();
  };
  
    ///Returns whether the type is really resolved( corresponds to an item in some model )
  virtual bool hasNode() const {
    return false;
  };
  
  virtual DeclarationInfo getDeclarationInfo() {
    return DeclarationInfo();
  }
  
  virtual TypePointer clone() {
    return new SimpleTypeImpl( this );
  }
  
private:
  int m_resolutionCount;
  ResolutionFlags m_resolutionFlags;
  QValueList<TypeDesc> m_trace; ///pointer to the previous type in the trace-chain
	
  SimpleTypeImpl( const SimpleTypeImpl& /*rhs*/ ) : KShared() {
  }
  
  SimpleTypeImpl& operator = ( const SimpleTypeImpl& /*rhs*/ ) {
  return *this;
  }

  
  void addResolutionFlag ( ResolutionFlags flag ) {
    m_resolutionFlags = (ResolutionFlags) (m_resolutionFlags | flag);
  }
  
  void removeResolutionFlag( ResolutionFlags flag ) {
    m_resolutionFlags = remFlag( m_resolutionFlags, flag );
  }
  
  void increaseResolutionCount() {
    ++m_resolutionCount;
  }
  
  void setResolutionCount( int val ) {
    m_resolutionCount = val;
  }
  
  void reg() {
    SimpleType::registerType( this );
  }
  
  void unreg() {
    SimpleType::unregisterType( this );
  }
  
protected:
  virtual void invalidateCache() {
  };
  
    ///Secondary cache also depends on the surrounding
  virtual void invalidateSecondaryCache() {
  }
  
  virtual void setSecondaryCacheActive( bool active ) {
    Q_UNUSED(active);
  }
  
    ///Primary cache is the one associated only with this object. It must only be cleared
    ///when the object pointed to changes
  virtual void invalidatePrimaryCache() {
  }
  
  static QString operatorToString( Operator op );
  
public:
    /*
    operator QString() const {
        return str();
    }
  
    inline operator QStringList() const {
        return m_scope;
    }*/
  
  int functionDepth() const {
    return m_desc.pointerDepth();
  }
  
  void setFunctionDepth( int pc ) {
    m_desc.setPointerDepth(pc);
  }
  
  void decreaseFunctionDepth() {
    m_desc.decreaseFunctionDepth();
  }
  
  int pointerDepth() const {
    return m_desc.pointerDepth();
  }
  
  void setPointerDepth( int pc ) {
    m_desc.setPointerDepth(pc);
  }
  
  SimpleType getFunctionReturnType( QString functionName, QValueList<SimpleType> params = QValueList<SimpleType>() );
  
    ///Tries to apply the operator and returns the new type. If it fails, it returns an invalid type.
  virtual SimpleType applyOperator( Operator op , QValueList<SimpleType> params = QValueList<SimpleType>() );
  
    ///returns the scope(including own name) as string
  QString str() const {
    if( m_scope.isEmpty() ) return "";
    return m_scope.join( "::" );
  }
  
    ///returns the scope(including own name) as string-list
  inline const QStringList& scope() const {
    return m_scope;
  }
  
    ///sets the parent-type(type this one is nested in)
  void setParent( TypePointer parent ) {
    invalidateSecondaryCache();
    m_parent = parent;
  }
  
    ///returns whether the type has template-parameters, or one of the parent-types has template-parameters.
  bool usingTemplates() const {
    return !m_desc.templateParams().isEmpty() || ( m_parent && m_parent->usingTemplates() );
  }
  
    /** In case of a class, returns all base-types */
  virtual QValueList<SimpleType> getBases() {
    return QValueList<SimpleType>();
  }
  
    ///An abstract class for building types lazily
  struct TypeBuildInfo : public KShared {
    virtual TypePointer build() = 0;
    virtual ~TypeBuildInfo() {
    }
    TypeBuildInfo() {
    }
  private:
    
    TypeBuildInfo& operator =( const TypeBuildInfo& rhs ) {
      Q_UNUSED(rhs);
      return *this;
    }
  TypeBuildInfo( const TypeBuildInfo& rhs ) : KShared() {
    Q_UNUSED(rhs);
  }
  };
    
    ///A class that stores information about a member of some SimpleType
  class MemberInfo {
    KSharedPtr<TypeBuildInfo> m_build;
  public:
    
    enum MemberType {
      NotFound = 0,
        Function = 1,
        Variable = 2,
        Typedef = 4,
        Template = 8,
        NestedType = 16,
        Namespace = 32,
        AllTypes = 0xffffffff
    } memberType;
    
    MemberInfo() {
      memberType = NotFound;
    }
    
    QString memberTypeToString() {
      switch( memberType ) {
      case Namespace:
        return "namespace";
      case Function:
        return "function";
      case Variable:
        return "variable";
      case NotFound:
        return "not found";
      case Typedef:
        return "typedef";
      case Template:
        return "template-parameter";
      case NestedType:
        return "nested-type";
      default:
        return "unknown";
      };
    }
    
    void setBuildInfo( KSharedPtr<TypeBuildInfo> build ) {
      m_build = build;
    }
    
    TypePointer build() {
      if( !m_build)
        return TypePointer();
      else {
        TypePointer r = m_build->build();
        m_build = 0;
        return r;
      }
    }
    
    operator bool() const {
      return memberType != NotFound;
    }
    
    QString name;
    TypeDesc type;
    
        ///This member is only filles for variables!
    DeclarationInfo decl;
  };
  
  enum LocateMode {
    Normal = 1,
      ExcludeTemplates = 2,
      ExcludeTypedefs = 4,
      ExcludeBases = 8,
      ExcludeParents = 16,
      ExcludeNestedTypes = 32,
      ExcludeNamespaces = 64,
      ForgetModeUpwards = 128, ///forgets everything, even NoFail, while passing control to the parent
      LocateBase = 4+8+32+64+128, ///searching in the scope visible while the base-declaration of a class
      NoFail = 256,
      TraceAliases = 512, ///Stores a copy whenever an alias is applied
      OnlyLocalTemplates = 4+8+16+32+64+256,
      OnlyTemplates = 4+8+32+256
  };
  
    ///replaces template-parameters from the given structure with their value-types
  TypeDesc replaceTemplateParams( TypeDesc desc, TemplateParamInfo& paramInfo );
  TypeDesc resolveTemplateParams( TypeDesc desc, LocateMode mode = Normal );
  
    /**By default templates are included while the resolution, so when the type should be addressed from
    outside of the class, ExcludeTemplates should be set as LocateMode, since templates can not be directly accessed    from the outside.
    The resulting type's template-params may not be completely resolved, but can all be resolved locally by that type*/
  
  SimpleType locateType( TypeDesc name , LocateMode mode = Normal, int dir = 0 ,  MemberInfo::MemberType typeMask = bitInvert( addFlag( MemberInfo::Variable, MemberInfo::Function ) ) ) ;
    ///This pair contains the found type, and additionally the member-information that helped finding the type
  struct TypeOfResult {
    SimpleType type;
    DeclarationInfo decl;
    
  TypeOfResult( SimpleType t = SimpleType(), DeclarationInfo d = DeclarationInfo() ) : type( t ), decl( d ) {
  }
    
    SimpleTypeImpl* operator -> () {
      return &(*type);
    }
    
    operator SimpleType() {
      return type;
    }
    
    operator bool() {
      return (bool)type;
    }
  };
  
  virtual TypeOfResult typeOf( const QString& name, MemberInfo::MemberType typ = addFlag( MemberInfo::Function, MemberInfo::Variable) );  
  
  
  virtual MemberInfo findMember( TypeDesc name, MemberInfo::MemberType type = (MemberInfo::MemberType)0xffffffff ) {
    Q_UNUSED(name);
    MemberInfo mem;
    mem.memberType = MemberInfo::NotFound;
    return mem;
  };
  
public:
  
  /**TypeDescs and SimpleTypeImpls usually have a cross-reference, which creates a circular dependency so that they are never freed using KShared. This function breaks the loop, and also breaks all other possible dependency-loops. After this function was called, the type still contains its private information, but can not not be used to resolve anything anymore. This function is called automatically while the destruction of SimpleTypeConfiguration */ 
  virtual void breakReferences();
  
    ///Returns either itself, or the (namespace-)proxy this type is a slave of.
  TypePointer bigContainer();
  
    ///Returns the parent, eg. the SimpleType this one is nested in.
  SimpleType parent();
  
    ///this must be a reference, so the desc can be manipulated in-place from outside
  const TypeDesc& desc();
  
  TypeDesc& descForEdit();
    
    ///short version
  QString fullType() const {
    return m_desc.fullName();
  }
  
    ///returns all information that is available constantly
  QString describe() const {
    QString description =  m_desc.fullName() + " (" + m_scope.join( "::" ) + ")";
    return description;
  }
    
    ///this completely evaluates everything
  QString fullTypeResolved( int depth = 0 );
    
    ///this completely evaluates everything
  QString fullTypeResolvedWithScope( int depth = 0 );
  
private:
  QStringList m_scope;
  TypePointer m_parent;
  
protected:
SimpleTypeImpl( SimpleTypeImpl* rhs ) : m_masterProxy( rhs->m_masterProxy ), m_resolutionCount( rhs->m_resolutionCount ), m_resolutionFlags( rhs->m_resolutionFlags ), m_trace( rhs->m_trace ), m_scope( rhs->m_scope ), m_parent( rhs->m_parent ), m_desc( rhs->m_desc )  {
   reg();
  }
  
  TypeDesc m_desc;  ///descibes the local type(so next() must be null)
  
    /** Tries to extract template-parameters from the scope, resets the params-list */
  virtual void checkTemplateParams ();
  
  void setScope( const QStringList& scope );
    
  TypeOfResult searchBases ( const TypeDesc& name );
};

///Interface that functions should implement
class SimpleTypeFunctionInterface {
    ///Since functions can be overloaded, many functions with the same name can exist. Other functions that belong to this one
    ///should be appended to this.
  SimpleType m_nextFunction;
public:
  
  SimpleTypeFunctionInterface() {
  }
  
  SimpleTypeFunctionInterface( SimpleTypeFunctionInterface* rhs ) {
    m_nextFunction = rhs->m_nextFunction;
  }
  
  void clearNextFunctions() {
    m_nextFunction = SimpleType();
  }
  
  void appendNextFunction( SimpleType func ) {
    if( !func )return;
    if( m_nextFunction && m_nextFunction->asFunction() ) {
      m_nextFunction->asFunction()->appendNextFunction( func );
    } else {
      m_nextFunction = func;
    }
  }
  
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
	SimpleTypeImpl* match( const QValueList<SimpleType>& /*params*/ ) {
        //      QValueList<TypeDesc> args = getArgumentTypes();
    return dynamic_cast<SimpleTypeImpl*>( this );
  }
  
protected:
  
  bool containsUndefinedTemplateParam( TypeDesc& desc, SimpleTypeImpl::TemplateParamInfo& paramInfo );
  
    ///Tries to match the types, filling implicit template-params into paramInfo
  void resolveImplicitTypes( TypeDesc& argType, TypeDesc& gottenArgType, SimpleTypeImpl::TemplateParamInfo& paramInfo );
  
    ///Tries to match the types, filling implicit template-params into paramInfo
  void resolveImplicitTypes( TypeDesc::TemplateParams& argTypes, TypeDesc::TemplateParams& gottenArgTypes, SimpleTypeImpl::TemplateParamInfo& paramInfo );
  
    ///Tries to match the types, filling implicit template-params into paramInfo
  void resolveImplicitTypes( QValueList<TypeDesc>& argTypes, QValueList<TypeDesc>& gottenArgTypes, SimpleTypeImpl::TemplateParamInfo& paramInfo );
};




//typedef SimpleTypeCacheBinder<SimpleTypeImpl> SimpleTypeImpl;

class SimpleTypeCodeModel;
class SimpleTypeCatalog;
class SimpleTypeNamespace;
template <class Base> class SimpleTypeCacheBinder;

typedef SimpleTypeCacheBinder<SimpleTypeCodeModel> SimpleTypeCachedCodeModel;
typedef SimpleTypeCacheBinder<SimpleTypeCatalog> SimpleTypeCachedCatalog;
typedef SimpleTypeCacheBinder<SimpleTypeNamespace> SimpleTypeCachedNamespace;

typedef SimpleTypeCachedCodeModel SimpleTypeUsedCodeModel;
typedef SimpleTypeCachedCatalog SimpleTypeUsedCatalog;
typedef SimpleTypeCachedNamespace SimpleTypeUsedNamespace;

class SimpleTypeCodeModel : public SimpleTypeImpl {
private:
  ItemDom m_item;
  
  bool findItem();
  
  void init();
  
protected:
  SimpleTypeCodeModel() : SimpleTypeImpl() {
  };
  
public:
  
  SimpleTypeCodeModel( SimpleTypeCodeModel* rhs ) : SimpleTypeImpl( rhs ), m_item( rhs->m_item) {
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
    return new SimpleTypeCodeModel( this );
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
  
  virtual const TypeDesc findTemplateParam( const QString& name );
  
    /** In case of a class, returns all base-types */
  virtual QValueList<SimpleType> getBases();
  
  ItemDom locateModelContainer( class CodeModel* m, TypeDesc t, ClassDom cnt = ClassDom() );
  
protected:
  
  struct CodeModelBuildInfo : public TypeBuildInfo {
    ItemDom m_item;
    TypeDesc m_desc;
    TypePointer m_parent;
  
    CodeModelBuildInfo( ItemDom item, TypeDesc& desc, TypePointer parent ) : m_item( item ), m_desc( desc ), m_parent( parent ) {
    }
    
    virtual TypePointer build();
  };
  
  virtual MemberInfo findMember( TypeDesc name , MemberInfo::MemberType type = MemberInfo::AllTypes) ;
};

template <class Base=SimpleTypeImpl>
class SimpleTypeFunction : public Base, public SimpleTypeFunctionInterface {
private:
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
  
  
  virtual SimpleTypeImpl::MemberInfo findMember( TypeDesc name , SimpleTypeImpl::MemberInfo::MemberType type ) 
  {
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
  
  
public:
  
  typedef SimpleTypeImpl* SIP;
  virtual SimpleType applyOperator( typename Base::Operator op , QValueList<SimpleType> params ) {
    Debug d("#apply#");
    if( !d )
      return SimpleType();
    
    if( op == SimpleTypeImpl::ParenOp ) {
            ///First, try to find an overloaded function matching the parameter-types.
      SimpleTypeImpl* f = match( params );
      if( f && f->asFunction() ) {
        dbg() << "applying Operator " << this->operatorToString( op ) << " to \"" << f->desc().fullNameChain() << "\"" <<  endl;
        
        TypeDesc rt = f->asFunction()->getReturnType();
        SimpleTypeImpl::TemplateParamInfo paramInfo = f->getTemplateParamInfo();
        if( containsUndefinedTemplateParam( rt, paramInfo ) ) {
                    /** This is the place where implicit template-function-instatiation takes place.
                     *  Match the given param-types with the argument-types to resolve new template-params.
                    */
          QValueList<TypeDesc> args = getArgumentTypes();
          QValueList<TypeDesc> paramDescs;
          for( QValueList<SimpleType>::iterator it = params.begin(); it != params.end(); ++it )
            paramDescs << (*it)->desc();
          resolveImplicitTypes( args, paramDescs, paramInfo );
                    ///paramInfo now contains the information for all implicit types
        }
        
        return this->parent()->locateType( f->replaceTemplateParams( rt, paramInfo ) );
      } else {
        dbg() << "failed to find a fitting overloaded method" << endl;
      }
    }
    return Base::applyOperator( op, params );
  }   
};

class SimpleTypeCodeModelFunction;
typedef SimpleTypeCodeModelFunction SimpleTypeUsedCodeModelFunction;

class SimpleTypeCodeModelFunction : public SimpleTypeFunction<SimpleTypeCodeModel> {
private:
  FunctionModel* asFunctionModel() {
    if( ! &(*item() ) ) return 0;
    return dynamic_cast<FunctionModel*>( &(*item() ) );
  }
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
  
  virtual TypeDesc getReturnType() {
    if( item() ) {
      if( FunctionModel* m = dynamic_cast<FunctionModel*>( &(*item() ) ) ) {
        return m->resultType();
      }
    }
    
    return TypeDesc();
  }
  
  virtual bool isConst() {
    if( asFunctionModel() )
      return asFunctionModel()->isConstant();
    
    return false;
  }
  
  virtual QValueList<TypeDesc> getArgumentTypes() {
    QValueList<TypeDesc> ret;
    
    if( item() ) {
      if( FunctionModel* m = dynamic_cast<FunctionModel*>( &(*item() ) ) ) {
        ArgumentList l = m->argumentList();
        for( ArgumentList::iterator it = l.begin(); it != l.end(); ++it )
          ret << TypeDesc( (*it)->type() );
      }
    }
    
    return ret;
  }
  
  virtual QStringList getArgumentNames() {
    QStringList ret;
    
    if( item() ) {
      if( FunctionModel* m = dynamic_cast<FunctionModel*>( &(*item() ) ) ) {
        ArgumentList l = m->argumentList();
        for( ArgumentList::iterator it = l.begin(); it != l.end(); ++it )
          ret << (*it)->name();
      }
    }
    
    return ret;
  }
  
  virtual QStringList getArgumentDefaults() {
    QStringList ret;
    
    if( item() ) {
      if( FunctionModel* m = dynamic_cast<FunctionModel*>( &(*item() ) ) ) {
        ArgumentList l = m->argumentList();
        for( ArgumentList::iterator it = l.begin(); it != l.end(); ++it )
          ret << (*it)->defaultValue();
      }
    }
    
    return ret;
  }
  
struct CodeModelFunctionBuildInfo : public TypeBuildInfo {
  FunctionList m_items;
  TypeDesc m_desc;
  TypePointer m_parent;
CodeModelFunctionBuildInfo( FunctionList items, TypeDesc& desc, TypePointer parent ) : m_items( items ), m_desc( desc ), m_parent( parent ) {
}
CodeModelFunctionBuildInfo( FunctionDefinitionList items, TypeDesc& desc, TypePointer parent ) : m_desc( desc ), m_parent( parent ) {
  
  for( FunctionDefinitionList::iterator it = items.begin(); it != items.end(); ++it ) {
    m_items << model_cast<FunctionDom>( *it );
  }
}
  
  virtual TypePointer build() {
    QValueList<TypePointer> ret;
    TypePointer last;
    for( FunctionList::iterator it = m_items.begin(); it != m_items.end(); ++it ) {
      TypePointer tp = new SimpleTypeUsedCodeModelFunction( model_cast<ItemDom>( *it ) );
      tp->takeTemplateParams( m_desc );
      tp->descForEdit().increaseFunctionDepth();
      tp->setParent( m_parent->bigContainer() );
      if( last && last->asFunction() ) last->asFunction()->appendNextFunction( SimpleType( tp) );
      last = tp;
      ret << tp;
    }
    
    if( ret.isEmpty() ) {
      dbg() << "error" << endl;
      return TypePointer();
    } else
      return ret.front();
  }
};
};

/**
The SimpleTypeCodeModel and SimpleTypeCatalog can represent namespaces too,
but they only represent a REAL(physical) namespace and only within either the catalog
or the code-model. This is a proxy-class that handles namespace-aliases, imports, and splits requests to both, the catalog, and the code-model.
There can be multiple namespaces aliased to the same one
*/


class SimpleTypeNamespace : public SimpleTypeImpl {
private:
  QValueList<SimpleType> m_activeSlaves;
    /// Maps local sub-namespace -> global namespace(multiple aliases are possible)
  typedef QMap<QString, QStringList > AliasMap;
  AliasMap m_aliases;
  
  
  void addScope( const QStringList& scope ) {
    invalidateCache();
    SimpleType ct = SimpleType( scope, Catalog );
    SimpleType cm = SimpleType( scope, CodeModel );
    ct->setMasterProxy( this );
    cm->setMasterProxy( this );
    m_activeSlaves << cm;
    m_activeSlaves << ct;
  }
  
  friend class NamespaceBuildInfo;
  
  struct NamespaceBuildInfo : public TypeBuildInfo {
    QStringList m_fakeScope;
    QStringList m_realScope;
    QValueList<QStringList> m_imports;
    
    
    NamespaceBuildInfo( QStringList fakeScope, QStringList realScope, const QValueList<QStringList>& imports ) {
      m_fakeScope = fakeScope;
      m_realScope = realScope;
      m_imports = imports;
    }
    
    virtual TypePointer build();
};
  
protected:
  
  virtual bool hasNode() const
  {
    for( QValueList<SimpleType>::const_iterator it = m_activeSlaves.begin(); it != m_activeSlaves.end(); ++it ) 
      if( (*it)->hasNode() ) return true;
    
    return false;
  };
  
  
  virtual bool isNamespace() const {
    return true;
  }
  
  
  virtual MemberInfo findMember( TypeDesc name, MemberInfo::MemberType type =  MemberInfo::AllTypes);
  
  MemberInfo setupMemberInfo( TypeDesc& subName, QStringList tscope, QValueList<QStringList> imports = QValueList<QStringList>() );
  
  QStringList locateNamespace( QString alias );
  
  
  void recurseAliasMap() ;
  
public:
  
SimpleTypeNamespace( QStringList fakeScope, QStringList realScope = QStringList() ) : SimpleTypeImpl( fakeScope ) {
  if( realScope.isEmpty() ) {
  dbg() << "\"" << str() << "\": created namespace-proxy" << endl;
    addScope( fakeScope );
  } else {
  dbg() << "\"" << str() << "\": created namespace-proxy with real scope \"" << realScope.join("::") << "\"" << endl;
    addScope( realScope );
  }
}
  
SimpleTypeNamespace( SimpleTypeNamespace* ns ) : SimpleTypeImpl( ns ) {
dbg() << "\"" << str() << "\": cloning namespace" << endl;
  m_aliases = ns->m_aliases;
  m_activeSlaves = ns->m_activeSlaves;
}
  
  bool isANamespace( SimpleType& t ) {
    return dynamic_cast<SimpleTypeNamespace*>(&(*t)) != 0;
  }
  
  
  virtual TypePointer clone() {
    
    return new SimpleTypeNamespace( this );
  }
  
  
  QValueList<SimpleType> getSlaves() {
    return m_activeSlaves;
  }
  
  
    ///empty name means an import
  void addAliasMap( QString name, QString alias , bool recurse = true );
  
    ///Takes a map of multiple aliases in form "A=B;C=D;....;" similar to the C++ "namespace A=B;" statement
  void addAliases( QString map );
};

class SimpleTypeCatalog : public SimpleTypeImpl {
private:
  Tag m_tag;
  
  int pointerDepthFromString( const QString& str ) {
    QRegExp ptrRx( "(\\*|\\&)" );
    QString ptr = str.mid( str.find( ptrRx ) );
    QStringList ptrList = QStringList::split( "", ptr );
    return ptrList.size();
  }
  
  Tag findSubTag( const QString& name );
  
  QValueList<Tag> getBaseClassList();
  
  void initFromTag();
  
  void init();
  
protected:
  const Tag& tag() {
    return m_tag;
  }
  
public:
  
  SimpleTypeCatalog() {
  };
  
  virtual DeclarationInfo getDeclarationInfo();
  
  SimpleTypeCatalog( SimpleTypeCatalog* rhs ) : SimpleTypeImpl( rhs ), m_tag( rhs->m_tag ) {
  };
  
  static CppCodeCompletion* data;
  
  virtual bool isNamespace() const {
    return m_tag.kind() == Tag::Kind_Namespace;
  }
  
    /** empty scope means global scope */
  SimpleTypeCatalog( const QStringList& scope ) : SimpleTypeImpl( scope ) {
    init();
  }
    
  SimpleTypeCatalog( SimpleTypeImpl* rhs ) : SimpleTypeImpl( rhs ) {
    init();
  };
  
  SimpleTypeCatalog( Tag& tag ) {
    m_tag = tag;
    initFromTag();
  }
  
  virtual QString comment() const {
    return m_tag.comment();
  };  
  
  virtual TypePointer clone() {
    return new SimpleTypeCatalog( this );
  }
  
  virtual Repository rep() const {
    return Catalog;
  }
  
  virtual bool hasNode() const {
    return (bool)m_tag;
  };
  
  virtual QValueList<SimpleType> getBases();
  
  virtual TemplateParamInfo getTemplateParamInfo();
  
  virtual const TypeDesc findTemplateParam( const QString& name );
  
  
protected:
  
  struct CatalogBuildInfo : public TypeBuildInfo {
    Tag m_tag;
    TypeDesc m_desc;
    TypePointer m_parent;
    
    CatalogBuildInfo( Tag tag , TypeDesc& desc, TypePointer parent ) : m_tag( tag ) , m_desc( desc ), m_parent( parent )   {
    }
    
    virtual TypePointer build();
  };
  
  
  virtual MemberInfo findMember( TypeDesc name, MemberInfo::MemberType type = MemberInfo::AllTypes);
};





class SimpleTypeCatalogFunction;
typedef SimpleTypeCatalogFunction SimpleTypeUsedCatalogFunction;

class SimpleTypeCatalogFunction : public SimpleTypeFunction<SimpleTypeCatalog> {
private:
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
  
  virtual TypePointer build() {
    QValueList<TypePointer> ret;
    TypePointer last;
    for( QValueList<Tag>::iterator it = m_tags.begin(); it != m_tags.end(); ++it ) {
      TypePointer tp = new SimpleTypeUsedCatalogFunction( *it );
      tp->takeTemplateParams( m_desc );
      tp->descForEdit().increaseFunctionDepth();
      if( m_parent ) tp->setParent( m_parent->bigContainer() );
      if( last && last->asFunction() ) last->asFunction()->appendNextFunction( SimpleType( tp) );
      last = tp;
      ret << tp;
    }
    
    if( ret.isEmpty() ) {
      dbg() << "error" << endl;
      return TypePointer();
    }
    return ret.front();
  }
};
  
  virtual TypeDesc getReturnType() {
    if( tag() ) {
      return tagType( tag() );
    }
    
    return TypeDesc();
  }
  
  virtual bool isConst () {
    Tag t = tag();
    CppFunction<Tag> tagInfo( t );
    return tagInfo.isConst();
  }
  
  
  virtual QStringList getArgumentDefaults() {
    return QStringList();
  }
  
  virtual QStringList getArgumentNames() {
    QStringList ret;
    Tag t = tag();
    CppFunction<Tag> tagInfo( t );
    return tagInfo.argumentNames();
  }
  
  virtual QValueList<TypeDesc> getArgumentTypes() {
    QValueList<TypeDesc> ret;
    Tag t = tag();
    CppFunction<Tag> tagInfo( t );
    QStringList arguments = tagInfo.arguments();
    for( QStringList::iterator it = arguments.begin(); it != arguments.end(); ++it )
      ret << TypeDesc( *it );
    return ret;
  }
};



#endif 
// kate: indent-mode csands; tab-width 4;

