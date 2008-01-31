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
#include <typeinfo>
#include "simpletypenamespace.h"
#include "simpletypecachebinder.h"

#include "safetycounter.h"

//#define PHYSICAL_IMPORT
//Necessary, because else nested members cannot search the correct scope
#define PHYSICALLY_IMPORT_NAMESPACES

extern SafetyCounter safetyCounter;

//SimpleTypeNamespace implementation

TypePointer SimpleTypeNamespace::clone() {
  return new SimpleTypeCachedNamespace( this );
}

SimpleTypeNamespace::SimpleTypeNamespace( const QStringList& fakeScope, const QStringList& realScope ) : SimpleTypeImpl( fakeScope ), m_currentSlaveId(0) {
  ifVerbose( dbg() << "\"" << str() << "\": created namespace-proxy with real scope \"" << realScope.join( "::" ) << "\"" << endl );
  SimpleType cm = SimpleType( realScope, HashedStringSet(), RepoCodeModel );
  SimpleType ct = SimpleType( realScope, HashedStringSet(), RepoCatalog );
  cm = SimpleType( cm->clone() );
  ct = SimpleType( ct->clone() );
  cm->setMasterProxy( this );
  ct->setMasterProxy( this );
  addImport( cm->desc() );
  addImport( ct->desc() );
}

SimpleTypeNamespace::SimpleTypeNamespace( const QStringList& fakeScope ) : SimpleTypeImpl( fakeScope ), m_currentSlaveId(0) {
  ifVerbose( dbg() << "\"" << str() << "\": created namespace-proxy" << endl );
}

SimpleTypeNamespace::SimpleTypeNamespace( SimpleTypeNamespace* ns ) : SimpleTypeImpl( ns ), m_currentSlaveId(0) {
  ifVerbose( dbg() << "\"" << str() << "\": cloning namespace" << endl );
  m_aliases = ns->m_aliases;
  m_activeSlaves = ns->m_activeSlaves;
  m_activeSlaveGroups = ns->m_activeSlaveGroups;
}

void SimpleTypeNamespace::breakReferences() {
  m_aliases.clear();
  m_activeSlaves.clear();
  SimpleTypeImpl::breakReferences();
}


SimpleTypeImpl::MemberInfo SimpleTypeNamespace::findMember( TypeDesc name, MemberInfo::MemberType type ) {
  std::set<HashedString> ignore;
  SimpleTypeImpl::MemberInfo ret = findMember( name, type, ignore );
///chooseSpecialization( ret ); should not be necessary
  return ret;
}

QValueList<TypePointer> SimpleTypeNamespace::getMemberClasses( const TypeDesc& name ) {
  std::set<HashedString> ignore;

  return getMemberClasses( name, ignore );
}

QValueList<TypePointer>  SimpleTypeNamespace::getMemberClasses( const TypeDesc& name, std::set<HashedString>& ignore )  {
  HashedString myName = HashedString( scope().join( "::" ) +"%"+typeid( *this ).name() );
  if ( ignore.find( myName ) != ignore.end() || !safetyCounter )
    return QValueList<TypePointer>();

  ignore.insert( myName );

  QValueList<TypePointer> ret;

  SlaveList l = getSlaves( name.includeFiles() );
  for ( SlaveList::iterator it = l.begin(); it != l.end(); ++it ) {
    if (( *it ).first.first.resolved() ) {
      SimpleTypeNamespace* ns = dynamic_cast<SimpleTypeNamespace*>(( *it ).first.first.resolved().data() );
      if ( !ns ) {
        HashedString thatName = HashedString(( *it ).first.first.resolved()->scope().join( "::" ) +"%"+typeid( *( *it ).first.first.resolved() ).name() );
        if ( ignore.find( thatName ) != ignore.end() ) continue;
        ignore.insert( thatName );
        ret += ( *it ).first.first.resolved()->getMemberClasses( name );
      } else {
        ret += ns->getMemberClasses( name, ignore );

      }

    }
  }

  return ret;
}

SimpleTypeImpl::MemberInfo SimpleTypeNamespace::findMember( TypeDesc name, MemberInfo::MemberType type, std::set
      <HashedString>& ignore ) {
  MemberInfo mem;
  mem.name = "";
  mem.memberType = MemberInfo::NotFound;
  HashedString myName = HashedString( scope().join( "::" ) +"%"+typeid( *this ).name() );
  if ( ignore.find( myName ) != ignore.end() || !safetyCounter )
    return mem;
  ignore.insert( myName );

  SlaveList l = getSlaves( name.includeFiles() );

  ImportList m_aliasImports;

  AliasMap::iterator itt = m_aliases.find( name.name() );

  if ( itt != m_aliases.end() && !( *itt ).empty() ) {
    ifVerbose( dbg() << "\"" << str() << "\": namespace-sub-aliases \"" << name.name() << "\"" << "\" requested, locating targets" << endl );

    for ( ImportList::iterator it = ( *itt ).begin(); it != ( *itt ).end(); ++it ) {
      if ( !( /*name.includeFiles().size() < 1 ||*/ ( *it ).files <= name.includeFiles() ) ) continue; //filter the slave by the include-files

      ifVerbose( dbg() << "\"" << str() << "\": namespace-sub-aliases \"" << name.name() << "\": taking target \"" << ( *it ).import.fullNameChain() << "\"" << endl );
      /*TypeDesc d( (*it).import );
      d.setIncludeFiles( name.includeFiles() );*/
      m_aliasImports.insert( *it ); //@todo: what include-files should be used for searching the namespace?
      /*LocateResult l = locateDecType( d, SimpleTypeImpl::Normal, 0, SimpleTypeImpl::MemberInfo::Namespace );
      if ( !l || !l->resolved() || !dynamic_cast<SimpleTypeNamespace*>( l->resolved().data() ) ) {
      ifVerbose( dbg() << "\"" << str() << "\": namespace-sub-aliases \"" << name.name() << "\" -> \"" << ( *it ).import.fullNameChain() << "\" could not be resolved" << endl );
      } else {
      m_aliasImports.insert( Import(  d.includeFiles(), l, this ) );
      }*/
    }
  }

  for ( SlaveList::iterator it = l.begin(); it != l.end(); ++it ) {
    if ( !( *it ).first.first.resolved() )
      continue;
    if ( ignore.find( HashedString(( *it ).first.first.resolved()->scope().join( "::" ) +"%"+ typeid( *( *it ).first.first.resolved() ).name() ) ) != ignore.end() ) continue;

    ifVerbose( dbg() << "\"" << str() << "\": redirecting search for \"" << name.name() << "\" to \"" << ( *it ) .first.first.fullNameChain() << "\"" << endl );
    if ( !( *it ).first.first.resolved() ) {
      ifVerbose( dbg() << "\"" << str() << "\": while search for \"" << name.name() << "\": Imported namespace \"" << ( *it ) .first.first.fullNameChain() << "\" is not resolved(should have been resolved in updateAliases)" << endl );
      continue;
    }
    ifVerbose( dbg() << "\"Class-type: " << typeid( *( *it ).first.first.resolved().data() ).name() << ")" << endl );
    SimpleTypeNamespace* ns = dynamic_cast<SimpleTypeNamespace*>(( *it ).first.first.resolved().data() );

    if ( ns )
      mem = ns->findMember( name , type, ignore );
    else
      mem = ( *it ).first.first.resolved()->findMember( name, type );

    if ( mem ) {
      if ( mem.memberType != MemberInfo::Namespace ) {
#ifdef PHYSICAL_IMPORT
        TypePointer b = mem.build();
        if ( b && !( b->parent()->masterProxy().data() == this ) ) {
          b = b ->clone(); //expensive, cache is not shared
          b->setParent( this );

          mem.setBuilt( b );
        }
#else
        if( mem.memberType == MemberInfo::NestedType )
          chooseSpecialization( mem );
        TypePointer b = mem.build();
        if( b && b->parent() && b->parent()->masterProxy().data() == this )
          b->setParent( this );
#endif
        return mem;
      } else {
        TypePointer b = mem.build();

        if ( b )
          m_aliasImports.insert( Import( IncludeFiles(), b->desc(), TypePointer() ) );
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
  mem.name = subName.join( "::" );
  mem.memberType = MemberInfo::NotFound;
  QStringList sc = scope();
  sc += subName;
  mem.type = sc.join( "::" );
  mem.memberType = MemberInfo::Namespace;
  mem.setBuildInfo( new NamespaceBuildInfo( sc, imports ) );
  return mem;
}

///This must be optimized
void SimpleTypeNamespace::addAliasMap( const TypeDesc& name, const TypeDesc& alias, const IncludeFiles& files, bool recurse, bool symmetric, const TypePointer& perspective ) {
  Debug db;
  if ( !db ) {
    kdDebug( 9007 ) << str() << " addAliasMap: cannot add alias \"" << name.fullNameChain() << "\" -> \"" << alias.fullNameChain() << "\", recursion too deep" << endl;
    return ;
  }
  if ( name.next() ) kdDebug( 9007 ) << "addAliasMap warning: type-alias-name has order higher than one: " << name.fullNameChain() << ", only " << name.name() << " will be used" <<  endl;
  if ( name == alias )
    return ;

  if ( symmetric )
    addAliasMap( alias, name, files, recurse, false );

  invalidateSecondaryCache();
  invalidatePrimaryCache( true ); //Only not-found items are cleared updated here for performance-reasons(found items will stay cached)

  AliasMap::iterator it = m_aliases.find( name.name() );
  if ( it == m_aliases.end() )
    it = m_aliases.insert( name.name(), ImportList() );

  Import a( files, alias, perspective );
  std::pair< ImportList::const_iterator, ImportList::const_iterator > rng = ( *it ).equal_range( a );
  while ( rng.first != rng.second ) {
    if ( rng.first->files == files )
      return ; //The same alias, with the same files, has already been added.
    ++rng.first;
  }

  ( *it ).insert( a );
  ifVerbose( dbg() << "\"" << str() << "\": adding namespace-alias \"" << name.name() << ( !symmetric ? "\" -> \"" : "\" = \"" ) << alias.name() << "\" files:\n[ " << files.print().c_str() << "]\n" << endl );
  ifVerbose( if ( alias.resolved() ) dbg() << "Resolved type of the imported namespace: " << typeid( *alias.resolved() ).name() );

  if ( name.name().isEmpty() ) {
    addImport( alias, files, perspective );
  }
}

std::set<size_t> SimpleTypeNamespace::updateAliases( const IncludeFiles& files/*, bool isRecursion */) {
  std::set<size_t> possibleSlaves;
  if ( m_activeSlaves.empty() || !safetyCounter.ok() ) return possibleSlaves;
//   if( !isRecursion ) {
//     ///Test the cache
//     SlavesCache::const_iterator it = m_slavesCache.find( files );
//     if( it != m_slavesCache.end() && it->second.first == m_slavesCache.size() ) return; ///The cache already contains a valid entry, and the work is done
//   }

  m_activeSlaveGroups.findGroups( files, possibleSlaves );
  if( possibleSlaves.empty() ) return possibleSlaves;

  std::list<size_t> disabled;
  for( std::set<size_t>::const_reverse_iterator it = possibleSlaves.rbegin(); it != possibleSlaves.rend(); ++it ) {
    //Disable all slaves with higher ids
    SlaveMap::iterator current = m_activeSlaves.find( *it );
    if( current == m_activeSlaves.end() ) {
      kdDebug( 9007 ) << "ERROR" << endl;
    }

    SlaveDesc& d( current->second );

    if ( !d.first.first.resolved() ) {
      for( SlaveMap::const_iterator itr = current; itr != m_activeSlaves.end(); ++it ) {
        if( m_activeSlaveGroups.isDisabled( itr->first ) ) break; //stop searching when hitting the first disabled one(assuming that all behind are disabled too)
        disabled.push_back( itr->first );
        m_activeSlaveGroups.disableSet( itr->first );
      }

      TypeDesc descS = d.first.first;
      TypePointer p = d.second; //perspective

      HashedStringSet importIncludeFiles = d.first.second;

      if ( !p ) p = this;

      TypeDesc desc = p->locateDecType( descS, SimpleTypeImpl::Normal, 0, SimpleTypeImpl::MemberInfo::Namespace );
      if ( !desc.resolved() ) {
        ///If the namespace could not be found, help out by including the include-files of the current search
        descS.setIncludeFiles( descS.includeFiles() + files );
        desc = p->locateDecType( descS, SimpleTypeImpl::Normal, 0, SimpleTypeImpl::MemberInfo::Namespace );
      }
      if ( desc.resolved() ) {
        ///If exactly the same namespace was already imported use the earlier imported instance, so they can share a single cache
        ///@todo make more efficient.
        for ( SlaveMap::const_iterator it = m_activeSlaves.begin(); it != m_activeSlaves.end(); ++it ) {
          if (( *it ).second.first.first.resolved() && ( *it ).second.first.first.resolved()->scope() == desc.resolved()->scope() && typeid( *( *it ).second.first.first.resolved().data() ) == typeid( desc.resolved().data() ) ) {
            desc.setResolved(( *it ).second.first.first.resolved() );
            break;
          }
        }
#ifdef PHYSICALLY_IMPORT_NAMESPACES
        if ( desc.resolved()->masterProxy().data() != this ) {
          desc.setResolved( desc.resolved()->clone() ); //expensive, cache is not shared
          desc.resolved()->setMasterProxy( this ); //Possible solution: don't use this, simply set the parents of all found members correctly
        }
#endif
        d.first.first = desc;
      }
    }
  }

  for( std::list<size_t>::const_iterator it = disabled.begin(); it != disabled.end(); ++it ) {
    m_activeSlaveGroups.enableSet( *it );
  }

  return possibleSlaves;
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

void SimpleTypeNamespace::invalidatePrimaryCache( bool onlyNegative ) {
  //m_slavesCache.clear();
  SimpleTypeImpl::invalidatePrimaryCache( onlyNegative );
}

void SimpleTypeNamespace::addImport( const TypeDesc& import, const IncludeFiles& files, TypePointer perspective ) {
  //ifVerbose( dbg() << "
  if ( !perspective ) perspective = this;
  invalidateCache();
  TypeDesc d = import;
  if ( d.resolved() ) {
    #ifdef PHYSICALLY_IMPORT_NAMESPACES

    if( d.resolved()->masterProxy().data() != this ) {
      d.setResolved( d.resolved()->clone() ); //Expensive because of lost caching, think about how necessary this is
      d.resolved()->setMasterProxy( this );
    }
    #endif
  }

  m_activeSlaves[ ++m_currentSlaveId ] =  std::make_pair( std::make_pair( d, files ) , perspective );
  m_activeSlaveGroups.addSet( m_currentSlaveId, files );

  if( d.resolved() ) ///Must be called after the above, because it may insert new slaves, and the order in m_activeSlaves MUST be preserved
    d.resolved()->addAliasesTo( this );
}

bool SimpleTypeNamespace::hasNode() const {
  return true;
}

SimpleTypeNamespace::SlaveList SimpleTypeNamespace::getSlaves( const IncludeFiles& files ) {
  /*  ///Test the cache
  SlavesCache::const_iterator it = m_slavesCache.find( files );
  if( it != m_slavesCache.end() && it->second.first == m_activeSlaves.size() ) return it->second.second; ///The cache already contains a valid entry, and the work is done*/

  std::set<size_t> allSlaves = updateAliases( files );
  SlaveList ret;
#ifdef IMPORT_DEBUG
  for ( SlaveList::const_iterator it = m_activeSlaves.begin(); it != m_activeSlaves.end(); ++it ) {
#ifdef IMPORT_DEBUG
    ifVerbose( dbg() << "\"" << str() << "\": Checking whether \"" << (*it).second.first.first.fullNameChain() << "\" should be imported, current include-files: " << files.print().c_str() << "\nNeeded include-files: " << (*it).second.first.second.print().c_str() << "\n"; )
#endif
    if ( !(( *it ).second.first.second <= files ) ) {
#ifdef IMPORT_DEBUG
      ifVerbose( dbg() << "not imported." );
#endif
      continue;
    }
#ifdef IMPORT_DEBUG
    ifVerbose( dbg() << "imported." << endl );
#endif
    ret.push_back( *it.second );
  }
#else
  ifVerbose( dbg() << str() << " getSlaves() called for \n[ " << files.print().c_str() << endl );
  
  for( std::set<size_t>::const_iterator it = allSlaves.begin(); it != allSlaves.end(); ++it ) {
    SlaveMap::const_iterator itr = m_activeSlaves.find( *it );
    if( itr != m_activeSlaves.end() ) {
      ifVerbose( dbg() << str() << "getSlaves() returning " << (*itr).second.first.first.fullNameChain() << endl );
      ret.push_back( (*itr).second );
    } else {
      kdDebug( 9007 ) << "ERROR in getSlaves()";
    }
  }
#endif
  /*if( it == m_slavesCache.end() || it->second.first < m_activeSlaves.size()
    ) {
    m_slavesCache.insert( std::make_pair( files, std::make_pair( m_activeSlaves.size(), ret ) ) );
  }*/
  return ret;
}

//SimpleTypeNamespace::NamespaceBuildInfo implementation

TypePointer SimpleTypeNamespace::NamespaceBuildInfo::build() {
  if ( m_built )
    return m_built;
  m_built = new SimpleTypeCachedNamespace( m_fakeScope );
  for ( ImportList::iterator it = m_imports.begin(); it != m_imports.end(); ++it )  {
    TypeDesc i = ( *it ).import;
    if ( i.resolved() ) {
      //            i.setResolved( i.resolved()->clone() );
    }

    (( SimpleTypeCachedNamespace* ) m_built.data() ) ->addAliasMap( TypeDesc(), i, ( *it ).files, true, false, ( *it ).perspective );
  }
  return m_built;
}

// kate: indent-mode csands; tab-width 4;
