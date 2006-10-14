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

SimpleTypeNamespace::SimpleTypeNamespace( QStringList fakeScope, QStringList realScope ) : SimpleTypeImpl( fakeScope ) {
  if ( realScope.isEmpty() ) {
    ifVerbose( dbg() << "\"" << str() << "\": created namespace-proxy" << endl );
    addScope( fakeScope );
  } else if ( realScope.front() == "*" ) {
    ifVerbose( dbg() << "\"" << str() << "\": created empty namespace-proxy" << endl );
  } else {
    ifVerbose( dbg() << "\"" << str() << "\": created namespace-proxy with real scope \"" << realScope.join( "::" ) << "\"" << endl );
    addScope( realScope );
  }
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
    updateAliases();
    QValueList<TypePointer> ret;

  ///@todo filter by include-files
  for ( SlaveList::iterator it = m_activeSlaves.begin(); it != m_activeSlaves.end(); ++it ) {
    ret += ( *it ).first->getMemberClasses( name );
  }

  return ret;
}

SimpleTypeImpl::MemberInfo SimpleTypeNamespace::findMember( TypeDesc name, MemberInfo::MemberType type, std::set
      <SimpleTypeNamespace*>& ignore ) {
          updateAliases();
          MemberInfo mem;
  mem.name = "";
  mem.memberType = MemberInfo::NotFound;
  if ( ignore.find( this ) != ignore.end() )
    return mem;
  ignore.insert( this );

  for ( SlaveList::iterator it = m_activeSlaves.begin(); it != m_activeSlaves.end(); ++it ) {
    if( !( name.includeFiles() <= (*it).second ) ) continue; //filter the slave by the include-files
    ifVerbose( dbg() << "\"" << str() << "\": redirecting search for \"" << name.name() << "\" to \"" << ( *it ).first ->fullType() << "\"" << endl );
    mem = ( *it ).first ->findMember( name , type );
    if ( mem ) {
      if ( mem.memberType != MemberInfo::Namespace ) {
        if ( mem.type->resolved() && !( mem.type->resolved() ->parent().get().data() == this ) ) {
          mem.type->setResolved( mem.type->resolved() ->clone() );
          mem.type->resolved() ->setParent( this );
        }
        return mem;
      } else {
        ///make sure that the namespace is built as an instnce of SimpleTypeNamespace
        AliasList allAliases;

        if ( m_aliases.contains( name.name() ) )
          allAliases = m_aliases[ name.name() ];

        return setupMemberInfo( name, mem.type->fullNameList(), allAliases );
      }
    }
  }

  AliasMap::iterator itt = m_aliases.find( name.name() );

  if ( itt != m_aliases.end() && !( *itt ).empty() ) {
    ifVerbose( dbg() << "\"" << str() << "\": namespace-sub-aliases \"" << name.name() << "\" -> \"" <<  /**itt*/"..." << "\" requested, locating targets" << endl );

    AliasList targets;
    for ( AliasList::iterator it = ( *itt ).begin(); it != ( *itt ).end(); ++it ) {
      QStringList l = locateNamespaceScope( ( *it ).alias ); ///@todo think about when this should be done
      if ( !l.isEmpty() ) {
        Alias a = *it;
        a.alias = l.join( "::" );
        targets.insert( a );
      }
    }

    if ( !targets.empty() ) {
      mem = setupMemberInfo( name, "*", targets );
      ifVerbose( dbg() << "\"" << str() << "\": namespace-sub-alias \"" << name.name() << "\" -> \"" <<  /*targets*/"..." << "\" <- successfully located" << endl );

    } else {
      ifVerbose( dbg() << "\"" << str() << "\": namespace-sub-aliases \"" << name.name() << "\" -> \"" <<  /**itt*/"..." << "\" no target could be located" << endl );
    }
  }

  return mem;
}

SimpleTypeImpl::MemberInfo SimpleTypeNamespace::setupMemberInfo( TypeDesc& subName, QStringList tscope, AliasList imports ) {
  MemberInfo mem;
  mem.name = subName.name();
  mem.memberType = MemberInfo::NotFound;
  QStringList sc = scope();
  if ( subName.hasTemplateParams() ) {
    ifVerbose( dbg() << "\"" << str() << "\": found namespace for " << subName.fullName() << " but the type has template-parameters!" << endl );
    return mem;
  }
  sc << subName.name();
  mem.type = sc.join( "::" );
  mem.memberType = MemberInfo::Namespace;
  mem.setBuildInfo( new NamespaceBuildInfo( sc, tscope, imports ) );
  return mem;
}


QStringList SimpleTypeNamespace::locateNamespaceScope( QString alias ) {
  TypePointer t = locateNamespace( alias );
  if ( !t )
    return QStringList();
  return t->scope();
}

TypePointer SimpleTypeNamespace::locateNamespace( QString alias ) {
  ifVerbose( dbg() << "\"" << str() << "\": locating namespace \"" << alias << "\"" << endl );
    updateAliases();
    
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
}

///This must be optimized
void SimpleTypeNamespace::recurseAliasMap() {
    updateAliases();
    
  bool changed = true;
  SafetyCounter s( 1000 );
  AliasMap::iterator import = m_aliases.find( "" );
  /*for ( AliasMap::iterator it = m_aliases.begin(); it != m_aliases.end(); ++it ) {
    qHeapSort( *it );
  }*/

  while ( changed && s ) {
    ///this has bad runtime-performance for high counts, but it's simple and we shouldn't have that many maps :-)
    changed = false;
    for ( AliasMap::iterator it = m_aliases.begin(); it != m_aliases.end(); ++it ) {
      AliasList& l = *it;
      for ( AliasList::iterator strIt = l.begin(); strIt != l.end(); ++strIt ) {
        AliasMap::iterator fit = m_aliases.find( ( *strIt ).alias );

        for ( AliasList::iterator oit = ( *fit ).begin(); oit != ( *fit ).end(); ++oit ) {
          if ( l.find( ( *oit ) ) != l.end() && it.key() != ( *oit ).alias ) {
            addAliasMap( it.key(), ( *oit ).alias, ( *oit ).files + ( *strIt ).files, true );
            changed = true;
          }
          if ( changed )
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
}

void SimpleTypeNamespace::addAliasMap( QString name, QString alias, const IncludeFiles& files, bool recurse, bool symmetric ) {
  Debug dbg;
  if ( !dbg ) {
    kdDebug( 9007 ) << str() << " addAliasMap: cannot add alias \"" << name << "\" -> \"" << alias << "\", recursion too deep" << endl;
    return ;
  }
  invalidateSecondaryCache();
  if ( name == alias )
    return ;

  if ( symmetric )
    addAliasMap( alias, name, files, recurse, false );

  AliasMap::iterator it = m_aliases.find( name );
  if ( it == m_aliases.end() )
    it = m_aliases.insert( name, AliasList() );

  Alias a( files, alias );
  std::pair< AliasList::const_iterator, AliasList::const_iterator > rng = ( *it ).equal_range( a );
  while ( rng.first != rng.second ) {
    if ( rng.first->files == files )
      return ; //The same alias, with the same files, has already been added.
    ++rng.first;
  }

  ( *it ).insert( a );
  ifVerbose( dbg() << "\"" << str() << "\": adding namespace-alias \"" << name << ( !symmetric ? "\" -> \"" : "\" = \"" ) << alias << "\"" << endl );

  if ( name.isEmpty() ) {
    TypePointer t = locateNamespace( alias );
    if ( t )
      addScope( t->scope(), files );
  }

  if ( recurse )
    recurseAliasMap();
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

void SimpleTypeNamespace::addScope( const QStringList& scope, const IncludeFiles& files ) {
  invalidateCache();
  SimpleType ct = SimpleType( scope, Catalog );
  SimpleType cm = SimpleType( scope, CodeModel );
  ct->setMasterProxy( this );
  cm->setMasterProxy( this );
  m_activeSlaves << SlavePair( cm, files );
  m_activeSlaves << SlavePair( ct, files );
  m_waitingAliases << cm;
  m_waitingAliases << ct;
}

bool SimpleTypeNamespace::hasNode() const {
    const_cast<SimpleTypeNamespace*>(this)->updateAliases();
  for ( SlaveList::const_iterator it = m_activeSlaves.begin(); it != m_activeSlaves.end(); ++it )
    if ( ( *it ).first ->hasNode() )
      return true;

  return false;
};

SimpleTypeNamespace::SlaveList SimpleTypeNamespace::getSlaves() {
  updateAliases();
  return m_activeSlaves;
}

void SimpleTypeNamespace::updateAliases() {
  while ( !m_waitingAliases.empty() ) {
    SimpleType t = m_waitingAliases.front();
    m_waitingAliases.pop_front();
    t->addAliasesTo( this );
  }
}

//SimpleTypeNamespace::NamespaceBuildInfo implementation

TypePointer SimpleTypeNamespace::NamespaceBuildInfo::build() {
  if ( m_built )
    return m_built;
  m_built = new SimpleTypeCachedNamespace( m_fakeScope, m_realScope );
  for ( AliasList::iterator it = m_imports.begin(); it != m_imports.end(); ++it )
    ( ( SimpleTypeCachedNamespace* ) m_built.data() ) ->addAliasMap( "", ( *it ).alias, ( *it ).files );
  return m_built;
}

// kate: indent-mode csands; tab-width 4;
