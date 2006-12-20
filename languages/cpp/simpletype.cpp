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
#include <klocale.h>

QMap<QString, QString> BuiltinTypes::m_types;
BuiltinTypes builtin; //Needed so BuiltinTypes::BuiltinTypes is called and the types are initialized

BuiltinTypes::BuiltinTypes() {
  m_types[ "void" ] = i18n( "typeless" );
  m_types[ "bool" ] = i18n("boolean value, 1 byte, ( \"true\" or \"false\" )");
  m_types["char" ] = i18n("signed/unsigned character, 1 byte");
  m_types["signed char" ] =  i18n("signed character, 1 byte, ranged -128 to 127");
  m_types["unsigned char"] = i18n("unsigned character, 1 byte, ranged 0 to 255");
  m_types["wchar_t"] = i18n("wide character, 2 bytes, ranged 0 to 65.535");
  m_types["long"] = m_types["long int"] = m_types["int"] = m_types["signed int"] = i18n("signed interger, 4 bytes, ranged -2.147.483.648 to 2.147.483.647");
  m_types["unsigned int"] = i18n("unsigned integer, 4 bytes, ranged 0 to 4.294.967.295");
  m_types["short"] = m_types["short int"] = i18n("short integer, 2 bytes, ranged -32.768 to 32.768");
  m_types["unsigned short int"] = i18n("unsigned short integer, 2 bytes, ranged 0 to 65.535");
  m_types["float"] = i18n("floating point value, 4 bytes, ranged ca. -3,4E+38 to 3.4E+38");
  m_types["double"] = i18n("double floating point value, 8 bytes, ranged ca. -1.8E+308 to 1.8E+308");
  m_types["long double"] = i18n("double long floating point value, 10 bytes, ranged ca. -3.4E+4932 to 3.4E+4932");
  m_types["size_t"] = i18n("unsigned integer, byte-count depending on operating-system" );
   
}

bool BuiltinTypes::isBuiltin( const TypeDesc& desc ) {
  return m_types.find( desc.name() ) != m_types.end();
}

QString BuiltinTypes::comment( const TypeDesc& desc ) {
  QMap<QString, QString>::iterator it = m_types.find( desc.name() );
  if( it != m_types.end() ) {
    return *it;
  } else {
    return QString::null;
  }
}

extern SafetyCounter safetyCounter;

TypePointer SimpleType::m_globalNamespace;
SimpleType::TypeStore SimpleType::m_typeStore;
SimpleType::TypeStore SimpleType::m_destroyedStore;
QString globalCurrentFile = "";

//SimpleType implementation

void SimpleType::resolve( Repository rep ) const {
  if ( !m_resolved ) {
    if ( m_globalNamespace ) {
      if ( ( rep == Undefined || rep == Both ) ) {
        m_resolved = true;
        if ( scope().isEmpty() || str().isEmpty() ) {
          m_type = m_globalNamespace;
          return ;
        } else {
            TypeDesc d( scope().join( "::" ) );
            d.setIncludeFiles( m_includeFiles );
          LocateResult t = m_globalNamespace->locateDecType( d );
          if ( t && t->resolved() ) {
            m_type = t->resolved();
            return ;
          } else {
            ifVerbose( dbg() << "\"" << scope().join( "::" ) << "\": The type could not be located in the global scope while resolving it" << endl );
          }
        }
      }
    } else {
      ifVerbose( dbg() << "warning: no global namespace defined! " << endl );
    }

    TypePointer cm;

    if ( rep == Undefined || rep == CodeModel ) {
      if ( !m_type ) {
        cm = TypePointer( new SimpleTypeCachedCodeModel( scope() ) );
      } else {
        cm = TypePointer( new SimpleTypeCachedCodeModel( &( *m_type ) ) );
      }

      if ( cm->hasNode() || rep == CodeModel ) {
        if ( cm->hasNode() ) {
          ifVerbose( dbg() << "resolved \"" << str() << "\" from the code-model" << endl );
          if ( cm->isNamespace() && rep != CodeModel ) {
            ifVerbose( dbg() << "\"" << str() << "\": is namespace, resolving proxy" << endl );
            resolve( Both );
            return ;
          }
        } else {
          ifVerbose( dbg() << "forced \"" << str() << "\" to be resolved from code-model" << endl );
        }
        m_type = cm;
        m_resolved = true;
        return ;
      }
    }
    if ( rep == Undefined || rep == Catalog ) {

      if ( !m_type ) {
        cm = TypePointer( new SimpleTypeCachedCatalog( scope() ) );
      } else {
        cm = TypePointer( new SimpleTypeCachedCatalog( &( *m_type ) ) );
      }

      if ( cm->hasNode() || rep == Catalog ) {
        if ( cm->hasNode() ) {
          ifVerbose( dbg() << "resolved \"" << str() << "\" from the catalog" << endl );
          if ( cm->isNamespace() && rep != Catalog ) {
            ifVerbose( dbg() << "\"" << str() << "\": is namespace, resolving proxy" << endl );
            resolve( Both );
            return ;
          }
        } else {
          ifVerbose( dbg() << "forced \"" << str() << "\" to be resolved from catalog" << endl );
        }
        m_type = cm;
        m_resolved = true;
        return ;
      }
    }

    if ( rep == Both ) {
      cm = new SimpleTypeCachedNamespace( scope() );
      m_type = cm;
      m_resolved = true;
      return ;
    }

    m_resolved = true;
    ifVerbose( dbg() << "could not resolve \"" << m_type->desc().fullNameChain() << "\"" << endl );
  }
}

void SimpleType::destroyStore() {
  resetGlobalNamespace();
  bool unregistered = true;
  int cnt = m_typeStore.size();
  kdDebug( 9007 ) << cnt << "types in type-store before destruction" << endl;

  SafetyCounter s( 30000 );
  while ( !m_typeStore.empty() && s ) {
    TypeStore::iterator it = m_typeStore.begin();
    TypePointer tp = *it;
    m_destroyedStore.insert( tp );
    m_typeStore.erase( it );
    tp->breakReferences();
  }

  if ( !m_destroyedStore.empty() ) {
    kdDebug( 9007 ) << "type-store is not empty, " << m_destroyedStore.size() << " types are left over" << endl;
    for ( TypeStore::iterator it = m_destroyedStore.begin(); it != m_destroyedStore.end(); ++it ) {
      kdDebug( 9007 ) << "type left: " << ( *it ) ->describe() << endl;
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

void SimpleType::init( const QStringList& scope, const HashedStringSet& files, Repository rep ) {
    m_includeFiles = files;

  m_type = TypePointer( new SimpleTypeImpl( scope ) );
  if ( rep != Undefined )
    resolve( rep );
}

SimpleType::SimpleType( ItemDom item ) : m_resolved( true ) {
  m_type = TypePointer( new SimpleTypeCachedCodeModel( item ) );
}
/*
SimpleType::SimpleType( Tag tag ) : m_resolved(true) {
 m_type = TypePointer( new SimpleTypeCatalog( tag ) );
}*/
//
//SimpleTypeImpl implementation

QValueList<LocateResult> SimpleTypeImpl::getBases() {
QValueList<LocateResult> ret;
  QStringList bases = getBaseStrings();
  for( QStringList::const_iterator it = bases.begin(); it != bases.end(); ++it ) {
    TypeDesc d( *it );
    d.setIncludeFiles( m_findIncludeFiles );
    LocateResult res = locateDecType( d, LocateBase );
    //if( res )
      ret << res;
  }
  return ret;
}

void SimpleTypeImpl::setFindIncludeFiles( const IncludeFiles& files ) {
  m_findIncludeFiles = files;
}

IncludeFiles SimpleTypeImpl::getFindIncludeFiles() {
  return m_findIncludeFiles;
}

/**
Searches for a member called "name", considering all types selected through "typ"*/
SimpleTypeImpl::TypeOfResult SimpleTypeImpl::typeOf( const TypeDesc& name, MemberInfo::MemberType typ ) {
  Debug d( "#to#" );
  if ( !d ) {
    ifVerbose( dbg() << "stopping typeOf-evaluation because the recursion-depth is too high" << endl );
    return TypeOfResult( LocateResult( TypeDesc( "CompletionError::too_much_recursion" ) ) );
  }
  ifVerbose( dbg() << "\"" << str() << "\"------------>: searching for type of member \"" << name.fullNameChain() << "\"" << endl );

  TypeDesc td = resolveTemplateParams( name );

  MemberInfo mem = findMember( td, typ );

  if ( mem ) {
    mem.type = resolveTemplateParams( mem.type );

    ifVerbose( dbg() << "\"" << str() << "\": found member " << name.fullNameChain() << ", type: " << mem.type->fullNameChain() << endl );
    if ( mem.memberType == MemberInfo::Function ) {
      ///For functions, find all functions with the same name, so that overloaded functions can be identified correctly
      TypePointer ret = mem.build();
      if ( ret && ret->asFunction() ) {
        ///Search all bases and append all functions with the same name to it.
        QValueList<LocateResult> bases = getBases();
        for ( QValueList<LocateResult>::iterator it = bases.begin(); it != bases.end(); ++it ) {
          if( !safetyCounter ) break;
          if ( ( *it ) ->resolved() ) {
            TypeOfResult rt = ( *it ) ->resolved() ->typeOf( name );
            if ( rt->resolved() )
              ret->asFunction() ->appendNextFunction( SimpleType( rt->resolved() ) );
          }
        }
        return TypeOfResult( LocateResult( ret->desc() ) );
      } else {
        ifVerbose( dbg() << "error, using old function-type-evaluation" << endl );

        TypeDesc d( mem.type );
        if( m_findIncludeFiles.size() != 0 )
          d.setIncludeFiles( m_findIncludeFiles );
        else
          d.setIncludeFiles( name.includeFiles() );

        return TypeOfResult( locateDecType( d ), mem.decl );
      }
    } else if ( mem.memberType == MemberInfo::Variable ) {
        TypeDesc d( mem.type );
        if( m_findIncludeFiles.size() != 0 )
          d.setIncludeFiles( m_findIncludeFiles );
        else
          d.setIncludeFiles( name.includeFiles() );
      
      return TypeOfResult( locateDecType( d ), mem.decl );
    } else {
      ifVerbose( dbg() << "while searching for the type of \"" << name.fullNameChain() << "\" in \"" << str() << "\": member has wrong type: \"" << mem.memberTypeToString() << "\"" << endl );
      return TypeOfResult();
    }
  }

  TypeOfResult ret = searchBases( td );
  if ( !ret ) {
    ifVerbose( dbg() << "\"" << str() << "\"------------>: failed to resolve the type of member \"" << name.fullNameChain() << "\"" << endl );
  } else {
    ifVerbose( dbg() << "\"" << str() << "\"------------>: successfully resolved the type of the member \"" << name.fullNameChain() << "\"" << endl );
  }
  return ret;
}

SimpleTypeFunctionInterface* SimpleTypeImpl::asFunction() {
  return dynamic_cast<SimpleTypeFunctionInterface*> ( this );
}

QString SimpleTypeImpl::operatorToString( Operator op ) {
  switch ( op ) {
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
    return QString( "%1" ).arg( ( long ) op );
  };
}

LocateResult SimpleTypeImpl::getFunctionReturnType( QString functionName, QValueList<LocateResult> params ) {
  LocateResult t = typeOf( functionName, MemberInfo::Function ).type;
  if ( t->resolved() && t->resolved() ->asFunction() ) {
    return t->resolved() ->applyOperator( ParenOp, params );
  } else {
    ifVerbose( dbg() << "error : could not find function \"" << functionName << "\" in \"" << str() << "\"" << endl );
    return LocateResult();
  }
}

LocateResult SimpleTypeImpl::applyOperator( Operator op , QValueList<LocateResult> params ) {
  Debug d( "#applyn#" );
  if ( !d || !safetyCounter )
    return LocateResult();

  ifVerbose( dbg() << "applying operator " << operatorToString( op ) << " to \"" << desc().fullNameChain() << "\"" << endl );
  LocateResult ret;
  if ( op == NoOp )
    return LocateResult( desc() );

  switch ( op ) {
    case IndexOp:
    return getFunctionReturnType( "operator [ ]", params );
    break;
    case StarOp:
    return getFunctionReturnType( "operator *", params );
    break;
    case ArrowOp:
    /** Dereference one more because the type must be a pointer */
    ret = getFunctionReturnType( "operator ->", params );
    if ( ret->pointerDepth() ) {
      ret->setPointerDepth( ret->pointerDepth() - 1 );
    } else {
      ifVerbose( dbg() << "\"" << str() << "\": " << " \"operator ->\" returns a type with the wrong pointer-depth" << endl );
    }
    return ret;
    break;
    case ParenOp:
    /** Dereference one more because the type must be a pointer */
    return getFunctionReturnType( "operator ( )", params );
    default:
    ifVerbose( dbg() << "wrong operator\n" );
  }

  return LocateResult();
}

TypeDesc SimpleTypeImpl::replaceTemplateParams( TypeDesc desc, TemplateParamInfo& paramInfo ) {
  Debug d( "#repl#" );
  if ( !d || !safetyCounter )
    return desc;

  TypeDesc ret = desc;
  if ( !ret.hasTemplateParams() && !ret.next() ) {
    TemplateParamInfo::TemplateParam t;
    if ( paramInfo.getParam( t, desc.name() ) ) {

      if ( t.value )
        ret = t.value;
      else if ( t.def )
        ret = t.def;

      if ( ret.name() != desc.name() )
        ret.setPointerDepth( ret.pointerDepth() + desc.pointerDepth() );
    }
  } else {
    TypeDesc::TemplateParams& params = ret.templateParams();
    for ( TypeDesc::TemplateParams::iterator it = params.begin(); it != params.end(); ++it ) {
      *it = new TypeDescShared( replaceTemplateParams( **it, paramInfo ) );
    }
  }

  if ( ret.next() ) {
    ret.setNext( new TypeDescShared( replaceTemplateParams( *ret.next(), paramInfo ) ) );
  }

  return ret;
}

TypeDesc SimpleTypeImpl::resolveTemplateParams( LocateResult desc, LocateMode mode ) {
  Debug d( "#resd#" );
  if ( !d || !safetyCounter )
    return desc;

  LocateResult ret = desc;
  if ( ret->hasTemplateParams() ) {
    TypeDesc::TemplateParams & params = ret->templateParams();
    for ( TypeDesc::TemplateParams::iterator it = params.begin(); it != params.end(); ++it ) {
      if ( !( *it ) ->resolved() && !( *it ) ->hasFlag( ResolutionTried ) ) {
        TypeDesc d( **it );
        if( d.includeFiles().size() == 0 )
          d.setIncludeFiles( this->getFindIncludeFiles() );
        *it = locateDecType( d, mode );
        ( *it ) ->setFlag( ResolutionTried );
      }
    }
  }

  if ( ret->next() ) {
    ret->setNext( new TypeDescShared( resolveTemplateParams( *ret->next(), mode ) ) );
  }

  return ret;
}

class TemplateParamMatch {
  public:
	TemplateParamMatch() : m_matched( false ), m_maxDepth( 0 ), m_candidate( 0 ) {}

	TemplateParamMatch( TypePointer candidate, const TypeDesc& params ) : m_matched( false ), m_maxDepth( 0 ), m_candidate( candidate ) {
	    m_candidateParams = candidate->getTemplateParamInfo();
	    TypeDesc specialization( candidate->specialization() );

	    TypeDesc  cleanParams = params;
	    cleanParams.setName( "" );
	    
	    m_matched = matchParameters( specialization, cleanParams );

		if( m_matched ) {
			//Make sure that all template-parameters were found
			for( int a = 0; a < m_candidateParams.count(); a++ ) {
				SimpleTypeImpl::TemplateParamInfo::TemplateParam t;
				if( m_candidateParams.getParam( t, a ) ) {
					if( !m_hadParameters.contains( t.name ) ) {
						m_matched = false;
					}
				} else {
					m_matched = false;
				}
			}
		}
    }

	///@todo: use all default-parameters if some are missing
	///@todo: also use decoration like "const" or "&" for specialization.
	bool matchParameters( const TypeDesc& specialization, const LocateResult& params, int depth = 0 ) {
		if( depth > m_maxDepth ) m_maxDepth = depth;

		if( specialization.name().isEmpty() ) {
			if( specialization.templateParams().count() != params->templateParams().count() )
				return false;
		} else {
				SimpleTypeImpl::TemplateParamInfo::TemplateParam t;
				if( m_candidateParams.getParam( t, specialization.name() ) ) {
					TypeDesc oldValue = t.value;

					//Check if the decoration of the specialization matches the decoration of the arguments, if not we have a mismatch.

					if( specialization.pointerDepth() > params->pointerDepth() ) {
						return false; //The decoration does not match the given argument
					} else {
						depth += specialization.pointerDepth();
						if( depth > m_maxDepth ) m_maxDepth = depth;
					}
					
					//Fill the template-parameter, or compare if the one we already found out matches this one
					LocateResult val;
					if( specialization.hasTemplateParams() ) {
						val = params->decoratedName();
					} else {
						val = params;  //No more parameters have to be checked, so take the value and return later
					}

					val->setPointerDepth( val->pointerDepth() - specialization.pointerDepth() );
					
					t.value = val;
					
					if( m_hadParameters.contains( t.name ) && oldValue != t.value ) {
						return false; ///We have a mismatch, two different values for the same template-parameter.
					} else {
						m_candidateParams.addParam( t );
						m_hadParameters[ t.name ] = val;
						if( !specialization.hasTemplateParams() ) return true;
					}
				} else {
					if( m_candidate->locateDecType( specialization.decoratedName() )->decoratedName() != params->decoratedName() ) {
					//We have a mismatch
					return false;
					}
				}
		}
			

		if( specialization.templateParams().count() != params->templateParams().count() ) {
			return false; //mismatch in count of template-parameters
		}
		
		TypeDesc::TemplateParams::const_iterator specialIt = specialization.templateParams().begin();
		TypeDesc::TemplateParams::const_iterator paramsIt = params->templateParams().begin();
		
		while( specialIt != specialization.templateParams().end() && paramsIt != params->templateParams().end() ) {
			if( !matchParameters( (*specialIt).desc(), (*paramsIt), depth+10 ) ) return false;
			
			++paramsIt;
			++specialIt;
		}
		return true;
	}

    operator bool() const {
      return m_matched;
    }

    ///True if this match is better than the given one
    bool operator > ( const TemplateParamMatch& rhs ) const {
	    if( !m_matched ) return false;
	    if(!rhs.m_matched ) return true;
	    return m_maxDepth > rhs.m_maxDepth;
    }

    TypePointer type() {
	    if( m_candidate ) {
		    TypePointer ret = m_candidate->clone();
		    ret->descForEdit().templateParams().clear();
		    for( int a = 0; a < m_candidateParams.count(); a++ ) {
			    SimpleTypeImpl::TemplateParamInfo::TemplateParam tp;
			    if( m_candidateParams.getParam( tp, a ) ) {
				    ret->descForEdit().templateParams().push_back( m_hadParameters[tp.name] );
			    } else {
				    ret->descForEdit().templateParams().push_back( LocateResult() ); //error
			    }
		    }
		    return ret;
	    } else {
		    return 0;
	    }
    }

	SimpleTypeImpl::TemplateParamInfo& templateParams() {
		return m_candidateParams;
	}
	
  private:
	TypePointer m_candidate;
    SimpleTypeImpl::TemplateParamInfo m_candidateParams;
	QMap<QString, LocateResult> m_hadParameters;
    bool m_matched;
	int m_maxDepth;
};

void SimpleTypeImpl::chooseSpecialization( MemberInfo& member ) {

  if ( member.memberType != MemberInfo::NestedType )
    return ;
  if ( !member.type->hasTemplateParams() )
    return ;

  TypePointer type = member.build();

  if ( !type )
    return ;

  //Get a list of all candidate-classes
  TypePointer t = this;
  if ( m_masterProxy )
    t = m_masterProxy;

  QValueList<TypePointer> classes = t->getMemberClasses( type->desc().name() );

  //Find the specialization that fits the given template-parameters the best

  if ( !type->specialization().isEmpty() ) {
    kdDebug( 9007 ) << "a specialized template-class was suggested as primary class while searching for specialization, search problematic" << endl;
    //return;
  } else {
    TemplateParamInfo params = type->getTemplateParamInfo();

	int dif = params.count() - member.type->templateParams().count();

    if ( dif > 0 ) {
      //fill up missing template-parameters with their default-parameters, maybe should be done in findMember
      for ( int a = member.type->templateParams().count(); a < params.count(); a++ ) {
        LocateResult r;
        TemplateParamInfo::TemplateParam tp;
        if ( params.getParam( tp, a ) ) {
          r = t->locateDecType( tp.value );
        }
		member.type->templateParams().push_back( r );
      }
    }
  }

  //now find the class that is most specialized and matches the template-parameters

  TemplateParamMatch bestMatch;

  for ( QValueList<TypePointer>::iterator it = classes.begin(); it != classes.end(); ++it ) {
    if ( ( *it ) ->specialization().isEmpty() )
      continue;
    TemplateParamMatch match( ( *it ), member.type.desc() );

    if ( match > bestMatch )
      bestMatch = match;
  }

  if ( bestMatch ) {
    TypePointer tp = bestMatch.type();
	if ( tp ) {
      member.setBuilt( tp );
	}
  }
}


LocateResult SimpleTypeImpl::locateType( TypeDesc name , LocateMode mode , int dir , MemberInfo::MemberType typeMask ) {
  Debug d( "#lo#" );
  if( BuiltinTypes::isBuiltin( name ) )
    return name;

  if ( !name || !safetyCounter || !d ) {
    return desc();
  }
  if ( !d ) {
    ifVerbose( dbg() << "stopping location because the recursion-depth is too high" << endl );
    return TypeDesc( "CompletionError::too_much_recursion" );
  }
    ifVerbose( dbg() << "\(" << uint(this) << ")\"" << str() << "\": locating type \"" << name.fullNameChain() << "\"" << endl );
  if ( name.resolved() && !name.next() ) {
    ifVerbose( dbg() << "\"" << desc().fullName() << "\": type \"" << name.fullNameChain() << "\" is already resolved, returning stored instance" << endl );
    return name;
  }
  /*
     if( name.resolved() && name.length() == name.resolved()->desc().length() ) {
     ifVerbose( dbg() << "\"" << desc().fullName() << "\": type \"" << name.fullNameChain() << "\" is already resolved, returning stored instance" << endl;
       SimpleType ret = SimpleType( name.resolved() );
       
       if( ! (name == ret->desc()) ) {
         ret.makePrivate();  ///Maybe some small parameters like the pointer-depth were changed, so customize those
         ret->parseParams( name );
       }
       
       return ret;
     }*/
  if( name.next() ) {
    //This is an optimization for better use of the cache: Find the elements separately, so searches
    //For elements that start with the same scope will be speeded up.
    LocateResult r = locateType( name.firstType(), mode, dir, typeMask );
    if( r && r->resolved() && r.locateMode().valid ) {
      ifVerbose( dbg() << "splitting location" );
      TypeDesc d( *name.next() );
      d.setIncludeFiles( name.includeFiles() );
      return r->resolved()->locateType( d, (LocateMode)r.locateMode().mode, r.locateMode().dir );
    }
  }

  LocateResult ret = name; ///In case the type cannot be located, this helps to find at least the best match
  //LocateResult ret;

  TypeDesc first = resolveTemplateParams( name.firstType(), mode );

  MemberInfo mem = findMember( first, typeMask );

  switch ( mem.memberType ) {
    case MemberInfo::Namespace:
    if ( mode & ExcludeNamespaces )
      break;
    case MemberInfo::NestedType: {
      if ( mem.memberType == MemberInfo::NestedType && mode & ExcludeNestedTypes )
        break;

      SimpleType sub;
      if ( TypePointer t = mem.build() ) {
        sub = SimpleType( t );
#ifdef PHYSICAL_IMPORT
        setSlaveParent( *sub );
#endif
      } else {
        ///Should not happen..
        kdDebug( 9007 ) << "\"" << str() << "\": Warning: the nested-type " << name.name() << " was found, but has no build-info" << endl;
        return TypeDesc( "CompletionError::unknown" );
      }

      TypeDesc rest;
      LocateMode newMode = addFlag( mode, ExcludeTemplates );
      int newDir = 1;
      if ( name.next() ) {
        ifVerbose( dbg() << "\"" << str() << "\": found nested type \"" << name.name() << "\", passing control to it\n" );
        ret = sub->locateType( resolveTemplateParams( *name.next(), Normal ), newMode, newDir ); ///since template-names cannot be referenced from outside, exclude them for the first cycle
        ret.increaseResolutionCount();
        if ( ret->resolved() )
          return ret.resetDepth();
      } else {
        ifVerbose( dbg() << "\"" << str() << "\": successfully located searched type \"" << name.fullNameChain() << "\"\n" );
        ret->setResolved( sub.get() );
        ret->resolved()->setFindIncludeFiles( name.includeFiles() );
        ret.locateMode().valid = true;
        ret.locateMode().mode = (uint)newMode;
        ret.locateMode().dir = newDir;
        return ret.resetDepth();
      }
      break;
    }
    case MemberInfo::Typedef:
    if ( mode & ExcludeTypedefs )
      break;
    case MemberInfo::Template: {
      if ( mem.memberType == MemberInfo::Template && ( mode & ExcludeTemplates ) )
        break;
      ifVerbose( dbg() << "\"" << str() << "\": found " << mem.memberTypeToString() << " \"" << name.name() << "\" -> \"" << mem.type->fullNameChain() << "\", recursing \n" );
      if ( name.hasTemplateParams() ) {
        ifVerbose( dbg() << "\"" << str() << "\":warning: \"" << name.fullName() << "\" is a " << mem.memberTypeToString() << ", but it has template-params itself! Not matching" << endl );
      } else {
        if ( mem.type->name() != name.name() ) {

          MemberInfo m = mem;
          if ( name.next() ) {
            mem.type->makePrivate();
            mem.type->append( name.next() );
          }
          ret = locateDecType( mem.type, remFlag( mode, ExcludeTemplates ) );

          if ( mem.memberType == MemberInfo::Template )
            ret.addResolutionFlag( HadTemplate );
          if ( mem.memberType == MemberInfo::Typedef )
            ret.addResolutionFlag( HadTypedef );
          ret.increaseResolutionCount();
          // 	          if( mode & TraceAliases && ret->resolved() )
          {
            m.name = "";

            if ( !scope().isEmpty() ) {
              m.name = fullTypeUnresolvedWithScope() + "::";
            }
            m.name += name.nameWithParams();
            //m.name += name.fullNameChain();

            if ( name.next() ) {
              if ( m.type.trace() ) {
                ret.trace() ->prepend( *m.type.trace(), 1 );
              }
              ret.trace() ->prepend( m, *name.next() );
            } else {
              if ( m.type.trace() )
                ret.trace() ->prepend( *m.type.trace(), 1 );
              ret.trace() ->prepend( m );
            }
          }

          if ( ret->resolved() )
            return ret.resetDepth();
        } else {
          ifVerbose( dbg() << "\"" << str() << "\"recursive typedef/template found: \"" << name.fullNameChain() << "\" -> \"" << mem.type->fullNameChain() << "\"" << endl );
        }
      }
      break;
    }
    ///A Function is treated similar to a type
    case MemberInfo::Function: {
      if ( !name.next() ) {
        TypePointer t = mem.build();
        if ( t ) {
          return t->desc();
        } else {
          ifVerbose( dbg() << "\"" << str() << "\"" << ": could not build function: \"" << name.fullNameChain() << "\"" );
        }
      } else {
        ifVerbose( dbg() << "\"" << str() << "\"" << ": name-conflict: searched for \"" << name.fullNameChain() << "\" and found function \"" << mem.name << "\"" );
      }
      break;
    };
    ///Currently there is no representation of a Variable as a SimpleType, so only the type of the variable is used.
    case MemberInfo::Variable: {
      return locateDecType( mem.type, remFlag( mode, ExcludeTemplates ) ).resetDepth();
    }
  }

  ///Ask bases but only on this level
  if ( ! ( mode & ExcludeBases ) ) {

    QValueList<LocateResult> bases = getBases();
    if ( !bases.isEmpty() ) {
      TypeDesc nameInBase = resolveTemplateParams( name, LocateBase ); ///Resolve all template-params that are at least visible in the scope of the base-declaration

      for ( QValueList<LocateResult>::iterator it = bases.begin(); it != bases.end(); ++it ) {
        if ( !( *it ) ->resolved() )
          continue;
        LocateResult t = ( *it ) ->resolved() ->locateType( nameInBase, addFlag( addFlag( mode, ExcludeTemplates ), ExcludeParents ), dir ); ///The searched Type cannot directly be a template-param in the base-class, so ExcludeTemplates. It's forgotten early enough.
        if ( t->resolved() )
          return t.increaseDepth();
        else
          if ( t > ret )
            ret = t.increaseDepth();
      }
    }
  }

  ///Ask parentsc
  if ( !scope().isEmpty() && dir != 1 && ! ( mode & ExcludeParents ) ) {
    LocateResult rett = parent() ->locateType( resolveTemplateParams( name, mode & ExcludeBases ? ExcludeBases : mode ), mode & ForgetModeUpwards ? Normal : mode );
    if ( rett->resolved() )
      return rett.increaseDepth();
    else
      if ( rett > ret )
        ret = rett.increaseDepth();
  }

  ///Ask the bases and allow them to search in their parents.
  if ( ! ( mode & ExcludeBases ) ) {
    TypeDesc baseName = resolveTemplateParams( name, LocateBase ); ///Resolve all template-params that are at least visible in the scope of the base-declaration
    QValueList<LocateResult> bases = getBases();
    if ( !bases.isEmpty() ) {
      for ( QValueList<LocateResult>::iterator it = bases.begin(); it != bases.end(); ++it ) {
        if ( !( *it ) ->resolved() )
          continue;
        LocateResult t = ( *it ) ->resolved() ->locateType( baseName, addFlag( mode, ExcludeTemplates ), dir ); ///The searched Type cannot directly be a template-param in the base-class, so ExcludeTemplates. It's forgotten early enough.
        if ( t->resolved() )
          return t.increaseDepth();
        else
          if ( t > ret )
            ret = t.increaseDepth();
      }
    }
  }

  ///Give the type a desc, so the nearest point to the searched type is stored
  ifVerbose( dbg() << "\"" << str() << "\": search for \"" << name.fullNameChain() << "\" FAILED" << endl );
  return ret;
}

void SimpleTypeImpl::breakReferences() {
  TypePointer p( this ); ///necessary so this type is not deleted in between
  m_parent = 0;
  m_desc.resetResolved();
  //	m_trace.clear();
  m_masterProxy = 0;
  invalidateCache();
}

TypePointer SimpleTypeImpl::bigContainer() {
  if ( m_masterProxy )
    return m_masterProxy;
  else
    return TypePointer( this );
}

SimpleType SimpleTypeImpl::parent() {
  if ( m_parent ) {
    //ifVerbose( dbg() << "\"" << str() << "\": returning parent" << endl;
    return SimpleType( m_parent );
  } else {
    ifVerbose( dbg() << "\"" << str() << "\": locating parent" << endl );
    invalidateSecondaryCache();
    QStringList sc = scope();

    if ( !sc.isEmpty() ) {
      sc.pop_back();
      SimpleType r = SimpleType( sc, m_desc.includeFiles() );
      if ( &( *r.get() ) == this ) {
        kdDebug( 9007 ) << "error: self set as parent: " << m_scope.join( "::" ) << "(" << m_scope.count() << ")" << ", " << sc.join( "::" ) << "(" << sc.count() << ")" /* << kdBacktrace()*/ << endl;
        return SimpleType( new SimpleTypeImpl( "" ) );
      }
      m_parent = r.get();
      return r;
    } else {
      ifVerbose( dbg() << "\"" << str() << "\"warning: returning parent of global scope!" << endl );
      return SimpleType( new SimpleTypeImpl( "" ) );
    }
  }
}

const TypeDesc& SimpleTypeImpl::desc() {
  if ( ! scope().isEmpty() )
    ;
  if ( m_desc.name().isEmpty() )
    m_desc.setName( cutTemplateParams( scope().back() ) );
  m_desc.setResolved( this );
  return m_desc;
}

TypeDesc& SimpleTypeImpl::descForEdit() {
  desc();
  invalidateCache();
  return m_desc;
}

QString SimpleTypeImpl::describeWithParams() {
  TemplateParamInfo pinfo = getTemplateParamInfo();
  int num = 0;
  TemplateParamInfo::TemplateParam param;
  QString str = desc().name();
  if ( desc().hasTemplateParams() ) {
    str += "< ";

    for ( TypeDesc::TemplateParams::const_iterator it = desc().templateParams().begin(); it != desc().templateParams().end(); ++it ) {
      if ( pinfo.getParam( param, num ) && !param.name.isEmpty() )
        str += param.name;
      else
        str += "[unknown name]";

      str += " = " + ( *it ) ->fullNameChain() + ", ";
      ++num;
    }

    str.truncate( str.length() - 2 );
    str += " >";
  }
  return str;
}

QString SimpleTypeImpl::fullTypeResolved( int depth ) {
  Debug d( "#tre#" );

  TypeDesc t = desc();
  if ( !scope().isEmpty() ) {
    if ( depth > 10 )
      return "KDevParseError::ToDeep";
    if ( !safetyCounter )
      return "KDevParseError::MaximumCountReached";

    ifVerbose( dbg() << "fully resolving type " << t.fullName() << endl );
    if ( scope().size() != 0 ) {
      t = resolveTemplateParams( t, LocateBase );
    }
  }

  return t.fullNameChain();
}


QString SimpleTypeImpl::fullTypeUnresolvedWithScope( ) {
  if ( m_parent && !m_parent->scope().isEmpty() ) {
    return m_parent->fullTypeUnresolvedWithScope() + "::" + m_desc.fullNameChain();
  } else {
    return m_desc.fullNameChain();
  }
}

QString SimpleTypeImpl::fullTypeResolvedWithScope( int depth ) {
  Q_UNUSED( depth );
  if ( !m_scope.isEmpty() && parent() ) {
    return parent() ->fullTypeResolvedWithScope() + "::" + fullTypeResolved();
  } else {
    return fullTypeResolved();
  }
}

void SimpleTypeImpl::checkTemplateParams () {
  invalidateCache();
  if ( ! m_scope.isEmpty() ) {
    QString str = m_scope.back();
    m_desc = str;
    if ( !m_desc.name().isEmpty() ) {
      m_scope.pop_back();
      m_scope << m_desc.name();
    } else {
      kdDebug() << "checkTemplateParams() produced bad scope-tail: \"" << m_desc.name() << "\", \"" << m_scope.join( "::" ) << "\"" << endl;
    }
  }
}

void SimpleTypeImpl::setScope( const QStringList& scope ) {
  invalidateCache();
  m_scope = scope;
  if ( m_scope.count() == 1 && m_scope.front().isEmpty() ) {
    kdDebug() << "bad scope set " << kdBacktrace() << endl;
    m_scope = QStringList();
  }
}

SimpleTypeImpl::TypeOfResult SimpleTypeImpl::searchBases ( const TypeDesc& name /*option!!*/ ) {
  QValueList<LocateResult> parents = getBases();
  for ( QValueList<LocateResult>::iterator it = parents.begin(); it != parents.end(); ++it ) {
    if ( !( *it ) ->resolved() )
      continue;
    TypeOfResult type = ( *it ) ->resolved() ->typeOf( name );
    if ( type )
      return type;
  }
  return TypeOfResult();
}

void SimpleTypeImpl::setSlaveParent( SimpleTypeImpl& slave ) {
  if ( ! m_masterProxy ) {
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
  if ( it != m_paramsByName.end() ) {
    target = *it;
    return true;
  }
  return false;
}

bool SimpleTypeImpl::TemplateParamInfo::getParam( TemplateParam& target, int number ) const {
  QMap<int, TemplateParam>::const_iterator it = m_paramsByNumber.find( number );
  if ( it != m_paramsByNumber.end() ) {
    target = *it;
    return true;
  }
  return false;
}

void SimpleTypeImpl::TemplateParamInfo::removeParam( int number ) {
  QMap<int, TemplateParam>::iterator it = m_paramsByNumber.find( number );
  if ( it != m_paramsByNumber.end() ) {
    m_paramsByName.remove( ( *it ).name );
    m_paramsByNumber.remove( it );
  }
}

void SimpleTypeImpl::TemplateParamInfo::addParam( const TemplateParam& param ) {
  m_paramsByNumber[ param.number ] = param;
  m_paramsByName[ param.name ] = param;
}

int SimpleTypeImpl::TemplateParamInfo::count() const {
  QMap<int, TemplateParam>::const_iterator it = m_paramsByNumber.end();
  if ( it != m_paramsByNumber.begin() ) {
	  --it;
    return ( *it ).number + 1;
  } else {
    return 0;
  }
}

void SimpleTypeConfiguration::setGlobalNamespace( TypePointer globalNamespace ) {
  if ( !globalNamespace->scope().isEmpty() ) {
    kdDebug( 9007 ) << "error while setting global scope\n" << kdBacktrace() << endl;
    SimpleType::setGlobalNamespace( new SimpleTypeImpl( "" ) );
  } else {
    SimpleType::setGlobalNamespace( globalNamespace );
  }
}

// kate: indent-mode csands; tab-width 4;

