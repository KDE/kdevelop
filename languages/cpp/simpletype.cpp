
/***************************************************************************
   begin                : Sat Jul 21 2001
   copyright            : (C) 2001 by Victor R�er
   email                : victor_roeder@gmx.de
   copyright            : (C) 2002,2003 by Roberto Raggi
   email                : roberto@kdevelop.org
   copyright            : (C) 2005 by Adam Treat
   email                : manyoso@yahoo.com
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

#include "simpletype.h"
#include "simpletypecachebinder.h"
#include "safetycounter.h"

extern SafetyCounter safetyCounter;

TypePointer SimpleType::m_globalNamespace;
SimpleType::TypeStore  SimpleType::m_typeStore;
bool SimpleType::m_unregistered = false;
QString globalCurrentFile = "";
CppCodeCompletion* cppCompetionInstance = 0;


//SimpleType implementation

void SimpleType::resolve( Repository rep )  const {
 if( !m_resolved ) {
  if( m_globalNamespace ) {
   if( (rep == Undefined || rep == Both) ) {
    m_resolved = true;
    if( scope().isEmpty() ) {
     m_type = m_globalNamespace;
    } else {
     SimpleType t = m_globalNamespace->locateType( scope().join("::") );
     if( t ) {
      m_type = &( *t );
      return;
     } else {
      dbg() << "\"" << scope().join("::") << "\": The type could not be located in the global scope while resolving it" << endl;
     }
    }
   }
  } else {
   dbg() << "warning: no global namespace defined! " << endl;
  }
  
  TypePointer cm;
  
  if( rep == Undefined || rep == CodeModel ) {
   if( !m_type ) {
    cm = TypePointer( new SimpleTypeUsedCodeModel( scope() ) );
   }else{
    cm = TypePointer( new SimpleTypeUsedCodeModel( &(*m_type) ) );
   }
   
   if( cm->hasNode() || rep == CodeModel ) {
    if( cm->hasNode() ) {
     dbg() << "resolved \"" << str() << "\" from the code-model" << endl;
     if( cm->isNamespace() &&rep != CodeModel ) {
     dbg() << "\"" << str() << "\": is namespace, resolving proxy" << endl; 
      resolve( Both );
      return;
     }
    }else{
     dbg() << "forced \"" << str() << "\" to be resolved from code-model" << endl;
    }
    m_type = cm;
    m_resolved = true;
    return;
   } 
  }
  if( rep == Undefined || rep == Catalog ) {
   
   if( !m_type ) {
    cm = TypePointer( new SimpleTypeUsedCatalog( scope() ) );
   }else{
    cm = TypePointer( new SimpleTypeUsedCatalog( &(*m_type) ) );
   }
   
   if( cm->hasNode() || rep == Catalog ) {
    if( cm->hasNode() ) {
     dbg() << "resolved \"" << str() << "\" from the catalog" << endl;
     if( cm->isNamespace() && rep != Catalog ) {
     dbg() << "\"" << str() << "\": is namespace, resolving proxy" << endl; 
      resolve( Both );
      return;
     }
    }else{
     dbg() << "forced \"" << str() << "\" to be resolved from catalog" << endl;
    }
    m_type = cm;
    m_resolved = true;
    return;
   }
  }
  
  if( rep == Both ) {
   cm = new SimpleTypeUsedNamespace( scope() );
   m_type = cm;
   m_resolved = true;
   return;
  }
  
  m_resolved = true;
  dbg() << "could not resolve \"" << m_type->desc().fullNameChain() << "\"" << endl;
 }
}

void SimpleType::destroyStore()
{
 resetGlobalNamespace();
 m_unregistered = true;
 SafetyCounter s( 10000 );
 while( m_unregistered && s ) {
  m_unregistered = false;
  
  for( TypeStore::iterator it = m_typeStore.begin(); it != m_typeStore.end(); ++it ) {
   {
    TypePointer tp = *it;
    tp->breakReferences();
   }
   if( m_unregistered ) break;
  }
 }
 
 if( !m_typeStore.empty() ) {
  kdDebug( 9007 ) << "type-store is not empty, " << m_typeStore.size() << " types are left" << endl;
  for( TypeStore::iterator it = m_typeStore.begin(); it != m_typeStore.end(); ++it ) {
   kdDebug( 9007 ) << "type left: " << (*it)->describe() << endl;
  }
 }
 
 m_typeStore.clear();
}

/*bool SimpleType::operator < ( SimpleType& rhs ) {
 return rhs->desc().deeper( (*this)->desc() );
}*/

///This does not necessarily make the TypeDesc's private, so before editing them
///their makePrivate must be called too
void SimpleType::makePrivate() {
 m_type = m_type->clone();
}

const QStringList& SimpleType::scope() const {
 return m_type -> scope();
}

const QString SimpleType::str() const {
 return m_type -> str();
}

void SimpleType::init( const QStringList& scope , Repository rep ) {
 
 m_type = TypePointer( new SimpleTypeImpl( scope ) );
 if( rep != Undefined) resolve( rep );
}

SimpleType::SimpleType( ItemDom item ) : m_resolved(true) {
 m_type = TypePointer( new SimpleTypeUsedCodeModel( item ) );
}
/*
SimpleType::SimpleType( Tag tag ) : m_resolved(true) {
 m_type = TypePointer( new SimpleTypeCatalog( tag ) );
}*/


//SimpleTypeImpl implementation

/**
Searches for a member called "name", considering all types selected through "typ"
TODO: cache this too */
SimpleTypeImpl::TypeOfResult SimpleTypeImpl::typeOf( const QString& name, MemberInfo::MemberType typ  ) {
 Debug d( "#to#" );
 if( !d ) {
  dbg() << "stopping typeOf-evaluation because the recursion-depth is too high" << endl;
  return SimpleType( QStringList("CompletionError::too_much_recursion") );
 }
 dbg() << "\"" << str() << "\"------------>: searching for type of member \"" << name << "\"" << endl;
 
 TypeDesc td = resolveTemplateParams( name );
 
 MemberInfo mem = findMember( td, typ );
 
 if( mem ) {
 dbg() << "\"" << str() << "\": found member " << name << ", type: " << mem.type.fullNameChain() << endl;
  if( mem.memberType == MemberInfo::Function ) {
   ///For functions, find all functions with the same name, so that overloaded functions can be identified correctly
   TypePointer ret = mem.build();
   if( ret && ret->asFunction() ) {
     ///Search all bases and append all functions with the same name to it.
    QValueList<SimpleType> bases = getBases();
    for( QValueList<SimpleType>::iterator it = bases.begin(); it != bases.end(); ++it ) {
     SimpleType rt = (*it)->typeOf( name );
     if( rt )
      ret->asFunction()->appendNextFunction( rt );
    }
    return SimpleType( ret );
   } else {
    dbg() << "error, using old function-type-evaluation" << endl;
    SimpleType ret = locateType( mem.type );
    if( !ret ) {
     return TypeOfResult( SimpleType( new SimpleTypeImpl( ret->desc() ) ), mem.decl );  ///Try to return a "pseudo-valid" type that is as close as possible to the real type,
    } else {
     return TypeOfResult( ret, mem.decl );
    }
   }
  } else if( mem.memberType == MemberInfo::Variable ) {
   SimpleType ret = locateType( mem.type );
   if( !ret ) {
    return TypeOfResult( SimpleType( new SimpleTypeImpl( ret->desc() ) ), mem.decl );  ///Try to return a "pseudo-valid" type that is as close as possible to the real type,
   } else {
    return TypeOfResult( ret, mem.decl );
   }
  } else {
  dbg() << "while searching for the type of \"" << name << "\" in \"" << str() << "\": member has wrong type: \"" << mem.memberTypeToString() << "\"" << endl;
   return TypeOfResult();
  }
 }
 
 TypeOfResult ret = searchBases( td );
 if( !ret )
  dbg() << "\"" << str() << "\"------------>: failed to resolve the type of member \"" << name << "\"" << endl;
 else
  dbg() << "\"" << str() << "\"------------>: successfully resolved the type of the member \"" << name << "\"" << endl;
 return ret;
} 

SimpleTypeFunctionInterface* SimpleTypeImpl::asFunction()
{
 return dynamic_cast<SimpleTypeFunctionInterface*> ( this );
}


QString SimpleTypeImpl::operatorToString( Operator op ) {
    switch( op ) {
    case NoOp:
      return "NoOp";
    case IndexOp:
      return "index-operator";
    case ArrowOp:
      return "arrow-operator";
    case StarOp:
      return "star-operator";
    case AddrOp:
      return "address-operator";
    case ParenOp:
      return "paren-operator";
    default:
      return QString("%1").arg((long)op);
    };
}

SimpleType SimpleTypeImpl::getFunctionReturnType( QString functionName, QValueList<SimpleType> params) {
    SimpleType t = typeOf( functionName, MemberInfo::Function );
    if( t->asFunction() ) {
      t = t->applyOperator( ParenOp, params );
      return t;
    } else {
      dbg() << "error " << endl;
      return SimpleType();
    }
}

SimpleType SimpleTypeImpl::applyOperator( Operator op , QValueList<SimpleType> params ) {
    if( !safetyCounter ) return SimpleType();
    
    Debug d("#applyn#");
    if( !d )
      return SimpleType();
    
    dbg() << "applying operator " << operatorToString( op ) << " to \"" << desc().fullNameChain() << "\"" <<  endl;
    SimpleType ret( this );
    if( op == NoOp ) return ret;
    
    switch( op ) {
    case IndexOp:
                ///one pointer less, if there is no pointer, try using the index-operator
      if( ret->pointerDepth() > 0 ) {
        ret.makePrivate();
        ret->setPointerDepth( ret->pointerDepth() - 1 );
        return ret;
      }else{
        return ret->getFunctionReturnType( "operator [ ]", params );
      }
      break;
    case StarOp:
                ///one pointer less, if there is no pointer, try using the star-operator
      if( ret->pointerDepth() > 0 ) {
        ret.makePrivate();
        ret->setPointerDepth( ret->pointerDepth() - 1 );
        return ret;
      }else{
        return ret->getFunctionReturnType( "operator *", params );
      }
      break;
    case AddrOp:
      ret.makePrivate();
      ret->setPointerDepth( ret->pointerDepth() + 1 );
      return ret;
      break;
    case ArrowOp:
                ///one pointer less, if there is no pointer, try using the arrow-operator
      if( ret->pointerDepth() > 0 ) {
        ret.makePrivate();
        ret->setPointerDepth( ret->pointerDepth() - 1 );
        return ret;
      }else{
                /** Dereference one more because the type must be a pointer */
        ret = ret->getFunctionReturnType( "operator ->", params );
        ret.makePrivate();
        if( ret->pointerDepth() ) {
          ret->setPointerDepth( ret->pointerDepth() - 1 );
        } else {
        dbg() << "\"" << str() << "\": " << " \"operator ->\" returns a type with the wrong pointer-depth" << endl;
        }
        return ret;
      }
      break;
    case ParenOp:
            ///functions are also marked by giving them a pointer-depth of return-type + 1
            ///this marking works good, since it isn't possible to call functions on pointers
      if( ret->functionDepth() > 0 ) {
        ret.makePrivate();
        ret->decreaseFunctionDepth();
        return ret;
      }else{
                /** Dereference one more because the type must be a pointer */
        return ret->getFunctionReturnType( "operator ( )", params );
      }
    default:
      dbg() << "wrong operator\n";
    }
    
    return SimpleType();
}
 

TypeDesc SimpleTypeImpl::replaceTemplateParams( TypeDesc desc, TemplateParamInfo& paramInfo ) {
    Debug d("#repl#");
    if( !d ) 
      return desc;
    
    TypeDesc ret = desc;
    if( !ret.hasTemplateParams() && !ret.next()) {
      TemplateParamInfo::TemplateParam t;
      if( paramInfo.getParam( t, desc.name() ) ) {
        
        if( t.value )
          ret = t.value;
        else if( t.def )
          ret = t.def;
        
        if( ret.name() != desc.name() ) ret.setPointerDepth( ret.pointerDepth() + desc.pointerDepth() );
      }
    } else {
      TypeDesc::TemplateParams& params = ret.templateParams();
      for( TypeDesc::TemplateParams::iterator it = params.begin(); it != params.end(); ++it ) {
        *it = new TypeDescShared( replaceTemplateParams( **it, paramInfo ) );
      }
    }
    
    if( ret.next() ) {
      ret.setNext( new TypeDescShared( replaceTemplateParams( *ret.next(), paramInfo ) ) );
    }
    
    return ret;
}       
  
TypeDesc SimpleTypeImpl::resolveTemplateParams( TypeDesc desc, LocateMode mode ) {
    Debug d("#resd#");
    if( !d )
      return desc;
    
    TypeDesc ret = desc;
    if( ret.hasTemplateParams() ) {
      TypeDesc::TemplateParams& params = ret.templateParams();
      for( TypeDesc::TemplateParams::iterator it = params.begin(); it != params.end(); ++it ) {
        if( !(*it)->resolved() ) {
          SimpleType t = locateType( **it, mode );
          *it = new TypeDescShared( t->desc() );
        }
      }
    }
    
    if( ret.next() ) {
      ret.setNext( new TypeDescShared( resolveTemplateParams( *ret.next(), mode ) ) );
    }
    
    return ret;
}

SimpleType SimpleTypeImpl::locateType( TypeDesc name , LocateMode mode , int dir ,  MemberInfo::MemberType typeMask ) {
    Debug d("#lo#");
    if( !name || !safetyCounter || !d ) {
      return SimpleType( this );
    }
    if( !d ) {
      dbg() << "stopping location because the recursion-depth is too high" << endl;
      return SimpleType( QStringList("CompletionError::too_much_recursion") );
    }
  dbg() << "\"" << desc().fullName() << "\": locating type \"" << name.fullNameChain() << "\"" << endl;
    
    if( name.resolved() && name.length() == name.resolved()->desc().length() ) {
    dbg() << "\"" << desc().fullName() << "\": type \"" << name.fullNameChain() << "\" is already resolved, returning stored instance" << endl;
      SimpleType ret = SimpleType( name.resolved() );
      
      if( ! (name == ret->desc()) ) {
        ret.makePrivate();  ///Maybe some small parameters like the pointer-depth were changed, so customize those
        ret->parseParams( name );
      }
      
      return ret;
    }
    
    SimpleType ret( new SimpleTypeImpl() ); ///In case the type cannot be located, this helps to find at least the best match
    ret->parseParams( name );
    
    MemberInfo mem = findMember( name.name(), typeMask );
    
    switch( mem.memberType ) {
    case MemberInfo::Namespace:
            ///TODO: Namespaces can be on the code-model as well as global
      if( mode & ExcludeNamespaces ) break;
    case MemberInfo::NestedType:
      {
        if( mem.memberType == MemberInfo::NestedType && mode & ExcludeNestedTypes ) break;
        
        name = resolveTemplateParams( name, Normal );  ///param-names may be shadowed, so everything must be resolved now
        
        SimpleType sub;
        if( TypePointer t = mem.build() ) {
          sub = SimpleType( t );
          setSlaveParent( *sub );
          sub->parseParams( name );
          sub->descForEdit().setNext( 0 );
        }else {
                    ///Should not happen..
        kdDebug( 9007 ) << "\"" << str() << "\": Warning: the nested-type " << name.name() << " was found, but has no build-info" << endl;
          return SimpleType();
        }
        
        TypeDescPointer rest = name.next();
        if( rest ) {
        dbg() << "\"" << str() << "\": found nested-type \"" << name.name() << "\", passing control to it\n";
          SimpleType rett = sub->locateType( *rest, addFlag( mode, ExcludeTemplates ), 1 ); ///since template-names cannot be referenced from outside, exclude them for the first cycle
          if( rett ) {
            return rett;
          } else {
            ret = rett; ///When there is no perfect match, at least create a helpful best match
            if( !ret -> hasResolutionFlag( HadAlias ) ) {
              ret.makePrivate();
              TypeDesc& dsc = ret->descForEdit();
              dsc.makePrivate();
              dsc = sub->desc() + dsc;
              ret->increaseResolutionCount();
            }
          }
        } else {
        dbg() << "\"" << str() << "\": successfully located searched type \"" << name.fullNameChain() << "\"\n";
          return sub;
        }
        break;
      }
    case MemberInfo::Typedef:
      if( mode & ExcludeTypedefs ) break;
    case MemberInfo::Template:
      {
        if( mem.memberType == MemberInfo::Template && (mode & ExcludeTemplates) ) break;
      dbg() << "\"" << str() << "\": found "<< mem.memberTypeToString() << " \"" << name.name() << "\" -> \"" << mem.type.fullNameChain() << "\", recursing \n";
        if( name.hasTemplateParams() ) {
        dbg() << "\"" << str() << "\":warning: \"" << name.fullName() << "\" is a " << mem.memberTypeToString() << ", but it has template-params itself! Not matching" << endl;
        } else {
          if( mem.type.name() != name.name() ) {
            TypeDesc memTypeBackup = mem.type;
            mem.type.clearInstanceInfo();   ///The cache-lookup also respects the instance-info, so back it up, clear it, and apply it afterwards, to get more cache-hits.
            
            mem.type.makePrivate();
            mem.type.append( name.next() );
            
            
            ret = locateType( mem.type, remFlag( mode, ExcludeTemplates ) ); ///it is becoming a locally defined type so template-params become relevant
            
            ret.makePrivate();
            ret->descForEdit().makePrivate();
            ret->descForEdit().takeInstanceInfo( memTypeBackup );
            ret->descForEdit().takeInstanceInfo( name );
            
            if( mode & TraceAliases ) ret->tracePrepend( name );
            
            if( mem.memberType == MemberInfo::Template )
              ret->addResolutionFlag( HadTemplate );
            if( mem.memberType == MemberInfo::Typedef )
              ret->addResolutionFlag( HadTypedef );
            ret->increaseResolutionCount();
            return ret;
          } else {
          dbg() << "\"" << str() << "\"recursive typedef/template found: \"" << name.fullNameChain() << "\" -> \"" << mem.type.fullNameChain() << "\"" << endl;
          }
        }
        break;
      }
    }
    
        ///Ask bases but just on this level
    if( ! ( mode & ExcludeBases ) ) {
      
      QValueList<SimpleType> bases = getBases();
      if( !bases.isEmpty() ) {
        TypeDesc nameInBase = resolveTemplateParams( name, LocateBase ); ///Resolve all template-params that are at least visible in the scope of the base-declaration              
        for( QValueList<SimpleType>::iterator it = bases.begin(); it != bases.end(); ++it ) {
          SimpleType t = (*it)->locateType( nameInBase, addFlag( addFlag( mode, ExcludeTemplates ), ExcludeParents ), dir ); ///The searched Type cannot directly be a template-param in the base-class, so ExcludeTemplates. It's forgotten early enough.
          if( t )
            return t;
          else
            if( t->resolutionCount() > ret->resolutionCount() )
              ret = t;
        }
      }
    }
    
        ///Ask parentsc
    if( !scope().isEmpty() && dir != 1 && ! ( mode & ExcludeParents ) ) {
    SimpleType rett = parent()->locateType( resolveTemplateParams( name, mode & ExcludeBases ? ExcludeBases : mode ), mode & ForgetModeUpwards ? Normal : mode );
      if( rett ) 
        return rett;
      else
        if( rett->resolutionCount() > ret->resolutionCount() )
          ret = rett;
    }
    
        ///Ask the bases and allow them to search in their parents.
    if( ! ( mode & ExcludeBases ) ) {
      TypeDesc baseName = resolveTemplateParams( name, LocateBase ); ///Resolve all template-params that are at least visible in the scope of the base-declaration
      QValueList<SimpleType> bases = getBases();
      if( !bases.isEmpty() ) {
        for( QValueList<SimpleType>::iterator it = bases.begin(); it != bases.end(); ++it ) {
          SimpleType t = (*it)->locateType( baseName, addFlag( mode, ExcludeTemplates ), dir ); ///The searched Type cannot directly be a template-param in the base-class, so ExcludeTemplates. It's forgotten early enough.
          if( t )
            return t;
          else
            if( t->resolutionCount() > ret->resolutionCount() )
              ret = t;
        }
      }
    }
    
        ///Give the type a desc, so the nearest point to the searched type is stored
  dbg() << "\"" << str() << "\": search for \"" << name.fullNameChain() << "\" FAILED" << endl;
    return ret;
  };
  

void SimpleTypeImpl::breakReferences() {
    m_parent = 0;
    m_desc.resetResolved();
    m_trace.clear();
    m_masterProxy = 0;
    invalidateCache();
}

TypePointer SimpleTypeImpl::bigContainer() {
    if( m_masterProxy )
      return m_masterProxy;
    else
      return TypePointer( this );
}

SimpleType SimpleTypeImpl::parent() {
    if ( m_parent ) {
            //dbg() << "\"" << str() << "\": returning parent" << endl;
      return SimpleType( m_parent );
    } else {
    dbg() << "\"" << str() << "\": locating parent" << endl;
      invalidateSecondaryCache();
      QStringList sc = scope();
      
      if( !sc.isEmpty() ) {
        sc.pop_back();
        SimpleType r = SimpleType( sc );
        m_parent = r.get();
        return r;
      } else {
      dbg() << "\"" << str() << "\"warning: returning parent of global scope!" << endl;
        return SimpleType( new SimpleTypeImpl("") );
      }
    }
}

const TypeDesc& SimpleTypeImpl::desc() {
    if( ! scope().isEmpty() );
    if( m_desc.name().isEmpty() )
      m_desc.setName( cutTemplateParams( scope().back()) );
    m_desc.setResolved( this );
    return m_desc;
}

TypeDesc& SimpleTypeImpl::descForEdit()  {
    desc();
    invalidateCache();
    return m_desc;
}

QString SimpleTypeImpl::fullTypeResolved( int depth ) {
    Debug d("#tre#");
    
    TypeDesc t = desc();
    if( !scope().isEmpty() ) {
      if( depth > 10 ) return "KDevParseError::ToDeep";
      if( !safetyCounter ) return "KDevParseError::MaximumCountReached";
      
      dbg() << "fully resolving type " << t.fullName() << endl;
      if( scope().size() != 0 ) {
        t = resolveTemplateParams( t, LocateBase ); 
      }
    }
    
    return t.fullNameChain();
}

QString SimpleTypeImpl::fullTypeResolvedWithScope( int depth ) {
    Q_UNUSED(depth);
    if( parent() ) {
      return parent()->fullTypeResolvedWithScope() + "::" + fullTypeResolved();
    } else {
      return fullTypeResolved();
    }
}

void SimpleTypeImpl::checkTemplateParams () {
    invalidateCache();
    if( ! m_scope.isEmpty() ) {
      QString str = m_scope.back();
      m_desc = str;
      m_scope.pop_back();
      m_scope << m_desc.name();
    }
}

void SimpleTypeImpl::setScope( const QStringList& scope ) {
    invalidateCache();
    m_scope = scope;
}

SimpleTypeImpl::TypeOfResult SimpleTypeImpl::searchBases ( const TypeDesc& name ) {
    QValueList<SimpleType> parents = getBases();
    for ( QValueList<SimpleType>::iterator it = parents.begin(); it != parents.end(); ++it )
    {
      TypeOfResult type = (*it)->typeOf( name.name() );
      if ( type )
        return type;
    }
    return TypeOfResult();
}


//SimpleTypeFunctionInterface implementation
  
QString SimpleTypeFunctionInterface::signature() 
  {
    QString sig = "( ";
    SimpleTypeImpl* asType = dynamic_cast<SimpleTypeImpl*>( this );
    
    QStringList argDefaults = getArgumentDefaults();
    QStringList argNames = getArgumentNames();
    QValueList<TypeDesc> argTypes = getArgumentTypes();
    QValueList<SimpleType> argRealTypes;
    
    if( asType ) {
      for( QValueList<TypeDesc>::iterator it = argTypes.begin(); it != argTypes.end(); ++it ) {
        argRealTypes << asType->locateType( *it );
      }
    }
    
    QStringList::iterator def = argDefaults.begin();
    QStringList::iterator name = argNames.begin();
    QValueList<SimpleType>::iterator realType = argRealTypes.begin();
    
    while( realType != argRealTypes.end() ) {
      if( sig != "( " )
        sig += ", ";
      
      sig += (*realType)->fullTypeResolved();
      ++realType;
      
      if( name != argNames.end() ) {
        if( !(*name).isEmpty() ) sig += " " + *name;
        ++name;
      }
      
      if( def != argDefaults.end() && !(*def).isEmpty() ) {
        sig += " = " + *def;
        ++def;
      }
    }
    
    sig += " )";
    return sig;
}

bool SimpleTypeFunctionInterface::containsUndefinedTemplateParam( TypeDesc& desc, SimpleTypeImpl::TemplateParamInfo& paramInfo ) 
{
  TypeDesc::TemplateParams& pm = desc.templateParams();
  SimpleTypeImpl::TemplateParamInfo::TemplateParam t;
  
  if( pm.isEmpty() && paramInfo.getParam( t, desc.name() ) )
    if( !t.value ) return true;
  
  if( desc.next() )
    if( containsUndefinedTemplateParam( *desc.next(), paramInfo ) )
      return true;
  
  for( TypeDesc::TemplateParams::iterator it = pm.begin(); it != pm.end(); ++it ) {
    if( containsUndefinedTemplateParam( **it, paramInfo ) ) return true;
  }
  
  return false;
}
  

void SimpleTypeFunctionInterface::resolveImplicitTypes( TypeDesc& argType, TypeDesc& gottenArgType, SimpleTypeImpl::TemplateParamInfo& paramInfo ) 
{
  if( argType.templateParams().isEmpty() ) {  ///Template-types may not be templates.
    SimpleTypeImpl::TemplateParamInfo::TemplateParam p;
    if( paramInfo.getParam( p, argType.name() ) && !p.value ) {
      dbg() << "choosing \"" << gottenArgType.fullNameChain() << "\" as implicit template-parameter for \"" << argType.name() << "\"" << endl;
      p.value = gottenArgType;
      p.value.makePrivate();
      for( int d = 0; d < argType.pointerDepth(); d++ )
        p.value.decreasePointerDepth();
      
      paramInfo.addParam( p );
    }
  } else {
    if( argType.name() == gottenArgType.name() )
      resolveImplicitTypes( argType.templateParams(), gottenArgType.templateParams(), paramInfo );
  }
}     

void SimpleTypeFunctionInterface::resolveImplicitTypes( TypeDesc::TemplateParams& argTypes, TypeDesc::TemplateParams& gottenArgTypes, SimpleTypeImpl::TemplateParamInfo& paramInfo ) 
{
  TypeDesc::TemplateParams::iterator it = argTypes.begin();
  TypeDesc::TemplateParams::iterator it2 = gottenArgTypes.begin();
  
  while( it != argTypes.end() && it2 != gottenArgTypes.end() ) {
    resolveImplicitTypes( **it, **it2, paramInfo );
    ++it;
    ++it2;
  }
}   

void SimpleTypeFunctionInterface::resolveImplicitTypes( QValueList<TypeDesc>& argTypes, QValueList<TypeDesc>& gottenArgTypes, SimpleTypeImpl::TemplateParamInfo& paramInfo ) 
{
  QValueList<TypeDesc>::iterator it = argTypes.begin();
  QValueList<TypeDesc>::iterator it2 = gottenArgTypes.begin();
  
  while( it != argTypes.end() && it2 != gottenArgTypes.end() ) {
    resolveImplicitTypes( *it, *it2, paramInfo );
    ++it;
    ++it2;
  }
}   


//SimpleTypeCatalog implementation

SimpleTypeImpl::MemberInfo SimpleTypeCatalog::findMember( TypeDesc name, SimpleTypeImpl::MemberInfo::MemberType type ) 
{
  MemberInfo ret;
  ret.name = name.name();
  ret.memberType = MemberInfo::NotFound;
  if( !name ) return ret;
  
  if(  ( type & MemberInfo::Template) ) {
    TypeDesc s = findTemplateParam( name.name() );
    if( s ) {
      ret.memberType = MemberInfo::Template;
      ret.type = s;
    }
  }
  
  QValueList<Catalog::QueryArgument> args;
  QTime t;
  
  t.start();
  args << Catalog::QueryArgument( "scope", scope() );
  args << Catalog::QueryArgument( "name", name.name() );
  
  QValueList<Tag> tags( cppCompetionInstance->m_repository->query( args ) );
  
  if( tags.isEmpty() )  return ret;
  
  Tag tag = tags.front();
  
  if( tag.kind() == Tag::Kind_Variable && (type & MemberInfo::Variable) ) {
    ret.memberType = MemberInfo::Variable;
    ret.type = tagType( tag );
    ret.decl.name = tag.name();
    ret.decl.comment = tag.comment();
    tag.getStartPosition( &ret.decl.startLine, &ret.decl.startCol );
    tag.getEndPosition( &ret.decl.endLine, &ret.decl.endCol );
    ret.decl.file = tag.fileName();
  } else if( (tag.kind() == Tag::Kind_FunctionDeclaration || tag.kind() == Tag::Kind_Function)  && ( type & MemberInfo::Function ) ) {
    ret.memberType = MemberInfo::Function;
    ret.type = tagType( tag );
    ret.type.increaseFunctionDepth();
    ret.setBuildInfo( new SimpleTypeCatalogFunction::CatalogFunctionBuildInfo( tags, name, TypePointer( this ) ) );
  } else if( tag.kind() == Tag::Kind_Typedef && ( type & MemberInfo::Typedef ) ) {
    ret.memberType = MemberInfo::Typedef;
    ret.type = tagType( tag );
  } else if ( tag.kind() == Tag::Kind_Namespace && ( type & MemberInfo::Namespace ) ){
    ret.setBuildInfo( new CatalogBuildInfo( tag , name, TypePointer( this ) ) );
    ret.memberType = MemberInfo::Namespace;
    ret.type = name;
  } else if ( tag.kind() == Tag::Kind_Class && ( type & MemberInfo::NestedType ) ){
    ret.setBuildInfo( new CatalogBuildInfo( tag, name, TypePointer( this ) ) );
    ret.memberType = MemberInfo::NestedType;
    ret.type = name;
  }
  
///Check if it is a template-name
  
  if( !ret.type) ret.memberType = MemberInfo::NotFound;
  return ret;
}


Tag SimpleTypeCatalog::findSubTag( const QString& name ) {
  if( name.isEmpty() ) return Tag();
  
  QValueList<Catalog::QueryArgument> args;
  QTime t;
  
  t.start();
  args << Catalog::QueryArgument( "scope", scope() );
  args << Catalog::QueryArgument( "name", name );
  
  QValueList<Tag> tags( cppCompetionInstance->m_repository->query( args ) );
  if( ! tags.isEmpty() ) {
            //dbg() << "findTag: \"" << str() << "\": tag \"" << name << "\" found " << endl;
    return tags.front();
  }else {
            //dbg() << "findTag: \"" << str() << "\": tag \"" << name << "\" not found " << endl;
    return Tag();
  }
}

QValueList<Tag> SimpleTypeCatalog::getBaseClassList( )
{
  if ( scope().isEmpty() )
    return QValueList<Tag>();
  return cppCompetionInstance->m_repository->getBaseClassList( scope().join("::"));
}

void SimpleTypeCatalog::initFromTag() {
  QStringList l = m_tag.scope();
  l << m_tag.name();
  setScope( l );
}

void SimpleTypeCatalog::init() {
  if( !scope().isEmpty() ) {
    QStringList l = scope();
    QStringList cp = l;
    cp.pop_back();
    setScope( cp );
    m_tag = findSubTag( l.back() );
    setScope( l );
    initFromTag( );
  }
}

DeclarationInfo SimpleTypeCatalog::getDeclarationInfo() {
  DeclarationInfo ret;
  
  ret.name = fullTypeResolved();
  if( m_tag ) {
    ret.file = m_tag.fileName();
    m_tag.getStartPosition( &ret.startLine, &ret.startCol );
    m_tag.getEndPosition( &ret.endLine, &ret.endCol );
    ret.comment = m_tag.comment();
  }
  
  return ret;
}

QValueList<SimpleType> SimpleTypeCatalog::getBases() {
  Debug d( "#getbases#" );
  if( !d ) {
  dbg() << "\"" << str() << "\": recursion to deep while getting bases" << endl;
    return QValueList<SimpleType>();
  }
  
  QValueList<SimpleType> ret;
                // try with parentsc
  QTime t;
  t.restart();
  QValueList<Tag> parents( getBaseClassList() );
  
  QValueList<Tag>::Iterator it = parents.begin();
  while ( it != parents.end() )
  {
    Tag & tag = *it;
    ++it;
    
    CppBaseClass<Tag> info( tag );
    
    ret << locateType( info.baseClass() , LocateBase );
  }
  return ret;
}

SimpleTypeImpl::TemplateParamInfo SimpleTypeCatalog::getTemplateParamInfo() {
  TemplateParamInfo ret;
  
  if( m_tag ) {
    if( m_tag.hasAttribute( "tpl" ) ) {
      QStringList l = m_tag.attribute( "tpl" ).asStringList();
      
      TypeDesc::TemplateParams templateParams = m_desc.templateParams();
      uint pi = 0;
      QStringList::const_iterator it = l.begin();
      while( it != l.end() ) {
        TemplateParamInfo::TemplateParam curr;
        curr.name = *it;
        curr.number = pi;
        ++pi;
        ++it;
        if( it != l.end() ) {
          curr.def = *it;
          ++it;
        }
        if( pi < templateParams.count() )
          curr.value = *templateParams[pi];
        ret.addParam( curr );
      };
    }
  }
  
  return ret;
}

const TypeDesc SimpleTypeCatalog::findTemplateParam( const QString& name ) {
  if( m_tag ) {
    if( m_tag.hasAttribute( "tpl" ) ) {
      QStringList l = m_tag.attribute( "tpl" ).asStringList();
                ///we need the index, so count the items through
      uint pi = 0;
      
      QStringList::const_iterator it = l.begin();
      while( it != l.end() && *it != name ) {
        ++pi;
        ++it;
        if( it != l.end() ) ++it;
      };
      
      TypeDesc::TemplateParams templateParams = m_desc.templateParams();
      
      if( it != l.end() &&  pi < templateParams.count() ) {
        return *templateParams[pi];
      } else {
        if( it != l.end() && *it == name && !(*it).isEmpty()) {
          ++it;
          if( it != l.end() && !(*it).isEmpty() ) {
            dbg() << "using default-template-type " << *it << " for " << name << endl;
            return *it;     ///return default-parameter
          }
        }
      }
    }
  }
  return TypeDesc();
};


TypePointer SimpleTypeCatalog::CatalogBuildInfo::build() {
  if( !m_tag ) 
    return TypePointer();
  else {
    TypePointer tp = new SimpleTypeUsedCatalog( m_tag );
    tp->parseParams( m_desc );
    if( m_parent ) tp->setParent( m_parent->bigContainer() );
    return tp;
  }
  
}

//SimpleTypeCodeModel implementation

SimpleTypeCodeModel::SimpleTypeCodeModel( ItemDom& item ) : m_item( item ) {
  CodeModelItem* i = &(*item);
  FunctionModel* m = dynamic_cast<FunctionModel*>( i );
  ClassModel* c = dynamic_cast<ClassModel*>( i );
  if( m ) {
    QStringList l = m->scope(); 
    l << m->name();
    setScope( l );
    return;
  }
  if( c ) {
    QStringList l = c->scope(); 
    l << c->name();
    setScope( l );
    return;
  }
  dbg() << "code-model-item has an unsupported type: " << i->name() << endl;
}

ItemDom SimpleTypeCodeModel::locateModelContainer( class CodeModel* m, TypeDesc t, ClassDom cnt)
{
  if( !cnt ) {
    if( m->globalNamespace() ) {
      cnt = model_cast<ClassDom>( m->globalNamespace() );
    } else {
      return ItemDom();
    }
  }
  if( t ) {
    if( cnt->hasClass( t.name() ) ) {
      ClassList l = cnt->classByName( t.name() );
      if( !l.isEmpty() ) {
        if( t.next() )
          return locateModelContainer( m, *t.next(), l.front() );
        else
          return model_cast<ItemDom>( l.front() );
      }
    }
    NamespaceModel* ns = dynamic_cast<NamespaceModel*>(&(*cnt));
    if( ns ) {
      NamespaceDom n = ns->namespaceByName( t.name() );
      if( t.next() )
        return locateModelContainer( m, *t.next(), model_cast<ClassDom>( n ) );
      else
        return model_cast<ItemDom>( n );
    }
  }
  
  return ItemDom();
}


bool SimpleTypeCodeModel::findItem() {
  QString key = str();
  m_item = locateModelContainer( cppCompetionInstance->m_pSupport->codeModel(), str() );
  return (bool) m_item;
}

void SimpleTypeCodeModel::init() {
  if( scope().isEmpty() ) {
    m_item = cppCompetionInstance->m_pSupport->codeModel() ->globalNamespace();
  }else{
    findItem();
  }
}

DeclarationInfo SimpleTypeCodeModel::getDeclarationInfo() {
  DeclarationInfo ret;
  ItemDom i = item();
  ret.name = fullTypeResolved();
  if( i ) {
    ret.file = i->fileName();
    i->getStartPosition( &ret.startLine, &ret.startCol );
    i->getEndPosition( &ret.endLine, &ret.endCol );
    ret.comment = i->comment();
  }
  return ret;
}


SimpleTypeImpl::TemplateParamInfo SimpleTypeCodeModel::getTemplateParamInfo() {
  TemplateParamInfo ret;
  
  if(m_item) {
    TemplateModelItem* ti = dynamic_cast<TemplateModelItem*> ( &( *m_item ) );
    TypeDesc::TemplateParams& templateParams = m_desc.templateParams();
    
    TemplateModelItem::ParamMap m =  ti->getTemplateParams();
    for( uint a = 0; a < m.size(); a++ ) {
      TemplateParamInfo::TemplateParam t;
      t.number = a;
      t.name = m[a].first;
      t.def = m[a].second;
      if( templateParams.count() > a )
        t.value = *templateParams[a];
      ret.addParam( t );
    }
  }
  
  return ret;
}

const TypeDesc SimpleTypeCodeModel::findTemplateParam( const QString& name ) {
  if(m_item) {
    TemplateModelItem* ti = dynamic_cast<TemplateModelItem*> ( &( *m_item ) );
    TypeDesc::TemplateParams& templateParams = m_desc.templateParams();
    int pi = ti->findTemplateParam( name );
    if( pi != -1 && (int)templateParams.count() > pi ) {
      return *templateParams[pi];
    } else {
      if( pi != -1 && !ti->getParam( pi ).second.isEmpty() ) { 
        QString def = ti->getParam( pi ).second;
      dbg() << "\"" << str() << "\": using default-template-parameter \"" << def << "\" for " << name << endl;
        return def;
      } else if( pi != -1 ) {
      dbg() << "\"" << str() << "\": template-type \"" << name << "\" has no pameter! " << endl;
      }
    }
  }
  return TypeDesc();
};

QValueList<SimpleType> SimpleTypeCodeModel::getBases() {
  Debug d( "#getbases#" );
  if( !d ) {
  dbg() << "\"" << str() << "\": recursion to deep while getting bases" << endl;
    return QValueList<SimpleType>();
  }
  
  QValueList<SimpleType> ret;
  
  ClassModel* klass;
  
  if( !m_item || ( klass = dynamic_cast<ClassModel*>( &(*m_item) ) ) == 0 ) return ret;
  
  QStringList parents = klass->baseClassList();
  for ( QStringList::Iterator it = parents.begin(); it != parents.end(); ++it )
  {
    ret << locateType( *it , LocateBase );
  }
  
  return ret;
}

TypePointer SimpleTypeCodeModel::CodeModelBuildInfo::build() {
  TypePointer tp = new SimpleTypeUsedCodeModel( m_item );
  tp->parseParams( m_desc );
  if( m_parent ) tp->setParent( m_parent->bigContainer() );
  return tp;
}


//SimpleTypeNameSpace implementation

TypePointer SimpleTypeNamespace::NamespaceBuildInfo::build() {
  SimpleTypeNamespace* ns = new SimpleTypeUsedNamespace( m_fakeScope, m_realScope );
  for( QValueList<QStringList>::iterator it = m_imports.begin(); it != m_imports.end(); ++it )
    ns->addAliasMap( "", (*it).join("::") );
  return ns;
}

SimpleTypeImpl::MemberInfo SimpleTypeNamespace::findMember( TypeDesc name, MemberInfo::MemberType type)
{
  MemberInfo mem;
  mem.name = "";
  mem.memberType = MemberInfo::NotFound;
  
  for( QValueList<SimpleType>::iterator it = m_activeSlaves.begin(); it != m_activeSlaves.end(); ++it ) { 
  dbg() << "\"" << str() << "\": redirecting search for \"" << name.name() << "\" to \"" << (*it)->fullType() << "\"" << endl;
    mem = (*it)->findMember( name , type );
    if( mem ) {
      if( mem.memberType != MemberInfo::Namespace ) {
        return mem;
      } else {
                    ///verify that the namespace is built as this class..
        QValueList<QStringList> allAliases;
        
        if( m_aliases.contains( name.name() ) ) {
          QStringList& aliases  = m_aliases[name.name()];
          for( QStringList::iterator it = aliases.begin(); it != aliases.end(); ++it)
            allAliases << splitType( *it );
        }
        
        return setupMemberInfo( name, mem.type.fullNameList(), allAliases );
      }
    }
  }
  
  AliasMap::iterator itt = m_aliases.find( name.name() );
  
  if( itt != m_aliases.end() && !(*itt).isEmpty() ) {
  dbg() << "\"" << str() << "\": namespace-sub-aliases \"" << name.name() << "\" -> \"" << *itt << "\" requested, locating targets" << endl;
    
    QValueList<QStringList> targets;
    for( QStringList::iterator it = (*itt).begin(); it != (*itt).end(); ++it ) {
      QStringList l = locateNamespace( *it );
      if( !l.isEmpty() )
        targets << l;
    }
    
    if( !targets.isEmpty() ) {
      QValueList<QStringList> targets2 = targets;
      targets2.pop_front();
      mem = setupMemberInfo( name, targets.front(), targets2 );
    dbg() << "\"" << str() << "\": namespace-sub-alias \"" << name.name() << "\" -> \"" << targets2 << "\" <- successfully located" << endl;                
      
    } else {
    dbg() << "\"" << str() << "\": namespace-sub-aliases \"" << name.name() << "\" -> \"" << *itt << "\" no target could be located" << endl;
    }
  }
  
  return mem;
}

SimpleTypeImpl::MemberInfo SimpleTypeNamespace::setupMemberInfo( TypeDesc& subName, QStringList tscope, QValueList<QStringList> imports ) {
  MemberInfo mem;
  mem.name = subName.name();
  mem.memberType = MemberInfo::NotFound;
  QStringList sc = scope();
  if( subName.hasTemplateParams() ) {
  dbg() << "\"" << str() << "\": found namespace for " << subName.fullName() << " but the type has templates!" << endl;
    return mem;
  }
  sc << subName.name();
  mem.type = sc.join( "::" );
  mem.memberType = MemberInfo::Namespace;
  mem.setBuildInfo( new NamespaceBuildInfo( sc, tscope, imports ) );
  return mem;
}


QStringList SimpleTypeNamespace::locateNamespace( QString alias ) {
dbg() << "\"" << str() << "\": locating namespace \"" << alias << "\"" << endl;
  SimpleType s = locateType( alias, addFlag( ExcludeNestedTypes, ExcludeTemplates ), 0, MemberInfo::Namespace );
  if( isANamespace( s ) ) {
    return s->scope();
  dbg() << "\"" << str() << "\": successfully located namespace \"" << s->str() << "\"" << endl;
  } else {
  dbg() << "\"" << str() << "\": searched for a namespace, but found \"" << s->str() << "\"" << endl; 
  }
  
dbg() << "\"" << str() << "\": failed to locate namespace \"" << alias << "\"" << endl; 
  
  return QStringList();
}

void SimpleTypeNamespace::recurseAliasMap() {
  bool changed = true;
  SafetyCounter s(1000);
  AliasMap::iterator import = m_aliases.find( "" );
  for( AliasMap::iterator it = m_aliases.begin(); it != m_aliases.end(); ++it )
    (*it).sort();
  
  while( changed && s ) {
            ///this has bad runtime-performance for high counts, but it's simple and we shouldn't have that much maps :-)
    changed = false;
    for( AliasMap::iterator it = m_aliases.begin(); it != m_aliases.end(); ++it ) {
      QStringList& l = *it;
      for( QStringList::iterator strIt = l.begin(); strIt != l.end(); ++strIt ) {
        AliasMap::iterator fit = m_aliases.find( *strIt );
        
        for( QStringList::iterator oit = (*fit).begin(); oit != (*fit).end(); ++oit ) {
          if( !l.contains( *oit ) ) {
            addAliasMap( it.key(), *oit , true );
            changed = true;
          }
          if( changed ) break;
        }
        if( changed ) break;
      }
      if( changed ) break;
    }
  }
  
  if( !s ) {
  dbg() << "\"" << str() << "\": too much recursion while applying namespace-aliases" << endl;
  }
}

void SimpleTypeNamespace::addAliasMap( QString name, QString alias , bool recurse) {
dbg() << "\"" << str() << "\": adding namespace-alias \"" << name << "\" -> \"" << alias << "\"" << endl;
  AliasMap::iterator it = m_aliases.find( name );
  if( it == m_aliases.end() )
    it = m_aliases.insert( name, QStringList() );
  
  *it << alias;
  
  if( name.isEmpty() ) {
    addScope( locateNamespace( alias ) );
  }
  
  if( recurse )recurseAliasMap();
}

void SimpleTypeNamespace::addAliases( QString map ) {
  while(!map.isEmpty() ) {
    int mid = map.find( "=" );
    if( mid == -1 ) break;
    int end = map.find( ";", mid+1 );
    if( end == -1 ) break;
    
    addAliasMap( map.left( mid ).stripWhiteSpace(), map.mid( mid+1, end - mid - 1 ).stripWhiteSpace() );
    map = map.mid( end + 1);
  }
};

// kate: indent-mode csands; tab-width 4;

