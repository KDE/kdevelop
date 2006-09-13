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
    <QString> ignore;
  return findMember( name, type, ignore );
}

SimpleTypeImpl::MemberInfo SimpleTypeNamespace::findMember( TypeDesc name, MemberInfo::MemberType type, std::set <QString>& ignore ) {
	MemberInfo mem;
  mem.name = "";
  mem.memberType = MemberInfo::NotFound;
	QString nm = scope().join( "::");
	if ( ignore.find( nm ) != ignore.end() )
		return mem;
	
  for ( QValueList<SimpleType>::iterator it = m_activeSlaves.begin(); it != m_activeSlaves.end(); ++it ) {
    ifVerbose( dbg() << "\"" << str() << "\": redirecting search for \"" << name.name() << "\" to \"" << ( *it ) ->fullType() << "\"" << endl );
    mem = ( *it ) ->findMember( name , type );
    if ( mem ) {
      if ( mem.memberType != MemberInfo::Namespace ) {
        if ( mem.type.resolved() ) {
          mem.type.setResolved( mem.type.resolved() ->clone() );
          mem.type.resolved() ->setParent( this );
        }
        return mem;
      } else {
        ///verify that the namespace is built as this class..
        QValueList<QStringList> allAliases;

        if ( m_aliases.contains( name.name() ) ) {
          QStringList & aliases = m_aliases[ name.name() ];
          for ( QStringList::iterator it = aliases.begin(); it != aliases.end(); ++it )
            allAliases << splitType( *it );
        }

        return setupMemberInfo( name, mem.type.fullNameList(), allAliases );
      }
    }
  }

  AliasMap::iterator itt = m_aliases.find( name.name() );

  if ( itt != m_aliases.end() && !( *itt ).isEmpty() ) {
    ifVerbose( dbg() << "\"" << str() << "\": namespace-sub-aliases \"" << name.name() << "\" -> \"" << *itt << "\" requested, locating targets" << endl );

    QValueList<QStringList> targets;
    for ( QStringList::iterator it = ( *itt ).begin(); it != ( *itt ).end(); ++it ) {
      QStringList l = locateNamespaceScope( *it );
      if ( !l.isEmpty() )
        targets << l;
    }

    if ( !targets.isEmpty() ) {
      QValueList<QStringList> targets2 = targets;
      targets2.pop_front();
      mem = setupMemberInfo( name, targets.front(), targets2 );
      ifVerbose( dbg() << "\"" << str() << "\": namespace-sub-alias \"" << name.name() << "\" -> \"" << targets2 << "\" <- successfully located" << endl );

    } else {
      ifVerbose( dbg() << "\"" << str() << "\": namespace-sub-aliases \"" << name.name() << "\" -> \"" << *itt << "\" no target could be located" << endl );
    }
  }

  return mem;
}

SimpleTypeImpl::MemberInfo SimpleTypeNamespace::setupMemberInfo( TypeDesc& subName, QStringList tscope, QValueList<QStringList> imports ) {
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
  SimpleTypeImpl::LocateResult res = locateDecType( alias, addFlag( ExcludeNestedTypes, ExcludeTemplates ), 0, MemberInfo::Namespace );
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

void SimpleTypeNamespace::recurseAliasMap() {
  bool changed = true;
  SafetyCounter s( 1000 );
  AliasMap::iterator import = m_aliases.find( "" );
  for ( AliasMap::iterator it = m_aliases.begin(); it != m_aliases.end(); ++it )
    ( *it ).sort();

  while ( changed && s ) {
    ///this has bad runtime-performance for high counts, but it's simple and we shouldn't have that many maps :-)
    changed = false;
    for ( AliasMap::iterator it = m_aliases.begin(); it != m_aliases.end(); ++it ) {
      QStringList& l = *it;
      for ( QStringList::iterator strIt = l.begin(); strIt != l.end(); ++strIt ) {
        AliasMap::iterator fit = m_aliases.find( *strIt );

        for ( QStringList::iterator oit = ( *fit ).begin(); oit != ( *fit ).end(); ++oit ) {
		  if ( !l.contains( *oit ) && it.key() != *oit ) {
            addAliasMap( it.key(), *oit , true );
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

void SimpleTypeNamespace::addAliasMap( QString name, QString alias , bool recurse, bool symmetric ) {
  invalidateSecondaryCache();if( name == alias ) return;
  ifVerbose( dbg() << "\"" << str() << "\": adding namespace-alias \"" << name << ( !symmetric ? "\" -> \"" : "\" = \"" ) << alias << "\"" << endl );
  TypePointer t = locateNamespace( alias );
  if ( !t )
    return ;
  /*
  if( symmetric ) {
  	TypePointer t2;
  	if( name.isEmpty() )
  		t2 = this;
  	else
  		t2 = locateNamespace( name );
  	if( t2 )
  		t->setMasterProxy( t2 );
  }*/ //
  if ( symmetric )
    addAliasMap( alias, name, recurse, false );

  AliasMap::iterator it = m_aliases.find( name );
  if ( it == m_aliases.end() )
    it = m_aliases.insert( name, QStringList() );

  *it << alias;

  if ( name.isEmpty() )
    addScope( t->scope() );

  if ( recurse )
    recurseAliasMap();
}

void SimpleTypeNamespace::addAliases( QString map ) {
  while ( !map.isEmpty() ) {
    int mid = map.find( "=" );
    int mid2 = map.find( "<<" );
    int found = mid;
    int len = 1;
    if ( mid2 != -1 && mid2 < found ) {
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

	  addAliasMap( map.left( found ).stripWhiteSpace(), map.mid( found + len, end - found - len ).stripWhiteSpace(), true, found == mid );
    map = map.mid( end + 1 );
  }
}

void SimpleTypeNamespace::addScope( const QStringList& scope ) {
  invalidateCache();
  SimpleType ct = SimpleType( scope, Catalog );
  SimpleType cm = SimpleType( scope, CodeModel );
  ct->setMasterProxy( this );
  cm->setMasterProxy( this );
  m_activeSlaves << cm;
  m_activeSlaves << ct;
}

bool SimpleTypeNamespace::hasNode() const {
  for ( QValueList<SimpleType>::const_iterator it = m_activeSlaves.begin(); it != m_activeSlaves.end(); ++it )
    if ( ( *it ) ->hasNode() )
      return true;

  return false;
};

//SimpleTypeNamespace::NamespaceBuildInfo implementation

TypePointer SimpleTypeNamespace::NamespaceBuildInfo::build() {
  SimpleTypeNamespace * ns = new SimpleTypeCachedNamespace( m_fakeScope, m_realScope );
  for ( QValueList<QStringList>::iterator it = m_imports.begin(); it != m_imports.end(); ++it )
    ns->addAliasMap( "", ( *it ).join( "::" ) );
  return ns;
}

// kate: indent-mode csands; tab-width 4;
