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
#include <klocale.h>
#include <stdlib.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qdir.h>

class DefaultSourceProvider: public SourceProvider
{
public:
    DefaultSourceProvider() {}

    virtual QString contents( const QString& fileName )
    {
	QString source;

	QFile f( fileName );
	if( f.open(IO_ReadOnly) ){
	    QTextStream s( &f );
	    source = s.read();
	    f.close();
	}
	return source;
    }

    virtual bool isModified( const QString& fileName )
    {
	Q_UNUSED( fileName );
	return true;
    }

private:
    DefaultSourceProvider( const DefaultSourceProvider& source );
    void operator = ( const DefaultSourceProvider& source );
};


Driver::Driver()
    : depresolv( FALSE ), lexer( 0 )
{
    m_sourceProvider = new DefaultSourceProvider();
}

Driver::~Driver()
{
    reset();
    delete( m_sourceProvider );
}

SourceProvider* Driver::sourceProvider()
{
    return m_sourceProvider;
}

void Driver::setSourceProvider( SourceProvider* sourceProvider )
{
    if( m_sourceProvider )
	delete( m_sourceProvider );
    m_sourceProvider = sourceProvider;
}

void Driver::reset( )
{
    m_dependences.clear();
    m_macros.clear();
    m_problems.clear();
    m_includePaths.clear();

    while( m_parsedUnits.size() ){
	TranslationUnitAST* unit = *m_parsedUnits.begin();
	m_parsedUnits.remove( m_parsedUnits.begin() );
	delete( unit );
    }
}

void Driver::remove( const QString & fileName )
{
    m_dependences.remove( fileName );
    m_problems.remove( fileName );
    removeAllMacrosInFile( fileName );

    QMap<QString, TranslationUnitAST*>::Iterator it = m_parsedUnits.find( fileName );
    if( it != m_parsedUnits.end() ){
	TranslationUnitAST* unit = *it;
	m_parsedUnits.remove( it );
	delete( unit );
    }
}

void Driver::removeAllMacrosInFile( const QString& fileName )
{
    QMap<QString, Macro>::Iterator it = m_macros.begin();
    while( it != m_macros.end() ){
        Macro m = *it++;
        if( m.fileName() == fileName )
            removeMacro( m.name() );
    }
}

TranslationUnitAST::Node Driver::takeTranslationUnit( const QString& fileName )
{
    QMap<QString, TranslationUnitAST*>::Iterator it = m_parsedUnits.find( fileName );
    TranslationUnitAST::Node unit( *it );
    //m_parsedUnits.remove( it );
    m_parsedUnits[ fileName] = 0;
    return unit;
}

TranslationUnitAST* Driver::translationUnit( const QString& fileName ) const
{
    QMap<QString, TranslationUnitAST*>::ConstIterator it = m_parsedUnits.find( fileName );
    return it != m_parsedUnits.end() ? *it : 0;
}

void Driver::addDependence( const QString & fileName, const Dependence & dep )
{
    QFileInfo fileInfo( dep.first );
    QString fn = fileInfo.absFilePath();

    if ( !depresolv ){
        findOrInsertDependenceList( fileName ).insert( fn, dep );
	return;
    }

    QString file = findIncludeFile( dep );
    findOrInsertDependenceList( fileName ).insert( file, dep );

    if ( m_parsedUnits.find(file) != m_parsedUnits.end() )
	return;

    if ( !QFile::exists( file ) ) {
	Problem p( i18n( "Couldn't find include file %1" ).arg( dep.first ),
		   lexer ? lexer->currentLine() : -1,
		   lexer ? lexer->currentColumn() : -1 );
	addProblem( fileName, p );
	return;
    }

    QString cfn = m_currentFileName;
    Lexer *l = lexer;
    parseFile( file );
    m_currentFileName = cfn;
    lexer = l;
}

void Driver::addMacro( const Macro & macro )
{
    m_macros.insert( macro.name(), macro );
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

QMap< QString, Macro > Driver::macros() const
{
    return m_macros;
}

QValueList < Problem > Driver::problems( const QString & fileName ) const
{
    QMap<QString, QValueList<Problem> >::ConstIterator it = m_problems.find( fileName );
    if( it != m_problems.end() )
	return it.data();
    return QValueList<Problem>();
}

void Driver::parseFile( const QString& fileName, bool onlyPreProcess, bool force )
{
    QFileInfo fileInfo( fileName );
    QString absFilePath = fileInfo.absFilePath();

    QMap<QString, TranslationUnitAST*>::Iterator it = m_parsedUnits.find( absFilePath );

    if( force && it != m_parsedUnits.end() ){
	takeTranslationUnit( absFilePath );
    } else if( it != m_parsedUnits.end() && *it != 0 ){
	// file already processed
	return;
    }

    m_dependences.remove( fileName );
    m_problems.remove( fileName );

    m_currentFileName = fileName;

    Lexer lex( this );
    lexer = &lex;
    setupLexer( &lex );

    lex.setSource( sourceProvider()->contents(fileName) );

    if( !onlyPreProcess ){
	Parser parser( this, &lex );
	setupParser( &parser );

        TranslationUnitAST :: Node translationUnit;
	parser.parseTranslationUnit( translationUnit );
        m_parsedUnits.insert( fileName, translationUnit.release() );
        fileParsed( fileName );
    }

    m_currentFileName = QString::null;
    lexer = 0;
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
    lexer->addSkipWord( "__STL_DEFAULT_ALLOCATOR",  SkipWordAndArguments, "T" );
    lexer->addSkipWord( "__STL_MUTEX_INITIALIZER" );
    lexer->addSkipWord( "__STL_NULL_TMPL_ARGS" );

    // antlr
    lexer->addSkipWord( "ANTLR_BEGIN_NAMESPACE", SkipWordAndArguments );
    lexer->addSkipWord( "ANTLR_USE_NAMESPACE", SkipWordAndArguments );
    lexer->addSkipWord( "ANTLR_USING_NAMESPACE", SkipWordAndArguments );
    lexer->addSkipWord( "ANTLR_END_NAMESPACE" );
    lexer->addSkipWord( "ANTLR_C_USING", SkipWordAndArguments );
    lexer->addSkipWord( "ANTLR_API" );

    // gnu
    lexer->addSkipWord( "__extension__", SkipWordAndArguments );
    lexer->addSkipWord( "__attribute__", SkipWordAndArguments );
    lexer->addSkipWord( "__BEGIN_DECLS" );
    lexer->addSkipWord( "__END_DECLS" );
    lexer->addSkipWord( "__THROW" );
    lexer->addSkipWord( "__restrict" );
    lexer->addSkipWord( "__restrict__" );
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
    lexer->addSkipWord( "__glibcpp_class_requires", SkipWordAndArguments );
    lexer->addSkipWord( "__glibcpp_class2_requires", SkipWordAndArguments );
    lexer->addSkipWord( "__glibcpp_class4_requires", SkipWordAndArguments );
    lexer->addSkipWord( "__glibcpp_function_requires", SkipWordAndArguments );
    lexer->addSkipWord( "restrict" );

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
    lexer->addSkipWord( "KDE_DEPRECATED" );

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
    lexer->addSkipWord( "__stdcall" );
    lexer->addSkipWord( "__cdecl" );
    lexer->addSkipWord( "_cdecl" );
    lexer->addSkipWord( "CALLBACK" );

   // gcc extensions
    addMacro( Macro("__asm__", "asm") );
    addMacro( Macro("__inline", "inline") );
    addMacro( Macro("__inline__", "inline") );
    addMacro( Macro("__const", "const") );
    addMacro( Macro("__const__", "const") );
    addMacro( Macro("__volatile__", "volatile") );
    addMacro( Macro("__complex__", "") );
}

void Driver::setupParser( Parser * parser )
{
    Q_UNUSED( parser );
}

void Driver::removeMacro( const QString& macroName )
{
    m_macros.remove( macroName );
}

void Driver::addIncludePath( const QString &path )
{
    if( !path.stripWhiteSpace().isEmpty() )
        m_includePaths << path;
}

QString Driver::findIncludeFile( const Dependence& dep ) const
{
    QString fileName = dep.first;

    if( dep.second == Dep_Local ){
        QString path = QFileInfo( currentFileName() ).dirPath( true );
        QFileInfo fileInfo( QFileInfo(path, fileName) );
	if ( fileInfo.exists() && fileInfo.isFile() )
	    return fileInfo.absFilePath();

    }

    for ( QStringList::ConstIterator it = m_includePaths.begin(); it != m_includePaths.end(); ++it ) {
        QFileInfo fileInfo( *it, fileName );
	if ( fileInfo.exists() && fileInfo.isFile() )
	    return fileInfo.absFilePath();
    }

    return QString::null;
}

void Driver::setResolveDependencesEnabled( bool enabled )
{
    depresolv = enabled;
    if ( depresolv )
	setupPreProcessor();
}

void Driver::setupPreProcessor()
{
}

void Driver::fileParsed( const QString & fileName )
{
    Q_UNUSED( fileName );
}
