/* This file is part of KDevelop
    Copyright (C) 2002,2003 Roberto Raggi <roberto@kdevelop.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.
*/

#include "driver.h"
#include "lexer.h"
#include "parser.h"
#include <kdebug.h>

Driver::Driver()
{
}

Driver::~Driver()
{
}

void Driver::reset( )
{
    m_dependences.clear();
    m_macros.clear();
    m_problems.clear();
}

void Driver::clear( const QString & fileName )
{
    m_dependences.remove( fileName );
    m_macros.remove( fileName );
    m_problems.remove( fileName );
}

void Driver::addDependence( const QString & fileName, const Dependence & dep )
{
    findOrInsertDependenceList( fileName ).insert( dep.first, dep );
}

void Driver::addMacro( const QString & fileName, const Macro & macro )
{
    findOrInsertMacroList( fileName ).insert( macro.name(), macro );
}

void Driver::addProblem( const QString & fileName, const Problem & problem )
{
    findOrInsertProblemList( fileName ).append( problem );
}

QMap< QString, Dependence >& Driver::findOrInsertDependenceList( const QString & fileName )
{
    QMap<QString, QMap<QString, Dependence> >::Iterator it = m_dependences.find( fileName );
    if( it != m_dependences.end() )
        return it.data();

    QMap<QString, Dependence> l;
    m_dependences.insert( fileName, l );
    return m_dependences[ fileName ];
}

QMap< QString, Macro >& Driver::findOrInsertMacroList( const QString & fileName )
{
    QMap<QString, QMap<QString, Macro> >::Iterator it = m_macros.find( fileName );
    if( it != m_macros.end() )
        return it.data();

    QMap<QString, Macro> l;
    m_macros.insert( fileName, l );
    return m_macros[ fileName ];
}

QValueList < Problem >& Driver::findOrInsertProblemList( const QString & fileName )
{
    QMap<QString, QValueList<Problem> >::Iterator it = m_problems.find( fileName );
    if( it != m_problems.end() )
        return it.data();

    QValueList<Problem> l;
    m_problems.insert( fileName, l );
    return m_problems[ fileName ];
}

QMap< QString, Dependence > Driver::dependences( const QString & fileName ) const
{
    QMap<QString, QMap<QString, Dependence> >::ConstIterator it = m_dependences.find( fileName );
    if( it != m_dependences.end() )
	return it.data();
    return QMap<QString, Dependence>();
}

QMap< QString, Macro > Driver::macros( const QString & fileName ) const
{
    QMap<QString, QMap<QString, Macro> >::ConstIterator it = m_macros.find( fileName );
    if( it != m_macros.end() )
	return it.data();
    return QMap<QString, Macro>();
}

QValueList < Problem > Driver::problems( const QString & fileName ) const
{
    QMap<QString, QValueList<Problem> >::ConstIterator it = m_problems.find( fileName );
    if( it != m_problems.end() )
	return it.data();
    return QValueList<Problem>();
}

TranslationUnitAST :: Node Driver::parseFile( const QString & fileName, const QString& source )
{
    clear( fileName );

    m_currentFileName = fileName;

    Lexer lexer( this );
    setupLexer( &lexer );

    lexer.setSource( source );
    Parser parser( this, &lexer );
    setupParser( &parser );

    TranslationUnitAST :: Node translationUnit;
    parser.parseTranslationUnit( translationUnit );
    
    m_currentFileName = QString::null;
    
    return translationUnit;
}

void Driver::setupLexer( Lexer * lexer )
{
    // stl
    lexer->addSkipWord( "__STL_BEGIN_NAMESPACE" );
    lexer->addSkipWord( "__STL_END_NAMESPACE" );
    lexer->addSkipWord( "__STL_BEGIN_RELOPS_NAMESPACE" );
    lexer->addSkipWord( "__STL_END_RELOPS_NAMESPACE" );
    lexer->addSkipWord( "__STL_TEMPLATE_NULL" );
    lexer->addSkipWord( "__STL_TRY" );
    lexer->addSkipWord( "__STL_UNWIND" );
    lexer->addSkipWord( "__STL_NOTHROW" );
    lexer->addSkipWord( "__STL_NULL_TMPL_ARGS" );
    lexer->addSkipWord( "__STL_UNWIND", SkipWordAndArguments );
    lexer->addSkipWord( "__GC_CONST" );
    lexer->addSkipWord( "__HASH_ALLOC_INIT", SkipWordAndArguments );
    lexer->addSkipWord( "_ROPE_SWAP_SPECIALIZATION", SkipWordAndArguments );
    lexer->addSkipWord( "__ROPE_DEFINE_ALLOCS", SkipWordAndArguments );

	// antlr
    lexer->addSkipWord( "ANTLR_BEGIN_NAMESPACE", SkipWordAndArguments );
    lexer->addSkipWord( "ANTLR_USE_NAMESPACE", SkipWordAndArguments );
    lexer->addSkipWord( "ANTLR_USING_NAMESPACE", SkipWordAndArguments );
    lexer->addSkipWord( "ANTLR_END_NAMESPACE" );
    lexer->addSkipWord( "ANTLR_C_USING", SkipWordAndArguments );

    // gnu
    lexer->addSkipWord( "__extension__" );
    lexer->addSkipWord( "__attribute__", SkipWordAndArguments );
    lexer->addSkipWord( "__BEGIN_DECLS" );
    lexer->addSkipWord( "__END_DECLS" );
    lexer->addSkipWord( "__THROW" );
    lexer->addSkipWord( "__const" );
    lexer->addSkipWord( "__const__" );
    lexer->addSkipWord( "__restrict" );
    lexer->addSkipWord( "__restrict__" );
    lexer->addSkipWord( "__inline" );
    lexer->addSkipWord( "__inline__" );
    lexer->addSkipWord( "__attribute_pure__" );
    lexer->addSkipWord( "__attribute_malloc__" );
    lexer->addSkipWord( "__attribute_format_strfmon__" );
    lexer->addSkipWord( "__asm__", SkipWordAndArguments );
    lexer->addSkipWord( "__devinit" );
    lexer->addSkipWord( "__devinit__" );
    lexer->addSkipWord( "__init" );
    lexer->addSkipWord( "__init__" );
    lexer->addSkipWord( "__signed" );
    lexer->addSkipWord( "__signed__" );
    lexer->addSkipWord( "__unsigned" );
    lexer->addSkipWord( "__unsigned__" );
    lexer->addSkipWord( "asmlinkage" );
    lexer->addSkipWord( "____cacheline_aligned" );
    
    lexer->addSkipWord( "__BEGIN_NAMESPACE_STD" );
    lexer->addSkipWord( "__END_NAMESPACE_STD" );
    lexer->addSkipWord( "__BEGIN_NAMESPACE_C99" );
    lexer->addSkipWord( "__END_NAMESPACE_C99" );
    lexer->addSkipWord( "__USING_NAMESPACE_STD", SkipWordAndArguments );

    // kde
    lexer->addSkipWord( "K_SYCOCATYPE", SkipWordAndArguments );
    lexer->addSkipWord( "EXPORT_DOCKCLASS" );
    lexer->addSkipWord( "K_EXPORT_COMPONENT_FACTORY", SkipWordAndArguments );
    lexer->addSkipWord( "K_SYCOCAFACTORY", SkipWordAndArguments );

    // qt
    lexer->addSkipWord( "Q_OVERRIDE", SkipWordAndArguments );
    lexer->addSkipWord( "Q_ENUMS", SkipWordAndArguments );
    lexer->addSkipWord( "Q_PROPERTY", SkipWordAndArguments );
    lexer->addSkipWord( "Q_CLASSINFO", SkipWordAndArguments );
    lexer->addSkipWord( "Q_SETS", SkipWordAndArguments );
    lexer->addSkipWord( "Q_UNUSED", SkipWordAndArguments );
    lexer->addSkipWord( "Q_CREATE_INSTANCE", SkipWordAndArguments );
    lexer->addSkipWord( "Q_DUMMY_COMPARISON_OPERATOR", SkipWordAndArguments );
    lexer->addSkipWord( "ACTIVATE_SIGNAL_WITH_PARAM", SkipWordAndArguments );
    lexer->addSkipWord( "Q_INLINE_TEMPLATES" );
    lexer->addSkipWord( "Q_TEMPLATE_EXTERN" );
    lexer->addSkipWord( "Q_TYPENAME" );
    lexer->addSkipWord( "Q_REFCOUNT" );
    lexer->addSkipWord( "Q_EXPLICIT" );
    lexer->addSkipWord( "QMAC_PASCAL" );
    lexer->addSkipWord( "QT_STATIC_CONST" );
    lexer->addSkipWord( "QT_STATIC_CONST_IMPL" );
    lexer->addSkipWord( "QT_WIN_PAINTER_MEMBERS" );
    lexer->addSkipWord( "QT_NC_MSGBOX" );
    lexer->addSkipWord( "Q_VARIANT_AS", SkipWordAndArguments );
    lexer->addSkipWord( "CALLBACK_CALL_TYPE" );

    // flex
    lexer->addSkipWord( "yyconst" );
    lexer->addSkipWord( "YY_RULE_SETUP" );
    lexer->addSkipWord( "YY_BREAK" );
    lexer->addSkipWord( "YY_RESTORE_YY_MORE_OFFSET" );

    // gtk
    lexer->addSkipWord( "G_BEGIN_DECLS" );
    lexer->addSkipWord( "G_END_DECLS" );
    lexer->addSkipWord( "G_GNUC_CONST" );
    lexer->addSkipWord( "G_CONST_RETURN" );
    lexer->addSkipWord( "GTKMAIN_C_VAR" );
    lexer->addSkipWord( "GTKVAR" );
    lexer->addSkipWord( "GDKVAR" );
    lexer->addSkipWord( "G_GNUC_PRINTF", SkipWordAndArguments );

    // windows
    lexer->addSkipWord( "WINAPI" );
    lexer->addSkipWord( "__declspec", SkipWordAndArguments );
    lexer->addSkipWord( "__stdcall" );
    lexer->addSkipWord( "__cdecl" );
    lexer->addSkipWord( "_cdecl" );
    lexer->addSkipWord( "CALLBACK" );
}

void Driver::setupParser( Parser * parser )
{
    Q_UNUSED( parser );
}
