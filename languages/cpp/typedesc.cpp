/***************************************************************************
                         cppcodecompletion.cpp  -  description
                            -------------------
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

#include "typedesc.h"
#include "stringhelpers.h"
#include "simpletype.h"

#include "driver.h"
#include "lexer.h"
#include "parser.h"

#include <kglobal.h>

const char* TypeDesc::functionMark = "[function] ";

using namespace StringHelpers;

LocateResult::LocateResult()  : m_desc( 0 ), m_resolutionCount(0), m_flags( NoFlag), m_trace( 0 ), m_locateDepth( 0 ) {
}

LocateResult::LocateResult( const TypeDesc& desc ) : m_desc( new TypeDescShared( desc ) ), m_resolutionCount(0), m_flags( NoFlag), m_trace( 0 ), m_locateDepth( 0 ) {
}

LocateResult::LocateResult( const TypeDescPointer& desc ) : m_desc( desc ), m_resolutionCount(0), m_flags( NoFlag), m_trace( 0 ), m_locateDepth( 0 ) {
}

LocateResult::LocateResult( TypeDescShared* desc ) : m_desc( desc ), m_resolutionCount(0), m_flags( NoFlag), m_trace( 0 ), m_locateDepth( 0 ) {
}

LocateResult::LocateResult( const LocateResult& rhs ) : m_desc( rhs.m_desc ), m_resolutionCount( rhs.m_resolutionCount ), m_flags( rhs.m_flags ), m_trace( 0 ), m_locateDepth( rhs.m_locateDepth ) {
	if( rhs.m_trace ) m_trace = new TypeTrace( *rhs.m_trace );
}


LocateResult::~LocateResult() {
	if( m_trace ) delete m_trace;
}

LocateResult& LocateResult::operator = ( const LocateResult& rhs ) {
	if( &rhs == this ) return *this;
	m_desc = rhs.m_desc;
	if( m_trace ) delete m_trace;
	if( !rhs.m_trace ) {
		m_trace = 0;
	} else {
		m_trace = new TypeTrace( *rhs.m_trace );
	}
	return  *this;
}


LocateResult::operator const TypeDesc&() const {
	if( !m_desc ) const_cast<TypeDescPointer&>(m_desc) = new TypeDescShared();
	return *m_desc;
}

LocateResult::operator TypeDesc&() {
	if( !m_desc ) m_desc = new TypeDescShared();
	return *m_desc;
}

TypeDesc& LocateResult::desc() {
	if( !m_desc ) m_desc = new TypeDescShared();
	return *m_desc;
}

const TypeDesc* LocateResult::operator ->() const {
	if( !m_desc ) const_cast<TypeDescPointer&>(m_desc) = new TypeDescShared();
	return m_desc;
}

TypeDesc* LocateResult::operator ->() {
	if( !m_desc ) m_desc = new TypeDescShared();
	return m_desc;
}

LocateResult::operator bool() const {
	return m_desc && *m_desc;
}


LocateResult::operator TypeDescPointer() {
	if( !m_desc ) m_desc = new TypeDescShared();
	return m_desc;
}


void LocateResult::addResolutionFlag( ResolutionFlags flag ) {
	m_flags = addFlag(flag, m_flags);
}

bool LocateResult::hasResolutionFlag( ResolutionFlags flag ) const {
	return (bool) ( m_flags & flag );
}

TypeTrace* LocateResult::trace() {
	if(!m_trace ) m_trace = new TypeTrace();
	return m_trace;
}

TypeDesc::TypeDesc()  {
}

TypeDesc::TypeDesc( const QString& name )  {
 init( name );
}

TypeDesc::TypeDesc( const TypeDesc& rhs )  {
 *this = rhs;
}

bool TypeDesc::isValidType() const {
	if( !m_data ) return false;
 if( m_data->m_cleanName.find("->") != -1 || m_data->m_cleanName.contains('.') || m_data->m_cleanName.contains(' ') || m_data->m_cleanName.isEmpty() ) return false;
 
 for( TemplateParams::const_iterator it = m_data->m_templateParams.begin(); it != m_data->m_templateParams.end(); ++it ) {
  if( !(*it)->isValidType() ) return false;
 }
 
 if( m_data->m_nextType ) if( !m_data->m_nextType->isValidType() ) return false;
 return true;
}

TypeDesc& TypeDesc::operator = ( const TypeDesc& rhs ) {
	m_data = rhs.m_data;
 return *this;
}

int TypeDesc::depth() const {
	if( !m_data ) return 0;
 int ret = 1;
 for( TemplateParams::const_iterator it = m_data->m_templateParams.begin(); it != m_data->m_templateParams.end(); ++it ) {
  ret = kMax( (*it)->depth() + 1, ret );
 }
 
 if( m_data->m_nextType ){
  ret = kMax( m_data->m_nextType->depth(), ret );
 }
 
 return ret;
}

int TypeDesc::length() const {
	if( !m_data ) return 0;
 if( !m_data->m_nextType && m_data->m_cleanName.isEmpty() ) return 0;
return m_data->m_nextType ? 1 + m_data->m_nextType->length() : 1;
}

 ///Something is wrong with this function.. so i use the string-comparison
int TypeDesc::compare ( const TypeDesc& rhs ) const {
	if( m_data == rhs.m_data ) return 0;
	if( !m_data ) return -1;
	if( !rhs.m_data ) return 1;
	
 if( m_data->m_functionDepth != rhs.m_data->m_functionDepth ) {
  if( m_data->m_functionDepth < rhs.m_data->m_functionDepth )
   return -1;
  else
   return 1;
 }
 
 if( m_data->m_pointerDepth != rhs.m_data->m_pointerDepth ) {
  if( m_data->m_pointerDepth < rhs.m_data->m_pointerDepth )
   return -1;
  else
   return 1;
 }
 
 if( m_data->m_cleanName != rhs.m_data->m_cleanName ) {
  if( m_data->m_cleanName < rhs.m_data->m_cleanName )
   return -1;
  else
   return 1;
 }
 if( m_data->m_templateParams.size() != rhs.m_data->m_templateParams.size() ) {
  if( m_data->m_templateParams.size() < rhs.m_data->m_templateParams.size() )
   return -1;
  else
   return 1;
 }
 
 TemplateParams::const_iterator it2 = rhs.m_data->m_templateParams.begin();
 for( TemplateParams::const_iterator it = m_data->m_templateParams.begin(); it != m_data->m_templateParams.end() && it2 != rhs.m_data->m_templateParams.end(); ) {
  if( int cmp =  (*it)->compare(**it2) != 0 ) {
   return cmp;
  }
  ++it2; ++it;
 }
 
 if( !((bool)m_data->m_nextType) != ((bool)rhs.m_data->m_nextType) ) {
  if( m_data->m_nextType )
   return 1;
  else
   return -1;
 }
 
 if( m_data->m_nextType && rhs.m_data->m_nextType ) {
  if( int cmp = m_data->m_nextType->compare( *rhs.m_data->m_nextType ) != 0 )
   return cmp;
 }
 
 return 0;
}

QString TypeDesc::nameWithParams() const {
	if( !m_data ) return "";

 QString ret = m_data->m_cleanName;
 if( !m_data->m_templateParams.isEmpty() ) {
  ret += "<";
  for( TemplateParams::const_iterator it = m_data->m_templateParams.begin(); it != m_data->m_templateParams.end(); ++it ) {
   ret += (*it)->fullNameChain();
   ret += ", ";
  }
  ret.truncate( ret.length() - 2 );
  ret += ">";
 }
 return ret;
}

QString TypeDesc::fullName( ) const {
	if( !m_data ) return "";

 QString ret = nameWithParams();
 for( int a=0; a < m_data->m_functionDepth; ++a) ret = QString( functionMark ) + ret;
 for( int a=0; a < m_data->m_pointerDepth; ++a) ret += "*";
 return m_data->m_dec.apply( ret );
}

QString TypeDesc::fullNameChain( ) const {
	if( !m_data ) return "";
 QString ret = fullName();
 if( m_data->m_nextType ) {
  ret += "::" + m_data->m_nextType->fullNameChain();
 }
 return m_data->m_dec.apply( ret );
}

QString TypeDesc::fullTypeStructure() const {
	if( !m_data ) return "";
	
	QString ret = m_data->m_cleanName;
	if( !m_data->m_templateParams.isEmpty() ) {
		ret += "<";
		for( TemplateParams::const_iterator it = m_data->m_templateParams.begin(); it != m_data->m_templateParams.end(); ++it ) {
			ret += (*it)->fullTypeStructure();
			ret += ", ";
		}
		ret.truncate( ret.length() - 2 );
		ret += ">";
	}
	return ret;
}


QStringList TypeDesc::fullNameList( ) const {
 if( !m_data ) return "";
 QStringList ret;
 ret << fullName();
 if( m_data->m_nextType ) {
  ret += m_data->m_nextType->fullNameList();
 }
 return ret;
};


 /// The template-params may be changed in-place
 /// this list is local, but the params pointed by them not
TypeDesc::TemplateParams& TypeDesc::templateParams() {
	makeDataPrivate();
 return m_data->m_templateParams;
}

const TypeDesc::TemplateParams& TypeDesc::templateParams() const {
	const_cast<TypeDesc*>( this ) ->maybeInit();
 return m_data->m_templateParams;
}

TypeDescPointer TypeDesc::next() {
	if( !m_data ) return 0;
 return m_data->m_nextType;
}

bool TypeDesc::hasTemplateParams() const {
	if( !m_data ) return false;
 return !m_data->m_templateParams.isEmpty();
}

void TypeDesc::setNext( TypeDescPointer type ) {
	makeDataPrivate();
 m_data->m_nextType = type;
}

void TypeDesc::append( TypeDescPointer type ) {
 if( type ) {
	 makeDataPrivate();
  if( m_data->m_nextType )
   m_data->m_nextType->append( type );
  else
   m_data->m_nextType = type;
 }
}

TypePointer TypeDesc::resolved() const {
	if( !m_data ) return 0;
 return m_data->m_resolved;
}

void TypeDesc::setResolved( TypePointer resolved ) {
	makeDataPrivate();
 m_data->m_resolved = resolved;
}

void TypeDesc::resetResolved() {
	if( !m_data ) return;
	makeDataPrivate();
 m_data->m_resolved = 0;
 if( m_data->m_nextType ) m_data->m_nextType->resetResolved();
}

 ///Resets the resolved-pointers of this type, and all template-types
void TypeDesc::resetResolvedComplete() {
	if( !m_data ) return;
	makeDataPrivate();
 resetResolved();
 for( TemplateParams::iterator it = m_data->m_templateParams.begin(); it != m_data->m_templateParams.end(); ++it )
  (*it)->resetResolvedComplete();
}

 ///these might be changed in future to an own data-member
void TypeDesc::increaseFunctionDepth() {
	makeDataPrivate();
 m_data->m_functionDepth++;
}

void TypeDesc::decreaseFunctionDepth() {
	makeDataPrivate();
 if( m_data->m_functionDepth > 0 ) m_data->m_functionDepth--;
}

int TypeDesc::functionDepth() const {
	if( !m_data ) return 0;
 return m_data->m_functionDepth;
}

void TypeDesc::takeInstanceInfo( const TypeDesc& rhs ) {
	makeDataPrivate();
 m_data->m_pointerDepth += rhs.m_data->m_pointerDepth;
 m_data->m_dec += rhs.m_data->m_dec;
}

void TypeDesc::clearInstanceInfo() {
	if( !m_data ) return;
	makeDataPrivate();
 m_data->m_pointerDepth = 0;
 m_data->m_dec = "";
}

void TypeDesc::takeTemplateParams( const QString& string ) {
	makeDataPrivate();
 m_data->m_templateParams.clear();
 for( ParamIterator it( "<>", string ); it; ++it )
  m_data->m_templateParams.append( new TypeDescShared( *it ));
}

void TypeDesc::makeDataPrivate() {
	if( !m_data ) { maybeInit(); return; }
	if( m_data.count() > 1 ) {
		m_data = new TypeDescData( *m_data );
	}
}

TypeDesc& TypeDesc::makePrivate() {
	makeDataPrivate();
 TemplateParams nList;
 for( TemplateParams::const_iterator it = m_data->m_templateParams.begin(); it != m_data->m_templateParams.end(); ++it ) {
  TypeDescPointer tp( new TypeDescShared( ) );
  *tp = **it;
  tp->makePrivate();
  nList.append( tp );
 }
 m_data->m_templateParams = nList;
 
 if( m_data->m_nextType ) {
  TypeDescPointer tmp = m_data->m_nextType;
  m_data->m_nextType = new TypeDescShared();
  *m_data->m_nextType = *tmp;
  m_data->m_nextType->makePrivate();
 }
 return *this;
}

void  TypeDesc::maybeInit() {
	if( m_data ) return;
	m_data = new TypeDescData();
	m_data->m_pointerDepth = 0;
	m_data->m_functionDepth = 0;
	m_data->m_nextType = 0;
	m_data->m_flags = Standard;
}

void TypeDesc::init( QString stri ) {
	m_data = 0;
	maybeInit();

 if ( stri.isEmpty() )
  return;
 
 m_data->m_dec = stri; ///Store the decoration
 
 QStringList ls = splitType( stri );
 QString str = ls.front();
 
  ///Extract multiple types that may be written as a scope and put them to the next-types-list
 if( !ls.isEmpty() ) {
  ls.pop_front();
  if( !ls.isEmpty() ) {
   m_data->m_nextType = TypeDescPointer( new TypeDescShared( ls.join("::") ) );
  }
 }
 
 if( str.startsWith( "typename " ) ) {
  str = str.right( str.length() - strlen( "typename " ) );
 }
 while( str.startsWith( QString( functionMark ) ) ) {
  m_data->m_functionDepth++;
  str = str.mid( strlen( functionMark ) ).stripWhiteSpace();
 }
 
 
 Driver d;
 Lexer lex( &d );
 lex.setSource( str );
 Parser parser( &d, &lex );
 
 TypeSpecifierAST::Node typeSpec;
 if ( parser.parseTypeSpecifier( typeSpec ) )
 {
  NameAST * name = typeSpec->name();
  
  QPtrList<ClassOrNamespaceNameAST> l = name->classOrNamespaceNameList();
  QPtrListIterator<ClassOrNamespaceNameAST> it( l );
  
  QString type;
  while ( it.current() )
  {
   if ( it.current() ->name() )
   {
    type += it.current() ->name() ->text() + "::";
   }
   ++it;
  }
  
  if ( name->unqualifiedName() && name->unqualifiedName() ->name() )
  {
   type += name->unqualifiedName() ->name() ->text();
  }
  
  m_data->m_cleanName = type.stripWhiteSpace();
  takeTemplateParams( str );
  m_data->m_pointerDepth = countExtract( '*', str );
 }
 
} 


TypeDesc operator + ( const TypeDesc& lhs, const TypeDesc& rhs ) {
  TypeDesc ret = lhs;
  ret.makePrivate();
  ret.append( new TypeDescShared( rhs ) );
  return ret;
}


