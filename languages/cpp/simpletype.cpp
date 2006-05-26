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

#include "simpletype.h"
#include "safetycounter.h"
#include "simpletypefunction.h"

extern SafetyCounter safetyCounter;

TypePointer SimpleType::m_globalNamespace;
SimpleType::TypeStore  SimpleType::m_typeStore;
SimpleType::TypeStore  SimpleType::m_destroyedStore;
QString globalCurrentFile = "";

//SimpleType implementation

void SimpleType::resolve( Repository rep )  const {
 if( !m_resolved ) {
  if( m_globalNamespace ) {
   if( (rep == Undefined || rep == Both) ) {
    m_resolved = true;
    if( scope().isEmpty() ) {
     m_type = m_globalNamespace;
    } else {
	    SimpleTypeImpl::SimpleTypeImpl::LocateResult t = m_globalNamespace->locateDecType( scope().join("::") );
	if( t && t->resolved() ) {
      m_type = t->resolved();
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
     cm = TypePointer( new SimpleTypeCachedCodeModel( scope() ) );
   }else{
     cm = TypePointer( new SimpleTypeCachedCodeModel( &(*m_type) ) );
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
     cm = TypePointer( new SimpleTypeCachedCatalog( scope() ) );
   }else{
     cm = TypePointer( new SimpleTypeCachedCatalog( &(*m_type) ) );
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
    cm = new SimpleTypeCachedNamespace( scope() );
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
	bool unregistered = true;
	int cnt = m_typeStore.size();
	kdDebug( 9007 ) << cnt << "types in type-store before destruction" << endl;
		
	SafetyCounter s( 30000 );
	while( !m_typeStore.empty() && s ) {
	  TypeStore::iterator it = m_typeStore.begin();
	  SimpleTypeImpl* tp = *it;
	  m_destroyedStore.insert( tp );
	  m_typeStore.erase( it );
	  tp->breakReferences();
	}
 
	if( !m_destroyedStore.empty() ) {
		kdDebug( 9007 ) << "type-store is not empty, " << m_destroyedStore.size() << " types are left over" << endl;
		for( TypeStore::iterator it = m_destroyedStore.begin(); it != m_destroyedStore.end(); ++it ) {
		   kdDebug( 9007 ) << "type left: " << (*it)->describe() << endl;
  		}
 	}

	///move them over so they will be cleared again next time, hoping that they will vanish
	m_typeStore = m_destroyedStore;
	m_destroyedStore.clear();
}

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
  m_type = TypePointer( new SimpleTypeCachedCodeModel( item ) );
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
	 return TypeOfResult( SimpleTypeImpl::LocateResult( TypeDesc( "CompletionError::too_much_recursion" ) ) );
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
    QValueList<SimpleTypeImpl::LocateResult> bases = getBases();
    for( QValueList<SimpleTypeImpl::LocateResult>::iterator it = bases.begin(); it != bases.end(); ++it ) {
	    if( (*it)->resolved() ) {
		    TypeOfResult rt = (*it)->resolved()->typeOf( name );
			if( rt->resolved() )
				ret->asFunction()->appendNextFunction( SimpleType( rt->resolved() ) );
	    }
    }
	   return TypeOfResult( SimpleTypeImpl::LocateResult( ret->desc() ) );
   } else {
    dbg() << "error, using old function-type-evaluation" << endl;
	   return TypeOfResult( locateDecType( mem.type ), mem.decl );
   }
  } else if( mem.memberType == MemberInfo::Variable ) {
	  return TypeOfResult( locateDecType( mem.type ), mem.decl );
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

SimpleTypeImpl::LocateResult SimpleTypeImpl::getFunctionReturnType( QString functionName, QValueList<SimpleTypeImpl::LocateResult> params) {
    SimpleTypeImpl::LocateResult t = typeOf( functionName, MemberInfo::Function ).type;
	if( t->resolved() && t->resolved()->asFunction() ) {
		return t->resolved()->applyOperator( ParenOp, params );
    } else {
    	dbg() << "error : could not find function \"" << functionName << "\" in \"" << str() << "\"" << endl;
      return SimpleTypeImpl::LocateResult();
    }
}

SimpleTypeImpl::LocateResult SimpleTypeImpl::applyOperator( Operator op , QValueList<SimpleTypeImpl::LocateResult> params ) {
    Debug d("#applyn#");
    if( !d || !safetyCounter )
      return SimpleTypeImpl::LocateResult();
    
    dbg() << "applying operator " << operatorToString( op ) << " to \"" << desc().fullNameChain() << "\"" <<  endl;
    SimpleTypeImpl::LocateResult ret;
	if( op == NoOp ) return SimpleTypeImpl::LocateResult( desc() );
    
    switch( op ) {
    case IndexOp:
        return getFunctionReturnType( "operator [ ]", params );
      break;
    case StarOp:
        return getFunctionReturnType( "operator *", params );
      break;
    case ArrowOp:
                /** Dereference one more because the type must be a pointer */
        ret = getFunctionReturnType( "operator ->", params );
        if( ret->pointerDepth() ) {
          ret->setPointerDepth( ret->pointerDepth() - 1 );
        } else {
        	dbg() << "\"" << str() << "\": " << " \"operator ->\" returns a type with the wrong pointer-depth" << endl;
        }
        return ret;
      break;
    case ParenOp:
                /** Dereference one more because the type must be a pointer */
        return getFunctionReturnType( "operator ( )", params );
    default:
      dbg() << "wrong operator\n";
    }
    
	return SimpleTypeImpl::LocateResult();
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
	      if( !(*it)->resolved() && !(*it)->hasFlag( TypeDesc::ResolutionTried ) ) {
	        *it = new TypeDescShared(  locateDecType( **it, mode ) );
		      (*it)->setFlag( TypeDesc::ResolutionTried );
        }
      }
    }
    
    if( ret.next() ) {
      ret.setNext( new TypeDescShared( resolveTemplateParams( *ret.next(), mode ) ) );
    }
    
    return ret;
}

SimpleTypeImpl::LocateResult SimpleTypeImpl::locateType( TypeDesc name , LocateMode mode , int dir ,  MemberInfo::MemberType typeMask ) {
    Debug d("#lo#");
    if( !name || !safetyCounter || !d ) {
	    return desc();
    }
    if( !d ) {
      dbg() << "stopping location because the recursion-depth is too high" << endl;
      return TypeDesc( "CompletionError::too_much_recursion" );
    }
  dbg() << "\"" << desc().fullName() << "\": locating type \"" << name.fullNameChain() << "\"" << endl;
	/*
    if( name.resolved() && name.length() == name.resolved()->desc().length() ) {
    dbg() << "\"" << desc().fullName() << "\": type \"" << name.fullNameChain() << "\" is already resolved, returning stored instance" << endl;
      SimpleType ret = SimpleType( name.resolved() );
      
      if( ! (name == ret->desc()) ) {
        ret.makePrivate();  ///Maybe some small parameters like the pointer-depth were changed, so customize those
        ret->parseParams( name );
      }
      
      return ret;
    }*/
    
    SimpleTypeImpl::LocateResult ret = name; ///In case the type cannot be located, this helps to find at least the best match
	ret->setResolved( 0 );
    
	TypeDesc first = name.firstType();
	MemberInfo mem = findMember( first, typeMask );
    
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
        }else {
            ///Should not happen..
        	kdDebug( 9007 ) << "\"" << str() << "\": Warning: the nested-type " << name.name() << " was found, but has no build-info" << endl;
	        return TypeDesc("CompletionError::unknown");
        }
        
        TypeDescPointer rest = name.next();
        if( rest ) {
          dbg() << "\"" << str() << "\": found nested type \"" << name.name() << "\", passing control to it\n";
          ret = sub->locateType( *rest, addFlag( mode, ExcludeTemplates ), 1 ); ///since template-names cannot be referenced from outside, exclude them for the first cycle
		  ret.increaseResolutionCount();
		  if( ret->resolved() )
            return ret;
        } else {
        	dbg() << "\"" << str() << "\": successfully located searched type \"" << name.fullNameChain() << "\"\n";
	        ret->setResolved( sub.get() );
          	return ret;
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
	          if( name.next() ) {
		          mem.type.makePrivate();
				  mem.type.append( name.next() );
	          }
	          ret = locateDecType( mem.type, remFlag( mode, ExcludeTemplates ) );

	          if( mem.memberType == MemberInfo::Template )
		          ret.addResolutionFlag( HadTemplate );
	          if( mem.memberType == MemberInfo::Typedef )
		          ret.addResolutionFlag( HadTypedef );
	          ret.increaseResolutionCount();
		          //if( mode & TraceAliases ) ret->tracePrepend( name );
	          
	          if( ret->resolved() )
				return ret;
          } else {
          	dbg() << "\"" << str() << "\"recursive typedef/template found: \"" << name.fullNameChain() << "\" -> \"" << mem.type.fullNameChain() << "\"" << endl;
          }
        }
        break;
      }
    }
    
        ///Ask bases but only on this level
    if( ! ( mode & ExcludeBases ) ) {
      
      QValueList<SimpleTypeImpl::LocateResult> bases = getBases();
      if( !bases.isEmpty() ) {
        TypeDesc nameInBase = resolveTemplateParams( name, LocateBase ); ///Resolve all template-params that are at least visible in the scope of the base-declaration
            
        for( QValueList<SimpleTypeImpl::LocateResult>::iterator it = bases.begin(); it != bases.end(); ++it ) {
	        if( !(*it)->resolved() ) continue;
	        SimpleTypeImpl::LocateResult t = (*it)->resolved()->locateType( nameInBase, addFlag( addFlag( mode, ExcludeTemplates ), ExcludeParents ), dir ); ///The searched Type cannot directly be a template-param in the base-class, so ExcludeTemplates. It's forgotten early enough.
		if( t->resolved() )
            return t;
          else
            if( t > ret )
              ret = t;
        }
      }
    }
    
        ///Ask parentsc
    if( !scope().isEmpty() && dir != 1 && ! ( mode & ExcludeParents ) ) {
    SimpleTypeImpl::LocateResult rett = parent()->locateType( resolveTemplateParams( name, mode & ExcludeBases ? ExcludeBases : mode ), mode & ForgetModeUpwards ? Normal : mode );
	if( rett->resolved() ) 
        return rett;
      else
        if( rett > ret )
          ret = rett;
    }
    
        ///Ask the bases and allow them to search in their parents.
    if( ! ( mode & ExcludeBases ) ) {
      TypeDesc baseName = resolveTemplateParams( name, LocateBase ); ///Resolve all template-params that are at least visible in the scope of the base-declaration
      QValueList<SimpleTypeImpl::LocateResult> bases = getBases();
      if( !bases.isEmpty() ) {
        for( QValueList<SimpleTypeImpl::LocateResult>::iterator it = bases.begin(); it != bases.end(); ++it ) {
	        if( !(*it)->resolved() ) continue;
	        SimpleTypeImpl::LocateResult t = (*it)->resolved()->locateType( baseName, addFlag( mode, ExcludeTemplates ), dir ); ///The searched Type cannot directly be a template-param in the base-class, so ExcludeTemplates. It's forgotten early enough.
		if( t->resolved() )
            return t;
          else
            if( t > ret )
              ret = t;
        }
      }
    }
    
        ///Give the type a desc, so the nearest point to the searched type is stored
  	dbg() << "\"" << str() << "\": search for \"" << name.fullNameChain() << "\" FAILED" << endl;
    return ret;
  };
  
void SimpleTypeImpl::breakReferences() {
	TypePointer p( this ); ///necessary so this type is not deleted in between
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
    QValueList<SimpleTypeImpl::LocateResult> parents = getBases();
    for ( QValueList<SimpleTypeImpl::LocateResult>::iterator it = parents.begin(); it != parents.end(); ++it )
    {
	    if( !(*it)->resolved() ) continue;
	    TypeOfResult type = (*it)->resolved()->typeOf( name.name() );
      if ( type )
        return type;
    }
    return TypeOfResult();
}

void SimpleTypeImpl::setSlaveParent( SimpleTypeImpl& slave ) {
  if( ! m_masterProxy ) {
    slave.setParent( this );
  } else {
    slave.setParent( m_masterProxy );
  }
}

void SimpleTypeImpl::parseParams( TypeDesc desc ) {
  invalidateCache();
  m_desc = desc;
  m_desc.clearInstanceInfo();
}

void SimpleTypeImpl::takeTemplateParams( TypeDesc desc ) {
  invalidateCache();
  m_desc.templateParams() = desc.templateParams();
}

//SimpleTypeImpl::TemplateParamInfo implementation

bool SimpleTypeImpl::TemplateParamInfo::getParam( TemplateParam& target, QString name ) const {
  QMap<QString, TemplateParam>::const_iterator it = m_paramsByName.find( name );
  if( it != m_paramsByName.end() ) {
    target = *it;
    return true;
  }
  return false;
}

bool SimpleTypeImpl::TemplateParamInfo::getParam( TemplateParam& target, int number ) const {
  QMap<int, TemplateParam>::const_iterator it = m_paramsByNumber.find( number );
  if( it != m_paramsByNumber.end() ) {
    target = *it;
    return true;
  }
  return false;
}

void SimpleTypeImpl::TemplateParamInfo::removeParam( int number ) {
  QMap<int, TemplateParam>::iterator it = m_paramsByNumber.find( number );
  if( it != m_paramsByNumber.end() ) {
    m_paramsByName.remove( (*it).name );
    m_paramsByNumber.remove( it );
  }
}

void SimpleTypeImpl::TemplateParamInfo::addParam( const TemplateParam& param ) {
  m_paramsByNumber[param.number] = param;
  m_paramsByName[param.name] = param;
}

// kate: indent-mode csands; tab-width 4;

