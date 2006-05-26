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
  
	//if( !ret.type) ret.memberType = MemberInfo::NotFound; //constructor..
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

QValueList<SimpleTypeImpl::LocateResult> SimpleTypeCatalog::getBases() {
  Debug d( "#getbases#" );
  if( !d ) {
  dbg() << "\"" << str() << "\": recursion to deep while getting bases" << endl;
    return QValueList<SimpleTypeImpl::LocateResult>();
  }
  
  QValueList<SimpleTypeImpl::LocateResult> ret;
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

	ret << locateDecType( info.baseClass() , LocateBase );
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
    TypePointer tp = new SimpleTypeCachedCatalog( m_tag );
    tp->parseParams( m_desc );
    if( m_parent ) tp->setParent( m_parent->bigContainer() );
    return tp;
  }
  
}

//SimpleTypeNameSpace implementation

TypePointer SimpleTypeNamespace::NamespaceBuildInfo::build() {
  SimpleTypeNamespace* ns = new SimpleTypeCachedNamespace( m_fakeScope, m_realScope );
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
  SimpleTypeImpl::LocateResult res = locateDecType( alias, addFlag( ExcludeNestedTypes, ExcludeTemplates ), 0, MemberInfo::Namespace );
	if( !res->resolved() ) return QStringList();
	if( isANamespace( res->resolved() ) ) {
		return res->resolved()->scope();
	dbg() << "\"" << str() << "\": successfully located namespace \"" << res->fullNameChain() << "\"" << endl;
  } else {
  dbg() << "\"" << str() << "\": searched for a namespace, but found \"" << res->fullNameChain() << "\"" << endl;
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

