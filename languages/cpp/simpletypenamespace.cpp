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
#include <qtl.h>
#include "simpletypenamespace.h"
#include "simpletypecachebinder.h"

#include "safetycounter.h"

//SimpleTypeNamespace implementation

TypePointer SimpleTypeNamespace::clone() {
 return new SimpleTypeCachedNamespace( this );
}

SimpleTypeNamespace::SimpleTypeNamespace( const QStringList& fakeScope, const QStringList& realScope ) : SimpleTypeImpl( fakeScope ) {
  ifVerbose( dbg() << "\"" << str() << "\": created namespace-proxy with real scope \"" << realScope.join( "::" ) << "\"" << endl );
     addImport( realScope.join("::") );
}

SimpleTypeNamespace::SimpleTypeNamespace( const QStringList& fakeScope ) : SimpleTypeImpl( fakeScope ) {
    ifVerbose( dbg() << "\"" << str() << "\": created namespace-proxy" << endl );
}

SimpleTypeNamespace::SimpleTypeNamespace( SimpleTypeNamespace* ns ) : SimpleTypeImpl( ns ) {
 ifVerbose( dbg() << "\"" << str() << "\": cloning namespace" << endl );
 m_aliases = ns->m_aliases;
 m_activeSlaves = ns->m_activeSlaves;
}

SimpleTypeImpl::MemberInfo SimpleTypeNamespace::findMember( TypeDesc name, MemberInfo::MemberType type ) {
 std::set
  <SimpleTypeNamespace*> ignore;
 SimpleTypeImpl::MemberInfo ret = findMember( name, type, ignore );
///chooseSpecialization( ret ); should not be necessary
 return ret;
}

QValueList<TypePointer> SimpleTypeNamespace::getMemberClasses( const TypeDesc& name ) {
 updateAliases( name.includeFiles() );
 QValueList<TypePointer> ret;

 SlaveList l = getSlaves( name.includeFiles() );
 for ( SlaveList::iterator it = l.begin(); it != l.end(); ++it ) {
     if( ( *it ).resolved() )
        ret += ( *it ).resolved()->getMemberClasses( name );
 }

 return ret;
}

SimpleTypeImpl::MemberInfo SimpleTypeNamespace::findMember( TypeDesc name, MemberInfo::MemberType type, std::set
   <SimpleTypeNamespace*>& ignore ) {
 updateAliases( name.includeFiles() );
 MemberInfo mem;
 mem.name = "";
 mem.memberType = MemberInfo::NotFound;
 if ( ignore.find( this ) != ignore.end() )
  return mem;
 ignore.insert( this );

 ImportList m_aliasImports;

 AliasMap::iterator itt = m_aliases.find( name.name() );

 if ( itt != m_aliases.end() && !( *itt ).empty() ) {
     ifVerbose( dbg() << "\"" << str() << "\": namespace-sub-aliases \"" << name.name() << "\"" << "\" requested, locating targets" << endl );

  for ( ImportList::iterator it = ( *itt ).begin(); it != ( *itt ).end(); ++it ) {
   if ( !( name.includeFiles().size() < 1 || (*it).files <= name.includeFiles() ) ) continue; //filter the slave by the include-files

  ifVerbose( dbg() << "\"" << str() << "\": namespace-sub-aliases \"" << name.name() << "\": taking target \"" << ( *it ).import.fullNameChain() << "\"" << endl );
  TypeDesc d( (*it).import );
   d.setIncludeFiles( name.includeFiles() );
   LocateResult l = locateDecType( d, SimpleTypeImpl::Normal, 0, SimpleTypeImpl::MemberInfo::Namespace );
   if ( !l || !l->resolved() || !dynamic_cast<SimpleTypeNamespace*>( l->resolved().data() ) ) {
    ifVerbose( dbg() << "\"" << str() << "\": namespace-sub-aliases \"" << name.name() << "\" -> \"" << ( *it ).import.fullNameChain() << "\" could not be resolved" << endl );
   } else {
    m_aliasImports.insert( Import(  d.includeFiles(), l ) );
   }
  }
 }

 SlaveList l = getSlaves( name.includeFiles() );
 for ( SlaveList::iterator it = l.begin(); it != l.end(); ++it ) {
     //if ( !( name.includeFiles().size() < 1 || (*it).includeFiles() <= name.includeFiles() ) ) continue; //filter the slave by the include-files(done in getSlaves now)
  ifVerbose( dbg() << "\"" << str() << "\": redirecting search for \"" << name.name() << "\" to \"" << ( *it ) .fullNameChain() << "\"" << endl );
  if( !( *it ).resolved() ) {
  ifVerbose( dbg() << "\"" << str() << "\": while search for \"" << name.name() << "\": Imported namespace \"" << ( *it ) .fullNameChain() << "\" is not resolved(should have been resolved in updateAliases)" << endl );
      continue;
  }
 ifVerbose( dbg() << "\"Class-type: " << typeid( *(*it).resolved().data() ).name() << ")" << endl );
  mem = ( *it ).resolved() ->findMember( name , type );
  if ( mem ) {
  if ( mem.memberType != MemberInfo::Namespace ) {
     TypePointer b = mem.build();
     if ( b && !( b->parent().get().data() == this ) ) {
     b = b ->clone();
     b->setParent( this );
     mem.setBuilt( b );
    }
    return mem;
   } else {
       TypePointer b = mem.build();
       
       if( b )
           m_aliasImports.insert( Import( IncludeFiles(), b->desc() ) );
       else
           ifVerbose( dbg() << "\"" << str() << "\": found namespace \"" << name.name() << "\", but it is not resolved" << endl );
   }
  }
 }

 if ( !m_aliasImports.empty() ) {
     return setupMemberInfo( name.fullNameList().join( "::" ), m_aliasImports );
     
 }

 return mem;
}

// LocateResult SimpleTypeNamespace::locateSlave( const SlaveList::const_iterator& target, const IncludeFiles& includeFiles ) {
//     for( SlaveList::const_iterator it = m_activeSlaves.begin(); it != target; ++it ) {
//         
//     }
// }

SimpleTypeImpl::MemberInfo SimpleTypeNamespace::setupMemberInfo( const QStringList& subName, const ImportList& imports ) {
 MemberInfo mem;
 mem.name = subName.join("::");
 mem.memberType = MemberInfo::NotFound;
 QStringList sc = scope();
 sc += subName;
 mem.type = sc.join( "::" );
 mem.memberType = MemberInfo::Namespace;
 mem.setBuildInfo( new NamespaceBuildInfo( sc, imports ) );
 return mem;
}


/*TypePointer SimpleTypeNamespace::locateNamespace( const TypeDesc& alias ) {
    ifVerbose( dbg() << "\"" << str() << "\": locating namespace \"" << alias.fullNameChain() << "\"" << endl );
    updateAliases( name.includeFiles() );

  TypePointer locateIn = this;
  if ( !scope().isEmpty() )
    locateIn = parent().get();
  LocateResult res;
  if ( locateIn )
    res = locateIn->locateDecType( alias, addFlag( ExcludeNestedTypes, ExcludeTemplates ), 0, MemberInfo::Namespace );
  if ( !res->resolved() )
    return 0;
  if ( isANamespace( res->resolved() ) ) {
    return res->resolved();
    ifVerbose( dbg() << "\"" << str() << "\": successfully located namespace \"" << res->fullNameChain() << "\"" << endl );
  } else {
    ifVerbose( dbg() << "\"" << str() << "\": searched for a namespace, but found \"" << res->fullNameChain() << "\"" << endl );
  }

  ifVerbose( dbg() << "\"" << str() << "\": failed to locate namespace \"" << alias << "\"" << endl );

  return 0;
}*/

///This must be optimized
/*void SimpleTypeNamespace::recurseAliasMap() {
    //updateAliases();

  bool changed = true;
  SafetyCounter s( 1000 );
  AliasMap::iterator import = m_aliases.find( "" );

  while ( changed && s ) {
    ///this has bad runtime-performance for high counts, but it's simple and we shouldn't have that many maps :-)
    changed = false;
    for ( AliasMap::iterator it = m_aliases.begin(); it != m_aliases.end(); ++it ) {
      ImportList& l = *it;
      for ( ImportList::iterator strIt = l.begin(); strIt != l.end(); ++strIt ) {
        AliasMap::iterator fit = m_aliases.find( ( *strIt ).alias );

        for ( ImportList::iterator oit = ( *fit ).begin(); oit != ( *fit ).end(); ++oit ) {
            if (  it.key() == ( *oit ).alias )
                continue;
            HashedStringSet cfiles = ( *oit ).files + ( *strIt ).files;
            std::pair< ImportList::const_iterator, ImportList::const_iterator > rng = ( l ).equal_range( *oit );
            bool have = false;
            while ( rng.first != rng.second ) {
                if ( rng.first->files == cfiles ) {
                    have = true ; //The same alias, with the same files, has already been added.
                    break;
                }
                ++rng.first;
            }
            if( have ) continue;

          addAliasMap( it.key(), ( *oit ).alias, cfiles, true );
          changed = true;
          break;
        }
        if ( changed )
          break;
      }
      if ( changed )
        break;
    }
  }

  if ( !s ) {
    ifVerbose( dbg() << "\"" << str() << "\": too much recursion while applying namespace-aliases" << endl );
  }
}*/

void SimpleTypeNamespace::addAliasMap( const TypeDesc& name, const TypeDesc& alias, const IncludeFiles& files, bool recurse, bool symmetric ) {
 Debug db;
 if ( !db ) {
  kdDebug( 9007 ) << str() << " addAliasMap: cannot add alias \"" << name.fullNameChain() << "\" -> \"" << alias.fullNameChain() << "\", recursion too deep" << endl;
  return ;
 }
if( alias.next() ) kdDebug( 9007 ) << "addAliasMap warning: type-alias has order higher than one: " << alias.fullNameChain() << ", only " << alias.name() << " will be used" <<  endl;
 if ( name == alias.name() )
  return ;

 if ( symmetric )
  addAliasMap( alias, name, files, recurse, false );

 invalidateSecondaryCache();
 invalidatePrimaryCache( true ); //Only not-found items are cleared updated here for performance-reasons(found items will stay cached)

 AliasMap::iterator it = m_aliases.find( name.name() );
 if ( it == m_aliases.end() )
  it = m_aliases.insert( name.name(), ImportList() );

 Import a( files, alias );
 std::pair< ImportList::const_iterator, ImportList::const_iterator > rng = ( *it ).equal_range( a );
 while ( rng.first != rng.second ) {
  if ( rng.first->files == files )
   return ; //The same alias, with the same files, has already been added.
  ++rng.first;
 }

 ( *it ).insert( a );
 ifVerbose( dbg() << "\"" << str() << "\": adding namespace-alias \"" << name.name() << ( !symmetric ? "\" -> \"" : "\" = \"" ) << alias.name() << "\"" << endl );

 if ( name.name().isEmpty() ) {
   addImport( alias, files );
 }
}

void SimpleTypeNamespace::updateAliases( const IncludeFiles& files ) {
    SlaveList tempList;
    if( m_activeSlaves.empty() ) return;
    while( !m_activeSlaves.empty() ) {
        if( !m_activeSlaves.back().resolved() && ( !( files.size() < 1 || m_activeSlaves.back().includeFiles() <= files ) ) ) {
            TypeDesc desc = m_activeSlaves.back();
            m_activeSlaves.erase( --m_activeSlaves.end() );
            desc = locateDecType( desc, SimpleTypeImpl::Normal, 0, SimpleTypeImpl::MemberInfo::Namespace );
            if( desc.resolved() ) {
                desc.setResolved( desc.resolved()->clone() );
                desc.resolved()->setMasterProxy( this );
                m_activeSlaves.push_back( desc );
                desc.resolved()->addAliasesTo( this );
            }
            
            updateAliases( files );
            break;
        } else {
            tempList.splice( tempList.begin(), m_activeSlaves, --m_activeSlaves.end() );
        }
    }
    m_activeSlaves.splice( m_activeSlaves.end(), tempList );
}


void SimpleTypeNamespace::addAliases( QString map, const IncludeFiles& files ) {
 while ( !map.isEmpty() ) {
  int mid = map.find( "=" );
  int mid2 = map.find( "<<" );
  int found = mid;
  int len = 1;
  if ( mid2 != -1 && ( mid2 < found || found == -1 ) ) {
   found = mid2;
   len = 2;
  }
  if ( found == -1 )
   break;

  int end = map.find( ";", found + len );
  if ( end == -1 ) {
   //break;
   end = map.length();
  }
  if ( end - ( found + len ) < 0 )
   break;

  addAliasMap( map.left( found ).stripWhiteSpace(), map.mid( found + len, end - found - len ).stripWhiteSpace(), files, true, found == mid );
  map = map.mid( end + 1 );
 }
}

void SimpleTypeNamespace::addImport( const TypeDesc& import, const IncludeFiles& files ) {
 invalidateCache();
    if( import.name().isEmpty() ) {
        ///The global namespace is the entry-point, fill it by hand:
        SimpleType cm = SimpleType( QString(), HashedStringSet(), CodeModel );
        SimpleType ct = SimpleType( QString(), HashedStringSet(), Catalog );
        cm->setMasterProxy( this );
        ct->setMasterProxy( this );
        m_activeSlaves.push_back( cm->desc() );
        m_activeSlaves.push_back( ct->desc() );
        cm->addAliasesTo( this );
        ct->addAliasesTo( this );
    } else {
        TypeDesc d = import;
        d.setIncludeFiles( files );
        m_activeSlaves.push_back( d );
        if( d.resolved() ) {
            d.setResolved( d.resolved()->clone() );
            d.resolved()->setMasterProxy( this );
            d.resolved()->addAliasesTo( this );
        }
    }
}

bool SimpleTypeNamespace::hasNode() const {
 return true;
}

SimpleTypeNamespace::SlaveList SimpleTypeNamespace::getSlaves( const IncludeFiles& files ) {
    updateAliases( files );
    SlaveList ret;
    for( SlaveList::const_iterator it = m_activeSlaves.begin(); it != m_activeSlaves.end(); ++it ) {
        if ( !( files.size() < 1 || (*it).includeFiles() <= files ) ) continue;
        ret.push_back( *it );
    }
 return ret;
}

//SimpleTypeNamespace::NamespaceBuildInfo implementation

TypePointer SimpleTypeNamespace::NamespaceBuildInfo::build() {
 if ( m_built )
  return m_built;
    m_built = new SimpleTypeCachedNamespace( m_fakeScope );
    for ( ImportList::iterator it = m_imports.begin(); it != m_imports.end(); ++it )  {
        TypeDesc i = (*it).import;
        if( i.resolved() ) {
            //            i.setResolved( i.resolved()->clone() );
        }
                                        
    ( ( SimpleTypeCachedNamespace* ) m_built.data() ) ->addAliasMap( TypeDesc(), i, ( *it ).files );
    }
 return m_built;
}

// kate: indent-mode csands; tab-width 4;
