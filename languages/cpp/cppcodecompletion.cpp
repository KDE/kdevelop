/***************************************************************************
                         cppcodecompletion.cpp  -  description
                            -------------------
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
#include "cppcodecompletion.h"
#include "cppcodecompletionconfig.h"
#include "backgroundparser.h"
#include "ast.h"
#include "ast_utils.h"
#include "codeinformationrepository.h"
#include "parser.h"
#include "lexer.h"
#include "tree_parser.h"
#include "cpp_tags.h"
#include "cppsupport_utils.h"
#include "tag_creator.h"
#include <qpopupmenu.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kmessagebox.h>
#include <kparts/part.h>
#include <kstatusbar.h>
#include <ktexteditor/document.h>

#include <qdatastream.h>
#include <qfile.h>
#include <qmap.h>
#include <qregexp.h>
#include <qstatusbar.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qpair.h>
#include <qvaluestack.h>

#include <kdevpartcontroller.h>
#include <kdevmainwindow.h>
#include <kdevproject.h>
#include <kdevcoderepository.h>
#include <codemodel_utils.h>
#include <codemodel.h>
#include <codebrowserfrontend.h>


///This can be used to toggle the complete tracing of the resolution-functions, which costs a lot of performance
//#define VERBOSE
//#define DISABLETOOLTIPS

/**
-- TODO: The parser and code-models currently do not correctly collect all the data necessary to handle namespace-imports etc. precisely
-- TODO: Does not yet use most of the code-completion-options. Maybe some should be removed, and new ones added.
-- TODO: Does not yet correctly search for overloaded functions and select the right one
-- TODO: The documentation shown in the calltips looks very bad, a better solution must be found(maybe an additional tooltip)
-- TODO: Make everything optional
*/

const QString hardCodedAliases = "std=_GLIBCXX_STD;";

typedef CppCodeCompletion::EvaluationResult EvaluationResult;

class CodeCompletionEntry : public KTextEditor::CompletionEntry
{
public:
	CodeCompletionEntry() : KTextEditor::CompletionEntry() {
	}
	CodeCompletionEntry( const CodeCompletionEntry& rhs ) : KTextEditor::CompletionEntry( rhs ) {
	}
	CodeCompletionEntry( const KTextEditor::CompletionEntry& rhs ) : KTextEditor::CompletionEntry( rhs ) {
	}
	
	bool operator < ( const CodeCompletionEntry& rhs ) {
		return userdata < rhs.userdata;
	}
	bool operator == ( const CodeCompletionEntry& rhs ) {
		return userdata == rhs.userdata;
	}
	bool operator > ( const CodeCompletionEntry& rhs ) {
		return userdata > rhs.userdata;
	}
	
	CodeCompletionEntry& operator = ( const KTextEditor::CompletionEntry& rhs ) {
		(*(KTextEditor::CompletionEntry*)this) = rhs;
		return *this;
	}
};

struct SafetyCounter {
	int safetyCounter;
	const int maxSafetyCounter;
	
	SafetyCounter( int max = 10000 ) : safetyCounter(0), maxSafetyCounter(max) {
	}
	
	void init() {
		safetyCounter = 0;
	}
	
	SafetyCounter& operator ++() {
		safetyCounter++;
		return *this;
	}
	
	operator bool() {
		safetyCounter++;
		bool ret = safetyCounter < maxSafetyCounter;
		if( !ret ) {
			kdDebug( 9007) << "WARNING: Safety-depth-counter reached count > " << maxSafetyCounter << ", operation stopped" << endl;
		}
		return ret;
	}
	
} safetyCounter;

namespace CompletionDebug {

template <class StreamType>
class KDDebugState {
private:
	StreamType m_stream;
	QStringList m_prefixStack;
	
public:
	typedef StreamType KStreamType;
	KDDebugState();
	
	KDDebugState( StreamType stream ) : m_stream( stream ) {
		
	}
	
	void push( const QString & txt ) {
		m_prefixStack.push_back( txt );
	}
	
	void pop() {
		m_prefixStack.pop_back();
	};
	
	StreamType& dbg() {
		for( QStringList::iterator it = m_prefixStack.begin(); it != m_prefixStack.end() ; ++it ) {
			m_stream << *it;
		}
		
		return m_stream;
	}
	
	int depth() {
		return m_prefixStack.size();
	}
	
} ;

template<>
KDDebugState<kdbgstream>::KDDebugState() : m_stream ( kdDebug( 9007 ) ) {
}

template<>
KDDebugState<kndbgstream>::KDDebugState() {
}

#ifdef VERBOSE
typedef KDDebugState<kdbgstream> DBGStreamType;
#else
typedef KDDebugState<kndbgstream> DBGStreamType;
#endif
DBGStreamType dbgState;

const int completionMaxDepth = 50;

///Class to help indent the debug-output correctly

class Debug {
private:
	DBGStreamType& m_state;
public:
	Debug( const QString& prefix = "#", DBGStreamType& st = dbgState ) : m_state( st ) {
		m_state.push( prefix );
	};
	~Debug() {
		m_state.pop();
	}
	
	DBGStreamType::KStreamType& dbg() {
		return m_state.dbg();
	}
	
	int depth() {
		return m_state.depth();
	}
	
	operator bool() {
		bool r = depth() < completionMaxDepth;
		
		if( !r ) {
			dbg() << "recursion is too deep";
		}
		
		return r;
	}
};


DBGStreamType::KStreamType& dbg() {
	return dbgState.dbg();
}

DBGStreamType::KStreamType& dbgMajor() {
	return dbgState.dbg();
}

}

using namespace CompletionDebug;

CppCodeCompletion* cppCompetionInstance = 0;


namespace StringHelpers {

void clearStr( QString& str, int start, int end ) {
for( int a = start; a < end; a++) str[a] = ' ';
}

///Fills all comments with whitespaces
QString clearComments( QString str ) {
	if( str.isEmpty() ) return "";
	
	SafetyCounter s( 1000 );
	int lastPos = 0;
	int pos;
	int len = str.length();
	while( (pos = str.find( "/*", lastPos )) != -1 ) {
		if( !s ) return str;
		int i = str.find( "*/", pos );
		if( i != -1 && i <= len - 2 ) {
			clearStr( str, pos, i+2 );
			lastPos = i+2;
			if( lastPos == len ) break;
		} else {
			break;
		}
	}
	
	lastPos = 0;
	while( (pos = str.find( "//", lastPos )) != -1 ) {
		if( !s ) return str;
		int i = str.find( "\n", pos );
		if( i != -1 && i <= len - 1 ) {
			clearStr( str, pos, i+1 );
			lastPos = i+1;
		} else {
			clearStr( str, pos, len );
			break;
		}
	}
	
	return str;
}

QString cutTemplateParams( QString str ) {
	int p;
	if( (p = str.find('<') ) != -1) {
		return str.left( p );
	}
	
	return str.stripWhiteSpace().replace('*',"");
}


QPair<QString, QString> splitTemplateParams( QString str ) {
	QPair<QString, QString> ret;
	int p;
	if( (p = str.find('<') ) != -1) {
		ret.first = str.left( p ).stripWhiteSpace();
		ret.second = str.mid( p ).stripWhiteSpace();
	} else {
		ret.first = str.stripWhiteSpace();
	}
	
	return ret;
}

inline bool parenFits( QChar c1, QChar c2 ) {
	if( c1 == '<' && c2 == '>' ) return true;
	else if( c1 == '(' && c2 == ')' ) return true;
	else if( c1 == '[' && c2 == ']' ) return true;
	else if( c1 == '{' && c2 == '}' ) return true;
	else
		return false;
}


inline bool isParen( QChar c1 ) {
	if( c1 == '<' || c1 == '>' ) return true;
	else if( c1 == '(' || c1 == ')' ) return true;
	else if( c1 == '[' || c1 == ']' ) return true;
	else if( c1 == '{' || c1 == '}' ) return true;
	else
		return false;
}

inline bool isTypeParen( QChar c1 ) {
	if( c1 == '<' || c1 == '>' ) return true;
	else
		return false;
}

inline bool isTypeOpenParen( QChar c1 ) {
	if( c1 == '<' ) return true;
	else
		return false;
}

inline bool isTypeCloseParen( QChar c1 ) {
	if( c1 == '>' ) return true;
	else
		return false;
}


inline bool isLeftParen( QChar c1 ) {
	if( c1 == '<' ) return true;
	else if( c1 == '(' ) return true;
	else if( c1 == '[' ) return true;
	else if( c1 == '{' ) return true;
	else
		return false;
}




/*only from left to right
searches a fitting closing sign ( a ')' for a '(', ']' for '['
ignores quoted text
comments are currently not allowed */
int findClose( const QString& str , int pos ) {
	int depth = 0;
	QValueList<QChar> st;
	QChar last = ' ';
	
	for( int a = pos; a < (int)str.length(); a++) {
		switch(str[a]) {
		case '<':
		case '(':
			case '[':
				case '{':
				st.push_front( str[a] );
			depth++;
			break;
		case '>':
			if( last == '-' ) break;
		case ')':
			case ']':
				case '}':
				if( !st.isEmpty() && parenFits(st.front(), str[a]) ) {
					depth--;
					st.pop_front();
				}
			break;
		case '"':
			last = str[a];
			a++;
			while( a < (int)str.length() && (str[a] != '"' || last == '\\')) {
				last = str[a];
				a++;
			}
			continue;
			break;
		}
		
		last = str[a];
		
		if( depth == 0 ) {
			return a;
		}
	}
	
	return -1;
}


QString tagType( const Tag& tag )
{
	if ( tag.hasAttribute( "t" ) )
	{
		QString type = tag.attribute( "t" ).toString();
		return type;
	}
	else if ( tag.kind() == Tag::Kind_Class || tag.kind() == Tag::Kind_Namespace )
	{
		QStringList l = tag.scope();
		l << tag.name();
		return l.join("::");
	}
	return QString();
}



int findCommaOrEnd( const QString& str , int pos, QChar validEnd = ' ' ) {
	
	for( int a = pos; a < (int)str.length(); a++) {
		switch(str[a]) {
		case '(':
			case '[':
				case '{':
				case '<':
				a = findClose( str, a );
			if( a == -1 ) return str.length();
			break;
		case ')':
			case ']':
				case '}':
				case '>':
				if( validEnd != ' ' && validEnd != str[a] )
					continue;
		case ',':
			return a;
		}
	}
	
	return str.length();
}


int countExtract( QChar c, const QString& str ) {
	int ret  = 0;
	for( int a = 0; a < (int)str.length(); a++) {
		if( str[a] == c ) ++ret;
		switch( str[a] ) {
		case '(':
			case '[':
				case '{':
				case '<':
				a = findCommaOrEnd( str, a );
		}
	}
	return ret;
}



QString templateParamFromString( int num, QString str ) {
	if( str.endsWith("::") ) str.truncate( str.length() - 2 );
	int begin = str.find('<');
	int end = str.findRev('>');
	
	if(begin == -1 || end == -1) return "";
	
	begin++;
	
	for(int a = 0; a < num; a++) {
		begin = findCommaOrEnd( str, begin );
		if( begin == (int)str.length() ) return "";
		begin++;
	}
	end = findCommaOrEnd( str, begin );
	
	if( end == (int)str.length() ) return "";
	
	return str.mid( begin, end - begin ).stripWhiteSpace();
}



QStringList splitType( QString str ) {
	QStringList ret;
	int currentStart = 0;
	bool was = false;
	for( int a = 0; a < (int)str.length(); ++a ) {
		if( isLeftParen( str[a] ) ) {
			a = findClose( str, a );
			if( a == -1 ) {
			dbg() << "misformatted type: " << str << endl;
				return ret;
			}
			was = false;
		} else {
			if( str[a] == ':' ) {
				if( was ) {
					if( currentStart < a - 1 ) 
						ret << str.mid( currentStart, (a - 1) - currentStart ).stripWhiteSpace();
					currentStart = a + 1;
				}
				was = true;
			} else {
				was = false;
			}
		}
	}
	if( currentStart < (int)str.length() ) 
		ret << str.mid( currentStart, str.length() - currentStart ).stripWhiteSpace();
	return ret;
}


class ParamIterator{
private:
	QString m_source;
	QString m_parens;
	int m_cur;
	int m_curEnd;
	
	int next() {
		return findCommaOrEnd( m_source, m_cur,  m_parens[1] );
	}
public:
ParamIterator( QString parens, QString source ) : m_source( source ), m_parens( parens ), m_cur( 0 ), m_curEnd ( 0 ) {
	int begin = m_source.find(m_parens[0]);
	int end = m_source.findRev(m_parens[1]);
	
	if(begin == -1 || end == -1 && end - begin > 1)
		m_cur = m_source.length();
	else {
		m_source = source.mid( begin+1, end - begin );
		m_curEnd = next();
	}
}
	
	ParamIterator& operator ++() {
		m_cur = m_curEnd + 1;
		if( m_cur < (int)m_source.length() ) {
			m_curEnd = next();
		}
		return *this;
	}
	
	QString operator *() {
		return m_source.mid( m_cur, m_curEnd - m_cur ).stripWhiteSpace();
	}
	
	operator bool() {
		return m_cur < (int)m_source.length();
	}
	
};


};

using namespace StringHelpers;

static inline int max( int a, int b ) {
return a > b ? a : b;
}

namespace BitHelpers {
template <class Type>
inline Type addFlag( Type loc, Type add ) {
	return (Type)( loc | add );
}

template <class Type>
inline Type remFlag( Type loc, Type rem ) {
	return (Type)( loc & (rem ^ 0xffffffff) );
}

template <class Type>
inline Type bitInvert( Type val ) {
	return (Type)(0xffffffff - (unsigned int)val);
}
}
using namespace BitHelpers;

class SimpleTypeImpl;
typedef KSharedPtr<SimpleTypeImpl> TypePointer;

class TypeDecoration {
	QString m_decoration_front, m_decoration_back;
	void init( QString str ) {
		str = str.stripWhiteSpace();
		m_decoration_front = "";
		m_decoration_back = "";
		
		if( str.startsWith( "const " ) ) m_decoration_front = "const ";
		if( str.endsWith( "&" ) ) m_decoration_back = "&";
	}
public:
	TypeDecoration( const QString& str = "" ) {
		init( str );
	}
	
	~TypeDecoration() {
	}
	
	TypeDecoration& operator = ( QString str ) {
		init( str );
		return *this;
	}
	
	QString apply( const QString& str ) const {
		QString ret = str;
		if( !ret.startsWith( m_decoration_front ) )
			ret = m_decoration_front + ret;
		
		if( !ret.endsWith( m_decoration_back ) )
			ret = ret + m_decoration_back;
		return ret;
	}
	
	void operator += ( const TypeDecoration& rhs ) {
		if( !m_decoration_front.contains( rhs.m_decoration_front) ) m_decoration_front += rhs.m_decoration_front;
		if( !m_decoration_back.contains( rhs.m_decoration_back) ) m_decoration_back += rhs.m_decoration_back;
	}
};


QString globalCurrentFile = "";

class TypeDesc;
class TypeDescShared;
typedef KSharedPtr<TypeDescShared> TypeDescPointer;

///TODO: Das KShared muss weg
class TypeDesc {
public:
	typedef QValueList<TypeDescPointer> TemplateParams;
	static const char* functionMark;
private:
	QString m_cleanName;
	int m_pointerDepth;
	int m_functionDepth;
	TemplateParams m_templateParams;
	TypeDescPointer m_nextType;
	TypePointer m_resolved;
	TypeDecoration m_dec;

	
	void init( QString stri );
public:
	void takeTemplateParams( const QString& string );
	
	TypeDesc( const QString& name = "" );
	
	TypeDesc( const TypeDesc& rhs );
	
	bool deeper( const TypeDesc& rhs ) const {
		return depth() > rhs.depth();
	}
	
	bool longer( const TypeDesc& rhs ) const {
		return length() > rhs.length();
	}
	
	TypeDesc& operator = ( const TypeDesc& rhs );
	
	TypeDesc& operator = ( const QString& rhs ) {
		init( rhs );
		return *this;
	}
	
	bool isValidType() const ;
	
	
	int depth() const;
	
	int length() const ;
	
	///Something is wrong with this function.. so i use the string-comparison
	int compare ( const TypeDesc& rhs ) const;
	
	bool operator < ( const TypeDesc& rhs ) const {
		return compare( rhs ) == -1;
	}
	
	bool operator > ( const TypeDesc& rhs ) const {
		return compare( rhs ) == 1;
	}
	
	bool operator == ( const TypeDesc& rhs ) const {
		return compare( rhs ) == 0;
	}
	
	
	QString nameWithParams() const;
	
	QString fullName( ) const;
	
	QString fullNameChain( ) const ;
	
	int pointerDepth() const {
		return m_pointerDepth;
	}
	
	void setPointerDepth( int d ) {
		m_pointerDepth = d;
	}
	
	void decreasePointerDepth() {
		if( m_pointerDepth > 0 )
			m_pointerDepth--;
	}
	
	QStringList fullNameList( ) const;
	
	QString name() const {
		return m_cleanName;
	};
	
	void setName( QString name ) {
		m_cleanName = name;
	}
	
	/// The template-params may be changed in-place
	/// this list is local, but the params pointed by them not
	TemplateParams& templateParams();
	
	const TemplateParams& templateParams() const;
	/*
	inline operator QString() const {
		return name();
	}*/
	
	///makes all template-params private so changing them will not affect any other TypeDesc-structures.
	TypeDesc& makePrivate();
	
	operator bool () const {
		return !m_cleanName.isEmpty();
	}
	
	TypeDescPointer next();
	
	bool hasTemplateParams() const ;
	
	void setNext( TypeDescPointer type );
	
	void append( TypeDescPointer type );
	
	TypePointer resolved();
	
	void setResolved( TypePointer resolved );
	
	void resetResolved();
	
	///Resets the resolved-pointers of this type, and all template-types
	void resetResolvedComplete();
	
	///these might be changed in future to an own data-member
	void increaseFunctionDepth();
	
	void decreaseFunctionDepth();
	
	int functionDepth() const;
	
	void takeInstanceInfo( const TypeDesc& rhs );
	
	void clearInstanceInfo();
};

class TypeDescShared : public TypeDesc, public KShared {
public:
	
	
	TypeDescShared( const TypeDescShared& rhs ) : TypeDesc(rhs), KShared() {
	}
	
	TypeDescShared( const TypeDesc& rhs ) : TypeDesc(rhs), KShared() {
	}
	
	TypeDescShared& operator = ( const TypeDesc& rhs ) {
		(*(TypeDesc*)this) = rhs;
		return *this;
	}	

	TypeDescShared( const QString& name = "" ) : TypeDesc( name ) {
	}
};

TypeDesc::TypeDesc( const QString& name )  {
	init( name );
}

TypeDesc::TypeDesc( const TypeDesc& rhs )  {
	*this = rhs;
}

bool TypeDesc::isValidType() const {
	if( m_cleanName.find("->") != -1 || m_cleanName.contains('.') || m_cleanName.contains(' ') || m_cleanName.isEmpty() ) return false;
	
	for( TemplateParams::const_iterator it = m_templateParams.begin(); it != m_templateParams.end(); ++it ) {
		if( !(*it)->isValidType() ) return false;
	}
	
	if( m_nextType ) if( !m_nextType->isValidType() ) return false;
	return true;
}

TypeDesc& TypeDesc::operator = ( const TypeDesc& rhs ) {
	m_cleanName = rhs.m_cleanName;
	m_dec = rhs.m_dec;
	m_functionDepth = rhs.m_functionDepth;
	m_nextType = rhs.m_nextType;
	m_resolved = rhs.m_resolved;
	m_pointerDepth = rhs.m_pointerDepth;
	m_templateParams = rhs.m_templateParams;
	return *this;
}

int TypeDesc::depth() const {
	int ret = 1;
	for( TemplateParams::const_iterator it = m_templateParams.begin(); it != m_templateParams.end(); ++it ) {
		ret = max( (*it)->depth() + 1, ret );
	}
	
	if( m_nextType ){
		ret = max( m_nextType->depth(), ret );
	}
	
	return ret;
}

int TypeDesc::length() const {
	if( !m_nextType && m_cleanName.isEmpty() ) return 0;
return m_nextType ? 1 + m_nextType->length() : 1;
}

	///Something is wrong with this function.. so i use the string-comparison
int TypeDesc::compare ( const TypeDesc& rhs ) const {
	
	
	if( m_functionDepth != rhs.m_functionDepth ) {
		if( m_functionDepth < rhs.m_functionDepth )
			return -1;
		else
			return 1;
	}
	
	if( m_pointerDepth != rhs.m_pointerDepth ) {
		if( m_pointerDepth < rhs.m_pointerDepth )
			return -1;
		else
			return 1;
	}
	
	if( m_cleanName != rhs.m_cleanName ) {
		if( m_cleanName < rhs.m_cleanName )
			return -1;
		else
			return 1;
	}
	if( m_templateParams.size() != rhs.m_templateParams.size() ) {
		if( m_templateParams.size() < rhs.m_templateParams.size() )
			return -1;
		else
			return 1;
	}
	
	TemplateParams::const_iterator it2 = rhs.m_templateParams.begin();
	for( TemplateParams::const_iterator it = m_templateParams.begin(); it != m_templateParams.end() && it2 != rhs.m_templateParams.end(); ) {
		if( int cmp =  (*it)->compare(**it2) != 0 ) {
			return cmp;
		}
		++it2; ++it;
	}
	
	if( !((bool)m_nextType) != ((bool)rhs.m_nextType) ) {
		if( m_nextType ) 
			return 1;
		else
			return -1;
	}
	
	if( m_nextType && rhs.m_nextType ) {
		if( int cmp = m_nextType->compare( *rhs.m_nextType ) != 0 )
			return cmp;
	}
	
	return 0;
}

QString TypeDesc::nameWithParams() const {
	QString ret = m_cleanName;
	if( !m_templateParams.isEmpty() ) {
		ret += "<";
		for( TemplateParams::const_iterator it = m_templateParams.begin(); it != m_templateParams.end(); ++it ) {
			ret += (*it)->fullNameChain();
			ret += ", ";
		}
		ret.truncate( ret.length() - 2 );
		ret += ">";
	}
	return ret;
}

QString TypeDesc::fullName( ) const {
	QString ret = nameWithParams();
	for( int a=0; a < m_functionDepth; ++a) ret = QString( functionMark ) + ret;
	for( int a=0; a < m_pointerDepth; ++a) ret += "*";
	return m_dec.apply( ret );
}

QString TypeDesc::fullNameChain( ) const {
	QString ret = fullName();
	if( m_nextType ) {
		ret += "::" + m_nextType->fullNameChain();
	}
	return m_dec.apply( ret );
}



QStringList TypeDesc::fullNameList( ) const {
	QStringList ret;
	ret << fullName();
	if( m_nextType ) {
		ret += m_nextType->fullNameList();
	}
	return ret;
};


	/// The template-params may be changed in-place
	/// this list is local, but the params pointed by them not
TypeDesc::TemplateParams& TypeDesc::templateParams() {
	return m_templateParams;
}

const TypeDesc::TemplateParams& TypeDesc::templateParams() const {
	return m_templateParams;
}

TypeDescPointer TypeDesc::next() {
	return m_nextType;
}

bool TypeDesc::hasTemplateParams() const {
	return !m_templateParams.isEmpty();
}

void TypeDesc::setNext( TypeDescPointer type ) {
	m_nextType = type;
}

void TypeDesc::append( TypeDescPointer type ) {
	if( type ) {
		if( m_nextType )
			m_nextType->append( type );
		else
			m_nextType = type;
	}
}

TypePointer TypeDesc::resolved() {
	return m_resolved;
}

void TypeDesc::setResolved( TypePointer resolved ) {
	m_resolved = resolved;
}

void TypeDesc::resetResolved() {
	m_resolved = 0;
	if( m_nextType ) m_nextType->resetResolved();
}

	///Resets the resolved-pointers of this type, and all template-types
void TypeDesc::resetResolvedComplete() {
	resetResolved();
	for( TemplateParams::iterator it = m_templateParams.begin(); it != m_templateParams.end(); ++it )
		(*it)->resetResolvedComplete();
}

	///these might be changed in future to an own data-member
void TypeDesc::increaseFunctionDepth() {
	m_functionDepth++;
}

void TypeDesc::decreaseFunctionDepth() {
	if( m_functionDepth > 0 ) m_functionDepth--;
}

int TypeDesc::functionDepth() const {
	return m_functionDepth;
}

void TypeDesc::takeInstanceInfo( const TypeDesc& rhs ) {
	m_pointerDepth += rhs.m_pointerDepth;
	m_dec += rhs.m_dec;
}

void TypeDesc::clearInstanceInfo() {
	m_pointerDepth = 0;
	m_dec = "";
}

void TypeDesc::takeTemplateParams( const QString& string ) {
	m_templateParams.clear();
	for( ParamIterator it( "<>", string ); it; ++it )
		m_templateParams.append( new TypeDescShared( *it ));
}


TypeDesc& TypeDesc::makePrivate() {
	TemplateParams nList;
	for( TemplateParams::const_iterator it = m_templateParams.begin(); it != m_templateParams.end(); ++it ) {
		TypeDescPointer tp( new TypeDescShared( ) );
		*tp = **it;
		tp->makePrivate();
		nList.append( tp );
	}
	m_templateParams = nList;
	
	if( m_nextType ) {
		TypeDescPointer tmp = m_nextType;
		m_nextType = new TypeDescShared();
		*m_nextType = *tmp;
		m_nextType->makePrivate();
	}
	return *this;
}


void TypeDesc::init( QString stri ) {
	m_templateParams.clear();
	m_cleanName = "";
	m_pointerDepth = 0;
	m_functionDepth = 0;
	m_nextType = 0;
	m_dec = "";
	
	if ( stri.isEmpty() )
		return;
	
	m_dec = stri;	///Store the decoration
	
	QStringList ls = splitType( stri );
	QString str = ls.front();
	
		///Extract multiple types that may be written as a scope and put them to the next-types-list
	if( !ls.isEmpty() ) {
		ls.pop_front();
		if( !ls.isEmpty() ) {
			m_nextType = TypeDescPointer( new TypeDescShared( ls.join("::") ) );
		}
	}
	
	if( str.startsWith( "typename " ) ) {
		str = str.right( str.length() - strlen( "typename " ) );
	}
	while( str.startsWith( QString( functionMark ) ) ) {
		m_functionDepth++;
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
		
		m_cleanName = type.stripWhiteSpace();
		takeTemplateParams( str );
		m_pointerDepth = countExtract( '*', str );
	}
	
}	

const char* TypeDesc::functionMark = "[function] ";

TypeDesc operator + ( const TypeDesc& lhs, const TypeDesc& rhs ) {
	TypeDesc ret = lhs;
	ret.makePrivate();
	ret.append( new TypeDescShared( rhs ) );
	return ret;
}

ItemDom locateModelContainer( CodeModel* m, TypeDesc t, ClassDom cnt = ClassDom() )
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


class SimpleTypeImpl;

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
	
	SimpleType( const SimpleType& rhs ) {
		*this = const_cast<SimpleType&>( rhs );///ugly but necessary
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
	
	bool operator < ( SimpleType& rhs );
	
	SimpleType( ItemDom item );
	
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
	
	SimpleType& operator = ( const QStringList& rhs ) {
		*this = SimpleType( rhs );
		return *this;
	}
	
	/** Just compares the scope */
	bool operator == ( const SimpleType& rhs ) const {
		return scope() == rhs.scope();
	}
	
	SimpleType& operator = ( const QString& rhs ) {
		*this = splitType( rhs );
		return *this;
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
	
private:	
	
	void init( const QStringList& scope , Repository rep );
	
	inline void resolve( Repository rep = Undefined ) const ;
	
	mutable TypePointer m_type;
	mutable bool m_resolved;
	static TypePointer m_globalNamespace; ///this is bad, but with the current parser we can't clearly determine the correct global-namespace for each class/file
	static bool m_unregistered;
	typedef std::set<SimpleTypeImpl*> TypeStore ;
	static TypeStore m_typeStore; ///This is necessary because TypeDescs ind SimpleTypeImpls can have cross-references, and thereby make themselves unreleasable, so each SimpleTypeImpl is stored in this list and destroyed at once by ConfigureSimpleTypes( it breaks all references )
	
	friend class SimpleTypeImpl;
	
	static void registerType( SimpleTypeImpl* tp ) {
		if( !tp ) return;
		m_typeStore.insert( tp );
	}
	
	static void unregisterType( SimpleTypeImpl* tp ) {
		m_typeStore.erase( tp );
		m_unregistered = true;
	}
	
public:
	static void setGlobalNamespace( TypePointer tp ) {
		m_globalNamespace = tp;
	}
	
	static void resetGlobalNamespace() {
		m_globalNamespace = 0;
	}
	
	
	///Since many cross-references are possible, this function breaks them all so that all SimpleTypeImpls can free themselves.
	static void destroyStore();
};

TypePointer SimpleType::m_globalNamespace;
SimpleType::TypeStore  SimpleType::m_typeStore;
bool SimpleType::m_unregistered = false;

class ConfigureSimpleTypes {
public:
	ConfigureSimpleTypes( QString currentFileName = "" ) {
		globalCurrentFile = currentFileName;
	}
	
	void setGlobalNamespace( TypePointer globalNamespace ) {
		SimpleType::setGlobalNamespace( globalNamespace );
	}
	
	virtual ~ConfigureSimpleTypes() {
		SimpleType::resetGlobalNamespace();
		SimpleType::destroyStore();
	}
};

struct CppCodeCompletion::ExpressionInfo {
private:
	QString m_expr;
	
public:
	
	enum Type {
		InvalidExpression = 0,
		NormalExpression = 1,
		TypeExpression = 2
	};
	
	Type t;
	int start, end;
	
	QString expr() {
		return m_expr;
	}
	
	void setExpr( const QString& str ) {
		m_expr = clearComments( str );
	}
	
	ExpressionInfo( QString str ) : t(  NormalExpression ), start( 0 ), end( str.length() ) {
		setExpr( str );
		
	}
	
	ExpressionInfo() : t( InvalidExpression ), start(0), end(0) {
	}
	
	operator bool() {
		return t != InvalidExpression && !m_expr.isEmpty();
	}
	
	bool isTypeExpression() {
		return t == TypeExpression && !m_expr.isEmpty();
	}
	
	bool canBeTypeExpression() {
		return t & TypeExpression && !m_expr.isEmpty();
	}
	
	bool isNormalExpression() {
		return t == NormalExpression && !m_expr.isEmpty();
	}
	
	bool canBeNormalExpression() {
		return t & NormalExpression && !m_expr.isEmpty();
	}
	
	QString typeAsString() {
		QString res ;
		if( t & NormalExpression )
			res += "NormalExpression, ";
		if( t & TypeExpression )
			res += "TypeExpression, ";
		if( t == InvalidExpression )
			res += "InvalidExpression, ";
		if( !res.isEmpty() ) {
			res = res.left( res.length() - 2 );
		} else {
			res = "Unknown";
		}
		return res;
	}
};

class CppCodeCompletion::EvaluationResult
{
public:
	EvaluationResult& operator = ( const EvaluationResult& rhs ) {
		resultType = rhs.resultType;
		sourceVariable = rhs.sourceVariable;
		expr = rhs.expr;
		return *this;
	}
	
	EvaluationResult( const EvaluationResult& rhs ) : resultType( rhs.resultType), expr( rhs.expr ), sourceVariable( rhs.sourceVariable ) {
	}
	SimpleType resultType; ///The resulting type
	
	ExpressionInfo expr; ///Information about the expression that was processed
	
	DeclarationInfo sourceVariable; ///If the type comes from a variable, this stores Information about it
	
	EvaluationResult( SimpleType tp = SimpleType(), DeclarationInfo var = DeclarationInfo() ): resultType( tp ), sourceVariable( var ) {
	}
	
	operator SimpleType () const {
		return resultType;
	}
	
	SimpleTypeImpl* operator -> () {
		return &(*resultType);
	}
	
	operator bool() const {
		return (bool)resultType;
	}
};

namespace CppEvaluation {

class Operator;


struct OperatorIdentification {
	QValueList<QString> innerParams; /** Inner parameters of the operator( for the vec["hello"] the "hello" ) */
	int start, end; /** Range the operator occupies */
	bool found;
	Operator* op; ///Can be 0 !
	
OperatorIdentification() : start(0), end(0), found(false), op(0) {
	}
	
	operator bool() {
		return found;
	}
};


class Operator {
public:
	enum BindingSide {
		Neutral = 0,
		Left = 1,
		Right = 2
	};
	enum Type {
		Unary = 1,
		Binary = 2,
		Ternary = 3
	};
	
	virtual ~Operator() {
	}
	
	virtual int priority() = 0;
	
	virtual Type type() = 0; 
	virtual int paramCount() = 0;
	
	///"binding" means that the operator needs the evaluated type of the expression on that side
	///The types of all bound sides will later be sent in the "params"-list of the apply-function
	virtual BindingSide binding() = 0;	///The side to which the operator binds
	
	///When this returns true, the ident-structure must be filled correctly
	virtual OperatorIdentification identify( QString& str ) = 0;
	
	///params 
	virtual EvaluationResult apply( QValueList<EvaluationResult> params, QValueList<EvaluationResult> innerParams ) = 0;
	
	virtual QString name() = 0;
	
protected:
	void log( const QString& msg ) {
	dbg() << "\"" << name() << "\": " << msg << endl;
	};
	
	QString printTypeList( QValueList<EvaluationResult>& lst );
};


class OperatorSet {
private:
	typedef QValueList< Operator* > OperatorList;
	OperatorList m_operators;
public:
	OperatorSet() {
	}
	
	~OperatorSet() {
		for( QValueList< Operator* >::iterator it = m_operators.begin(); it != m_operators.end(); ++it ) {
			delete *it;
		}
	}
	
	void registerOperator( Operator* op ) {
		m_operators << op;
	}
	
	OperatorIdentification identifyOperator( const QString& str_ , Operator::BindingSide allowedBindings = (Operator::BindingSide) (Operator::Left | Operator::Right | Operator::Neutral) ) {
		QString str = str_.stripWhiteSpace();
		for( OperatorList::iterator it = m_operators.begin(); it != m_operators.end(); ++it ) {
			if( ((*it)->binding() & allowedBindings) == (*it)->binding() ) {
				if( OperatorIdentification ident = (*it)->identify( str ) ) {
					return ident;
				}
			}
		}
		
		return OperatorIdentification();
	}

	
} AllOperators;


template <class OperatorType>
class RegisterOperator {
public:
	RegisterOperator( OperatorSet& set ) {
		set.registerOperator( new OperatorType() );
	}
	~RegisterOperator() {
	}
};

QString nameFromType( SimpleType t );


class UnaryOperator : public Operator{
private:
	int m_priority;
	QString m_identString;
	QString m_name;
	Operator::BindingSide m_binding;
protected:
	
	inline QString  identString() const {
		return m_identString;
	}
	
public:
	UnaryOperator( int priority , QString identString, QString description, Operator::BindingSide binding ) : Operator(), m_priority( priority ), m_identString( identString ), m_name( description ), m_binding( binding )  {
	}
	
	virtual int priority() {    return m_priority;    }
	
	virtual Operator::Type type() {   return Operator::Unary;   }
	
	virtual Operator::BindingSide binding() {   return m_binding;   }
	
	virtual int paramCount() {  return 1; }
	
	virtual OperatorIdentification identify( QString& str ) {
		OperatorIdentification ret;
		if( str.startsWith( m_identString ) ) {
			ret.start = 0;
			ret.end = m_identString.length();
			ret.found = true;
			ret.op = this;
		}
		return ret;
	}
	
	virtual EvaluationResult unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& innerParams ) = 0;
	
	virtual bool checkParams( const QValueList<EvaluationResult>& params ) {
		return !params.isEmpty() && params[0];
	}
	
	
	virtual EvaluationResult apply( QValueList<EvaluationResult> params, QValueList<EvaluationResult> innerParams ) {
		if( !checkParams( params ) ) {
			log( QString("parameter-check failed: %1 params: ").arg( params.size() ) + printTypeList( params ) );
				return EvaluationResult();
		} else {
			EvaluationResult t = unaryApply( params.front(), innerParams );
			if( !t ) {
				if( params.front() )
					log( "could not apply \"" + name() + "\" to \"" + nameFromType( params.front() ) + "\"");
				else
					log( "operator \"" + name() + "\" applied on \"" + nameFromType( params.front() )  + "\": returning unresolved type \"" + nameFromType( t ) + "\"");
			}
			return t;
		}
	}
	
	virtual QString name() {
		return m_name;
	}
};


class NestedTypeOperator : public UnaryOperator {
public:
	NestedTypeOperator() : UnaryOperator( 18, "::", "nested-type-operator", Operator::Left ) {
	}
	virtual EvaluationResult unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& /*innerParams*/ ) { 
		return param;
	}
};

//RegisterOperator< NestedTypeOperator > NestedTypeReg( AllOperators );	///This registers the operator to the list of all operators

class DotOperator : public UnaryOperator {
public:
	DotOperator() : UnaryOperator( 17, ".", "dot-operator", Operator::Left ) {
	}
	
	virtual EvaluationResult unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& /*innerParams*/ ) { 
		return param;
	}
};

RegisterOperator< DotOperator > DotReg( AllOperators ); ///This registers the operator to the list of all operators
}

class SimpleTypeFunctionInterface;


using namespace CppEvaluation;


class SimpleTypeImpl : public KShared {
private:
	int m_resolutionCount;
	enum ResolutionFlags {
		NoFlag = 0,
		HadTypedef = 1,
		HadTemplate = 2,
		HadAlias = 3
	};
	
	QValueList<TypeDesc> m_trace; ///pointer to the previous type in the trace-chain

	SimpleTypeImpl( const SimpleTypeImpl& /*rhs*/ ) : KShared() {
	}
	
	SimpleTypeImpl& operator = ( const SimpleTypeImpl& /*rhs*/ ) {
		return *this;
	}
	
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
	
private:
	
	ResolutionFlags m_resolutionFlags;
	
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
	
public:
	
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
	
protected:
	virtual void invalidateCache() {
	};
	
	///Secondary cache also depends on the surrounding
	virtual void invalidateSecondaryCache() {
	}
	
	virtual void setSecondaryCacheActive( bool active ) {
	}
	
	///Primary cache is the one associated only with this object. It must only be cleared
	///when the object pointed to changes
	virtual void invalidatePrimaryCache() {
	}
	
	static QString operatorToString( Operator op ) {
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
	
	SimpleType getFunctionReturnType( QString functionName, QValueList<SimpleType> params = QValueList<SimpleType>() ) {
		SimpleType t = typeOf( functionName, MemberInfo::Function );
		if( t->asFunction() ) {
			t = t->applyOperator( ParenOp, params );
			return t;
		} else {
			dbg() << "error " << endl;
			return SimpleType();
		}
	}
	
	///Tries to apply the operator and returns the new type. If it fails, it returns an invalid type.
	virtual SimpleType applyOperator( Operator op , QValueList<SimpleType> params = QValueList<SimpleType>() ) {
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
			return *this;
		}
		TypeBuildInfo( const TypeBuildInfo& rhs ) : KShared() {
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
	TypeDesc replaceTemplateParams( TypeDesc desc, TemplateParamInfo& paramInfo ) {
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
	
	TypeDesc resolveTemplateParams( TypeDesc desc, LocateMode mode = Normal ) {
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


	/**By default templates are included while the resolution, so when the type should be addressed from
	outside of the class, ExcludeTemplates should be set as LocateMode, since templates can not be directly accessed 	from the outside.
	The resulting type's template-params may not be completely resolved, but can all be resolved locally by that type*/

	virtual SimpleType locateType( TypeDesc name , LocateMode mode = Normal, int dir = 0 ,  MemberInfo::MemberType typeMask = bitInvert( addFlag( MemberInfo::Variable, MemberInfo::Function ) ) ) {
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
				ret.makePrivate();	///Maybe some small parameters like the pointer-depth were changed, so customize those
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
						ret = rett;	///When there is no perfect match, at least create a helpful best match
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
						mem.type.clearInstanceInfo();	///The cache-lookup also respects the instance-info, so back it up, clear it, and apply it afterwards, to get more cache-hits.
						
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
		MemberInfo mem;
		mem.memberType = MemberInfo::NotFound;
		return mem;
	};
		
public:

	/**TypeDescs and SimpleTypeImpls usually have a cross-reference, which creates a circular dependency so that they are never freed using KShared. This function breaks the loop, and also breaks all other possible dependency-loops. After this function was called, the type still contains its private information, but can not not be used to resolve anything anymore. This function is called automatically while the destruction of ConfigureSimpleTypes */
	virtual void breakReferences() {
		m_parent = 0;
		m_desc.resetResolved();
		m_trace.clear();
		m_masterProxy = 0;
		invalidateCache();
	}
	
	///Returns either itself, or the (namespace-)proxy this type is a slave of.
	inline TypePointer bigContainer() {
		if( m_masterProxy )
			return m_masterProxy;
		else
			return TypePointer( this );
	}
	
	///Returns the parent, eg. the SimpleType this one is nested in.
	SimpleType parent() {
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
	
	
	///this must be a reference, so the desc can be manipulated in-place from outside
	const TypeDesc& desc() {
		if( ! scope().isEmpty() );
		if( m_desc.name().isEmpty() )
			m_desc.setName( cutTemplateParams( scope().back()) );
		m_desc.setResolved( this );
		return m_desc;
	}
	
	TypeDesc& descForEdit()  {
		desc();
		invalidateCache();
		return m_desc;
	}
	

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
	QString fullTypeResolved( int depth = 0 ) {
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


	///this completely evaluates everything
	QString fullTypeResolvedWithScope( int depth = 0 ) {
		if( parent() ) {
			return parent()->fullTypeResolvedWithScope() + "::" + fullTypeResolved();
		} else {
			return fullTypeResolved();
		}
	}
	
	
	
	QStringList m_scope;
	QMap<QString, QString> m_localTypedefs;
	int m_pointerDepth;
	TypePointer m_parent;
	bool m_isGlobal;


protected:
	SimpleTypeImpl( SimpleTypeImpl* rhs ) : m_resolutionCount( rhs->m_resolutionCount ), m_pointerDepth( rhs-> m_pointerDepth), m_parent( rhs->m_parent ), m_isGlobal( rhs->m_isGlobal ), m_resolutionFlags( rhs->m_resolutionFlags ), m_trace( rhs->m_trace), m_masterProxy( rhs->m_masterProxy ), m_scope( rhs->m_scope), m_desc( rhs->m_desc )  {
		reg();
	}
	
	TypeDesc m_desc;  ///descibes the local type(so next() must be null)
	
	/** Tries to extract template-parameters from the scope, resets the params-list */
	virtual void checkTemplateParams () {
		invalidateCache();
		if( ! m_scope.isEmpty() ) {
			QString str = m_scope.back();
			m_desc = str;
			m_scope.pop_back();
			m_scope << m_desc.name();
		}
	}
	
	void setScope( const QStringList& scope ) {
		invalidateCache();
		m_scope = scope;
	}
	
	
	TypeOfResult searchBases ( const TypeDesc& name ) {
		QValueList<SimpleType> parents = getBases();
		for ( QValueList<SimpleType>::iterator it = parents.begin(); it != parents.end(); ++it )
		{
			TypeOfResult type = (*it)->typeOf( name.name() );
			if ( type )
				return type;
		}
		return TypeOfResult();
	}
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
	
	virtual QString signature() 
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
	
	
///TODO: This function should locate the correct overloaded method in the chain, fitting the parameters
	///should also moved into another class then
	SimpleTypeImpl* match( const QValueList<SimpleType>& params ) {
		//		QValueList<TypeDesc> args = getArgumentTypes();
		return dynamic_cast<SimpleTypeImpl*>( this );
	}
	
protected:
	
	bool containsUndefinedTemplateParam( TypeDesc& desc, SimpleTypeImpl::TemplateParamInfo& paramInfo ) 
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
	
	///Tries to match the types, filling implicit template-params into paramInfo
	void resolveImplicitTypes( TypeDesc& argType, TypeDesc& gottenArgType, SimpleTypeImpl::TemplateParamInfo& paramInfo ) 
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
	
	///Tries to match the types, filling implicit template-params into paramInfo
	void resolveImplicitTypes( TypeDesc::TemplateParams& argTypes, TypeDesc::TemplateParams& gottenArgTypes, SimpleTypeImpl::TemplateParamInfo& paramInfo ) 
	{
		TypeDesc::TemplateParams::iterator it = argTypes.begin();
		TypeDesc::TemplateParams::iterator it2 = gottenArgTypes.begin();
		
		while( it != argTypes.end() && it2 != gottenArgTypes.end() ) {
			resolveImplicitTypes( **it, **it2, paramInfo );
			++it;
			++it2;
		}
	}	
	
	///Tries to match the types, filling implicit template-params into paramInfo
	void resolveImplicitTypes( QValueList<TypeDesc>& argTypes, QValueList<TypeDesc>& gottenArgTypes, SimpleTypeImpl::TemplateParamInfo& paramInfo ) 
	{
		QValueList<TypeDesc>::iterator it = argTypes.begin();
		QValueList<TypeDesc>::iterator it2 = gottenArgTypes.begin();
		
		while( it != argTypes.end() && it2 != gottenArgTypes.end() ) {
			resolveImplicitTypes( *it, *it2, paramInfo );
			++it;
			++it2;
		}
	}	
};



template <class Base>
class SimpleTypeCacheBinder : public Base {
private:
bool secondaryActive, primaryActive;
public:

	SimpleTypeCacheBinder( SimpleTypeCacheBinder<Base>* b ) : Base( b ), secondaryActive( b->secondaryActive ), 	primaryActive( b->primaryActive ), m_locateCache( b->m_locateCache ), m_memberCache( b->m_memberCache), m_basesCache( b->m_basesCache )  {
	}
		
	SimpleTypeCacheBinder() : Base(), secondaryActive( true ), primaryActive( true ) {
	}
	
	template<class InitType> 
		SimpleTypeCacheBinder( InitType t ) : Base ( t ), secondaryActive( true ), primaryActive( true ) {
		}
	
	template<class InitType1, class InitType2> 
		SimpleTypeCacheBinder( InitType1 t, InitType2 t2 ) : Base ( t, t2 ), secondaryActive( true ), primaryActive( true ) {
		}
	
	using Base::LocateMode;
	
	struct LocateDesc {
		TypeDesc mname;
		QString fullName;
		SimpleTypeImpl::LocateMode mmode;
		int mdir;
		SimpleTypeImpl::MemberInfo::MemberType mtypeMask;
		LocateDesc() {
		}
		
		
		LocateDesc( TypeDesc name, SimpleTypeImpl::LocateMode mode, int dir, SimpleTypeImpl::MemberInfo::MemberType typeMask )  : mname( name ), mmode( mode ) , mdir( dir ) , mtypeMask( typeMask ) {
			fullName = mname.fullNameChain();
		}
		
		int compare( const LocateDesc& rhs ) const {
			QString a = fullName; //mname.fullNameChain();
			QString b = rhs.fullName; //mname.fullNameChain();
			if( a != b ) {
				if( a < b )
					return -1;
				else
					return 1;
			}
			
			if( mmode != rhs.mmode ) {
				if( mmode < rhs.mmode )
					return -1;
				else
					return 1;
			}
			if( mdir != rhs.mdir ) {
				if( mdir < rhs.mdir )
					return -1;
				else
					return 1;
			}
				if( mtypeMask != rhs.mtypeMask ) {
					if( mtypeMask < rhs.mtypeMask )
					return -1;
				else
					return 1;
			}
			return 0;
		}
		
		bool operator < ( const LocateDesc& rhs ) const {
			return compare( rhs ) == -1;
		}
		
		bool operator == ( const LocateDesc& rhs ) const {
			return compare( rhs ) == 0;
		}
		
		bool operator > ( const LocateDesc& rhs ) const {
			return compare( rhs ) == 1;
		}
	};
	
	struct MemberFindDesc {
		TypeDesc m_desc;
		QString fullName;
		SimpleTypeImpl::MemberInfo::MemberType findType;
		MemberFindDesc() {
		}
		MemberFindDesc( TypeDesc d, SimpleTypeImpl::MemberInfo::MemberType ft ) : m_desc( d ), findType( ft ) {
			//m_desc.makePrivate();
			fullName = m_desc.fullNameChain();
		}
	
		int compare( const MemberFindDesc& rhs ) const {
			QString a = fullName; //m_desc.fullNameChain();
			QString b = rhs.fullName; //m_desc.fullNameChain();
			if( a != b ) {
				if( a < b )
					return -1;
				else
					return 1;
			}/*
			if( int cmp = m_desc.compare( rhs.m_desc ) != 0 ) 
				return cmp;*/
			
			if( findType != rhs.findType ) {
				if( findType < rhs.findType )
					return -1;
				else
					return 1;
			}
			
			return 0;
		}
		
		bool operator < ( const MemberFindDesc& rhs ) const {
			return compare( rhs ) == -1;
		}
		
		bool operator == ( const MemberFindDesc& rhs ) const {
			return compare( rhs ) == 0;
		}
		
		bool operator > ( const MemberFindDesc& rhs ) const {
			return compare( rhs ) == 1;
		}
		
	};
	
	
	typedef QMap<LocateDesc, SimpleType> LocateMap;
	typedef QMap<MemberFindDesc, SimpleTypeImpl::MemberInfo > MemberMap;
private:
	LocateMap m_locateCache;
	MemberMap m_memberCache;
	QValueList<SimpleType> m_basesCache;
public:
	
	virtual SimpleTypeImpl::MemberInfo findMember( TypeDesc name , SimpleTypeImpl::MemberInfo::MemberType type )  {
		if( !primaryActive ) return Base::findMember( name, type );
		MemberFindDesc key( name, type );
		typename MemberMap::iterator it = m_memberCache.find( key );
		
		if( it != m_memberCache.end() ) {
			dbg() << "\"" << Base::str() << "\" took member-info for \"" << name.fullNameChain() << "\" from the cache" << endl;
			return m_memberCache[key];
		} else {
			SimpleTypeImpl::MemberInfo mem = Base::findMember( name, type );
			m_memberCache[key] = mem;
			/*typename MemberMap::iterator it = m_memberCache.find( key );
			if( it == m_memberCache.end() ) dbg() << "\"" << Base::str() << "\"remap failed with \""<< name.fullNameChain() << "\"" << endl;*/
			
			return mem;
		}
	}
		
	
	virtual SimpleType locateType( TypeDesc name , SimpleTypeImpl::LocateMode mode, int dir,  SimpleTypeImpl::MemberInfo::MemberType typeMask )
	{
		if( !secondaryActive ) return  Base::locateType( name, mode, dir, typeMask );
		LocateDesc desc( name, mode, dir, typeMask );
		
		typename LocateMap::iterator it = m_locateCache.find( desc );
		
		if( it != m_locateCache.end() ) {
			Debug d("#lo#");
			dbg() << "\"" << Base::str() << "\" located \"" << name.fullNameChain() << "\" from the cache" << endl;
			return *it;
		} else {
			SimpleType t = Base::locateType( name, mode, dir, typeMask );
			m_locateCache[ desc ] = t;
			/*typename LocateMap::iterator it = m_locateCache.find( desc );
			if( it == m_locateCache.end() ) dbg() << "\"" << Base::str() << "\"remap failed with \""<< name.fullNameChain() << "\"" << endl;*/
			return t;
		}
	}
	
	virtual QValueList<SimpleType> getBases() {
		if( !m_basesCache.isEmpty() ) {
			dbg() << "\"" << Base::str() << "\" took base-info from the cache" << endl;
			return m_basesCache;
		} else
			return Base::getBases();
	}
protected:
	
	virtual typename Base::TypePointer clone() {
		return new SimpleTypeCacheBinder<Base>( this );
	}
	
	virtual void invalidatePrimaryCache() {
		if( !m_memberCache.isEmpty() ) dbg() << "\"" << Base::str() << "\" primary caches cleared" << endl;
		m_memberCache.clear();
	}
	virtual void invalidateSecondaryCache() {
		if( !m_locateCache.isEmpty() ) dbg() << "\"" << Base::str() << "\" secondary caches cleared" << endl;
		m_locateCache.clear();
		m_basesCache.clear();
	}
	
	virtual void setSecondaryCacheActive( bool active ) {
		secondaryActive = active;
	}
	
	virtual void setPrimaryCacheActive( bool active ) {
		primaryActive = active;
	}
	
	virtual void invalidateCache() {
		invalidatePrimaryCache();
		invalidateSecondaryCache();
	};
	
private:
	
	/*	typedef QMap<QString, TypePointer> AliasMap;
	AliasMap m_aliasCache;
	
	bool haveCachedAlias( const QString& type ) {
		AliasMap::iterator it = m_aliasCache.find( type );
		if( it != m_aliasCache.end() )
			return true;
		else
			return false;
	}
	
	void setCachedAlias( const QString& key, TypePointer val ) {
		m_aliasCache[ key ] = val;
	}
	
	TypePointer getCachedAlias( SimpleType type ) {
		AliasMap::iterator it = m_aliasCache.find( type );
		if( it != m_aliasCache.end() )
			return &(**it);
		else
			dbg() << "getCachedAlias: trying to get alias which is not available in \"" << str() << "\" alias: \"" << type.str() << "\"";
		return TypePointer( );
	}
	
	
	
	void clearAliases() {
		m_aliasCache.clear();
	}	*/
};



//typedef SimpleTypeCacheBinder<SimpleTypeImpl> SimpleTypeImpl;

class SimpleTypeCodeModel;
class SimpleTypeCatalog;
class SimpleTypeNamespace;

typedef SimpleTypeCacheBinder<SimpleTypeCodeModel> SimpleTypeCachedCodeModel;
typedef SimpleTypeCacheBinder<SimpleTypeCatalog> SimpleTypeCachedCatalog;
typedef SimpleTypeCacheBinder<SimpleTypeNamespace> SimpleTypeCachedNamespace;

typedef SimpleTypeCachedCodeModel SimpleTypeUsedCodeModel;
typedef SimpleTypeCachedCatalog SimpleTypeUsedCatalog;
typedef SimpleTypeCachedNamespace SimpleTypeUsedNamespace;

class SimpleTypeCodeModel : public SimpleTypeImpl {
private:
	ItemDom m_item;
	
	bool findItem() {
		QString key = str();
		m_item = locateModelContainer( cppCompetionInstance->m_pSupport->codeModel(), str() );
		return (bool) m_item;
	}
	
	
	
	void init() {
		if( scope().isEmpty() ) {
			m_item = cppCompetionInstance->m_pSupport->codeModel() ->globalNamespace();
		}else{
			findItem();
		}
	}
	
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
	
	virtual DeclarationInfo getDeclarationInfo() {
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
	
	SimpleTypeCodeModel( ItemDom& item ) : m_item( item ) {
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
	};
	
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
	
	virtual TemplateParamInfo getTemplateParamInfo() {
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
	
	virtual const TypeDesc findTemplateParam( const QString& name ) {
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
	
	/** In case of a class, returns all base-types */
	virtual QValueList<SimpleType> getBases() {
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
	
	
protected:
	
	struct CodeModelBuildInfo : public TypeBuildInfo {
		ItemDom m_item;
		TypeDesc m_desc;
		TypePointer m_parent;
		CodeModelBuildInfo( ItemDom item, TypeDesc& desc, TypePointer parent ) : m_item( item ), m_desc( desc ), m_parent( parent ) {
		}
		
		virtual TypePointer build() {
			TypePointer tp = new SimpleTypeUsedCodeModel( m_item );
			tp->parseParams( m_desc );
			if( m_parent ) tp->setParent( m_parent->bigContainer() );
			return tp;
		}
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

///Until header-parsing is implemented, this tries to find the class that is most related to this item
ClassDom pickMostRelated( ClassList lst, QString fn ) {
	if( lst.isEmpty() ) return ClassDom();
	if( fn.isEmpty() ) return lst.front();
	
	ClassDom best = lst.front();
	int bestMatch = 0;
	//kdDebug() << "searching most related to " << fn << endl;
	
	for( ClassList::iterator it = lst.begin(); it != lst.end(); ++it ) {
		//kdDebug() << "comparing " << (*it)->fileName() << endl;
		QString str = (*it)->fileName();
		int len = str.length();
		if( fn.length() < len ) len = fn.length();
		
		int matchLen = 0;
		for( int a = 0; a < len; a++ ) {
			if( str[a] == fn[a] ) 
				matchLen++;
			else
				break;
		}
		
		if( matchLen > bestMatch ) {
			//kdDebug() << "picking " << str << endl;
			bestMatch = matchLen;
			best = *it;
		}
	}
	
	//kdDebug() << "picked " << best->fileName() << endl;
	
	return best;
}

SimpleTypeImpl::MemberInfo SimpleTypeCodeModel::findMember( TypeDesc name , SimpleTypeImpl::MemberInfo::MemberType type ) 
{
	MemberInfo ret;
	ret.name = name.name();
	ret.memberType = MemberInfo::NotFound;
	if( !name || !m_item ) return ret;
	
	ClassModel* klass = dynamic_cast<ClassModel*> ( & (*m_item) );
	if( !klass ) {
	dbg() << "\"" << str() << "\": search for member " << name.name() << " unsuccessful because the own type is invalid" << endl;
		return ret;
	}
	NamespaceModel* ns = dynamic_cast<NamespaceModel*>(klass);
	
	if( klass->hasVariable( name.name() )  && ( type & MemberInfo::Variable ) ) {
		ret.memberType = MemberInfo::Variable;
		VariableDom d = klass->variableByName( name.name() );
		if( d ) {
			ret.type = d->type();
			ret.decl.name = d->name();
			ret.decl.file = d->fileName();
			ret.decl.comment = d->comment();
			d->getStartPosition( &ret.decl.startLine, &ret.decl.startCol );
			d->getEndPosition( &ret.decl.endLine, &ret.decl.endCol );
		}
	} else if( klass->hasFunction( name.name() )  && ( type & MemberInfo::Function ) ) {
		ret.memberType = MemberInfo::Function;
		FunctionList l = klass->functionByName( name.name() );
		if( !l.isEmpty() && l.front() ) {
			ret.setBuildInfo( new SimpleTypeCodeModelFunction::CodeModelFunctionBuildInfo( l, name , TypePointer(this) ) );
			ret.type = l.front()->resultType();
			ret.type.increaseFunctionDepth();
		}
	} else if( klass->hasFunctionDefinition( name.name() )  && ( type & MemberInfo::Function ) ) {
		ret.memberType = MemberInfo::Function;
		FunctionDefinitionList l = klass->functionDefinitionByName( name.name() );
		if( !l.isEmpty() && l.front() ) {
			ret.setBuildInfo( new SimpleTypeCodeModelFunction::CodeModelFunctionBuildInfo( l, name, TypePointer(this) ) );
			ret.type = l.front()->resultType();
			ret.type.increaseFunctionDepth();
		}
	} else if( klass->hasTypeAlias( name.name() ) && ( type & MemberInfo::Typedef ) ) {
		ret.memberType = MemberInfo::Typedef;
		TypeAliasList l = klass->typeAliasByName( name.name() );
		if( !l.isEmpty() && l.front() ) {
			ret.type = l.front()->type();
		}
	} else if ( klass->hasClass( name.name() ) && ( type & MemberInfo::NestedType ) ) {
		ClassList l = klass->classByName( name.name() );
		
		if( !l.isEmpty() ) {
			ClassDom i = pickMostRelated( l, globalCurrentFile );
			if( i ) {
				ret.setBuildInfo( new CodeModelBuildInfo( model_cast<ItemDom>( i ), name, TypePointer( this ) ) );
				
				ret.memberType = MemberInfo::NestedType;
				ret.type = name;
			}
		}
	} else if ( ns && ns->hasNamespace( name.name() )  && ( type & MemberInfo::Namespace ) ) {
		ret.setBuildInfo( new CodeModelBuildInfo( model_cast<ItemDom>( ns->namespaceByName( name.name() )), name, TypePointer( this ) ) );
		ret.memberType = MemberInfo::Namespace;
		ret.type = name;
	} else {
		if( type & MemberInfo::Template ) {
			TypeDesc s = findTemplateParam( name.name() );
			if( s ) {
				ret.memberType = MemberInfo::Template;
				ret.type = s;
			}
		}
	}
	if( !ret.type ) ret.memberType = MemberInfo::NotFound;
	return ret;
}

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

class SimpleTypeCatalog : public SimpleTypeImpl {
private:
	Tag m_tag;
	
    int pointerDepthFromString( const QString& str ) {
        QRegExp ptrRx( "(\\*|\\&)" );
        QString ptr = str.mid( str.find( ptrRx ) );
        QStringList ptrList = QStringList::split( "", ptr );
        return ptrList.size();
    }
	
	Tag findSubTag( const QString& name ) {
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
	
	QValueList<Tag> getBaseClassList( )
	{
		if ( scope().isEmpty() )
			return QValueList<Tag>();
		return cppCompetionInstance->m_repository->getBaseClassList( scope().join("::"));
	}
	
    void initFromTag() {
	    QStringList l = m_tag.scope();
	    l << m_tag.name();
	    setScope( l );
    }
	
	void init() {
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
	
protected:
	const Tag& tag() {
		return m_tag;
	}
	
public:
	
	SimpleTypeCatalog() {
	};
	
	virtual DeclarationInfo getDeclarationInfo() {
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
	
	virtual QValueList<SimpleType> getBases() {
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
	
	virtual TemplateParamInfo getTemplateParamInfo() {
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
	
	virtual const TypeDesc findTemplateParam( const QString& name ) {
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
	
	
protected:
	
	struct CatalogBuildInfo : public TypeBuildInfo {
		Tag m_tag;
		TypeDesc m_desc;
		TypePointer m_parent;
		
		CatalogBuildInfo( Tag tag , TypeDesc& desc, TypePointer parent ) : m_tag( tag ) , m_desc( desc ), m_parent( parent )   {
		}
		
		virtual TypePointer build() {
			if( !m_tag ) 
				return TypePointer();
			else {
				TypePointer tp = new SimpleTypeUsedCatalog( m_tag );
				tp->parseParams( m_desc );
				if( m_parent ) tp->setParent( m_parent->bigContainer() );
				return tp;
			}
				
		}
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


QString Operator::printTypeList( QValueList<EvaluationResult>& lst )
{
	QString ret;
	for( QValueList<EvaluationResult>::iterator it = lst.begin(); it != lst.end(); ++it ) {
		ret += "\"" + (*it)->fullType() + "\", ";
	}
	ret.truncate( ret.length() - 3 );
	return ret;
}

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
		
		virtual TypePointer build() {
			SimpleTypeNamespace* ns = new SimpleTypeUsedNamespace( m_fakeScope, m_realScope );
			for( QValueList<QStringList>::iterator it = m_imports.begin(); it != m_imports.end(); ++it )
				ns->addAliasMap( "", (*it).join("::") );
			return ns;
		}
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
		
	
	virtual MemberInfo findMember( TypeDesc name, MemberInfo::MemberType type =  MemberInfo::AllTypes)
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
	
	
	MemberInfo setupMemberInfo( TypeDesc& subName, QStringList tscope, QValueList<QStringList> imports = QValueList<QStringList>() ) {
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
	
	
	QStringList locateNamespace( QString alias ) {
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
	
	
	
	void recurseAliasMap() {
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
	void addAliasMap( QString name, QString alias , bool recurse = true ) {
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
	
	///Takes a map of multiple aliases in form "A=B;C=D;....;" similar to the C++ "namespace A=B;" statement
	void addAliases( QString map ) {
		while(!map.isEmpty() ) {
			int mid = map.find( "=" );
			if( mid == -1 ) break;
			int end = map.find( ";", mid+1 );
			if( end == -1 ) break;
			
			addAliasMap( map.left( mid ).stripWhiteSpace(), map.mid( mid+1, end - mid - 1 ).stripWhiteSpace() );
			map = map.mid( end + 1);
		}
	};
};



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


template<class To, class From>
QValueList<To> convertList( const QValueList<From>& from ) {
	QValueList<To> ret;
	for( typename QValueList<From>::const_iterator it = from.begin(); it != from.end(); ++it ) {
		ret << (To)*it;
	}
	return ret;
}

namespace CppEvaluation {


class ArrowOperator : public UnaryOperator{
public:
ArrowOperator() : UnaryOperator( 17, "->", "arrow-operator", Operator::Left ) {
}
	
	virtual EvaluationResult unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& innerParams ) { 
		return param->applyOperator( SimpleTypeImpl::ArrowOp , convertList<SimpleType, EvaluationResult>(innerParams) );
	}
};

RegisterOperator< ArrowOperator > ArrowReg( AllOperators );  ///This registers the operator to the list of all operators



class StarOperator : public UnaryOperator{
public:
StarOperator() : UnaryOperator( 15, "*", "star-operator", Operator::Right ) { ///Normally this should have a priority of 16, but that would need changes to the expression-parsing-loop
}
	
	virtual EvaluationResult unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& innerParams ) { 
		return param->applyOperator( SimpleTypeImpl::StarOp );
	}
};

RegisterOperator< StarOperator > StarReg( AllOperators );  ///This registers the operator to the list of all operators

class AddressOperator : public UnaryOperator{
public:
AddressOperator() : UnaryOperator( 16, "&", "address-operator", Operator::Right ) {
}
	
	virtual EvaluationResult unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& innerParams ) { 
		return param->applyOperator( SimpleTypeImpl::AddrOp );
	}
};

RegisterOperator< AddressOperator > AddressReg( AllOperators );  ///This registers the operator to the list of all operators


class UnaryParenOperator : public UnaryOperator {
public:
	///Identstring should be both parens, for Example "[]" or "()"
	UnaryParenOperator( int priority , QString identString, QString description, Operator::BindingSide binding ) : UnaryOperator( priority, identString, description, binding ) {
}
	
	virtual OperatorIdentification identify( QString& str ) {
		OperatorIdentification ret;
		if( str.startsWith( QString( identString()[0] ) ) ) {
			ret.start = 0;
			ret.end = findClose( str, 0 );
			if( ret.end == -1 ) {
				ret.found = false;
				ret.end = 0;
			} else {
				if( str[ret.end] == identString()[1] ) {
					ret.found = true;
					ret.end += 1;
					ret.op = this;
					
					///Try to extract the parameter-strings.
					ParamIterator it( identString(), str.mid( ret.start, ret.end - ret.start ) );
					
					while( it ) {
						ret.innerParams << (*it).stripWhiteSpace();
						
						++it;
					}
					
				} else {
					ret.end = 0;
				}
			}
		}
		return ret;
	}
};

class IndexOperator : public UnaryParenOperator {
public:
IndexOperator() : UnaryParenOperator( 17, "[]", "index-operator", Operator::Left ) {
	}
	
	virtual EvaluationResult unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& innerParams ) {
		return param->applyOperator( SimpleTypeImpl::IndexOp, convertList<SimpleType>( innerParams ) );
	}
};

RegisterOperator< IndexOperator > IndexReg( AllOperators );  ///This registers the operator to the list of all operators


class ParenOperator : public UnaryParenOperator {
public:
ParenOperator() : UnaryParenOperator( 16, "()", "paren-operator", Operator::Left ) {
}
	
	virtual bool checkParams( const QValueList<EvaluationResult>& params ) {
		return !params.isEmpty();
	}
	
	virtual EvaluationResult unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& innerParams ) {
		if( param ) {
			return param->applyOperator( SimpleTypeImpl::ParenOp, convertList<SimpleType>(innerParams) );
		} else {
			return innerParams[0];
		}
	}
};

RegisterOperator< ParenOperator > ParenReg( AllOperators );  ///This registers the operator to the list of all operators



class NeutralParenOperator : public UnaryParenOperator {
public:
NeutralParenOperator() : UnaryParenOperator( 16, "()", "neutral-paren-operator", Operator::Neutral ) {
}
	
	virtual EvaluationResult unaryApply( EvaluationResult param, const QValueList<EvaluationResult>& innerParams ) {
		if( innerParams.count() != 1 ) {
			log( QString( "wrong count of inner parameters" ).arg( innerParams.count() ) );
			return SimpleType();
		} else {
			return innerParams[0];
		}
	}
};

RegisterOperator< NeutralParenOperator > NeutralParenReg( AllOperators );  ///This registers the operator to the list of all operators
}



QString CppEvaluation::nameFromType( SimpleType t ) {
	return t->fullTypeResolved();
}



class SimpleVariable
{
public:
	SimpleVariable()
	{
		startLine = endLine = startCol = endCol = 0;
	}
	
	SimpleVariable( const SimpleVariable& source )
			: name( source.name ), 
			comment(source.comment),
			startLine(source.startLine),
			startCol(source.startCol),
			endLine(source.endLine),
			endCol(source.endCol),
			type( source.type ),
			ptrList( source.ptrList )
	{}
	~SimpleVariable()
	{}

	SimpleVariable& operator = ( SimpleVariable& source )
	{
		name = source.name;
		type = source.type;
		ptrList = source.ptrList;
		comment = source.comment;
		startLine = source.startLine;
		startCol = source.startCol;
		endLine = source.endLine;
		endCol = source.endCol;
		return *this;
	}

	QString name;
	QString comment;
	int startLine, startCol;
	int endLine, endCol;
	TypeDesc type;
	QStringList ptrList;
	
	DeclarationInfo toDeclarationInfo( QString activeFileName ) {
		DeclarationInfo decl;
		decl.name = name;
		decl.file = activeFileName;
		decl.comment = comment;
		decl.startLine = startLine;decl.startCol = startCol;
		decl.endLine = endLine; decl.endCol = endCol;
		return decl;
	}
};



SimpleType getGlobal( SimpleType t ) {
	SimpleType global = t;
	SafetyCounter s( 50 );
	while( !global.scope().isEmpty() && s ) {
		if( !s ) { kdDebug( 9007 ) << "error" << endl; break; }
		global = global->parent();
	}
	
	return global;
}


class SimpleContext
{
public:
	SimpleContext( SimpleType container = SimpleType(), SimpleContext* prev = 0 )
		: m_prev( prev ), m_container( container )
	{
		(*m_container); ///Make the type physically create itself
	}

	virtual ~SimpleContext()
	{
		if ( m_prev )
		{
			delete( m_prev );
			m_prev = 0;
		}
	}

	SimpleContext* prev() const
	{
		return m_prev;
	}

	void attach( SimpleContext* ctx )
	{
		m_prev = ctx;
	}

	void detach()
	{
		m_prev = 0;
	}

	const QValueList<SimpleVariable>& vars() const
	{
		return m_vars;
	}

	void add( const SimpleVariable& v )
	{
		m_vars.append( v );
	}
	
	void add( const QValueList<SimpleVariable>& vars )
	{
		m_vars += vars;
	}
	
	void offset( int lineOffset, int colOffset ) {
		for( QValueList<SimpleVariable>::iterator it = m_vars.begin(); it != m_vars.end(); ++it ) {
			if( (*it).endLine != (*it).startLine || (*it).endCol != (*it).startCol) {
				if( (*it).startLine == 0 ) {
					(*it).startCol += colOffset;
				}
				if( (*it).endLine == 0 ) {
					(*it).endCol += colOffset;
				}
				(*it).startLine += lineOffset;
				(*it).endLine += lineOffset;
			}
		}
	}

	SimpleVariable findVariable( const QString& varname )
	{
		SimpleContext * ctx = this;
		while ( ctx )
		{
			const QValueList<SimpleVariable>& vars = ctx->vars();
			for ( int i = vars.count() - 1; i >= 0; --i )
			{
				SimpleVariable v = vars[ i ];
				if ( v.name == varname )
					return v;
			}
			ctx = ctx->prev();
		}
		return SimpleVariable();
	}
	
	SimpleType global() {
		return getGlobal( container() );
	}
	
	SimpleType& container() {
		return m_container;
	}
	
	void setContainer( SimpleType cnt ) {
		m_container = cnt;
		(*m_container); ///make the type physically create itself
	}

private:
	QValueList<SimpleVariable> m_vars;
	SimpleContext* m_prev;
	SimpleType m_container;
};

struct RecoveryPoint
{
	int kind;
	QStringList scope;
	QValueList<QStringList> imports;

	int startLine, startColumn;
	int endLine, endColumn;

	RecoveryPoint()
			: kind( 0 ), startLine( 0 ), startColumn( 0 ),
			endLine( 0 ), endColumn( 0 )
	{}
	
	///Registers the recovery-points imports into the given namespace
	void registerImports( SimpleType ns ) {
		SimpleTypeNamespace* n = dynamic_cast<SimpleTypeNamespace*>( &(*ns) );
		if( !n ) {
			kdDebug( 9007 ) << "the global namespace was not resolved correctly " << endl;
		} else {
			///put the imports into the global namespace
			for( QValueList<QStringList>::iterator it = imports.begin(); it != imports.end(); ++it ) {
				kdDebug( 9007 ) << "inserting import " << *it << " into the global scole" << endl;
				n->addAliasMap( "", (*it).join("::") );
			}
			n->addAliases( hardCodedAliases );
		}
	}

private:
	RecoveryPoint( const RecoveryPoint& source );
	void operator = ( const RecoveryPoint& source );
};

struct CppCodeCompletionData
{
	QPtrList<RecoveryPoint> recoveryPoints;
	QStringList classNameList;

	CppCodeCompletionData()
	{
		recoveryPoints.setAutoDelete( true );
	}

	RecoveryPoint* findRecoveryPoint( int line, int column )
	{
		if ( recoveryPoints.count() == 0 )
			return 0;

		QPair<int, int> pt = qMakePair( line, column );

		QPtrListIterator<RecoveryPoint> it( recoveryPoints );
		RecoveryPoint* recPt = 0;

		while ( it.current() )
		{
			QPair<int, int> startPt = qMakePair( it.current() ->startLine, it.current() ->startColumn );
			QPair<int, int> endPt = qMakePair( it.current() ->endLine, it.current() ->endColumn );

			if ( pt < startPt ) {
				break;
			}

			if( startPt < pt && pt < endPt  )
				recPt = it.current();

			++it;
		}

		return recPt;
	}

};

static QString toSimpleName( NameAST* name )
{
	if ( !name )
		return QString::null;

	QString s;
	QPtrList<ClassOrNamespaceNameAST> l = name->classOrNamespaceNameList();
	QPtrListIterator<ClassOrNamespaceNameAST> nameIt( l );
	while ( nameIt.current() )
	{
		if ( nameIt.current() ->name() )
		{
			s += nameIt.current() ->name() ->text() + "::";
		}
		++nameIt;
	}

	if ( name->unqualifiedName() && name->unqualifiedName() ->name() )
		s += name->unqualifiedName() ->name() ->text();

	return s;
}

bool operator < ( const CodeCompletionEntry& e1, const CodeCompletionEntry& e2 )
{
	return e1.text < e2.text;
}

template <class ItemType>
static QValueList<ItemType> unique( const QValueList<ItemType>& entryList )
{

	QValueList< ItemType > l;
	QMap<QString, bool> map;
	typename QValueList< ItemType >::ConstIterator it = entryList.begin();
	while ( it != entryList.end() )
	{
		CodeCompletionEntry e = *it++;
		QString key = ( e.type + " " +
		                e.prefix + " " +
		                e.text + " " +
		                e.postfix + " " ).simplifyWhiteSpace().stripWhiteSpace();
		if ( map.find( key ) == map.end() )
		{
			map[ key ] = TRUE;
			l << e;
		}
	}
	return l;
}

static QStringList unique( const QStringList& entryList )
{

	QStringList l;
	QMap<QString, bool> map;
	QStringList::ConstIterator it = entryList.begin();
	while ( it != entryList.end() )
	{
		QString e = *it++;
		if ( map.find( e ) == map.end() )
		{
			map[ e ] = TRUE;
			l << e;
		}
	}
	return l;
}

static QStringList unique( const QValueList<QStringList>& entryList )
{
	
	QStringList l;
	QMap<QString, bool> map;
	QValueList<QStringList>::ConstIterator it = entryList.begin();
	while ( it != entryList.end() )
	{
		QStringList li = (*it++);
		QString e = li.join( "\n" );
		if ( map.find( e ) == map.end() )
		{
			map[ e ] = TRUE;
			l += li;
		}
	}
	
	return l;
}


CppCodeCompletion::CppCodeCompletion( CppSupportPart* part )
		: d( new CppCodeCompletionData ),
		//Matches on includes
		m_includeRx( "^\\s*#\\s*include\\s+[\"<]" ),
		//Matches on C++ and C style comments as well as literal strings
		m_cppCodeCommentsRx("(//([^\n]*)(\n|$)|/\\*.*\\*/|\"([^\\\\]|\\\\.)*\")"),
		//Matches on alpha chars and '.'
		m_codeCompleteChRx("([A-Z])|([a-z])|(\\.)"),
		//Matches on "->" and "::"
		m_codeCompleteCh2Rx("(->)|(\\:\\:)")

{
	cppCompetionInstance = this;
	m_cppCodeCommentsRx.setMinimal( true );
		
	m_pSupport = part;

	m_activeCursor = 0;
	m_activeEditor = 0;
	m_activeCompletion = 0;
	m_activeHintInterface = 0;
	m_ccTimer = new QTimer( this );
	m_showStatusTextTimer = new QTimer( this );
	
	m_ccLine = 0;
	m_ccColumn = 0;
	connect( m_ccTimer, SIGNAL( timeout() ), this, SLOT( slotTimeout() ) );
	connect( m_showStatusTextTimer, SIGNAL( timeout() ), this, SLOT( slotStatusTextTimeout() ) );
	
	computeFileEntryList();

	CppSupportPart* cppSupport = m_pSupport;
	connect( cppSupport->project(), SIGNAL( addedFilesToProject( const QStringList& ) ), 
	         this, SLOT( computeFileEntryList() ) );
	connect( cppSupport->project(), SIGNAL( removedFilesFromProject( const QStringList& ) ), 
	         this, SLOT( computeFileEntryList() ) );

	m_bArgHintShow = false;
	m_bCompletionBoxShow = false;
	m_blockForKeyword = false;
	m_demandCompletion = false;
	m_completionMode = NormalCompletion;

	m_repository = new CodeInformationRepository( cppSupport->codeRepository() );
	setupCodeInformationRepository();

	if ( part->partController() ->parts() )
	{
		QPtrListIterator<KParts::Part> it( *part->partController() ->parts() );
		while ( KParts::Part * part = it.current() )
		{
			integratePart( part );
			++it;
		}
	}

	if ( part->partController() ->activePart() )
		slotActivePartChanged( part->partController() ->activePart() );

	connect( part->partController( ), SIGNAL( partAdded( KParts::Part* ) ),
	         this, SLOT( slotPartAdded( KParts::Part* ) ) );
	connect( part->partController( ), SIGNAL( activePartChanged( KParts::Part* ) ),
	         this, SLOT( slotActivePartChanged( KParts::Part* ) ) );

	connect( part, SIGNAL( fileParsed( const QString& ) ), 
	         this, SLOT( slotFileParsed( const QString& ) ) );
}

CppCodeCompletion::~CppCodeCompletion( )
{
	delete m_repository;
	delete d;
}

void CppCodeCompletion::addStatusText( QString text, int timeout ) {
	m_statusTextList.append( QPair<int, QString>( timeout, text ) );
	if( !m_showStatusTextTimer->isActive() ) {
		slotStatusTextTimeout();
	}
}

void CppCodeCompletion::clearStatusText() {
	m_statusTextList.clear();
	m_showStatusTextTimer->stop();
}

void CppCodeCompletion::slotStatusTextTimeout() {
	if( m_statusTextList.isEmpty() || !m_pSupport ) return;
	m_pSupport->mainWindow() ->statusBar() ->message( m_statusTextList.front().second, m_statusTextList.front().first );
	m_showStatusTextTimer->start( m_statusTextList.front().first , true );
	m_statusTextList.pop_front();
}

void CppCodeCompletion::slotTimeout()
{
	if ( !m_activeCursor || !m_activeEditor || !m_activeCompletion )
		return ;

	uint nLine, nCol;
	m_activeCursor->cursorPositionReal( &nLine, &nCol );

	if ( nLine != m_ccLine || nCol != m_ccColumn )
		return ;

	QString textLine = m_activeEditor->textLine( nLine );
	QChar ch = textLine[ nCol ];
	if ( ch.isLetterOrNumber() || ch == '_' )
		return ;

	completeText();
}

void CppCodeCompletion::slotArgHintHidden()
{
	//kdDebug(9007) << "CppCodeCompletion::slotArgHintHidden()" << endl;
	m_bArgHintShow = false;
}

void CppCodeCompletion::slotCompletionBoxHidden()
{
	//kdDebug( 9007 ) << "CppCodeCompletion::slotCompletionBoxHidden()" << endl;
	m_bCompletionBoxShow = false;
}


void CppCodeCompletion::integratePart( KParts::Part* part )
{
	if ( !part || !part->widget() )
		return ;

	KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>( part );
	if ( doc )
	{
		kdDebug( 9007 ) << k_funcinfo << "integrate document: " << doc << endl;

		if ( m_pSupport && m_pSupport->codeCompletionConfig() ->automaticCodeCompletion() )
		{
			kdDebug( 9007 ) << k_funcinfo << "enabling code completion" << endl;
			connect( part, SIGNAL( textChanged() ), this, SLOT( slotTextChanged() ) );
			connect( part->widget(), SIGNAL( completionDone() ), this,
			         SLOT( slotCompletionBoxHidden() ) );
			connect( part->widget(), SIGNAL( completionAborted() ), this,
			         SLOT( slotCompletionBoxHidden() ) );
			connect( part->widget(), SIGNAL( argHintHidden() ), this,
			         SLOT( slotArgHintHidden() ) );
		}
	}
}

void CppCodeCompletion::slotPartAdded( KParts::Part *part )
{
	integratePart( part );
}

void CppCodeCompletion::slotActivePartChanged( KParts::Part *part )
{
	if( m_activeHintInterface ) {
		KParts::Part* oldPart = dynamic_cast<KParts::Part*>( m_activeHintInterface );
		if( oldPart && oldPart->widget() )
			disconnect(oldPart->widget() , SIGNAL( needTextHint(int, int, QString &) ), this, SLOT( slotTextHint(int, int, QString&) ) );
		
		m_activeHintInterface = 0;
	}
	if ( !part )
		return ;

	kdDebug( 9007 ) << k_funcinfo << endl;

	m_activeFileName = QString::null;

	KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>( part );
	if ( !doc )
		return ;

	m_activeFileName = doc->url().path();

	// if the interface stuff fails we should disable codecompletion automatically
	m_activeEditor = dynamic_cast<KTextEditor::EditInterface*>( part );
	if ( !m_activeEditor )
	{
		kdDebug( 9007 ) << "Editor doesn't support the EditDocumentIface" << endl;
		return ;
	}

	m_activeCursor = dynamic_cast<KTextEditor::ViewCursorInterface*>( part->widget() );
	if ( !m_activeCursor )
	{
		kdDebug( 9007 ) << "The editor doesn't support the CursorDocumentIface!" << endl;
		return ;
	}

	m_activeCompletion = dynamic_cast<KTextEditor::CodeCompletionInterface*>( part->widget() );
	if ( !m_activeCompletion )
	{
		kdDebug( 9007 ) << "Editor doesn't support the CompletionIface" << endl;
		return ;
	}

	m_activeHintInterface = dynamic_cast<KTextEditor::TextHintInterface*>( part->widget() );
	
	char* q = 0;
	kdDebug() << q << endl;
	
	if( m_activeHintInterface ) 
	{
#ifndef DISABLETOOLTIPS
		m_activeHintInterface->enableTextHints( 500 );
		connect( part->widget(), SIGNAL( needTextHint(int, int, QString &) ), this, SLOT( slotTextHint(int, int, QString&) ) );
#endif
	} else {
		kdDebug( 9007 ) << "editor has no text-hint-interface" << endl;
	}
	
	kdDebug( 9007 ) << k_funcinfo << "-- end" << endl;
}

void CppCodeCompletion::slotTextChanged()
{
	m_ccTimer->stop();

	if ( !m_activeCursor )
		return ;

	unsigned int nLine, nCol;
	m_activeCursor->cursorPositionReal( &nLine, &nCol );

	QString strCurLine = m_activeEditor->textLine( nLine );
	QString ch = strCurLine.mid( nCol - 1, 1 );
	QString ch2 = strCurLine.mid( nCol - 2, 2 );
	
	// Tell the completion box to _go_away_ when the completion char
	// becomes empty or whitespace and the box is already showing.
	// !!WARNING!! This is very hackish, but KTE doesn't offer a way
	// to tell the completion box to _go_away_
	if ( ch.simplifyWhiteSpace().isEmpty() && 
	     !strCurLine.simplifyWhiteSpace().contains("virtual") &&
	     m_bCompletionBoxShow )
	{
		QValueList<KTextEditor::CompletionEntry> entryList;
		m_bCompletionBoxShow = true;
		m_activeCompletion->showCompletionBox( entryList, 0 );
	}
	
	m_ccLine = 0;
	m_ccColumn = 0;
	
	bool argsHint = m_pSupport->codeCompletionConfig() ->automaticArgumentsHint();
	bool codeComplete = m_pSupport->codeCompletionConfig() ->automaticCodeCompletion();
	bool headComplete = m_pSupport->codeCompletionConfig() ->automaticHeaderCompletion();
	
	// m_codeCompleteChRx completes on alpha chars and '.'
	// m_codeCompleteCh2Rx completes on "->" and "::"

	if ( ( argsHint && ch == "(" ) ||
	     ( strCurLine.simplifyWhiteSpace().contains("virtual") ) ||
	     ( codeComplete && ( m_codeCompleteChRx.search( ch ) != -1 || 
	                         m_codeCompleteCh2Rx.search( ch2 ) != -1 ) ) ||
	     ( headComplete && ( ch == "\"" || ch == "<" ) && m_includeRx.search( strCurLine ) != -1 ) )
	{
		int time;
		m_ccLine = nLine;
		m_ccColumn = nCol;
		if ( ch == "(" )
			time = m_pSupport->codeCompletionConfig() ->argumentsHintDelay();
		else
			time = m_pSupport->codeCompletionConfig() ->codeCompletionDelay();
		m_ccTimer->start( time, true );
	}
}

enum { T_ACCESS, T_PAREN, T_BRACKET, T_IDE, T_UNKNOWN, T_TEMP };


QString CppCodeCompletion::replaceCppComments( const QString& contents ) {
	QString text = contents;
	
	int pos = 0;
	while ( (pos = m_cppCodeCommentsRx.search( text, pos )) != -1 ) 
	{
		if ( m_cppCodeCommentsRx.cap( 1 ).startsWith( "//" ) ) 
		{
			QString before = m_cppCodeCommentsRx.cap( 1 );
			QString after;
			after.fill(' ', before.length() - 5 );
			after.prepend( "/*" );
			after.append( "*/" );
			text.replace( pos, before.length() - 1, after );
			pos += after.length();
		}
		else
		{
			pos += m_cppCodeCommentsRx.matchedLength();
		}
	}
	return text;
}

int CppCodeCompletion::expressionAt( const QString& contents, int index )
{
	kdDebug( 9007 ) << k_funcinfo << endl;

	/* C++ style comments present issues with finding the expr so I'm
		matching for them and replacing them with empty C style comments
		of the same length for purposes of finding the expr. */

	QString text = clearComments( contents );
	
	

	int last = T_UNKNOWN;
	int start = index;
	while ( index > 0 )
	{
		while ( index > 0 && text[ index ].isSpace() )
		{
			--index;
		}

		QChar ch = text[ index ];
		QString ch2 = text.mid( index - 1, 2 );
		if ( ( last != T_IDE ) && ( ch.isLetterOrNumber() || ch == '_' ) )
		{
			while ( index > 0 && ( text[ index ].isLetterOrNumber() || text[ index ] == '_' ) )
			{
				--index;
			}
			last = T_IDE;
		}
		else if ( last != T_IDE && ch == ')' )
		{
			int count = 0;
			while ( index > 0 )
			{
				QChar ch = text[ index ];
				if ( ch == '(' )
				{
					++count;
				}
				else if ( ch == ')' )
				{
					--count;
				}
				else if ( count == 0 )
				{
					index;
					last = T_PAREN;
					break;
				}
				--index;
			}
		}
		else if ( last != T_IDE && ch == '>' && ch2 != "->" )
		{
			int count = 0;
			while ( index > 0 )
			{
				QChar ch = text[ index ];
				if ( ch == '<' )
				{
					++count;
				}
				else if ( ch == '>' )
				{
					--count;
				}
				else if ( count == 0 )
				{
					//--index;
					last = T_TEMP;
					break;
				}
				--index;
			}
		}		
		else if ( ch == ']' )
		{
			int count = 0;
			while ( index > 0 )
			{
				QChar ch = text[ index ];
				if ( ch == '[' )
				{
					++count;
				}
				else if ( ch == ']' )
				{
					--count;
				}
				else if ( count == 0 )
				{
					//--index;
					last = T_BRACKET;
					break;
				}
				--index;
			}
		}
		else if ( ch == '.' )
		{
			--index;
			last = T_ACCESS;
		}
		else if ( ch2 == "::" )
		{
			index -= 2;
			last = T_ACCESS;
		}
		else if ( ch2 == "->" )
		{
			index -= 2;
			last = T_ACCESS;
		}
		else
		{
			if ( start > index )
			{
				++index;
			}
			last = T_UNKNOWN;
			break;
		}
	}
	return index;
}

QStringList CppCodeCompletion::splitExpression( const QString& text )
{
#define ADD_CURRENT()\
 if( current.length() ) { l << current; /*kdDebug(9007) << "add word " << current << endl;*/ current = ""; }

	QStringList l;
	uint index = 0;
	QString current;
	while ( index < text.length() )
	{
		QChar ch = text[ index ];
		QString ch2 = text.mid( index, 2 );

		if ( ch == '.' )
		{
			current += ch;
			ADD_CURRENT();
			++index;
		}
		else if ( ch == '(' )
		{
			int count = 0;
			while ( index < text.length() )
			{
				QChar ch = text[ index ];
				if ( ch == '(' )
				{
					++count;
				}
				else if ( ch == ')' )
				{
					--count;
				}
				else if ( count == 0 )
				{
					break;
				}
				current += ch;
				++index;
			}
		}
		else if ( ch == '[' )
		{
			int count = 0;
			while ( index < text.length() )
			{
				QChar ch = text[ index ];
				if ( ch == '[' )
				{
					++count;
				}
				else if ( ch == ']' )
				{
					--count;
				}
				else if ( count == 0 )
				{
					break;
				}
				current += ch;
				++index;
			}
		}
		else if ( ch2 == "->" )
		{
			current += ch2;
			ADD_CURRENT();
			index += 2;
		}
		else
		{
			current += text[ index ];
			++index;
		}
	}
	ADD_CURRENT();
	return l;
}




typedef CppCodeCompletion::ExpressionInfo ExpressionInfo;

namespace CppEvaluation {

class ExpressionEvaluation {
private:
	CppCodeCompletion* m_data;
	SimpleContext* m_ctx;
	ExpressionInfo m_expr;
	bool m_global;
	OperatorSet& m_operators;
	
public:
	ExpressionEvaluation( CppCodeCompletion* data, ExpressionInfo expr, OperatorSet& operators, SimpleContext* ctx = 0 ) : m_data( data ), m_ctx( ctx ), m_expr( expr ), m_global(false), m_operators( operators ) {
		safetyCounter.init();
		
	kdDebug( 9007 ) << "Initializing evaluation of expression " << expr << endl;
	
		if ( expr.expr().startsWith( "::" ) )
		{
			expr.setExpr( expr.expr().mid( 2 ) );
			m_global = true;
		}
		
		m_expr = m_data->splitExpression( expr.expr() ).join("");
	}
	
	CppCodeCompletion::EvaluationResult evaluate() {
		CppCodeCompletion::EvaluationResult res;
		res = evaluateExpressionInternal( m_expr.expr(), m_ctx->global(), m_ctx, m_ctx, m_expr.canBeTypeExpression() );
		res.expr = m_expr;
		
		return res;
	}
	
private:
	/**
	recursion-method:
	1. Find the rightmost operator with the lowest priority, split the expression
	
	vector[ (*it)->position ]().
	*/
	typedef CppCodeCompletion::EvaluationResult EvaluationResult;
	
	
	
	virtual CppCodeCompletion::EvaluationResult evaluateExpressionInternal( QString expr, SimpleType scope, SimpleContext * ctx, SimpleContext* innerCtx , bool canBeTypeExpression = false ) {
		Debug d( "#evl#" );
		if( !safetyCounter ) return SimpleType();
		if( expr.isEmpty() ) {
			return scope;
		}
	
		dbg() << "evaluateExpressionInternal(\"" << expr << "\") scope: \"" << scope->str() << "\" context: " << ctx << endl;
		
			
		expr = expr.stripWhiteSpace();
		
		///Find the rightmost operator with the lowest priority, for the first split.
		QValueList<OperatorIdentification> idents;
		for( uint a = 0; a < expr.length(); ++a ) {
			QString part = expr.mid( a );
			OperatorIdentification ident = m_operators.identifyOperator( part );
			if( ident ) {
				dbg() << "identified \"" << ident.op->name() << "\" in string " << part << endl;
				ident.start += a;
				ident.end += a;
				idents << ident;
				a += ident.end;
			} else {
				if( isLeftParen( part[0] ) ) {
					int jump = findClose( part, 0 );
					if( jump != -1 )
						a += jump;
				}
			}
		}
		
		if( !idents.isEmpty() ) {
			OperatorIdentification lowest;
			
			for( QValueList<OperatorIdentification>::iterator it = idents.begin(); it != idents.end(); ++it ) {
				if( lowest ) { 
					if( lowest.op->priority() >= (*it).op->priority() )
						lowest = *it;
				} else {
					lowest = *it;
				}
			}
			
			if( lowest ) {
				QString leftSide = expr.left( lowest.start ).stripWhiteSpace();
				QString rightSide = expr.right( expr.length() - lowest.end ).stripWhiteSpace();
				
				EvaluationResult left, right;
				if( !leftSide.isEmpty() ) {
					left = evaluateExpressionInternal( leftSide, scope, ctx, innerCtx );
				} else {
					left = scope;
				}
				
				if( !left && (lowest.op->binding() & Operator::Left) ) {
					dbg() << "problem while evaluating expression \"" << expr << "\", the operator \"" << lowest.op->name() << "\" has a binding to the left side, but no left side could be evaluated: \"" << leftSide << "\"" << endl;
				}
				
				if( !rightSide.isEmpty() && (lowest.op->binding() & Operator::Right) )
					right = evaluateExpressionInternal( rightSide, SimpleType(), ctx, innerCtx );
				
				if( !right && (lowest.op->binding() & Operator::Right) ) {
					dbg() << "problem while evaluating expression \"" << expr << "\", the operator \"" << lowest.op->name() << "\" has a binding to the right side, but no right side could be evaluated: \"" << rightSide << "\"" << endl;
				}
				
				QValueList<EvaluationResult> innerParams;
				QValueList<EvaluationResult> params;
				if( lowest.op->binding() & Operator::Left ) params << left;
				if( lowest.op->binding() & Operator::Right ) params << right;
				
				for( QValueList<QString>::iterator it = lowest.innerParams.begin(); it != lowest.innerParams.end(); ++it ) {
					dbg() << "evaluating inner parameter \"" << *it << "\"" << endl;
					innerParams << evaluateExpressionInternal( (*it), SimpleType(), innerCtx, innerCtx );
				}
				
				EvaluationResult applied = lowest.op->apply( params, innerParams );
				if( !applied ) {
					dbg() << "\"" << expr << "\": failed to apply the operator \"" << lowest.op->name() << "\"" << endl;
				}
				
				if( ! (lowest.op->binding() & Operator::Left) &&  !leftSide.isEmpty() ) {
					///When the operator has no binding to the left, the left side should be empty.
					dbg() << "\"" << expr << "\": problem with the operator \"" << lowest.op->name() << ", it has no binding to the left side, but the left side is \""<< leftSide << "\"" << endl;
				}
				
				if( ! (lowest.op->binding() & Operator::Right) && !rightSide.isEmpty() ) {
					///When the operator has no binding to the right, we should continue evaluating the right side, using the left type as scope.
					return evaluateExpressionInternal( rightSide, applied, 0, innerCtx );
				}
				
				return applied;
			} else {
				dbg() << " could not find an operator in " << expr << endl;
				QStringList lst; lst << expr;
				return evaluateAtomicExpression( expr, scope, ctx );
			}
		}
		
		//dbg() << " could not evaluate " << expr << endl;
		dbg() << "evaluating \"" << expr << "\" using the old evaluation-method" << endl;
		QStringList lst = m_data->splitExpression( expr );
		EvaluationResult res = evaluateAtomicExpression( lst, scope, ctx, canBeTypeExpression );
		return res;
	}
	
	///This does the simplest work
	EvaluationResult evaluateAtomicExpression( QStringList exprList, SimpleTypeImpl::TypeOfResult scope, SimpleContext * ctx  = 0, bool canBeTypeExpression = false ) {
		Debug d( "#evt#");
		if( !safetyCounter || !d ) return SimpleType();
		
		dbg() << "evaluateAtomicExpression(\"" << exprList.join(" ") << "\") scope: \"" << scope->str() << "\" context: " << ctx << endl;
		
		if( exprList.isEmpty() )
			return EvaluationResult( scope.type, scope.decl );
		
		QString currentExpr = exprList.front().stripWhiteSpace();
		exprList.pop_front();
		
		CppCodeCompletion::MemberAccessOp accessOp = CppCodeCompletion::NoOp;
		
		SimpleTypeImpl::TypeOfResult searchIn = scope;
		if( ctx ) searchIn = ctx->container();
		
		QStringList split = splitType( currentExpr );
		
		if ( !split.isEmpty() && (currentExpr.endsWith( "::" ) || split.size() > 1 || canBeTypeExpression ) )
		{
			currentExpr = split.front();
			
			SimpleTypeImpl::TypeOfResult type = searchIn->locateType( currentExpr );
			if ( type )
			{
				if( !split.isEmpty()) split.pop_front();
				return evaluateAtomicExpression( split + exprList, type, 0, true );
			} else {
				dbg() << "\"" << scope->scope() << "\"could not locate " << currentExpr << endl;
			}
		}
		
		if ( ctx )
		{
			// find the variable type in the current context
			SimpleVariable var = ctx->findVariable( currentExpr );
			
			if ( var.type ) {
				SimpleTypeImpl::TypeOfResult t = ctx->container()->locateType( var.type );
				t.decl = var.toDeclarationInfo( "current_file" );
				EvaluationResult res = evaluateAtomicExpression(  exprList, t );
				return res;
			}
			
			SimpleType current = ctx->container();
			
			SimpleTypeImpl::TypeOfResult type;
			
			SafetyCounter s( 20 );
			bool ready = false;
			while( !ready && s )
			{
				if( !current ) ready = true;
				
				type = current->typeOf( currentExpr );
				if ( type || type->desc() )
					return evaluateAtomicExpression( exprList, type );
				
				if( !ready ) current = current->parent();
			}
			
			if( !canBeTypeExpression && exprList.isEmpty() && !scope ) {
				exprList << currentExpr;
				///Try as a type again
				return evaluateAtomicExpression( exprList, type, ctx, true );
			} else {
				return EvaluationResult();
			}
		}
		
		SimpleTypeImpl::TypeOfResult type = scope->typeOf( currentExpr );
		return evaluateAtomicExpression( exprList, type );
	}
};
}

                   

bool CppCodeCompletion::correctAccessOpAccurate( QStringList ptrList, MemberAccessOp accessOp )
{
	//Remove the vars that don't correspond to the member access operator
	//that we are using.  
	
	///@todo: Take into account the de-reference operator...
	bool arrowOp = accessOp == ArrowOp && ptrList.count() && ptrList[0] == "*";
	bool dotOp =  accessOp == DotOp && ( !ptrList.count() || ptrList[0] == "&" );
	return arrowOp || dotOp;
}


bool CppCodeCompletion::correctAccessOp( QStringList ptrList, MemberAccessOp accessOp )
{
	if ( m_demandCompletion || accessOp == NoOp )
		return true;
	
	return correctAccessOpAccurate( ptrList, accessOp );
}


QStringList formatComment( const QString& comment, int maxCols = 120 ) {
	QStringList ret;
	SafetyCounter s( 14 );	///maximum of 14 lines
	
	QStringList lines = QStringList::split( "\n", comment );
	for( QStringList::iterator it = lines.begin(); it != lines.end(); ++it ) {
		QStringList words = QStringList::split( " ", *it );
		while( !words.isEmpty() && s ) {
			int cnt = words.count();
			QString line = "? ";
			int len = 0;
			while( !words.isEmpty() && len < maxCols ) {
				len += words.front().length();
				line += words.front() + " ";
				words.pop_front();
			}
			ret << line;
		}
	}
	if( !s ) ret << "? comment has too many lines";
	
	return ret;
}

bool isValidIdentifierSign( const QChar& c ) {
	if( c.isLetter() || c.isDigit() || c == '_' ) return true;
	else return false;
}


///Before calling this, a ConfigureSimpleTypes-object should be created, so that the ressources will be freed when that object is destroyed
EvaluationResult CppCodeCompletion::evaluateExpressionAt( int line, int column , ConfigureSimpleTypes& conf, bool ifUnknownSetType ) {
	kdDebug( 9007 ) << "CppCodeCompletion::evaluateExpressionAt( " << line << ", " << column << " )" << endl;
	
	if ( !m_pSupport || !m_activeEditor )
		return EvaluationResult();
	
	{
		QString curLine = m_activeEditor->textLine( line );
		
	///move column to the last letter of the pointed word
		while( column+1 < (int)curLine.length() && isValidIdentifierSign( curLine[column] ) && isValidIdentifierSign( curLine[column+1] ) ) {
			column++;
		}
		
	//if( column > 0 ) column--;
		
		if( column >= (int)curLine.length() || curLine[ column ].isSpace() ) return EvaluationResult();
			
		QString expr = curLine.left( column +1 );
		kdDebug( 9007 ) << "evaluating \"" << expr.stripWhiteSpace() << "\"" << endl;
		
		if( curLine[column] == '-' || curLine[column] == ';' ) --column;
		
	EvaluationResult type = evaluateExpressionType( line, column + 1, conf, ifUnknownSetType ? addFlag( DefaultEvaluationOptions, DefaultAsTypeExpression ) : DefaultEvaluationOptions );
		
		kdDebug( 9007 ) << "type: " << type.resultType->fullTypeResolved() << endl;
		
		return type;
	}
}

void CppCodeCompletion::popupAction( int number ) {
	PopupActions::iterator it = m_popupActions.find( number );
	if( it != m_popupActions.end() ) {
		KURL url;
		if ( (*it).file != "current_file" )
			url.setPath( (*it).file );
		else
			url.setPath( m_activeFileName );
		m_pSupport->partController() ->editDocument( url, (*it).startLine );
	} else {
		kdDebug( 9007 ) << "error" << endl;
	}
}

void CppCodeCompletion::selectItem( ItemDom item )
{
	Extensions::KDevCodeBrowserFrontend* f = m_pSupport->extension< Extensions::KDevCodeBrowserFrontend > ( "KDevelop/CodeBrowserFrontend" );
	
	if(f != 0) {
		ItemDom itemDom( &(*item) );
		f->jumpedToItem( itemDom );
	} else {
		kdDebug() << "could not find the proper extension\n";
	}
}

void CppCodeCompletion::popupClassViewAction( int number ) {
	PopupClassViewActions::iterator it = m_popupClassViewActions.find( number );
	if( it != m_popupClassViewActions.end() ) {
		if( (*it ) ) selectItem( *it );
	} else {
		kdDebug( 9007 ) << "error" << endl;
	}
}

QString cleanForMenu( QString txt ) {
	//	return txt.replace( "&", "§" );
	return txt.replace( "&", "$" );
}


struct PopupFillerHelpStruct {
	CppCodeCompletion* receiver;
	PopupFillerHelpStruct( CppCodeCompletion* rec ) {
		receiver = rec;
	}
	
	void insertItem ( QPopupMenu* parent, TypeDesc d , QString prefix ) {
		QString txt;
		
		if( d.resolved() && d.resolved()->isNamespace() ) return;
		
		if( d.resolved() && d.resolved()->hasNode() ) {
			QString n = d.name();
			if( d.resolved()->asFunction() )
				n = receiver->buildSignature( SimpleType( &(*d.resolved()) ) );
				                    
			txt = prefix + i18n("jump to %1").arg( cleanForMenu( n ) );
		} else {
			txt = prefix + d.name() + i18n(" is unresolved");
		}
		
		int id = parent->insertItem( txt, receiver, SLOT( popupAction( int ) ) );
		
		if( d.resolved() ) receiver->m_popupActions.insert( id, d.resolved()->getDeclarationInfo() );
	}
};


struct PopupClassViewFillerHelpStruct {
	CppCodeCompletion* receiver;
	PopupClassViewFillerHelpStruct( CppCodeCompletion* rec ) {
		receiver = rec;
	}
	
	void insertItem ( QPopupMenu* parent, TypeDesc d , QString prefix ) {
		QString txt;
		
		ItemDom dom;
		
		if( d.resolved() ) {
			SimpleTypeCodeModel* cm = dynamic_cast<SimpleTypeCodeModel*>( d.resolved().data() );
			if( cm ) dom = cm->item();
		}
		
		
		if( d.resolved() && d.resolved()->hasNode() ) {
			if( !dom && d.resolved()->isNamespace() ) {
				SimpleTypeUsedNamespace* ns = dynamic_cast<SimpleTypeUsedNamespace*>( d.resolved().data() );
				if( ns ) {
					QValueList<SimpleType> slaves = ns->getSlaves();
					for( QValueList<SimpleType>::iterator it = slaves.begin(); it != slaves.end(); ++it ) {
						SimpleTypeCodeModel* cm = dynamic_cast<SimpleTypeCodeModel*>( (*it).get().data() );
						if( cm ) dom = cm->item();
					}
				}
			}
				
			
			if( dom ) {
				QString n = d.name();
				if( d.resolved()->asFunction() ) {
					n = receiver->buildSignature( SimpleType( &(*d.resolved()) ) );
				}
				txt = prefix + i18n("show %1").arg( cleanForMenu( n ) );
			} else {
				txt = prefix + d.name() + " not in code-model";
			}
		} else {
			txt = prefix + d.name() + i18n(" is unresolved");
		}
		
		int id = parent->insertItem( txt, receiver, SLOT( popupClassViewAction( int ) ) );
		
		if( dom ) receiver->m_popupClassViewActions.insert( id, dom );
	}
};


template <class HelpStruct>
class PopupFiller {
	HelpStruct struk;
	QString depthAdd;
	SafetyCounter s;
public:
	PopupFiller( HelpStruct s , QString dAdd, int maxCount = 100 ) : struk( s ), depthAdd( dAdd), s(maxCount) {
	}
	
	void fill( QPopupMenu* parent, TypeDesc d, QString prefix = "" ) {
		if( !s ) {
			kdDebug( 9007 ) << "safety-counter reached while filling the menu-structure, stopping" << endl;
			return;
		}
		struk.insertItem( parent, d, prefix );
		
		TypeDesc::TemplateParams p = d.templateParams();
		for( TypeDesc::TemplateParams::iterator it = p.begin(); it != p.end(); ++it ){
			if( (*it)->resolved() ) {
				QPopupMenu * m = new QPopupMenu( parent );
				int gid = parent->insertItem( i18n( "template-param \"%1\"" ).arg( cleanForMenu( (*it)->resolved()->fullTypeResolved() ) ), m );
				fill( m, **it );
			} else {
				fill( parent, **it, prefix + depthAdd );
			}
		}
		
		if( d.resolved() ) {
			if( d.resolved()->asFunction() ) {
				SimpleType rt = d.resolved()->locateType( d.resolved()->asFunction()->getReturnType() );
				if( rt || rt->desc() ) {
					QPopupMenu * m = new QPopupMenu( parent );
					int gid = parent->insertItem( i18n( "return-type \"%1\"" ).arg( cleanForMenu( rt->fullTypeResolved() ) ), m );
					fill( m, rt->desc() );
				}
				
				QValueList<TypeDesc> args = d.resolved()->asFunction()->getArgumentTypes();
				QStringList argNames = d.resolved()->asFunction()->getArgumentNames();
				if( !args.isEmpty() ) {
					QPopupMenu * m = new QPopupMenu( parent );
					int gid = parent->insertItem( i18n( "arguments-types" ), m );
					QStringList::iterator it2 = argNames.begin();
					for( QValueList<TypeDesc>::iterator it = args.begin(); it != args.end(); ++it ) 
					{
						SimpleType at = d.resolved()->locateType( *it );
						QString name ="";
						if( it2 != argNames.end() ) {
							name = *it2;
							++it2;
						}
						QPopupMenu * mo = new QPopupMenu( m );
						int gid = m->insertItem( i18n( "argument \"%1\"" ).arg( cleanForMenu( at->fullTypeResolved() + " " + name ) ), mo );
						fill( mo, at->desc() );
						
					}
				}
				
			}
			QValueList<SimpleType> bases = d.resolved()->getBases();
			for( QValueList<SimpleType>::iterator it = bases.begin(); it != bases.end(); ++it ) {
				QPopupMenu * m = new QPopupMenu( parent );
				int gid = parent->insertItem( i18n( "base-class \"%1\"" ).arg( cleanForMenu( (*it)->fullTypeResolved() ) ), m );
				fill( m, (*it)->desc() );
			}
			
			if( d.resolved()->parent() && d.resolved()->parent()->desc() ) {
				QPopupMenu * m = new QPopupMenu( parent );
				int gid = parent->insertItem( i18n( "nested in \"%1\"" ).arg( cleanForMenu( d.resolved()->parent()->fullTypeResolved() ) ), m );
				fill( m, d.resolved()->parent()->desc() );
			}
		}
	}
};

void CppCodeCompletion::contextEvaluationMenus ( QPopupMenu *popup, const Context *context, int line, int column ) {
	kdDebug( 9007 ) << "CppCodeCompletion::contextEvaluationMenu()" << endl;
	m_popupActions.clear();
	m_popupClassViewActions.clear();
	
	if ( !m_pSupport || !m_activeEditor )
		return ;
	
	ConfigureSimpleTypes conf( m_activeFileName );
	
	EvaluationResult type = evaluateExpressionAt( line, column, conf );

	if( !type && !type.sourceVariable ) return;
	
	QString name = type.resultType->fullTypeResolved();
	if( type.sourceVariable )
		name += " " + type.sourceVariable.name;
	if( type.resultType->asFunction() )
		name = buildSignature(type.resultType );
	
	///Fill the jump-menu
	{
		PopupFillerHelpStruct h(this);
		PopupFiller<PopupFillerHelpStruct> filler( h, "<" );
		
		QPopupMenu * m = new QPopupMenu( popup );
		int gid = popup->insertItem( i18n( "Navigate by \"%1\"" ).arg( cleanForMenu( name ) ), m );
		popup->setWhatsThis( gid, i18n( "<b>Navigation</b><p>Provides a menu to navigate to positions of items that are involved in this expression" ) );
		
		if( type.sourceVariable && type.sourceVariable.name != "this" ) {
			int id = m->insertItem( i18n("jump to variable-declaration \"%1\"").arg( type.sourceVariable.name ) , this, SLOT( popupAction( int ) ) );
			
			m_popupActions.insert( id, type.sourceVariable );
		}
		
		filler.fill( m, type.resultType->desc() );
	}
	if( !type ) return;
	
	///Now fill the class-view-browsing-stuff
	{
		QPopupMenu * m = new QPopupMenu( popup );
		int gid = popup->insertItem( i18n( "Navigate Class-View by \"%1\"" ).arg( cleanForMenu( name ) ), m );
		popup->setWhatsThis( gid, i18n( "<b>Navigation</b><p>Provides a menu to show involved items in the class-view " ) );
		
		PopupClassViewFillerHelpStruct h(this);
		PopupFiller<PopupClassViewFillerHelpStruct> filler( h, "<" );
		
		filler.fill( m, type.resultType->desc() );
	}
}





void CppCodeCompletion::slotTextHint(int line, int column, QString &text) {
	kdDebug( 9007 ) << "CppCodeCompletion::slotTextHint()" << endl;
	clearStatusText();
	text = "";
	if ( !m_pSupport || !m_activeEditor )
		return ;
	
	ConfigureSimpleTypes conf( m_activeFileName );
	
	EvaluationResult type = evaluateExpressionAt( line, column, conf );
	
	if( type.expr.expr().stripWhiteSpace().isEmpty() ) return; ///Expression could not be found
	
	if( type.sourceVariable ) {
		text += type.sourceVariable.toText() + "\n";
	}
	
	if( type.resultType ) {
		SimpleTypeFunctionInterface* f = type.resultType->asFunction();
		if( f ) {
			text += "function: \"" + buildSignature( type.resultType ) + "\"";
		} else {
			QValueList<TypeDesc> trace = type.resultType->trace();
			if( !trace.isEmpty() ) {
				for( QValueList<TypeDesc>::iterator it = trace.begin(); it != trace.end(); ++it ) {
					text += (*it).fullNameChain() + " --> ";
				}
				text += "\n";
			}
			text += "type: \"" + type.resultType->fullTypeResolved() + "\"";
		}
		if( type.resultType->parent()) text += "\nnested in: \"" + type.resultType->parent()->fullTypeResolvedWithScope() + "\"";
		DeclarationInfo i = type.resultType->getDeclarationInfo();
		if( i ) text += "\n" + i.locationToText();
		
		if( !type.resultType->comment().isEmpty() ) text +=  "\n\n" + type.resultType->comment() + "";
		
	} else {
	}
	
	kdDebug( 9007 ) << "showing: \n" << text << endl;
	const int timeout = 1300;
		
	if( type.resultType ) {
		addStatusText( i18n( "Type of \"%1\" is \"%2\"" ).arg( type.expr.expr() ).arg( type.resultType->fullTypeResolved() ), timeout );
		if( type.sourceVariable && !type.sourceVariable.comment.isEmpty() ) {
		addStatusText( i18n( "Comment on variable %1: \"%1\"").arg( type.sourceVariable.name ).arg( type.sourceVariable.comment ) , 10000 );
		}
		if( !type.resultType->comment().isEmpty() ) {
			addStatusText( i18n( "Comment on type %1: \"%1\"").arg( type.resultType->desc().name() ).arg( type.resultType->comment() ) , 10000 );
		} 
		if( type.resultType->comment().isEmpty() ) {
			addStatusText( i18n( "Type %1 has no comment").arg( type.resultType->desc().name() ) , timeout );
		}
	} else {
		if( type.resultType->desc() ) {
			addStatusText( i18n( "Type of \"%1\" is unresolved, name: \"%2\"" ).arg( type.expr.expr() ).arg( type.resultType->desc().fullNameChain() ), 2*timeout );
		} else {
			addStatusText( i18n( "Type of \"%1\" could not be evaluated! Tried to evaluate expression as \"%2\"" ).arg( type.expr.expr() ).arg( type.expr.typeAsString() ), 2*timeout );
		}
	}
	
	
	text = ""; ///Don't really use tooltips since those are not implemented in katepart, and don't work right in the qt-designer based part
};

///not good..
bool CppCodeCompletion::isTypeExpression( const QString& expr ) {
	TypeDesc d( expr );
	if( !d.isValidType() ) return false;
	
	QString ex = d.fullNameChain();
	QStringList lex = QStringList::split( " ", ex );
	QStringList lexpr = QStringList::split( " ", expr );
	return lex.join( " " ) == lexpr.join( " " );
}


bool CppCodeCompletion::mayBeTypeTail( int line, int column, QString& append, bool inFunction ) {
QString tail = clearComments( m_activeEditor->text( line, column+1, line+10 > (int)m_activeEditor->numLines() ? (int)m_activeEditor->numLines() : line + 10, 0 ));
		tail.replace("\n", " ");
		SafetyCounter s ( 100 );
		bool hadSpace = false;
		while( !tail.isEmpty() && s ) {
			if( tail[0] == ';' ) {
				return false;
			} else if( ( !inFunction && tail[0] == ',' ) || tail[0] == '&' || tail[0] == '*' || tail[0] == '{' ||  tail[0] == ':' ) {
				return true;
			} else if( isTypeOpenParen( tail[0] ) ) {
				///TODO: use findClose to make the whole expression include template-params
				int to = findClose( tail, 0 );
				if( to != -1 ) {
					append = tail.left( to + 1 );
					tail = tail.mid( to + 1 );
				} else {
					return false;
				}
			} else if( isTypeCloseParen( tail[0] ) ) {
				return true;
			} else if( tail[0].isSpace() ) {
				tail = tail.mid( 1 );
				hadSpace = true;
			} else if( tail[0].isLetter() ) {
				return hadSpace;
			} else {
				break;
			}
		}
	
	return false;
}

bool tokenAt( const QString& text, const QString& token, int textPos ) {
	if( text.isEmpty() ) return false;
	
	int tokenPos = token.length() - 1;
	if( tokenPos <= 0 || textPos <= 0 ) return false;
	
	while( text[textPos] == token[tokenPos] ) {
		
		--tokenPos;
		--textPos;
		
		if( tokenPos == 0 || textPos == 0 ) {
			if( tokenPos == 0 ) {
				if( textPos >= 1 && text[textPos] == token[tokenPos] ) {
					QChar c = text[ textPos-1];
					return c.isSpace() || c == '{' || c == '}' || c == ';';
				} else {
					return false;
				}
			} else {
				return false;
			}
		}
	}
	return false;
}


bool CppCodeCompletion::canBeTypePrefix( const QString& prefix, bool inFunction ) {
	
	bool hadSpace = false;
	for( int p = prefix.length() - 1 ; p >= 0; --p ) {
		if( prefix[p].isSpace() ) {
			continue;
		}
		
		if( prefix[p] == ';' || prefix[p] == '<' || prefix[p] == ':' || ( !inFunction && (prefix[p] == '(' || prefix[p] == ',') ) || prefix[p] == '}' || prefix[p] == '{' ) {
			return true;
		}
		
		///@todo: make this a simple regex
		if( prefix[p].isLetterOrNumber() && ( tokenAt( prefix, "class", p ) || tokenAt( prefix, "struct", p ) || tokenAt( prefix, "const", p ) || tokenAt( prefix, "typedef", p ) || tokenAt( prefix, "public", p ) || tokenAt( prefix, "protected", p ) || tokenAt( prefix, "private", p )|| tokenAt( prefix, "virtual", p ) || tokenAt( prefix, "static", p ) || tokenAt( prefix, "virtual", p ) ) )
			return true;
		else {
			return false;
		}
	}
	
	return true;
}


///This function is just a litte hack und should be remade, it doesn't work for all cases
CppCodeCompletion::ExpressionInfo CppCodeCompletion::findExpressionAt( int line, int column, int startLine, int startCol, bool inFunction )  {
	ExpressionInfo ret;
	
	QString contents = clearComments( getText( startLine, startCol, line, column ) );
	
	
	int start_expr = expressionAt( contents, contents.length() - 1 );
	
	if ( start_expr != int( contents.length() ) - 1 ) {
		ret.setExpr( contents.mid( start_expr, contents.length() - start_expr ).stripWhiteSpace() );
		if( !ret.expr().isEmpty() ) ret.t = ExpressionInfo::NormalExpression;
	}
	
	if( ret ) {
		///Check whether it may be a type-expression
		bool mayBeType = true;
		QString append;
		if( !mayBeTypeTail( line, column - 1, append, inFunction ) )
			mayBeType = false;
		if( mayBeType ) {
			if( !canBeTypePrefix( contents.left( start_expr ), inFunction ) )
				mayBeType = false;
		}
		
		//make this a regexp
		QString e = ret.expr();
		if( e.contains(".") || e.contains("->") || e.contains("(") || e.contains(")") || e.contains("=") || e.contains("-") ) mayBeType = false;
		
		if( mayBeType ) {
			ret.setExpr( ret.expr() +  append );
			ret.t = ExpressionInfo::TypeExpression;
		}
	}
	
	return ret;
}
	


SimpleContext* CppCodeCompletion::computeFunctionContext( FunctionDom f, int line, int col ) {
	if( !f ) return 0;
	int modelStartLine, modelStartColumn;
	int modelEndLine, modelEndColumn;
	
	f->getStartPosition( &modelStartLine, &modelStartColumn );
	f->getEndPosition( &modelEndLine, &modelEndColumn );
	
	QString textLine = m_activeEditor->textLine( modelStartLine );
	kdDebug( 9007 ) << "startLine = " << textLine << endl;
	
	QString contents = getText( modelStartLine, modelStartColumn, line, col );
		
	Driver d;
	Lexer lexer( &d );
		/// @todo setup the lexer(i.e. adds macro, special words, ...
		
	lexer.setSource( contents );
	Parser parser( &d, &lexer );
	
	DeclarationAST::Node recoveredDecl;
	RecoveryPoint* recoveryPoint = this->d->findRecoveryPoint( line, col );
	
	parser.parseDeclaration( recoveredDecl );
	if ( recoveredDecl.get() )
	{
		bool isFunDef = recoveredDecl->nodeType() == NodeType_FunctionDefinition;
		kdDebug( 9007 ) << "is function definition= " << isFunDef << endl;
		
		int startLine, startColumn;
		int endLine, endColumn;
		recoveredDecl->getStartPosition( &startLine, &startColumn );
		recoveredDecl->getEndPosition( &endLine, &endColumn );
		/*if( startLine != modelStartLine || endLine != modelEndLine || startColumn != modelStartColumn || endColumn != modelEndColumn ) {
		kdDebug( 9007 ) << "code-model and real file are out of sync \nfunction-bounds in code-model: " << endl;
			kdDebug( 9007 ) << "(l " << modelStartLine << ", c " << modelStartColumn << ") - (l " << modelEndLine  << ", c " << modelEndColumn << ") " << "parsed function-bounds: " << endl; 
			kdDebug( 9007 ) << "(l " << startLine << ", c " << startColumn << ") - (l " << endLine << ", c " << endColumn << ") " << endl;
		}*/
		
		if ( isFunDef )
		{
			FunctionDefinitionAST * def = static_cast<FunctionDefinitionAST*>( recoveredDecl.get() );
			
			SimpleContext* ctx = computeContext( def, endLine, endColumn, modelStartLine, modelStartColumn );
			if( !ctx ) return 0;
			
			QStringList scope = f->scope();
			
			if ( !scope.isEmpty() )
			{
				SimpleType parentType = SimpleType( scope.join("::") );
				parentType->setPointerDepth( 1 );
				ctx->setContainer( parentType );
			}
			
			SimpleType global = ctx->global();
			
			if( recoveryPoint ) {
				recoveryPoint->registerImports( global );
			} else {
				kdDebug( 9007 ) << "no recovery-point, cannot use imports" << endl;
			}
			
			///Insert the "this"-type(container) and correctly resolve it using imported namespaces
			if ( ctx->container() )
			{
				TypeDesc td = ctx->container()->desc();
				td.makePrivate();
				td.resetResolved( );
				SimpleType this_type = ctx->container()->locateType( td, SimpleTypeImpl::LocateBase );
				this_type->setPointerDepth( 1 );
				
				SimpleVariable var;
				var.type = this_type->desc();
				var.name = "this";
				var.comment = this_type->comment();
				ctx->add( var );
				ctx->setContainer( this_type );
			}
			
			return ctx;
		} else {
			kdDebug( 9007 ) << "computeFunctionContext: context is no function-definition" << endl;
		}
	} else {
		kdDebug( 9007 ) << "computeFunctionContext: could not find a valid declaration to recover" << endl;
	}
	return 0;
}

bool CppCodeCompletion::functionContains( FunctionDom f , int line, int col ) {
	if( !f ) return false;
	int sl, sc, el, ec;
	f->getStartPosition( &sl, &sc );
	f->getEndPosition( &el, &ec );
	QString t = clearComments( getText(sl, sc, el, ec ) );
	if( t.isEmpty() ) return false;
	
	int i = t.find('{');
	if( i == -1 ) return false;
	int lineCols = 0;
	for( int a = 0; a < i; a++ ) {
		if( t[a] == '\n' ) {
			sl++;
			lineCols = 0;
		}else {
			lineCols++;
		}
	}
	
	sc += lineCols;
	
	return (line > sl || (line == sl && col >= sc ) ) && (line < el || ( line == el && col < ec ) );
}

///WArning: yet check how to preserve the ConfigureSimpleTypes..
CppCodeCompletion::EvaluationResult CppCodeCompletion::evaluateExpressionType( int line, int column, ConfigureSimpleTypes& conf, EvaluateExpressionOptions opt ) {
	EvaluationResult ret;
	
	FileDom file = m_pSupport->codeModel()->fileByName( m_activeFileName );
	
	if( !file ) {
		m_pSupport->mainWindow() ->statusBar() ->message( i18n( "File %1 does not exist in the code-model" ).arg( m_activeFileName ), 1000 );
	kdDebug( 9007 ) << "Error: file " << m_activeFileName << " could not be located in the code-model, code-completion stopped\n";
		return SimpleType();
	}
	
	CodeModelUtils::CodeModelHelper fileModel( m_pSupport->codeModel(), file );
	
	int nLine = line, nCol = column;
	
	QString strCurLine = m_activeEditor->textLine( nLine );
	
	QString ch = strCurLine.mid( nCol - 1, 1 );
	QString ch2 = strCurLine.mid( nCol - 2, 2 );
	
	while( ch[0].isSpace() && nCol >= 3 ) {
		nCol -= 1;
		ch = strCurLine.mid( nCol - 1, 1 );
		ch2 = strCurLine.mid( nCol - 2, 2 );
	}
	
	if ( ch2 == "->" || ch == "." || ch == "(" )
	{
		int pos = ch2 == "->" ? nCol - 3 : nCol - 2;
		QChar c = strCurLine[ pos ];
		while ( pos > 0 && c.isSpace() )
			c = strCurLine[ --pos ];
		
		if ( !( c.isLetterOrNumber() || c == '_' || c == ')' || c == ']' || c == '>' ) )
			return SimpleType();
	}
	bool showArguments = false;
	
	if ( ch == "(" )
	{
		--nCol;
		while ( nCol > 0 && strCurLine[ nCol ].isSpace() )
			--nCol;
		showArguments = true;
	}
	
	SimpleType this_type;
	QString word;
	
	
	
	m_pSupport->backgroundParser() ->lock ();
	
	FunctionDom currentFunction = fileModel.functionAt( line, column );
	
	if( opt & SearchInFunctions ) {
		//currentFunction = fileModel.functionAt( line, column );
		
		if( currentFunction && functionContains( currentFunction, line, column ) ) {
			SimpleContext* ctx = computeFunctionContext( currentFunction, line, column );
			
			if( ctx ) {
				opt = remFlag( opt, SearchInClasses );
				conf.setGlobalNamespace( &(*ctx->global()) );
				int startLine, endLine;
				currentFunction->getStartPosition( &startLine, &endLine );
				ExpressionInfo exp = findExpressionAt( line, column , startLine, endLine, true );
				if( (opt & DefaultAsTypeExpression) && ( !exp.canBeNormalExpression() && !exp.canBeTypeExpression() ) && !exp.expr().isEmpty() ) exp.t = ExpressionInfo::TypeExpression;
				
				
				if( exp.canBeTypeExpression() ) {
					{
						if( ! (opt & IncludeTypeExpression) ) {
							kdDebug( 9007 ) << "recognized a type-expression, but another expression-type is desired" << endl;
						} else {
							ret.resultType = ctx->container()->locateType( exp.expr(), SimpleTypeImpl::TraceAliases );
						}
						
					}
				}
				if( exp.canBeNormalExpression() ) {
					{
						if( ! (opt & IncludeStandardExpressions) ) {
							kdDebug( 9007 ) << "recognized a standard-expression, but another expression-type is desired" << endl;
						} else {
							///Remove the not completely typed last word while normal completion
							if( !showArguments && (opt & CompletionOption) ) {
								QString e = exp.expr();
								int idx = e.length() - 1;
								while ( e[ idx ].isLetterOrNumber() || e[ idx ] == '_' )
									--idx;
								
								if ( idx != int( e.length() ) - 1 )
								{
									++idx;
									word = e.mid( idx ).stripWhiteSpace();
									exp.setExpr( e.left( idx ).stripWhiteSpace() );
								}
							}
							
							ret = evaluateExpression( exp, ctx );
						}
					}
				}
			} else {
				kdDebug( 9007 ) << "could not compute context" << endl;
			}
			if( ctx )
				delete ctx;
		} else {
			kdDebug( 9007 ) << "could not find context-function in code-model" << endl;
		}
	}
					
	if( (opt & SearchInClasses ) && !ret && (!currentFunction || !functionContains( currentFunction, line, column ) ) )
	{
		ClassDom currentClass = fileModel.classAt( line, column );
		int startLine = 0, startCol = 0;
		
		RecoveryPoint* recoveryPoint = this->d->findRecoveryPoint( line, column );
		
		QStringList scope;
		
		if( !currentClass ) {
			kdDebug( 9007 ) << "no container-class found" << endl;
			if( !recoveryPoint ) {
				kdDebug( 9007 ) << "no recovery-point found" << endl;
			} else {
				startLine = recoveryPoint->startLine;
				startCol = recoveryPoint->startColumn;
				scope = recoveryPoint->scope;
			}
		} else {
			scope = currentClass->scope();
			scope << currentClass->name();
			currentClass->getStartPosition( &startLine, &startCol );
		}
		
		SimpleType container( scope );
		
		SimpleType global = getGlobal( container );
		conf.setGlobalNamespace( &(*global) );
		
		if( recoveryPoint )
			recoveryPoint->registerImports( global );
		
		ExpressionInfo exp = findExpressionAt( line, column , startLine, startCol );
		exp.t = ExpressionInfo::TypeExpression;	///Outside of functions, we can only handle type-expressions
		ret.expr = exp;
		
		if( exp && (exp.t & ExpressionInfo::TypeExpression) ) {
			kdDebug( 9007 ) << "locating \"" << exp.expr() << "\" in " << container->fullTypeResolvedWithScope() << endl;
			ret.resultType = container->locateType( exp.expr(), SimpleTypeImpl::TraceAliases );
		} else {
			if( exp ) {
				kdDebug( 9007 ) << "wrong expression-type recognized" << endl;
			} else {
				kdDebug( 9007 ) << "expression could not be recognized" << endl;
			}
		}
	}

	m_pSupport->backgroundParser() ->unlock();

	return ret;
}


QString CppCodeCompletion::buildSignature( SimpleType currType ) 
{
	SimpleTypeFunctionInterface* f = currType->asFunction();
	if( !f ) return "";
	
	QString ret;
	SimpleType rtt = currType->locateType( f->getReturnType() );
	if( rtt->desc() ) {
		if( rtt ) {
			ret = rtt->fullTypeResolvedWithScope();
		} else {
			ret = rtt->desc().fullNameChain();
		}
	} else {
		ret = f->getReturnType().fullNameChain();
	}
	
	TypeDesc desc = currType->desc();
	desc.decreaseFunctionDepth();
	
	QString sig = ret + " " + desc.fullNameChain() + f->signature();
	if( f->isConst() ) sig += " const";
	return sig;
}


///TODO: make this use findExpressionAt etc. (like the other expression-evaluation-stuff)
void CppCodeCompletion::completeText( bool invokedOnDemand /*= false*/ )
{
	kdDebug( 9007 ) << "CppCodeCompletion::completeText()" << endl;

	if ( !m_pSupport || !m_activeCursor || !m_activeEditor || !m_activeCompletion )
		return ;
	
	m_demandCompletion = invokedOnDemand;
	
	FileDom file = m_pSupport->codeModel()->fileByName( m_activeFileName );
	
	if( !file ) {
		m_pSupport->mainWindow() ->statusBar() ->message( i18n( "File %1 does not exist in the code-model" ).arg( m_activeFileName ), 1000 );
		kdDebug( 9007 ) << "Error: file " << m_activeFileName << " could not be located in the code-model, code-completion stopped\n";
		return;
	}
	
	CodeModelUtils::CodeModelHelper fileModel( m_pSupport->codeModel(), file );
	
	unsigned int line, column;
	m_activeCursor->cursorPositionReal( &line, &column );

	
	int nLine = line, nCol = column;

	QString strCurLine = m_activeEditor->textLine( nLine );

	QString ch = strCurLine.mid( nCol - 1, 1 );
	QString ch2 = strCurLine.mid( nCol - 2, 2 );
	
	while( ch[0].isSpace() && nCol >= 3 ) {
		nCol -= 1;
		ch = strCurLine.mid( nCol - 1, 1 );
		ch2 = strCurLine.mid( nCol - 2, 2 );
	}

	if ( m_includeRx.search( strCurLine ) != -1 )
	{
		if ( !m_fileEntryList.isEmpty() )
		{
			m_bCompletionBoxShow = true;
			m_activeCompletion->showCompletionBox( m_fileEntryList, column - m_includeRx.matchedLength() );
		}
		return ;
	}

	bool showArguments = false;
	bool isInstance = true;
	m_completionMode = NormalCompletion;
		
	if ( ch2 == "->" || ch == "." || ch == "(" )
	{
		int pos = ch2 == "->" ? nCol - 3 : nCol - 2;
		QChar c = strCurLine[ pos ];
		while ( pos > 0 && c.isSpace() )
			c = strCurLine[ --pos ];

		if ( !( c.isLetterOrNumber() || c == '_' || c == ')' || c == ']' || c == '>' ) )
			return ;
	}

	if ( ch == "(" )
	{
		--nCol;
		while ( nCol > 0 && strCurLine[ nCol ].isSpace() )
			--nCol;

		showArguments = TRUE;
	}
	
	SimpleType type;
	SimpleType this_type;
	QString expr, word;

	DeclarationAST::Node recoveredDecl;
	TypeSpecifierAST::Node recoveredTypeSpec;

	SimpleContext* ctx = 0;
	ConfigureSimpleTypes conf( m_activeFileName );
	
	m_pSupport->backgroundParser() ->lock ();
	
	FunctionDom currentFunction = fileModel.functionAt( line, column );
	RecoveryPoint * recoveryPoint = d->findRecoveryPoint( line, column );
	if ( recoveryPoint || currentFunction )
	{
		QStringList scope;
		
		int startLine, startColumn;
		if( currentFunction ) { ///maybe change the priority of these
			kdDebug( 9007 ) << "using code-model for completion" << endl;
			currentFunction->getStartPosition( &startLine, &startColumn );
			scope = currentFunction->scope();
		} else {
			kdDebug( 9007 ) << "recovery-point, node-kind = " << nodeTypeToString( recoveryPoint->kind ) << endl;
			startLine = recoveryPoint->startLine;
			startColumn = recoveryPoint->startColumn;
			scope = recoveryPoint->scope;
		}
		
		QString textLine = m_activeEditor->textLine( startLine );
		kdDebug( 9007 ) << "startLine = " << textLine << endl;

		if ( currentFunction || recoveryPoint->kind == NodeType_FunctionDefinition )
		{

			QString textToReparse = getText( startLine, startColumn, line, showArguments ? nCol : column );
			
 			kdDebug(9007) << "-------------> reparse text" << endl << textToReparse << endl
 			             << "--------------------------------------------" << endl;

			Driver d;
			Lexer lexer( &d );
			/// @todo setup the lexer(i.e. adds macro, special words, ...

			lexer.setSource( textToReparse );
			Parser parser( &d, &lexer );

			parser.parseDeclaration( recoveredDecl );
/*			kdDebug(9007) << "recoveredDecl = " << recoveredDecl.get() << endl;*/
			if ( recoveredDecl.get() )
			{

				bool isFunDef = recoveredDecl->nodeType() == NodeType_FunctionDefinition;
				kdDebug( 9007 ) << "is function definition= " << isFunDef << endl;

				int endLine, endColumn;
				recoveredDecl->getEndPosition( &endLine, &endColumn );
				kdDebug( 9007 ) << "endLine = " << endLine << ", endColumn " << endColumn << endl;

				/// @todo check end position

				if ( isFunDef )
				{
					FunctionDefinitionAST * def = static_cast<FunctionDefinitionAST*>( recoveredDecl.get() );

					/// @todo remove code duplication

					QString contents = textToReparse;
					int start_expr = expressionAt( contents, contents.length() - 1 );

					// kdDebug(9007) << "start_expr = " << start_expr << endl;
					if ( start_expr != int( contents.length() ) - 1 )
						expr = contents.mid( start_expr, contents.length() - start_expr ).stripWhiteSpace();

					if ( expr.startsWith( "SIGNAL" ) || expr.startsWith( "SLOT" ) )
					{
						m_completionMode = expr.startsWith( "SIGNAL" ) ? SignalCompletion : SlotCompletion;

						showArguments = false;
						int end_expr = start_expr - 1;
						while ( end_expr > 0 && contents[ end_expr ].isSpace() )
							--end_expr;

						if ( contents[ end_expr ] != ',' )
						{
							expr = QString::null;
						}
						else
						{
							--end_expr;
							start_expr = expressionAt( contents, end_expr );
							expr = contents.mid( start_expr, end_expr - start_expr + 1 ).stripWhiteSpace();
						}
					}
					else
					{
						if( !showArguments ) {
							int idx = expr.length() - 1;
							while ( expr[ idx ].isLetterOrNumber() || expr[ idx ] == '_' )
								--idx;
	
							if ( idx != int( expr.length() ) - 1 )
							{
								++idx;
								word = expr.mid( idx ).stripWhiteSpace();
								expr = expr.left( idx ).stripWhiteSpace();
							}
						}
					}

					ctx = computeContext( def, endLine, endColumn, startLine, startColumn );
					DeclaratorAST* d = def->initDeclarator() ->declarator();
					NameAST* name = d->declaratorId();

					QStringList nested;

					QPtrList<ClassOrNamespaceNameAST> l;
					if ( name )
					{
						l = name->classOrNamespaceNameList();
					}
					//		    QPtrList<ClassOrNamespaceNameAST> l = name->classOrNamespaceNameList();
					QPtrListIterator<ClassOrNamespaceNameAST> nameIt( l );
					while ( nameIt.current() )
					{
						if ( nameIt.current() ->name() )
						{
							nested << nameIt.current() ->name() ->text();
						}
						++nameIt;
					}

					if( currentFunction ) {
						scope = currentFunction->scope();
						if( !scope.isEmpty() ) {
							//scope.pop_back(); 
						} else {
							kdDebug( 9007 ) << "scope is empty" << endl;
						}
					} else {
						scope += nested;
					}
					
					if ( !scope.isEmpty() )
					{
						SimpleType parentType = SimpleType( scope.join("::") );
						this_type = parentType;
						this_type->setPointerDepth( 1 );
						ctx->setContainer( this_type );
					}
					
					* ctx->container(); ///this is necessary, to make sure that the SimpleType is physically instatiated. Copying a not instatiated type will make changes made to the copy lost.
					
					SimpleType global = ctx->container();
					
					while( global.scope().size() != 0 ) {
						if( !safetyCounter ) break;
						global = global->parent();
					}
					
						
						SimpleTypeNamespace* n = dynamic_cast<SimpleTypeNamespace*>( &(*global) );
						if( !n ) {
							kdDebug( 9007 ) << "the global namespace was not resolved correctly " << endl;
						} else {
						
							if( recoveryPoint ) {
						///put the imports into the global namespace
								for( QValueList<QStringList>::iterator it = recoveryPoint->imports.begin(); it != recoveryPoint->imports.end(); ++it ) {
									kdDebug( 9007 ) << "inserting import " << *it << " into the global scole" << endl;
									n->addAliasMap( "", (*it).join("::") );
								}
							
							} else {
								kdDebug( 9007 ) << "WARNING: no recovery-point, cannot use imports" << endl;
							}
							
							n->addAliases( hardCodedAliases );
						}
						
					
					conf.setGlobalNamespace( &(*global) );
					
					///Now locate the local type using the imported namespaces
					if ( !scope.isEmpty() )
					{
						TypeDesc td = ctx->container()->desc();
						td.makePrivate();
						td.resetResolved( );
						this_type = ctx->container()->locateType( td, SimpleTypeImpl::LocateBase );
						this_type->setPointerDepth( 1 );
						
						SimpleVariable var;
						var.type = this_type->desc();
						var.comment= this_type->comment();
						var.name = "this";
						ctx->add( var );
						ctx->setContainer( this_type );
					} else {
						this_type = global;
					}
					
					
					type = evaluateExpression( expr, ctx );
				}
			}
			else
			{
				kdDebug( 9007 ) << "no valid declaration to recover!!!" << endl;
			}
		}
		else if ( recoveryPoint->kind == NodeType_ClassSpecifier )
		{
			QString textToReparse = getText( recoveryPoint->startLine, recoveryPoint->startColumn,
			                                 recoveryPoint->endLine, recoveryPoint->endColumn, line );
// 			kdDebug(9007) << "-------------> please reparse only text" << endl << textToReparse << endl
// 			             << "--------------------------------------------" << endl;

			Driver d;
			Lexer lexer( &d );
			/// @todo setup the lexer(i.e. adds macro, special words, ...

			lexer.setSource( textToReparse );
			Parser parser( &d, &lexer );

			parser.parseClassSpecifier( recoveredTypeSpec );
/*			kdDebug(9007) << "recoveredDecl = " << recoveredTypeSpec.get() << endl;*/
			if ( recoveredTypeSpec.get() )
			{

				ClassSpecifierAST * clazz = static_cast<ClassSpecifierAST*>( recoveredTypeSpec.get() );

				QString keyword = getText( line, 0, line, column ).simplifyWhiteSpace();
	
				kdDebug(9007) << "===========================> keyword is: " << keyword << endl;
	
				if ( keyword == "virtual" )
				{/*
					BaseClauseAST *baseClause = clazz->baseClause();
					if ( baseClause )
					{
						QPtrList<BaseSpecifierAST> baseList = baseClause->baseSpecifierList();
						QPtrList<BaseSpecifierAST>::iterator it = baseList.begin();
	
						for ( ; it != baseList.end(); ++it )
							type.append( ( *it )->name()->text() );
						
						ctx = new SimpleContext();
						
						showArguments = false;
						m_completionMode = VirtualDeclCompletion;
	
						kdDebug(9007) << "------> found virtual keyword for class specifier '" 
									<< clazz->text() << "'" << endl;
					}*/
				}
				else if ( QString("virtual").find( keyword ) != -1 )
					m_blockForKeyword = true;
				else
					m_blockForKeyword = false;
			}
		}
	}

	if ( !recoveredDecl.get() && !recoveredTypeSpec.get() )
	{
		TranslationUnitAST * ast = m_pSupport->backgroundParser() ->translationUnit( m_activeFileName );
		if ( AST * node = findNodeAt( ast, line, column ) )
		{
			kdDebug( 9007 ) << "------------------- AST FOUND --------------------" << endl;
			kdDebug( 9007 ) << "node-kind = " << nodeTypeToString( node->nodeType() ) << endl;

			if ( FunctionDefinitionAST * def = functionDefinition( node ) )
			{
				kdDebug( 9007 ) << "------> found a function definition" << endl;

				int startLine, startColumn;
				def->getStartPosition( &startLine, &startColumn );

				QString contents = getText( startLine, startColumn, line, showArguments ? nCol : column );


				/// @todo remove code duplication
				int start_expr = expressionAt( contents, contents.length() - 1 );

				// kdDebug(9007) << "start_expr = " << start_expr << endl;
				if ( start_expr != int( contents.length() ) - 1 )
					expr = contents.mid( start_expr, contents.length() - start_expr ).stripWhiteSpace();

				if ( expr.startsWith( "SIGNAL" ) || expr.startsWith( "SLOT" ) )
				{
					m_completionMode = expr.startsWith( "SIGNAL" ) ? SignalCompletion : SlotCompletion;

					showArguments = false;
					int end_expr = start_expr - 1;
					while ( end_expr > 0 && contents[ end_expr ].isSpace() )
						--end_expr;

					if ( contents[ end_expr ] != ',' )
					{
						expr = QString::null;
					}
					else
					{
						--end_expr;
						start_expr = expressionAt( contents, end_expr );
						expr = contents.mid( start_expr, end_expr - start_expr + 1 ).stripWhiteSpace();
					}
				}
				else
				{
					int idx = expr.length() - 1;
					while ( expr[ idx ].isLetterOrNumber() || expr[ idx ] == '_' )
						--idx;

					if ( idx != int( expr.length() ) - 1 )
					{
						++idx;
						word = expr.mid( idx ).stripWhiteSpace();
						expr = expr.left( idx ).stripWhiteSpace();
					}
				}

				ctx = computeContext( def, line, column, startLine, startColumn );

				QStringList scope;
				scopeOfNode( def, scope );
				this_type = scope;

				if ( scope.size() )
				{/*
					SimpleVariable var;
					var.type = scope;
					var.name = "this";
					ctx->add( var );*/
					//kdDebug(9007) << "add variable " << var.name << " with type " << var.type << endl;
				}

				type = evaluateExpression( expr, ctx );
			}
		}
	}
	m_pSupport->backgroundParser() ->unlock();

	if ( !ctx )
		return ;

	if ( ch2 == "::" || expr.isEmpty() )
	{
		isInstance = false;
	}

	kdDebug( 9007 ) << "===========================> type is: " << type->fullTypeResolved() << endl;
	kdDebug( 9007 ) << "===========================> word is: " << word << endl;

	if ( !showArguments )
	{
		QValueList<CodeCompletionEntry> entryList;
		
		if( !type && invokedOnDemand && this_type && ( expr.isEmpty() || expr.endsWith(";")) ) {
			
			QValueList<SimpleType> tstack;
			SimpleType t = this_type;
			bool ready = false;
			SafetyCounter cnt( 20 );
			while( !ready & cnt ) {
				if( t->scope().isEmpty() ) {
					ready = true;
				} else {
					//	tstack.push_front( t );
					computeCompletionEntryList( t, entryList, t->scope(), isInstance );
					t = t->parent();
				}
			}
			/*
			while( !tstack.isEmpty() && cnt ){
				computeCompletionEntryList( tstack.front(), entryList, tstack.front()->scope(), isInstance );
				tstack.pop_front();
			}//*//*
			kdDebug( 9007 ) << "listing members of local class " << this_type->fullType() << endl;
			computeCompletionEntryList( entryList, ctx, isInstance );
			computeCompletionEntryList( this_type, entryList, this_type->scope(), isInstance );*/
			
		} else if ( type && expr.isEmpty() )
		{
			computeCompletionEntryList( entryList, ctx, isInstance );
			
			if ( m_pSupport->codeCompletionConfig() ->includeGlobalFunctions() )
				computeCompletionEntryList( type, entryList, QStringList(), false );

			if ( this_type.scope().size() )
				computeCompletionEntryList( this_type, entryList, this_type.scope(), isInstance );
				computeCompletionEntryList( type, entryList, type->scope(), isInstance );

		}
		else if ( type )
		{
			computeCompletionEntryList( type, entryList, type.scope(), isInstance );
		}

		QStringList trueMatches;

		if ( invokedOnDemand )
		{
			// find matching words
			QValueList<CodeCompletionEntry>::Iterator it;
			for ( it = entryList.begin(); it != entryList.end(); ++it )
			{
				if ( (*it).text.startsWith( word ) )
				{
					trueMatches << (*it).text;

					// if more than one entry matches, abort immediately
					if ( trueMatches.size() > 1 )
						break;
				}
			}
		}

		if ( invokedOnDemand && trueMatches.size() == 1 )
		{
			// erbsland: get the cursor position now, because m_ccLine and m_ccColumn
			//           are not set until the first typed char.
			unsigned int nLine, nCol;
			m_activeCursor->cursorPositionReal( &nLine, &nCol );
			// there is only one entry -> complete immediately
			m_activeEditor->insertText( nLine, nCol,
				trueMatches[0].right( trueMatches[0].length() - word.length() ) );
		}
		else if ( entryList.size() )
		{
			entryList = unique( entryList );
			qHeapSort( entryList );
			
			m_bCompletionBoxShow = true;
			///Warning: the conversion is only possible because CodeCompletionEntry is binary compatible with KTextEditor::CompletionEntry,
			///never change that!
			m_activeCompletion->showCompletionBox( *((QValueList<KTextEditor::CompletionEntry>*)(&entryList)), word.length() );
		}
	}
	else
	{
		QValueList<QStringList> signatureList;

		signatureList = computeSignatureList( type );
		
		QString methodName = type->desc().name();
		
		///Search for variables with ()-operator in the context
		if( ctx ) {
			SimpleVariable var = ctx->findVariable( methodName );
			if( !var.name.isEmpty() ) {
				signatureList += computeSignatureList( ctx->container()->locateType( var.type ) );
			}
		}

		///search for fitting methods/classes in the current context
		SimpleType t = this_type;
		bool ready = false;
		SafetyCounter s( 20 );
		do {
			if( !t ) ready = true;
			SimpleType method = t->typeOf( methodName );
			if( method ) 
				signatureList += computeSignatureList( method );
			if( t ) t = t->parent();
		} while( !ready && s );
		
		if ( !signatureList.isEmpty() )
		{
			//signatureList = unique( signatureList );
			//qHeapSort( signatureList );
			m_bArgHintShow = true;
			m_activeCompletion->showArgHint( unique( signatureList ), "()", "," );
		}
	}

	delete( ctx );
	ctx = 0;
}


QValueList<QStringList> CppCodeCompletion::computeSignatureList( SimpleType type ) {
	QValueList<QStringList> retList;
	SimpleTypeFunctionInterface* f = type->asFunction();
	SimpleType currType = type;
	
	if( !f && !type->isNamespace() ) {
		SimpleType t = type->typeOf( "operator ( )", SimpleTypeImpl::MemberInfo::Function );
		
		if( t ) {
			f = t->asFunction();
			currType = t;
		}
	}
	
	while( f ) {
		QStringList lst;
		QString sig = buildSignature( currType );
		QString comment = currType->comment();
		QStringList commentList;
		if( !comment.isEmpty() ) {
			if( sig.length() + comment.length() < 130 ) {
			sig += ":  \"" + currType->comment() + "\"";
			} else {
				commentList = formatComment( comment );
			}
		}
		
		lst << sig;
		lst += commentList;
		
		currType = f->nextFunction();
		
			///Maybe try to apply implicit template-params in this place
		
		retList << lst;
		f = currType->asFunction();
	}
	return retList;
}


void CppCodeCompletion::slotFileParsed( const QString& fileName )
{
	if ( fileName != m_activeFileName || !m_pSupport || !m_activeEditor )
		return ;

	m_pSupport->backgroundParser() ->lock ();
	computeRecoveryPoints();
	m_pSupport->backgroundParser() ->unlock();
}


void CppCodeCompletion::setupCodeInformationRepository( )
{}


/**
This function takes a string from the point of view from within ctx,
extracts all information, and tries to locate the resulting type globally using ctx
*/
SimpleType CppCodeCompletion::typeName( QString str )
{
	if ( str.isEmpty() )
		return QStringList();
	/*
	
	if( str.contains("::" ) ) {
		///Build a chain with corrent parents
		QStringList l = splitType( str );
		if( l.count() > 1 ) {
			SimpleType current;
			QStringList cScope;
			
			for( QStringList::iterator it = l.begin(); it != l.end(); ++it ) {
				SimpleType old = current;
				current = typeName( *it );
				current->prependScope( cScope );
				if( old )
					current->setParent( &(*old) );
				cScope = current->scope();
			}
			return current;
		}
	}*/
	
	if( str.startsWith( "typename " ) ) {
		str = str.right( str.length() - strlen( "typename " ) );
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

		SimpleType ret ( type );
		ret->parseParams( str );
		return ret;
	}

	return QStringList();
}

SimpleContext* CppCodeCompletion::computeContext( FunctionDefinitionAST * ast, int line, int col, int lineOffset, int colOffset )
{
	kdDebug( 9007 ) << "CppCodeCompletion::computeContext() -- main" << endl;

	SimpleContext* ctx = new SimpleContext();

	if ( ast && ast->initDeclarator() && ast->initDeclarator() ->declarator() )
	{
		DeclaratorAST * d = ast->initDeclarator() ->declarator();
		if ( ParameterDeclarationClauseAST * clause = d->parameterDeclarationClause() )
		{
			if ( ParameterDeclarationListAST * params = clause->parameterDeclarationList() )
			{
				QPtrList<ParameterDeclarationAST> l( params->parameterList() );
				QPtrListIterator<ParameterDeclarationAST> it( l );
				while ( it.current() )
				{
					ParameterDeclarationAST * param = it.current();
					++it;

					SimpleVariable var;
					
					QStringList ptrList;
					QPtrList<AST> ptrOpList = param->declarator()->ptrOpList();
					QPtrList<AST>::iterator it = ptrOpList.begin();
					for ( ; it != ptrOpList.end(); ++it )
					{
						ptrList.append( ( *it )->text() );
					}
					
					var.ptrList = ptrList;
					var.type = param->typeSpec() ->text() + ptrList.join("");
					var.name = declaratorToString( param->declarator(), QString::null, true );
					var.comment = param->comment();
					param->getStartPosition( &var.startLine, &var.startCol );
					param->getEndPosition( &var.endLine, &var.endCol );
					
					if ( var.type )
					{
						ctx->add( var );
						//kdDebug(9007) << "add argument " << var.name << " with type " << var.type << endl;
					}
				}
			}
		}
	}

	
	if ( ast )
		computeContext( ctx, ast->functionBody(), line, col );
	
	if( ctx ) {
		ctx->offset( lineOffset, colOffset );
	}
	
	return ctx;
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, StatementAST* stmt, int line, int col )
{
	if ( !stmt )
		return ;

	switch ( stmt->nodeType() )
	{
	case NodeType_IfStatement:
		computeContext( ctx, static_cast<IfStatementAST*>( stmt ), line, col );
		break;
	case NodeType_WhileStatement:
		computeContext( ctx, static_cast<WhileStatementAST*>( stmt ), line, col );
		break;
	case NodeType_DoStatement:
		computeContext( ctx, static_cast<DoStatementAST*>( stmt ), line, col );
		break;
	case NodeType_ForStatement:
		computeContext( ctx, static_cast<ForStatementAST*>( stmt ), line, col );
		break;
	case NodeType_SwitchStatement:
		computeContext( ctx, static_cast<SwitchStatementAST*>( stmt ), line, col );
		break;
	case NodeType_TryBlockStatement:
		computeContext( ctx, static_cast<TryBlockStatementAST*>( stmt ), line, col );
		break;
	case NodeType_DeclarationStatement:
		computeContext( ctx, static_cast<DeclarationStatementAST*>( stmt ), line, col );
		break;
	case NodeType_StatementList:
		computeContext( ctx, static_cast<StatementListAST*>( stmt ), line, col );
		break;
	case NodeType_ExpressionStatement:
		break;
	}
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, StatementListAST* ast, int line, int col )
{
	if ( !inContextScope( ast, line, col, false, true ) )
		return;
	
	QPtrList<StatementAST> l( ast->statementList() );
	QPtrListIterator<StatementAST> it( l );
	while ( it.current() )
	{
		StatementAST * stmt = it.current();
		++it;

		computeContext( ctx, stmt, line, col );
	}
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, IfStatementAST* ast, int line, int col )
{
	if ( !inContextScope( ast, line, col ) )
		return;
	
	computeContext( ctx, ast->condition(), line, col );
	computeContext( ctx, ast->statement(), line, col );
	computeContext( ctx, ast->elseStatement(), line, col );
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, ForStatementAST* ast, int line, int col )
{
	if ( !inContextScope( ast, line, col ) )
		return;
	
	computeContext( ctx, ast->initStatement(), line, col );
	computeContext( ctx, ast->condition(), line, col );
	computeContext( ctx, ast->statement(), line, col );
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, DoStatementAST* ast, int line, int col )
{
	if ( !inContextScope( ast, line, col ) )
		return;
	
	//computeContext( ctx, ast->condition(), line, col );
	computeContext( ctx, ast->statement(), line, col );
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, WhileStatementAST* ast, int line, int col )
{
	if ( !inContextScope( ast, line, col ) )
		return;
	
	computeContext( ctx, ast->condition(), line, col );
	computeContext( ctx, ast->statement(), line, col );
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, SwitchStatementAST* ast, int line, int col )
{
	if ( !inContextScope( ast, line, col ) )
		return;
	
	computeContext( ctx, ast->condition(), line, col );
	computeContext( ctx, ast->statement(), line, col );
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, TryBlockStatementAST* ast, int line, int col )
{
	if ( !inContextScope( ast, line, col ) )
		return;
	
	computeContext( ctx, ast->statement(), line, col );
	computeContext( ctx, ast->catchStatementList(), line, col );
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, CatchStatementListAST* ast, int line, int col )
{
	if ( !inContextScope( ast, line, col, false, true ) )
		return;
	
	QPtrList<CatchStatementAST> l( ast->statementList() );
	QPtrListIterator<CatchStatementAST> it( l );
	while ( it.current() )
	{
		CatchStatementAST * stmt = it.current();
		++it;
		
		computeContext( ctx, stmt, line, col );
	}
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, CatchStatementAST* ast, int line, int col )
{
	if ( !inContextScope( ast, line, col ) )
		return;
	
	computeContext( ctx, ast->condition(), line, col );
	computeContext( ctx, ast->statement(), line, col );
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, DeclarationStatementAST* ast, int line, int col )
{
	if ( !ast->declaration() || ast->declaration() ->nodeType() != NodeType_SimpleDeclaration )
		return ;

	if ( !inContextScope( ast, line, col, true, false ) )
		return;
	
	SimpleDeclarationAST* simpleDecl = static_cast<SimpleDeclarationAST*>( ast->declaration() );
	TypeSpecifierAST* typeSpec = simpleDecl->typeSpec();

	InitDeclaratorListAST* initDeclListAST = simpleDecl->initDeclaratorList();
	if ( !initDeclListAST )
		return ;

	QPtrList<InitDeclaratorAST> l = initDeclListAST->initDeclaratorList();
	QPtrListIterator<InitDeclaratorAST> it( l );
	while ( it.current() )
	{
		DeclaratorAST * d = it.current() ->declarator();
		++it;

		if ( d->declaratorId() )
		{
			SimpleVariable var;
			
			QStringList ptrList;
			QPtrList<AST> ptrOpList = d->ptrOpList();
			QPtrList<AST>::iterator it = ptrOpList.begin();
			for ( ; it != ptrOpList.end(); ++it )
			{
				ptrList.append( ( *it )->text() );
			}
			
			var.ptrList = ptrList;
			var.type = typeSpec->text() + ptrList.join("");
			var.name = toSimpleName( d->declaratorId() );
			var.comment = d->comment();
			d->getStartPosition( &var.startLine, &var.startCol );
			d->getEndPosition( &var.endLine, &var.endCol );
			
			ctx->add( var );
			//kdDebug(9007) << "add variable " << var.name << " with type " << var.type << endl;
		}
	}
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, ConditionAST* ast, int line, int col )
{
	if ( !ast->typeSpec() || !ast->declarator() || !ast->declarator() ->declaratorId() )
		return ;

	if ( !inContextScope( ast, line, col, true, false ) )
		return;
	
	SimpleVariable var;
	
	QStringList ptrList;
	QPtrList<AST> ptrOpList = ast->declarator()->ptrOpList();
	QPtrList<AST>::iterator it = ptrOpList.begin();
	for ( ; it != ptrOpList.end(); ++it )
	{
		ptrList.append( ( *it )->text() );
	}
	
	var.ptrList = ptrList;
	var.type = ast->typeSpec() ->text() + ptrList.join("");
	var.name = toSimpleName( ast->declarator() ->declaratorId() );
	var.comment = ast->comment();
	ast->getStartPosition( &var.startLine, &var.startCol );
	ast->getEndPosition( &var.endLine, &var.endCol );
	ctx->add( var );
	//kdDebug(9007) << "add variable " << var.name << " with type " << var.type << endl;
}

bool CppCodeCompletion::inContextScope( AST* ast, int line, int col, bool checkStart, bool checkEnd )
{
	int startLine, startColumn;
	int endLine, endColumn;
	ast->getStartPosition( &startLine, &startColumn );
	ast->getEndPosition( &endLine, &endColumn );
	
// 	kdDebug(9007) << k_funcinfo << endl;
// 	kdDebug(9007) << "current char line: " << line << " col: " << col << endl;
// 	
// 	kdDebug(9007) << nodeTypeToString( ast->nodeType() )
// 		<< " start line: " << startLine
// 		<< " col: " << startColumn << endl;
// 	kdDebug(9007) << nodeTypeToString( ast->nodeType() )
// 		<< " end line: " << endLine
// 		<< " col: " << endColumn << endl;
	
	bool start = line > startLine || ( line == startLine && col >= startColumn );
	bool end = line < endLine || ( line == endLine && col <= endColumn );
	
	if ( checkStart && checkEnd  )
		return start && end;
	else if ( checkStart )
		return start;
	else if ( checkEnd )
		return end;
	
	return false;
}

FunctionDefinitionAST * CppCodeCompletion::functionDefinition( AST* node )
{
	
	while ( node )
	{
		if ( node->nodeType() == NodeType_FunctionDefinition )
			return static_cast<FunctionDefinitionAST*>( node );
		node = node->parent();
	}
	return 0;
}

QString CppCodeCompletion::getText( int startLine, int startColumn, int endLine, int endColumn, int omitLine )
{
	if ( startLine == endLine )
	{
		QString textLine = m_activeEditor->textLine( startLine );
		return textLine.mid( startColumn, endColumn - startColumn );
	}

	QStringList contents;

	for ( int line = startLine; line <= endLine; ++line )
	{
		if ( line == omitLine )
			continue;

		QString textLine = m_activeEditor->textLine( line );

		if ( line == startLine )
			textLine = textLine.mid( startColumn );
		if ( line == endLine )
			textLine = textLine.left( endColumn );

		contents << textLine;
	}
	return contents.join( "\n" );
}

// namespace?
class ComputeRecoveryPoints: public TreeParser
{
public:
	ComputeRecoveryPoints( QPtrList<RecoveryPoint>& points )
			: recoveryPoints( points )
	{}

	virtual void parseTranslationUnit( TranslationUnitAST* ast )
	{
		QValueList<QStringList> dummy;

		m_imports.push( dummy );
		TreeParser::parseTranslationUnit( ast );
		m_imports.pop();

		kdDebug( 9007 ) << "found " << recoveryPoints.count() << " recovery points" << endl;
	}

	virtual void parseUsingDirective( UsingDirectiveAST* ast )
	{
		if ( !ast->name() )
			return ;

		QStringList type = CppCodeCompletion::typeName( ast->name() ->text() ).scope();
		m_imports.top().push_back( type );
	}

	virtual void parseNamespace( NamespaceAST* ast )
	{
		//insertRecoveryPoint( ast );
		m_currentScope.push_back( ast->namespaceName() ->text() );

		m_imports.push( m_imports.top() ); // dup
		m_imports.top().push_back( m_currentScope );

		TreeParser::parseNamespace( ast );

		m_imports.pop();
		m_currentScope.pop_back();
	}

	void parseTemplateDeclaration( TemplateDeclarationAST* ast )
	{
		if ( ast->declaration() )
			parseDeclaration( ast->declaration() );
		
		TreeParser::parseTemplateDeclaration( ast );
	}
	
	
	virtual void parseSimpleDeclaration( SimpleDeclarationAST* ast )
	{
		TypeSpecifierAST * typeSpec = ast->typeSpec();
		//InitDeclaratorListAST* declarators = ast->initDeclaratorList();

		if ( typeSpec )
			parseTypeSpecifier( typeSpec );

		//insertRecoveryPoint( ast );
		TreeParser::parseSimpleDeclaration( ast );
	}

	virtual void parseFunctionDefinition( FunctionDefinitionAST* ast )
	{
		m_imports.push( m_imports.top() ); // dup
		insertRecoveryPoint( ast );
		m_imports.pop();
	}

	virtual void parseClassSpecifier( ClassSpecifierAST* ast )
	{
		insertRecoveryPoint( ast );
		m_currentScope.push_back( toSimpleName( ast->name() ) );
		TreeParser::parseClassSpecifier( ast );
		m_currentScope.pop_back();
	}

	void insertRecoveryPoint( AST* ast )
	{
		if ( !ast )
			return ;

		RecoveryPoint* pt = new RecoveryPoint();
		pt->kind = ast->nodeType();
		pt->scope = m_currentScope;
		ast->getStartPosition( &pt->startLine, &pt->startColumn );
		ast->getEndPosition( &pt->endLine, &pt->endColumn );
		pt->imports = m_imports.top();

		recoveryPoints.append( pt );
	}

private:
	QPtrList<RecoveryPoint>& recoveryPoints;
	QValueStack< QValueList<QStringList> > m_imports;
	QStringList m_currentScope;
};


void CppCodeCompletion::computeRecoveryPoints( )
{
	if ( m_blockForKeyword )
		return;

	kdDebug( 9007 ) << "CppCodeCompletion::computeRecoveryPoints" << endl;

	d->recoveryPoints.clear();
	TranslationUnitAST* unit = m_pSupport->backgroundParser() ->translationUnit( m_activeFileName );
	if ( !unit )
		return ;

	ComputeRecoveryPoints walker( d->recoveryPoints );
	walker.parseTranslationUnit( unit );
}


void CppCodeCompletion::computeCompletionEntryList( SimpleType typeR, QValueList< CodeCompletionEntry > & entryList, const QStringList & type, bool isInstance )
{
	Debug d("#cel#");
	if( !safetyCounter || !d ) return;
	CppCodeCompletionConfig * cfg = m_pSupport->codeCompletionConfig();
	SimpleTypeImpl* m = &(*typeR) ;
	
	if ( dynamic_cast<SimpleTypeCodeModel*>(m) )
	{
		ItemDom item = ( dynamic_cast<SimpleTypeCodeModel*>(m) )->item();
		if( item)
			if( ClassModel* mod = dynamic_cast<ClassModel*> (&(*item)) )
				computeCompletionEntryList( typeR, entryList, ClassDom( mod ) , isInstance );
	} /*else*/ {
		QValueList<Catalog::QueryArgument> args;
		QValueList<Tag> tags;

		args.clear();
		args << Catalog::QueryArgument( "kind", Tag::Kind_FunctionDeclaration )
		<< Catalog::QueryArgument( "scope", type );
		tags = m_repository->query( args );
		computeCompletionEntryList( typeR, entryList, tags, isInstance );

		args.clear();
		args << Catalog::QueryArgument( "kind", Tag::Kind_Variable )
		<< Catalog::QueryArgument( "scope", type );
		tags = m_repository->query( args );
		computeCompletionEntryList( typeR, entryList, tags, isInstance );

		if ( !isInstance && cfg->includeEnums() )
		{
			args.clear();
			args << Catalog::QueryArgument( "kind", Tag::Kind_Enumerator )
			<< Catalog::QueryArgument( "scope", type );
			tags = m_repository->query( args );
			computeCompletionEntryList( typeR, entryList, tags, isInstance );
		}

		if ( !isInstance && cfg->includeTypedefs() )
		{
			args.clear();
			args << Catalog::QueryArgument( "kind", Tag::Kind_Typedef )
			<< Catalog::QueryArgument( "scope", type );
			tags = m_repository->query( args );
			computeCompletionEntryList( typeR, entryList, tags, isInstance );
		}

		args.clear();
		args << Catalog::QueryArgument( "kind", Tag::Kind_Base_class );
		QString fullname = type.join( "::" );
		/*    	if( fullname.length() >=2 )
		            args << Catalog::QueryArgument( "prefix", fullname.left(2) );*/
		args << Catalog::QueryArgument( "name", fullname );

		
		QValueList<SimpleType> parents = typeR->getBases();
		for ( QValueList<SimpleType>::Iterator it = parents.begin(); it != parents.end(); ++it )
		{
			SimpleType tp = *it;
			if( tp ) computeCompletionEntryList( tp, entryList, tp.scope(), isInstance );
		}
	}
}

struct CompTypeProcessor : public TypeProcessor {
	SimpleType m_scope;
	CompTypeProcessor( SimpleType scope )  : m_scope(scope) {
	}
	
	virtual QString parentType() {
		return m_scope->fullType();
	}
		
	virtual QString processType( const QString& type ) {
		///TODO: Option: should arguments be processed? If no, just return "type"
		TypeDecoration dec( type );
		SimpleType t = m_scope->locateType( type );
		if( t || t->desc() )
			return dec.apply( t->fullTypeResolved() );
		else
			return type;
		
	}
};


void CppCodeCompletion::computeCompletionEntryList( SimpleType type, QValueList< CodeCompletionEntry > & entryList, QValueList< Tag > & tags, bool /*isInstance*/ )
{
	Debug d("#cel#");
	if( !safetyCounter || !d ) return;
	
	CompTypeProcessor proc( type );
	
	QValueList<Tag>::Iterator it = tags.begin();
	bool fullOutput = type->usingTemplates();
	while ( it != tags.end() )
	{
		Tag & tag = *it;
		++it;

		if ( tag.name().isEmpty() )
		{
			continue;
		}
		else if ( m_completionMode != NormalCompletion )
		{
			if ( tag.kind() != Tag::Kind_FunctionDeclaration )
				continue;

			CppFunction<Tag> info( tag );

			if ( m_completionMode == SlotCompletion && !info.isSlot() )
				continue;
			else if ( m_completionMode == SignalCompletion && !info.isSignal() )
				continue;
			else if ( m_completionMode == VirtualDeclCompletion && !info.isVirtual() )
				continue;
		}
		
		CodeCompletionEntry e = CodeInformationRepository::toEntry( tag, m_completionMode, &proc );
			
		TagFlags fl;
		fl.flags = tag.flags();
		int num = fl.data.access;
		
		QString str = "public";
		if( num != 0 ) {
			str = TagUtils::accessToString( num );
		} else {
			num = 0;
		}
		// 0 = protected, 1 = public, 2 = private
		
		if( str == "public" )
			num = 0;
		else if( str == "protected" )
			num = 1;
		else if( str == "private" )
			num = 2;
		
		e.userdata = QString("%1").arg( num ); // num > 0 ? num - 1 : 0 );
		
		if( !type->isNamespace() ) {
			if( num == 1 ) 
				e.postfix += "; (protected in " + proc.parentType() + ")";
			if( num == 2 ) 
				e.postfix += "; (private in " + proc.parentType() + ")";
		}
		
		
		QString prefix;
			
		if( fullOutput && (tag.kind() == Tag::Kind_FunctionDeclaration || tag.kind() == Tag::Kind_Function || tag.kind() == Tag::Kind_Variable || tag.kind() == Tag::Kind_Typedef)) 
		{
			QString tt = tagType( tag );
			if( !tt.isEmpty() ) {
				TypeDecoration dec ( tt );
				SimpleType et =  type->locateType( tt );
				
				if( et || et->desc() ) {
					prefix = et->fullTypeResolved();
				} else {
					prefix = tt;
				}
			}
		}
		
		if( e.prefix.isEmpty() )
			e.prefix = prefix;
		else
			if( !prefix.isEmpty() )
				e.prefix+= " " + prefix;
		
		if( str != "private" )
			entryList << e;
	}
}

void CppCodeCompletion::computeCompletionEntryList( SimpleType type, QValueList< CodeCompletionEntry > & entryList, ClassDom klass, bool isInstance )
{
	Debug d("#cel#");
	if( !safetyCounter || !d ) return;
	
	computeCompletionEntryList( type, entryList, klass->functionList(), isInstance );
	if ( m_completionMode == NormalCompletion )
		computeCompletionEntryList( type, entryList, klass->variableList(), isInstance );

	QValueList<SimpleType> parents = type->getBases();
	for ( QValueList<SimpleType>::Iterator it = parents.begin(); it != parents.end(); ++it )
	{
		SimpleTypeImpl* i = &(**it);
		SimpleTypeCodeModel* m = dynamic_cast<SimpleTypeCodeModel*> ( i );
		if( m ) {
			ItemDom item = m->item();
			ClassModel* kl = dynamic_cast<ClassModel*> ( &( *item ) );
			if( kl ) {
				computeCompletionEntryList( *it, entryList, ClassDom ( kl ), isInstance );
			}
		}
	}
}

void CppCodeCompletion::computeCompletionEntryList( SimpleType type, QValueList< CodeCompletionEntry > & entryList, NamespaceDom scope, bool isInstance )
{
	Debug d("#cel#");
	if( !safetyCounter || !d ) return;
	
	CppCodeCompletionConfig * cfg = m_pSupport->codeCompletionConfig();

	if ( cfg->includeGlobalFunctions() )
	{
		computeCompletionEntryList( type, entryList, scope->functionList(), isInstance );

		if ( m_completionMode == NormalCompletion )
			computeCompletionEntryList( type, entryList, scope->variableList(), isInstance );
	}

	if ( !isInstance && cfg->includeTypes() )
	{
		computeCompletionEntryList( type, entryList, scope->classList(), isInstance );
		computeCompletionEntryList( type, entryList, scope->namespaceList(), isInstance );
	}
}

void CppCodeCompletion::computeCompletionEntryList( SimpleType type, QValueList< CodeCompletionEntry > & entryList, const ClassList & lst, bool isInstance )
{
	Debug d("#cel#");
	if( !safetyCounter || !d ) return;
	
	CppCodeCompletionConfig * cfg = m_pSupport->codeCompletionConfig();

	ClassList::ConstIterator it = lst.begin();
	while ( it != lst.end() )
	{
		ClassDom klass = *it;
		++it;

		CodeCompletionEntry entry;
		entry.prefix = "class";
		entry.text = klass->name();
		entry.comment = klass->comment();
		entryList << entry;

		if ( cfg->includeTypes() )
		{
			computeCompletionEntryList( type, entryList, klass->classList(), isInstance );
		}
	}
}

void CppCodeCompletion::computeCompletionEntryList( SimpleType type, QValueList< CodeCompletionEntry > & entryList, const NamespaceList & lst, bool /*isInstance*/ )
{
	Debug d("#cel#");
	if( !safetyCounter || !d ) return;
	
	NamespaceList::ConstIterator it = lst.begin();
	while ( it != lst.end() )
	{
		NamespaceDom scope = *it;
		++it;

		CodeCompletionEntry entry;
		entry.prefix = "namespace";
		entry.text = scope->name();
		entry.comment = scope->comment();
		entryList << entry;
	}
}

void CppCodeCompletion::computeCompletionEntryList( SimpleType type, QValueList< CodeCompletionEntry > & entryList, const FunctionList & methods, bool isInstance )
{
	Debug d("#cel#");
	if( !safetyCounter || !d ) return;
	
	bool resolve = type->usingTemplates();
	CompTypeProcessor proc( type );
	
	FunctionList::ConstIterator it = methods.begin();
	while ( it != methods.end() )
	{
		FunctionDom meth = *it;
		++it;

		if ( isInstance && meth->isStatic() )
			continue;
		else if ( m_completionMode == SignalCompletion && !meth->isSignal() )
			continue;
		else if ( m_completionMode == SlotCompletion && !meth->isSlot() )
			continue;
		else if ( m_completionMode == VirtualDeclCompletion && !meth->isVirtual() )
			continue;

		CodeCompletionEntry entry;
		
		entry.comment = meth->comment();
		
		if( ! resolve ) {
			entry.prefix = meth->resultType();
		}else{
			QString tt = meth->resultType();
			TypeDecoration dec( tt );
			SimpleType t = type->locateType( tt );
			//SimpleType t = type->typeOf( meth->name() );
			if( t || t->desc().length() != 0 ) {
				//t = t->applyOperator( SimpleTypeImpl::ParenOp );
				entry.prefix = dec.apply( t->fullTypeResolved() );
			}
			else
				entry.prefix = meth->resultType();
		}

		QString text;

		ArgumentList args = meth->argumentList();
		ArgumentList::Iterator argIt = args.begin();
		/*
		if ( m_completionMode == VirtualDeclCompletion )
		{
			//Ideally the type info would be a entry.prefix, but we need them to be
			//inserted upon completion so they have to be part of entry.text
			entry.text = meth->resultType();
			entry.text += " ";
			entry.text += meth->name();
		}
		else*/
			entry.text = meth->name();
		
		if ( !args.size() )
			entry.text += "(";
		else
			entry.text += "( ";
		
		while ( argIt != args.end() )
		{
			ArgumentDom arg = *argIt;
			++argIt;

			text += proc.processType( arg->type() );
			if ( m_completionMode == NormalCompletion ||
			     m_completionMode == VirtualDeclCompletion )
				text += QString( " " ) +  arg->name();

			if ( argIt != args.end() )
				text += ", ";
		}

		if ( text.isEmpty() )
			entry.text += ")";
		else
			text += " )";

		
		entry.userdata += QString("%1").arg( meth->access() );
		
		if ( m_completionMode == VirtualDeclCompletion )
			entry.text += text + ";";
		if ( m_completionMode != NormalCompletion )
			entry.text += text;
		else
			entry.postfix = text;

		if ( meth->isConstant() )
			entry.postfix += " const";
		if( !type->isNamespace() ) {
			if( meth->access() == CodeModelItem::Protected ) 
				entry.postfix += "; (protected in " + type->fullType() + ")";
			if( meth->access() == CodeModelItem::Private ) 
				entry.postfix += "; (private in " + type->fullType() + ")";
		}
		
		entryList << entry;
	}
}

void CppCodeCompletion::computeCompletionEntryList( SimpleType type, QValueList< CodeCompletionEntry > & entryList, const VariableList & attributes, bool isInstance )
{
	Debug d("#cel#");
	if( !safetyCounter || !d ) return;
	
	if ( m_completionMode != NormalCompletion )
		return ;
	bool resolve = type->usingTemplates();
	
	VariableList::ConstIterator it = attributes.begin();
	while ( it != attributes.end() )
	{
		VariableDom attr = *it;
		++it;

		if ( isInstance && attr->isStatic() )
			continue;

		CodeCompletionEntry entry;
		entry.text = attr->name();
		entry.comment = attr->comment();
		entry.userdata += QString("%1").arg( attr->access() );
		
		
		if( ! resolve ) {
			entry.prefix = attr->type();
		}else{
			QString tt = attr->type();
			TypeDecoration dec( tt );
			SimpleType t = type->locateType( tt );
			//SimpleType t = type->typeOf( attr->name() );
			if( t || t->desc().length() != 0 ) 
				entry.prefix = dec.apply( t->fullTypeResolved() );
			else
				entry.prefix = attr->type();
		}		
		if( attr->access() == CodeModelItem::Protected ) 
			entry.postfix += "; (protected in " + type->fullType() + ")";
		if( attr->access() == CodeModelItem::Private ) 
			entry.postfix += "; (private in " + type->fullType() + ")";
		
		entryList << entry;
	}
}

void CppCodeCompletion::computeCompletionEntryList( QValueList< CodeCompletionEntry > & entryList, SimpleContext * ctx, bool /*isInstance*/ )
{
	Debug d("#cel#");
	if( !safetyCounter || !d ) return;
	
	while ( ctx )
	{
		QValueList<SimpleVariable> vars = ctx->vars();
		QValueList<SimpleVariable>::ConstIterator it = vars.begin();
		while ( it != vars.end() )
		{
			const SimpleVariable & var = *it;
			++it;

			CodeCompletionEntry entry;
			entry.text = var.name;
			entryList << entry;
		}
		ctx = ctx->prev();
	}
	
}



CppCodeCompletion::EvaluationResult CppCodeCompletion::evaluateExpression( ExpressionInfo expr, SimpleContext* ctx )
{
	safetyCounter.init();

	d->classNameList = typeNameList( m_pSupport->codeModel() );
	
	CppEvaluation::ExpressionEvaluation obj( this, expr, AllOperators, ctx );
	
	CppCodeCompletion::EvaluationResult res;
	res = obj.evaluate();
	
	m_pSupport->mainWindow() ->statusBar() ->message( i18n( "Type of %1 is %2" ).arg( expr.expr() ).arg( res->fullTypeResolved() ), 1000 );
	
	return res;
}


void CppCodeCompletion::computeFileEntryList( )
{
	m_fileEntryList.clear();

	QStringList fileList = m_pSupport->project() ->allFiles();
	for ( QStringList::Iterator it = fileList.begin(); it != fileList.end(); ++it )
	{
		if ( !m_pSupport->isHeader( *it ) )
			continue;

		CodeCompletionEntry entry;
		entry.text = QFileInfo( *it ).fileName();
		m_fileEntryList.push_back( entry );
	}

	m_fileEntryList = unique( m_fileEntryList );
}

#include "cppcodecompletion.moc"
//kate: indent-mode csands; tab-width 4; space-indent off;
