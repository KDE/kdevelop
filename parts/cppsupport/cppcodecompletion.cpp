/***************************************************************************
                          cppcodecompletion.cpp  -  description
                             -------------------
    begin                : Sat Jul 21 2001
    copyright            : (C) 2001 by Victor Röder
    email                : victor_roeder@gmx.de
    copyright            : (C) 2002 by Roberto Raggi
    email                : roberto@kdevelop.org
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
#include "kdevregexp.h"
#include "backgroundparser.h"
#include "ast.h"
#include "ast_utils.h"

#include <kdebug.h>
#include <klocale.h>
#include <kmainwindow.h>
#include <kmessagebox.h>
#include <kparts/part.h>
#include <kregexp.h>
#include <kstatusbar.h>
#include <ktempfile.h>
#include <ktexteditor/document.h>

#include <qdatastream.h>
#include <qfile.h>
#include <qmap.h>
#include <qregexp.h>
#include <qstatusbar.h>
#include <qstring.h>
#include <qstringlist.h>

#include "kdevpartcontroller.h"
#include "kdevmainwindow.h"
#include "kdevcore.h"
#include "classstore.h"
#include "parsedscopecontainer.h"

static QValueList<KTextEditor::CompletionEntry>
unique( const QValueList<KTextEditor::CompletionEntry>& entryList )
{
    QValueList< KTextEditor::CompletionEntry > l;
    QMap<QString, bool> map;
    QValueList< KTextEditor::CompletionEntry >::ConstIterator it=entryList.begin();
    while( it != entryList.end() ){
        KTextEditor::CompletionEntry e = *it++;
        QString key = e.type + " " +
                      e.text + " " +
                      e.prefix + " " +
                      e.postfix + " ";
        if( map.find(key) == map.end() ){
            map[ key ] = TRUE;
            l << e;
        }
    }
    return l;
}

static QString remove( QString text, const QChar& l, const QChar& r )
{
    QString s;

    unsigned int index = 0;
    int count = 0;
    while( index < text.length() ){
        if( text[index] == l ){
            ++count;
        } else if( text[index] == r ){
            --count;
        } else if( count == 0 ){
            s += text[ index ];
        }
        ++index;
    }
    return s;
}

static QString remove_keywords( QString text ){
    QRegExp ide_rx( "[_a-zA-Z0-9][_a-zA-Z0-9]*" );
    QStringList keywords = QStringList::split( "|",
            "unsigned|signed|case|delete|return|if|then|else|for|while|do|"
            "const|static|volatile|extern|struct" );

    QString s;
    int index = 0;
    while( index < (int)text.length() ){
        int len = 0;
        int pos = ide_rx.match( text, index, &len );
        if( pos == -1 ){
            break;
        }
        s += text.mid( index, pos - index );
        QString ide = text.mid( pos, len );
        if( keywords.findIndex(ide) == -1 ){
            s += ide;
        }
        index = pos + ide.length();
    }
    s += text.mid( index );
    return s;
}

static QString remove_comment( QString text ){
    QString s;
    unsigned int index = 0;
    bool skip = FALSE;
    while( index < text.length() ){
        if( text.mid(index, 2) == "/*" ){
            skip = TRUE;
            index += 2;
            continue;
        } else if( text.mid(index, 2) == "*/" ){
            skip = FALSE;
            index += 2;
            continue;
        } else if( !skip ){
            s += text[ index ];
        }
        ++index;
    }
    return s;
}

static QString purify( const QString& decl )
{
    QString s = decl;

    QRegExp rx1( "\\*" );
    QRegExp rx2( "&" );
    s = s.replace( rx1, "" ).replace( rx2, "" );
    s = remove_keywords( s );
    s = remove( s, '[', ']' );
    s = s.simplifyWhiteSpace();

    return s;
}


CppCodeCompletion::CppCodeCompletion( CppSupportPart* part, ClassStore* pStore, ClassStore* pCCStore )
{
    m_pSupport = part;
    m_pCore    = part->core( );
    m_pStore   = pStore;
    m_pCCStore = pCCStore;

    m_pTmpFile     = 0;
#ifdef DANIEL_CC
    m_pParser      = 0;
#endif
    m_pCursorIface = 0;
    m_pEditIface   = 0;
    m_pCompletionIface = 0;
    m_ccTimer = new QTimer( this );
    connect( m_ccTimer, SIGNAL(timeout()), this, SLOT(completeText()) );

    m_bArgHintShow       = false;
    m_bCompletionBoxShow = false;
    
    connect( part->partController( ), SIGNAL( activePartChanged( KParts::Part* ) ),
	     this, SLOT( slotActivePartChanged( KParts::Part* ) ) );
    
    connect( part, SIGNAL(fileParsed(const QString&)), this, SLOT(slotFileParsed(const QString&)) );
}

CppCodeCompletion::~CppCodeCompletion( )
{
#ifdef DANIEL_CC
    delete m_pParser;
#endif
    delete m_pTmpFile;
}

void
CppCodeCompletion::slotArgHintHided( )
{
    m_bArgHintShow = false;
}

void
CppCodeCompletion::setEnableCodeCompletion( bool setEnable )
{
    m_bCodeCompletion = setEnable;
}

void
CppCodeCompletion::slotCompletionBoxHided( KTextEditor::CompletionEntry entry )
{
    // we use the QString type in entry for this
    if( m_pSupport && m_pSupport->getCHWidget( ) ) {
        m_pSupport->getCHWidget( )->setCHText( m_CHCommentList[ entry.type.toInt( ) ] );
    }
    m_CHCommentList.clear( );

    m_bCompletionBoxShow = false;
}


void
CppCodeCompletion::slotActivePartChanged(KParts::Part *part)
{
    kdDebug( 9007 ) << "CppCodeCompletion::slotActivePartChanged()" << endl;

    if( !part )
      return;

    m_currentFileName = QString::null;
    
    KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>( part );
    if( !doc )
	return;
    
    m_currentFileName = doc->url().path();
    
    // if the interface stuff fails we should disable codecompletion automatically
    m_pEditIface = dynamic_cast<KTextEditor::EditInterface*>(part);
    if( !m_pEditIface ){
        kdDebug( 9007 ) << "Editor doesn't support the EditDocumentIface" << endl;
        return;
    }

    m_pCursorIface = dynamic_cast<KTextEditor::ViewCursorInterface*>(part->widget());
    if( !m_pCursorIface ){
        kdDebug( 9007 ) << "The editor doesn't support the CursorDocumentIface!" << endl;
        return;
    }

    m_pCompletionIface = dynamic_cast<KTextEditor::CodeCompletionInterface*>(part->widget());
    if( !m_pCompletionIface ){
        kdDebug( 9007 ) << "Editor doesn't support the CompletionIface" << endl;
        return;
    }
    
    // here we have to investigate :)
    if( m_pSupport && m_pSupport->getEnableCC( ) == true ){
        kdDebug( 9007 ) << "enabling code completion" << endl;
/*
	QObject::connect(part->widget(), SIGNAL( cursorPositionChanged() ), this,
                 SLOT( slotCursorPositionChanged() ) );
*/
	QObject::connect(part, SIGNAL(charactersInteractivelyInserted(int,int,const QString&)),
		this, SLOT(slotTextChanged( int, int, const QString& ) ) );

/*
        connect( m_pCompletionIface, SIGNAL( argHintHided( ) ), this,
                 SLOT( slotArgHintHided( ) ) );
*/
	QObject::connect(part->widget(), SIGNAL( completionDone( KTextEditor::CompletionEntry ) ), this,
                 SLOT( slotCompletionBoxHided( KTextEditor::CompletionEntry ) ) );
    }
    
    kdDebug(9007) << "CppCodeCompletion::slotActivePartChanged() -- end" << endl;
}

void
CppCodeCompletion::slotCursorPositionChanged()
{
    kdDebug(9007) << "Cursor position changed" << endl;
    
    if( !m_pSupport )
	return;

    uint nLine, nCol;
    m_pCursorIface->cursorPosition(&nLine, &nCol);
    QString text = typingTypeOf( nLine, nCol );
    if( !text.isEmpty( ) )
	m_pSupport->mainWindow()->statusBar()->message( text, 1000 );
}

QString
CppCodeCompletion::typingTypeOf( int nLine, int nCol )
{
    kdDebug(9007) << "CppCodeCompletion::typingTypeOf( )" << endl;

#if 0
    QString strCurLine = m_pEditIface->textLine( nLine );
    QValueList<KTextEditor::CompletionEntry> entries;

    QString className;
    QString contents = getMethodBody( nLine, nCol, &className );
    kdDebug(9007) << "contents = " << contents << endl;

    SimpleVariable = SimpleParser::localVariables( contents );
    SimpleVariable v;
    v.name = "this";
    v.scope = 1;
    v.type = className;
    variableList.append( v );

    int start_expr = expressionAt( contents, contents.length() - 1 );
    QString expr;
    if( start_expr != (int)contents.length() - 1 ){
        expr = contents.mid( start_expr, contents.length() - start_expr );
        expr = expr.simplifyWhiteSpace();
    }

    kdDebug(9007) << "expr = |" << expr << "|" << endl;

    QString type = evaluateExpression( expr, variableList, m_pStore );

    kdDebug(9007) << "the type of expression is " << type << endl;

    return ( type.isEmpty() ? QString( "" ) : type );
#endif
    return "";
}


void
CppCodeCompletion::slotTextChanged( int nLine, int nCol, const QString& /*text*/ )
{
#ifdef DANIEL_CC
    QString strCurLine = m_pEditIface->textLine( nLine );

    // should be done once and destroyed by destructor, shouldn't it ?
    if( !m_pParser ) m_pParser = new CppCCParser( );
    

    // we use more than once
    int nNodePos = getNodePos( nLine, nCol );
    // daniel - avoid additional scope in if's
    if( !nNodePos ){
	kdDebug(9007) << "nNodePos = 0" << endl;
        return;
    }

    // CC for "Namespace::" or "Class::" for example
    // each case could be an extra method so the source is more readable
    if( strCurLine.right( 2 ) == "::" ){
        QString strNodeText = getNodeText( nNodePos, nLine );
        QValueList< KTextEditor::CompletionEntry > completionList;

        completionList = getEntryListForNamespace( strNodeText );
        if( completionList.count( ) > 0 ){
            kdDebug( 9007 ) << "found namespace '" << strNodeText << "' in classstore" << endl;
            m_pCompletionIface->showCompletionBox( completionList );
            return;
        }

        // no entry in namespace was found so we are looking for static things
        completionList = getEntryListForClass( strNodeText );
        if ( completionList.count( ) > 0 ){
            kdDebug( 9007 ) << "found class '" << strNodeText << "' in classstore" << endl;
            m_pCompletionIface->showCompletionBox( completionList );
            // seems here is missing something ?
            // QStringList functionList;
            return;
        }

        // maybe the class is within a namespace
        completionList = getEntryListForClassOfNamespace( strNodeText, getNodeText( ( getNodePos( nLine, nCol ) - 1 ), nLine ) );
        if( completionList.count( ) > 0 ){
            kdDebug( 9007 ) << "found class '" << strNodeText << "' in namespace" << endl;
            m_pCompletionIface->showCompletionBox( completionList );
            // seems here is missing something
            // QStringList functionList; ?
            return;
        }
    }

    /* daniel - arghint section deleted */

    // CC for "normal" objects
    // couldn't we use it for "->" as well ?
    if( strCurLine.right( 1 ) == "." ){
        QString strNodeText = getNodeText ( nNodePos, nLine );

        // should always be 0 !
        if( m_pTmpFile )
            delete m_pTmpFile;
        m_pTmpFile = new KTempFile( );

        // no warning to user / log ?
        if( m_pTmpFile->status( ) != 0 )
            return;

        QString strFileName = createTmpFileForParser( nLine );

        // new method that only searches for variables with name strNodeText
        if( m_pParser )
            m_pParser->parse ( strFileName, strNodeText );
        else
            return;

        // for debugging: keep file - m_pTmpFile->unlink( );
        if( m_pTmpFile->status( ) != 0 )
            return;

        // local var declaration not found - trying member completion
        if( !m_pParser->variableList.count( ) ){
            ParsedClass* pClass = m_pStore->getClassByName( m_currentClassName );

            // found absolutely nothing
            if( !pClass ){
                kdDebug( 9007 ) << "didn't find class '" << m_currentClassName << "' in store" << endl;
                return;
            }

            ParsedAttribute* pAttr = pClass->getAttributeByName( strNodeText );
	    if( !pAttr )
		return;
            QString type = pAttr->type( );
            // classstore does it that way: type = "QPrinter *"
            // is there always a space between class and type ?
            type.remove( type.find( " " ), 999 );
            kdDebug( 9007 ) << "attribute type is '" << type << "'" << endl;

            QValueList< KTextEditor::CompletionEntry > completionList;
            completionList = getEntryListForClass( type );
            if( completionList.count( ) > 0 )
                m_pCompletionIface->showCompletionBox( completionList );
            else
                kdDebug( 9007 ) << "completionList.count( ) = 0 )" << endl;
            return;
        }

        // original code
        for( CParsedVariable* pVar = m_pParser->variableList.first( ); pVar != 0;
             pVar = m_pParser->variableList.next( ) ){
            if( pVar->sVariableName == strNodeText ){
                kdDebug( 9007 ) << "Type of variable: '" << pVar->sVariableType << "'" << endl;
                QValueList< KTextEditor::CompletionEntry > completionList;
                completionList = getEntryListForClass( pVar->sVariableType );

                if( completionList.count( ) > 0 )
                    m_pCompletionIface->showCompletionBox( completionList );
                else
                    kdDebug( 9007 ) << "completionList.count( ) = 0 )" << endl;

                // break for because we (didn't) found what we looked for
                break;
            }

        }

        // remove the parsed variables from CppCCParser!
        m_pParser->variableList.clear( );
    }
#else
    m_ccTimer->stop();
    
    QString strCurLine = m_pEditIface->textLine( nLine );
    QString ch = strCurLine.mid( nCol, 1 );
    QString ch2 = strCurLine.mid( nCol-1, 2 );
    
    //kdDebug(9007) << "ch = " << ch << " -- ch2 = " << ch2 << endl;

    if ( ch == "." || ch2 == "->" ){
    	m_ccTimer->start( 500, true );
    }    
#endif
}

/**** TODO: replace this method with a parsing mechanism - very buggy! ****/
QString
CppCodeCompletion::createTmpFileForParser( int iLine )
{
    // regular expression for matching a method implementation
    // looks weird but maintaining is much easier :)
    QString reg  = "([\\s\t]*[\\w\\d_]+[\\*|\\&]?"; 	// return value
            reg += "[\\s\t]*([\\w\\d_]+)::[\\w\\d_]+";	// method class and -name
	    reg += "\\([\\w\\d\\&\\*\\s\t,_]*\\)"; 	// parameters
//	    reg += "[\\s\t]*[:([\\s\t\\w\\d_])+]?[\\{]?)" ; // base class - not finished yet :)
//	    reg += "[\\s\t]*[^;])";			// avoid static method calls

    QRegExp regMethod( reg );
    QString strLine;
    int     iMethodBegin = 0;

    for( int i = iLine; i > 0; i-- ){
        strLine = m_pEditIface->textLine( i );
	kdDebug( 9007 ) << "checking line: " << strLine << endl;

        if( regMethod.search( strLine ) > -1 ){
            iMethodBegin = i;

            kdDebug( 9007 ) << "method begins @ line '" << iMethodBegin << "'" << endl;
            // test to figure out the current classname
            m_currentClassName = regMethod.cap( 2 );
            kdDebug( 9007 ) << "method's classname is '" << m_currentClassName << "'" << endl;
            break;
        }
    }

    if( iMethodBegin == 0 ){
        kdDebug( 9007 ) << "no method declaration found" << endl;
        return QString::null;
    }

    QString strCopy;
    for( int i = iMethodBegin; i < iLine; i++ ){
        strCopy += m_pEditIface->textLine( i ) + "\n";
    }

    QFile *pFile = m_pTmpFile->file( );
    pFile->writeBlock( strCopy.latin1( ), strCopy.length( ) );
    pFile->flush( );

    kdDebug( 9007 ) << "name of tempfile '" << m_pTmpFile->name( ) << "'" << endl;

    return m_pTmpFile->name( );
}


/**** Here begins some "parsing" stuff - to be replaced by a real parser ****/

// seems to be okay
QString
CppCodeCompletion::getCompletionText( int nLine, int nCol )
{
    int nOffset = nCol;

    QString strCurLine = m_pEditIface->textLine ( nLine );

    while( nOffset > 0 ){
        if ( strCurLine[ nOffset - 1] == '-'  && strCurLine[ nOffset ] == '>' ||
             strCurLine[ nOffset - 1] == ':'  && strCurLine[ nOffset ] == ':' ||
             strCurLine[ nOffset ] == '.'     ||
             strCurLine[ nOffset ] == ' '     || strCurLine[ nOffset ] == ';' ||
             strCurLine[ nOffset ] == '\t'    || strCurLine[ nOffset ] == '}')
        {
            nOffset++;
            break;
        }
        else {
            nOffset--;
        }

        if( strCurLine[ nOffset ] == ':' && strCurLine[ nOffset - 1 ] != ':' ||
            strCurLine[ nOffset ] == '-' && strCurLine[ nOffset - 1 ] != '>' )
        {
            return QString::null;
        }
    }

    if( ( nCol - nOffset ) >= 0 )
        return( strCurLine.mid( nOffset, ( nCol - nOffset ) ) );

    return QString::null;
}

// seems to be okay
int
CppCodeCompletion::getNodePos( int nLine, int nCol )
{
    int nOffset  = 0;
    int nNodePos = 0;

    QString strCurLine = m_pEditIface->textLine( nLine );
    kdDebug( 9007 ) << "getNodePos( int nLine = " << nLine << ", int nCol = " << nCol << " )" << endl;
    kdDebug( 9007 ) << "currentLine '" << endl << strCurLine << endl << "'" << endl;

    // changed from < to <= ; new KTextEditor that starts with col 0
    while( nOffset <= nCol ){
        if ( strCurLine[ nOffset ] == '.' ||
             strCurLine[ nOffset ] == '-' && strCurLine[ nOffset + 1 ] == '>' ||
             strCurLine[ nOffset ] == ':' && strCurLine[ nOffset + 1 ] == ':' )
        {
            nNodePos++;
        }

        nOffset++;
    }

    return nNodePos;
}

// seems to be okay
QString
CppCodeCompletion::getNodeText( int nNode, int nLine )
{
    if( nNode <= 0 )
        return QString::null;

    int nNodePos       = 0;
    int nFrom          = 0;
    unsigned int nTo   = 0; // avoid compiler warnings
    QString strCurLine = m_pEditIface->textLine( nLine );

    while( nTo < strCurLine.length( ) ){
        if( strCurLine[ nTo ] == '.' ){
            nNodePos++;

            if( nNodePos < nNode )
                nFrom = nTo + 1;
        }

        if( strCurLine[ nTo ] == '-' && strCurLine[ nTo + 1 ] == '>' ||
            strCurLine[ nTo ] == ':' && strCurLine[ nTo + 1 ] == ':' )
        {
            nNodePos++;
            if( nNodePos < nNode )
                nFrom = nTo + 2;
        }

        if( nNodePos == nNode ){
            for( nTo = nFrom; nTo < strCurLine.length( ); nTo++ ){
                if( strCurLine[ nTo ] == '.' ){
                    if( nFrom == 0 ){
                        for( nFrom = nTo -1; nFrom > 0; --nFrom ){
                            if ( strCurLine[ nFrom ] == ' ' || strCurLine[ nFrom ] == '\t' ||
                                 strCurLine[ nFrom ] == '}' || strCurLine[ nFrom ] == ';'  ||
                                 strCurLine[ nFrom ] == ':' && strCurLine[ nFrom - 1 ] == ':' ||
                                 strCurLine[ nFrom ] == '>' && strCurLine[ nFrom - 1 ] == '-' ||
                                 strCurLine[ nFrom ] == '.')
                            {
                                nFrom++;
                                break;
                            }
                        }
                    }
                    return strCurLine.mid( nFrom, ( nTo - nFrom ) );
                }

                if( strCurLine[ nTo ] == '-' && strCurLine[ nTo + 1 ] == '>' ||
                    strCurLine[ nTo ] == ':' && strCurLine[ nTo + 1 ] == ':' ){
                    if( nFrom == 0 ){
                        for( nFrom = nTo; nFrom > 0; --nFrom ){
                            if( strCurLine[ nFrom ] == ' ' || strCurLine[ nFrom ] == '\t' ||
                                strCurLine[ nFrom ] == '}' || strCurLine[ nFrom ] == ';'  ||
                                strCurLine[ nFrom ] == ':' && strCurLine[ nFrom - 1 ] == ':' ||
                                strCurLine[ nFrom ] == '>' && strCurLine[ nFrom - 1 ] == '-' ||
                                strCurLine[ nFrom ] == '.' )
                            {
                                nFrom++;
                                break;
                            }
                        }
                    }
                    return strCurLine.mid( nFrom, ( nTo - nFrom ) );
                }
            }
        }
        nTo++;
    }

    return QString::null;
}

// seems to be ok ?
QString
CppCodeCompletion::getNodeDelimiter( int nNode, int nLine )
{
    if( nNode <= 0 )
        return QString::null;

    QString strCurLine = m_pEditIface->textLine( nLine );

    int nNodePos     = 0;
    int nFrom        = 0;
    unsigned int nTo = 0; // avoid compiler warnings

    while( nTo < strCurLine.length( ) ){
        if( strCurLine[ nTo ] == '.' ||
            strCurLine[ nTo ] == '-' && strCurLine[ nTo + 1 ] == '>' ||
            strCurLine[ nTo ] == ':' && strCurLine[ nTo + 1 ] == ':' )
        {
            nNodePos++;
            if( nNodePos < nNode )
                nFrom = nTo + 1;
        }

        if( nNodePos == nNode ){
            for( nTo = nFrom; nTo < strCurLine.length( ); nTo++ ){
                if( strCurLine[ nTo ] == '.' ){
                    return strCurLine.mid( nTo, 1 );
                }

                if( strCurLine[ nTo ] == '-' && strCurLine[ nTo + 1 ] == '>' ||
                    strCurLine[ nTo ] == ':' && strCurLine[ nTo + 1 ] == ':' )
                {
                    return strCurLine.mid( nTo, 2 );
                }
            }
        }
        nTo++;
    }

    return QString::null;
}

/**** Here begin some ClassStore queries - I think they are nearly stable */
/**** (expected that nothing else is mentioned) ****/

QValueList< KTextEditor::CompletionEntry >
CppCodeCompletion::getEntryListForClass( QString strClass )
{
    QValueList< KTextEditor::CompletionEntry > entryList;

    // first we look into the cc-classstore and then in project-classstore
    ParsedClass* pClass = m_pCCStore->getClassByName( strClass );
    if( !pClass )
        pClass = m_pStore->getClassByName( strClass );

    // found absolutely nothing
    if( !pClass ){
        kdDebug( 9007 ) << "getEntryListForClass( '" << strClass << "' ) not found" << endl;
        return entryList;
    }

    // Load the methods, slots, signals of the current class and its parents into the list
    QValueList<ParsedMethod*> methodList = getMethodListForClassAndAncestors( pClass );
    QValueList<ParsedMethod*>::ConstIterator methodIt;

    // create the completion list
    int i = 0;
    for (methodIt = methodList.begin(); methodIt != methodList.end(); ++methodIt) {
        KTextEditor::CompletionEntry entry;

        // we should decide if return-types have to be shown and possibly truncate them
        // ToDo: should it be configurable ? maybe showing the return-type or not ?
        // is not that tricky :)
        if( (*methodIt)->type( ).length( ) > 7 )
            entry.prefix = (*methodIt)->type( ).left( 4 ) + "...";
        else
            entry.prefix = (*methodIt)->type( );

        entry.text   = (*methodIt)->name( ) + "(";

        // creating postfix-text (attributes) which is not displayed when a selection was made
        QString text;
        for( ParsedArgument *pArg = (*methodIt)->arguments.first( );
             pArg != 0;
             pArg = (*methodIt)->arguments.next( ) ){
            if( pArg != (*methodIt)->arguments.getFirst( ) )
                text += ", ";
            text += pArg->toString( );
        }
        text += ")";
        entry.postfix = text;
	entry.type.setNum( i++ );
	m_CHCommentList.append( (*methodIt)->comment( ) );

        entryList << entry;
    }

    // Load the attributes of the current class and its parents into the list
    QValueList<ParsedAttribute*> attrList = getAttributeListForClassAndAncestors( pClass );
    QValueList<ParsedAttribute*>::ConstIterator attrIt;

    // trying how it looks like - symbol needed ?
    KTextEditor::CompletionEntry entry;
    entry.text = "--- attributes";
    entryList << entry;
    for (attrIt = attrList.begin(); attrIt != attrList.end(); ++attrIt) {
        KTextEditor::CompletionEntry entry;
        entry.text = (*attrIt)->name( );
        entry.postfix = "";
	m_CHCommentList.append( (*attrIt)->name( ) );
        entryList << entry;
    }

    return entryList;
}

QValueList< KTextEditor::CompletionEntry >
CppCodeCompletion::getEntryListForClassOfNamespace( QString strClass, const QString& strNamespace )
{
    QValueList< KTextEditor::CompletionEntry > entryList;

    ParsedScopeContainer* pScope = m_pCCStore->getScopeByName ( strNamespace );
    if( pScope ){
        ParsedClass* pClass = pScope->getClassByName( strClass );
        if ( pClass ){
            // Load the methods, slots, signals of the current class and its parents into the list
            QValueList<ParsedMethod*> methodList = getMethodListForClassAndAncestors( pClass );

            QValueList<ParsedMethod*>::ConstIterator methodIt;
            for (methodIt = methodList.begin(); methodIt != methodList.end(); ++methodIt) {
                if( (*methodIt)->isStatic( ) ) {
                    KTextEditor::CompletionEntry entry;
                    entry.text = (*methodIt)->name();
                    m_CHCommentList.append( (*methodIt)->comment( ) );
                    entry.postfix = "()";
                    entryList << entry;
                } else
                    kdDebug( 9007 ) << "rejecting: '" << (*methodIt)->name( ) << "'" << endl;
            }

            KTextEditor::CompletionEntry entry;
            entry.text = "--- Attributes";
            entryList << entry;

            // Load the attributes of the current class and its parents into the list

            QValueList<ParsedAttribute*> attrList = getAttributeListForClassAndAncestors( pClass );

            QValueList<ParsedAttribute*>::ConstIterator attrIt;
            for (attrIt = attrList.begin(); attrIt != attrList.end(); ++attrIt) {
                if( (*attrIt)->isStatic( ) ){
                    KTextEditor::CompletionEntry entry;
                    entry.prefix = (*attrIt)->type( );
                    entry.text = (*attrIt)->name();
		    m_CHCommentList.append( (*attrIt)->comment( ) );
                    // needed ? entry.postfix = "";
                    entryList << entry;
                }
                else
                    kdDebug( 9007 ) << "rejecting: '" << (*attrIt)->name( ) << "'" << endl;
            }
        }
    }

    return entryList;
}

QValueList< KTextEditor::CompletionEntry >
CppCodeCompletion::getEntryListForNamespace( const QString& strNamespace )
{
    kdDebug( 9007 ) << "getEntryListForNamespace starts with '" << strNamespace << "'" << endl;

    QValueList< KTextEditor::CompletionEntry > entryList;
    ParsedScopeContainer* pScope = m_pCCStore->getScopeByName( strNamespace );

    if( pScope ){
	QValueList<ParsedClass*> classList = pScope->getSortedClassList( );

        QValueList<ParsedClass*>::ConstIterator classIt;
        for (classIt = classList.begin(); classIt != classList.end(); ++classIt) {
	    KTextEditor::CompletionEntry entry;
	    entry.text = (*classIt)->name( );
	    m_CHCommentList.append( (*classIt)->comment( ) );
	    // needed ? entry.postfix = "";
	    entryList << entry;
	}
    }

    return entryList;
}

QValueList< KTextEditor::CompletionEntry >
CppCodeCompletion::getEntryListForStruct( const QString& strStruct )
{
    QValueList< KTextEditor::CompletionEntry > entryList;

    // FIXME: is this right for namespaces?
    // or should it be store->getStructByName()?
    ParsedScopeContainer* pScope = m_pCCStore->globalScope();
    if( pScope ){

        ParsedStruct* pStruct = pScope->getStructByName( strStruct );
        if ( pStruct ){
            QValueList<ParsedAttribute*> attrList = pStruct->getSortedAttributeList();
            QValueList<ParsedAttribute*>::ConstIterator attrIt;

            for (attrIt = attrList.begin(); attrIt != attrList.end(); ++attrIt) {
                KTextEditor::CompletionEntry entry;
                entry.text = (*attrIt)->name( );
		m_CHCommentList.append( (*attrIt)->comment( ) );
                // needed ? entry.postfix = "";
                entryList << entry;
            }
        }
    }

    return entryList;
}


QString
CppCodeCompletion::getMethodBody( int iLine, int iCol, QString* classname )
{
    kdDebug(9007) << "CppCodeCompletion::getMethodBody()" << endl;

    // TODO: add support for function pointer arguments
    KDevRegExp regMethod( "[ \t]*([a-zA-Z0-9_]+[ \t]*::)?[ \t]*[~a-zA-Z0-9_][a-zA-Z0-9_]*[ \t]*\\(([^)]*)\\)[ \t]*(:[^{]*)?\\{" );

    //QRegExp qt_rx( "Q_[A-Z]+" );
    QRegExp strconst_rx( "\"[^\"]*\"" );
    QRegExp chrconst_rx( "'[^']*'" );
    QRegExp newline_rx( "\n" );
    QRegExp comment_rx( "//[^\n]*" );
    QRegExp preproc_rx( "^[ \t]*#[^\n]*$" );

    QString text = m_pEditIface->textLine( iLine ).left( iCol );
    --iLine;
    while( iLine >= 0 ){

        text.prepend( m_pEditIface->textLine( iLine ).simplifyWhiteSpace() + "\n" );
        if( (iLine % 50) == 0 ){
            // kdDebug(9007) << "---> iLine = " << iLine << endl;

            QString contents = text;

            //kdDebug(9007) << ".... 2 " << endl;

            contents = contents
                       // .replace( qt_rx, "" )
                       .replace( comment_rx, "" )
                       .replace( preproc_rx, "" )
                       .replace( strconst_rx, "\"\"" )
                       .replace( chrconst_rx, "''" )
                       .replace( newline_rx, " " );

            contents = remove_comment( contents );
            contents = remove_keywords( contents );
            contents = remove( contents, '[', ']' );

            //kdDebug(9007) << ".... 3 " << endl;

            QValueList<KDevRegExpCap> methods = regMethod.findAll( contents );
            if( methods.count() != 0 ){

                //kdDebug(9007) << ".... 4 " << endl;

                KDevRegExpCap m = methods.last();

                contents = contents.mid( m.start() );
                regMethod.search( m.text() );
                contents.prepend( regMethod.cap( 2 ).replace( QRegExp(","), ";" ) + ";\n" );
                //kdDebug(9007) << "-----> text = " << m.text() << endl;
                if( classname ){
                    QString s = regMethod.cap( 1 ).stripWhiteSpace();
                    if( s.length() ){
                        // remove "::"
                        s = s.left( s.length() - 2 ).stripWhiteSpace();
                    }
                    *classname = s;
                }

                return contents;
            }

            //kdDebug(9007) << ".... 5 " << endl;

        }

        --iLine;
    }

    return QString::null;
}

QValueList<KTextEditor::CompletionEntry>
CppCodeCompletion::getEntryListForExpr( const QString& expr,
                                        SimpleContext* ctx )
{
    QString type = evaluateExpression( expr, ctx, m_pStore );
    kdDebug(9007) << "--------> type = " << type << endl;
    QValueList<KTextEditor::CompletionEntry> entries = getEntryListForClass( type );
    return entries;
}

enum { T_ACCESS, T_PAREN, T_BRACKET, T_IDE, T_UNKNOWN };

int
CppCodeCompletion::expressionAt( const QString& text, int index )
{
    kdDebug(9007) << "CppCodeCompletion::expressionAt()" << endl;

    int last = T_UNKNOWN;
    int start = index;
    while( index > 0 ){
        while( index > 0 && text[index].isSpace() ){
            --index;
        }

        QChar ch = text[ index ];
        QString ch2 = text.mid( index-1, 2 );
        if( (last != T_IDE) && (ch.isLetterOrNumber() || ch == '_') ){
            while( index > 0 && (text[index].isLetterOrNumber() || text[index] == '_') ){
                --index;
            }
            last = T_IDE;
        } else if( last != T_IDE && ch == ')' ){
            int count = 0;
            while( index > 0 ){
                QChar ch = text[ index ];
                if( ch == '(' ){
                    ++count;
                } else if( ch == ')' ){
                    --count;
                } else if( count == 0 ){
                    --index;
                    last = T_PAREN;
                    break;
                }
                --index;
            }
        } else if( ch == ']' ){
            int count = 0;
            while( index > 0 ){
                QChar ch = text[ index ];
                if( ch == '[' ){
                    ++count;
                } else if( ch == ']' ){
                    --count;
                } else if( count == 0 ){
                    --index;
                    last = T_BRACKET;
                    break;
                }
                --index;
            }
        } else if( ch == '.' ){
            --index;
            last = T_ACCESS;
        } else if( ch2 == "::" ){
            index -= 2;
        } else if( ch2 == "->" ){
            index -= 2;
            last = T_ACCESS;
        } else {
            if( start > index ){
                ++index;
            }
            last = T_UNKNOWN;
            break;
        }
    }
    return index;
}

QStringList
CppCodeCompletion::splitExpression( const QString& text )
{
#define ADD_CURRENT()\
 if( current.length() ) { l << current; /*kdDebug(9007) << "add word " << current << endl;*/ current = ""; }

    QStringList l;
    int index = 0;
    QString current;
    while( index < (int)text.length() ){
        QChar ch = text[ index ];
        QString ch2 = text.mid( index, 2 );

        if( ch == '.' ){
            ADD_CURRENT();
            ++index;
        } else if( ch == '(' ){
            int count = 0;
            while( index < (int)text.length() ){
                QChar ch = text[ index ];
                if( ch == '(' ){
                    ++count;
                } else if( ch == ')' ){
                    --count;
                } else if( count == 0 ){
                    break;
                }
                current += ch;
                ++index;
            }
        } else if( ch == '[' ){
            int count = 0;
            while( index < (int)text.length() ){
                QChar ch = text[ index ];
                if( ch == '[' ){
                    ++count;
                } else if( ch == ']' ){
                    --count;
                } else if( count == 0 ){
                    break;
                }
                current += ch;
                ++index;
            }
        } else if( ch2 == "->" ){
            ADD_CURRENT();
            index += 2;
        } else {
            current += text[ index ];
            ++index;
        }
    }
    ADD_CURRENT();
    return l;
}

QString
CppCodeCompletion::evaluateExpression( const QString& expr,
                                       SimpleContext* ctx,
                                       ClassStore* sigma )
{
    QStringList exprs = splitExpression( expr );
//    for( QStringList::Iterator it=exprs.begin(); it!=exprs.end(); ++it ){
//        kdDebug(9007) << "expr " << (*it) << endl;
//    }


    SimpleVariable v_this = ctx->findVariable( "this" );
    ParsedClass* pThis = sigma->getClassByName( v_this.type );
    QString type;

    if( exprs.count() == 0 ){
        return v_this.type;
    }

    QString e1 = exprs.first().stripWhiteSpace();
    exprs.pop_front();

    kdDebug(9007) << "---> e1 = " << e1 << endl;

    if( e1.isEmpty() ){
        type = v_this.type;
    } else if( e1.endsWith("::") ){
        type = e1.left( e1.length() - 2 ).stripWhiteSpace();
    } else {
        int first_paren_index = 0;
        if( (first_paren_index = e1.find('(')) != -1 ){
            if( first_paren_index == 0 ){
                if( e1[e1.length()-1] == ')' ){
                    // e1 is a subexpression
                    QString subexpr = e1.mid( 1, e1.length() - 2 );
                    subexpr = subexpr.stripWhiteSpace();
                    if( subexpr[0] != '(' ){
                        int start_expr = expressionAt( subexpr, subexpr.length()-1 );
                        if( start_expr != (int)subexpr.length()-1 ){
                            subexpr = subexpr.mid( start_expr );
                            kdDebug(9007) << "subexpr = " << subexpr << endl;
                        }
                    }
                    type = evaluateExpression( subexpr, ctx, sigma );
                } else {
                    // e1 is cast
                    //kdDebug(9007) << "maybe a cast = " << e1 << endl;
                    KDevRegExp cast_rx( "^\\([ \t]*([a-zA-Z_][a-zA-Z0-9_]*)[^)]*)" );
                    if( cast_rx.search(e1) == 0 ){
                        type = cast_rx.cap( 1 );
                        kdDebug(9007) << "cast type = " << type << endl;
                    } else {
                        type = QString::null;
                    }
                }
            } else {
                e1 = e1.left( first_paren_index ).stripWhiteSpace();
                if( pThis ){
                    type = getTypeOfMethod( pThis, e1 );
                }
                if( type.isEmpty() ){
                    type = getTypeOfMethod( m_pStore->globalScope(),
                                            e1 );
                }
            }
        } else {
            SimpleVariable v = ctx->findVariable( e1 );
            if( v.type ){
                // e1 is a local variable
                type = v.type;
            } else {
                // e1 is an attribute
                if( pThis ){
                    type = getTypeOfAttribute( pThis, e1 );
                }
                if( type.isEmpty() ){
                    type = getTypeOfAttribute( m_pStore->globalScope(),
                                               e1 );
                }
            }
        }
    }

    type = purify( type );
    ParsedContainer* pContainer = sigma->getClassByName( type );
    if( !pContainer ){
        pContainer = sigma->globalScope()->getStructByName( type );
        kdDebug(9007) << "is a struct??" << endl;
    }
    kdDebug(9007) << "pContainer = " << pContainer << endl;
    while( pContainer && exprs.count() ){

        QString e = exprs.first().stripWhiteSpace();
        exprs.pop_front();
        type = "";  // no type

        kdDebug(9007) << "----------> evaluate " << e << endl;

        int first_paren_index;
        if( e.isEmpty() ){
            break;
        } else if( (first_paren_index = e.find('(')) != -1 ){
            e = e.left( first_paren_index );
            type = getTypeOfMethod( pContainer, e );
            pContainer = sigma->getClassByName( type );
            if( !pContainer ){
                pContainer = sigma->globalScope()->getStructByName( type );
            }
        } else {
            type = getTypeOfAttribute( pContainer, e );
            pContainer = sigma->getClassByName( type );
            if( !pContainer ){
                pContainer = sigma->globalScope()->getStructByName( type );
            }
        }
    }

    kdDebug(9007) << "-------------> last type = " << type << endl;

    return type;
}

void
CppCodeCompletion::completeText( )
{
    kdDebug(9007) << "CppCodeCompletion::completeText()" << endl;

    if( !m_pCursorIface || !m_pEditIface || !m_pCompletionIface ){
        kdDebug(9007) << "!!!!!!!!!!!!!!!!!!!!!!!!" << endl;
        return;
    }

    uint nLine, nCol;
    m_pCursorIface->cursorPositionReal( &nLine, &nCol );
    QString strCurLine = m_pEditIface->textLine( nLine );

    QString className;
    QString contents;
    bool showArguments = FALSE;

    if( strCurLine[ nCol-1 ] == '(' ){
        --nCol;
        showArguments = TRUE;
    }

    contents = getMethodBody( nLine, nCol, &className );
    kdDebug(9007) << "contents = " << contents << endl;
    kdDebug(9007) << "classname = " << className << endl;

    SimpleContext* ctx = SimpleParser::localVariables( contents );
    if( !ctx )
        return;

    QValueList<SimpleVariable> variableList;
    SimpleVariable v;
    v.name = "this";
    v.type = className;
    variableList.append( v );
    ctx->add( v );

    QString word;
    int start_expr = expressionAt( contents, contents.length() - 1 );
    kdDebug(9007) << "start_expr = " << start_expr << endl;
    QString expr;
    if( start_expr != int(contents.length()) - 1 ){
        expr = contents.mid( start_expr, contents.length() - start_expr );
        expr = expr.stripWhiteSpace();
    }

    int idx = expr.length() - 1;
    while( expr[idx].isLetterOrNumber() || expr[idx] == '_' ){
        --idx;
    }
    if( idx != int(expr.length()) - 1 ){
        ++idx;
        word = expr.mid( idx ).stripWhiteSpace();
        expr = expr.left( idx ).stripWhiteSpace();
    }

    kdDebug(9007) << "prefix = |" << word << "|" << endl;
    kdDebug(9007) << "expr = |" << expr << "|" << endl;

    if( expr == "." || expr == "->" ){
        kdDebug(9007) << "---------> no expression ;-)" << endl;
    } else {
        if( showArguments ){
            QString type = evaluateExpression( expr, ctx, m_pStore );
            QStringList functionList;

            functionList = getSignatureListForClass( type, word );

            if( functionList.count() == 0 ){
                functionList = getGlobalSignatureList( word );
            }

            if( functionList.count() ){
                m_pCompletionIface->showArgHint( functionList, "()", "," );
            }
        } else {
            QValueList<KTextEditor::CompletionEntry> entries;
            QString type;

            SimpleVariable v = ctx->findVariable( word );
            if( expr.isEmpty() && !v.type.isEmpty() ){
                type = v.type;
            } else {
                type = evaluateExpression( expr, ctx, m_pStore );
            }

            entries = unique( getEntryListForClass( type ) );
            if( entries.count() ){
                m_pCompletionIface->showCompletionBox( entries, word.length() );
            }
        }
    }

    delete( ctx );
    ctx = 0;
}

void
CppCodeCompletion::typeOf( )
{
#if 0
    kdDebug(9007) << "CppCodeCompletion::completeText()" << endl;

    if( !m_pCursorIface || !m_pEditIface || !m_pCompletionIface ){
        return;
    }

    uint nLine, nCol;
    m_pCursorIface->cursorPositionReal( &nLine, &nCol );

    QString strCurLine = m_pEditIface->textLine( nLine );
    QValueList<KTextEditor::CompletionEntry> entries;


    QString className;
    QString contents = getMethodBody( nLine, nCol, &className );
    kdDebug(9007) << "contents = " << contents << endl;

    QValueList<SimpleVariable> variableList = SimpleParser::localVariables( contents );
    SimpleVariable v;
    v.name = "this";
    v.scope = 1;
    v.type = className;
    variableList.append( v );

    int start_expr = expressionAt( contents, contents.length() - 1 );
    QString expr;
    if( start_expr != (int)contents.length() - 1 ){
        expr = contents.mid( start_expr, contents.length() - start_expr );
        expr = expr.simplifyWhiteSpace();
    }

    kdDebug(9007) << "expr = |" << expr << "|" << endl;

    QString type = evaluateExpression( expr, variableList, m_pStore );
    if( type.isEmpty() ){
        type = "unknown";
    }

    kdDebug(9007) << "the type of expression is " << type << endl;
    m_pSupport->mainWindow()->statusBar()->message( type.isEmpty() ? i18n("no type for expression") : type, 1000 );

    QStringList functionList;
    QString strFunction;
    strFunction.sprintf( "type_of_expression_is(%s)", type.latin1() );
    functionList.append ( strFunction );
    m_pCompletionIface->showArgHint ( functionList, "()", "," );
#endif
}

void
CppCodeCompletion::slotTextChangedRoberto( int nLine, int nCol, const QString &/*text*/)
{
    kdDebug(9007) << "CompletionEntry::slotTextChangedRoberto()" << endl;
    QString strCurLine = m_pEditIface->textLine( nLine );
    QString ch = strCurLine.mid( nCol-1, 1 );
    QString ch2 = strCurLine.mid( nCol-2, 2 );

    if ( ch == "." || ch2 == "->" || ch2 == "::" || ch == "(" ){
        completeText();
    }
}

QString
CppCodeCompletion::getTypeOfMethod( ParsedContainer* pContainer, const QString& name )
{
    if( !pContainer ){
        return QString::null;
    }

    QValueList<ParsedMethod*> methodList = pContainer->getMethodByName( name );
    if( methodList.count() != 0 ){
        // TODO: check for method's arguments
        QString type = (*methodList.begin())->type();
        return purify( type );
    }


    ParsedClass* pClass = dynamic_cast<ParsedClass*>( pContainer );
    if( !pClass )
        return QString::null;

    QPtrList<ParsedParent> parentList = pClass->parents;
    for( ParsedParent* pParent=parentList.first(); pParent!=0; pParent=parentList.next() ){
        pClass = m_pCCStore->getClassByName( pParent->name() );
        if( !pClass )
            pClass = m_pStore->getClassByName( pParent->name() );

        QString type = getTypeOfMethod( pClass, name );
        type = purify( type );
        if( !type.isEmpty() ){
            return type;
        }
    }
    return QString::null;
}

QString
CppCodeCompletion::getTypeOfAttribute( ParsedContainer* pContainer, const QString& name )
{
    if( !pContainer ){
        return QString::null;
    }

    ParsedAttribute* pAttr = pContainer->getAttributeByName( name );
    if( pAttr ){
        QString type = pAttr->type();
        return purify( type );
    }

    ParsedClass* pClass = dynamic_cast<ParsedClass*>( pContainer );
    if( !pClass )
        return QString::null;

    QPtrList<ParsedParent> parentList = pClass->parents;
    for( ParsedParent* pParent=parentList.first(); pParent!=0; pParent=parentList.next() ){
        ParsedClass* pClass;
        pClass = m_pCCStore->getClassByName( pParent->name() );
        if( !pClass )
            pClass = m_pStore->getClassByName( pParent->name() );

        QString type = getTypeOfAttribute( pClass, name );
        type = purify( type );
        if( !type.isEmpty() ){
            return type;
        }
    }
    return QString::null;
}


QValueList<ParsedMethod*> CppCodeCompletion::getMethodListForClassAndAncestors( ParsedClass* pClass )
{
    QValueList<ParsedMethod*> retVal = pClass->getSortedMethodList( );
    retVal += pClass->getSortedSlotList( );
    retVal += pClass->getSortedSignalList( );

    QPtrList<ParsedParent> parentList = pClass->parents;

    for ( ParsedParent* pPClass = parentList.first( ); pPClass != 0; pPClass = parentList.next( ) ) {
        pClass = m_pCCStore->getClassByName( pPClass->name( ) );

        if ( pClass )
            retVal += getMethodListForClassAndAncestors(pClass);
        else {
            // TODO: look in ClassStore for Namespace classes
        }
    }

    return retVal;
}


QValueList<ParsedAttribute*> CppCodeCompletion::getAttributeListForClassAndAncestors( ParsedClass* pClass )
{
    QValueList<ParsedAttribute*> retVal = pClass->getSortedAttributeList( );

    QPtrList<ParsedParent> parentList = pClass->parents;
    for( ParsedParent* pPClass = parentList.first( ); pPClass != 0; pPClass = parentList.next( ) ) {

        pClass = m_pCCStore->getClassByName( pPClass->name( ) );
        if ( pClass )
            retVal += getAttributeListForClassAndAncestors( pClass );
        else {
            // TODO: look in ClassStore for Namespace classes
        }
    }

    return retVal;
}


QStringList CppCodeCompletion::getGlobalSignatureList(const QString &functionName)
{
    return m_pStore->globalScope()->getSortedMethodSignatureList(functionName);
}


QStringList CppCodeCompletion::getSignatureListForClass( QString strClass, QString strMethod )
{
     ParsedClass* pClass = m_pCCStore->getClassByName( strClass );
     if( !pClass )
         pClass = m_pStore->getClassByName ( strClass );
     if ( !pClass )
         return QStringList();

     QStringList retVal = pClass->getSortedMethodSignatureList(strMethod);
     retVal += pClass->getSortedSlotSignatureList(strMethod);
     retVal += pClass->getSortedSignalSignatureList(strMethod);
     retVal += getParentSignatureListForClass( pClass, strMethod );

     return retVal;
}


QStringList CppCodeCompletion::getParentSignatureListForClass( ParsedClass* pClass,
                                                               QString strMethod )
{
    QStringList retVal;

    QPtrList<ParsedParent> parentList = pClass->parents;
    for ( ParsedParent* pParentClass = parentList.first(); pParentClass != 0; pParentClass = parentList.next() )
    {
        pClass = m_pStore->getClassByName ( pParentClass->name() );

        if ( pClass )
        {
            retVal = pClass->getSortedMethodSignatureList(strMethod);
            retVal += pClass->getSortedSignalSignatureList(strMethod);
            retVal += pClass->getSortedSlotSignatureList(strMethod);
            retVal += getParentSignatureListForClass( pClass, strMethod );
        }
        /*else
          {
          // TODO: look in ClassStore for Namespace classes
          } */
    }
    
    return retVal;
}

QString CppCodeCompletion::getText( unsigned int startLine, unsigned int startColumn,
				    unsigned int endLine, unsigned int endColumn )
{
    QString text;
    
    if( !m_pCursorIface )
	return text;
    
    for( unsigned int i=startLine; i<=endLine; ++i ){
	QString textLine = m_pEditIface->textLine( i );
	if( i == startLine )
	    textLine = textLine.mid( startColumn );
	else if( i == endLine )
	    textLine = textLine.left( endColumn );
	
	text += textLine;
	
	if( i != endLine )
	    text += "\n";
    }
 
    return text;
}

void CppCodeCompletion::slotFileParsed( const QString& fileName )
{
    if( fileName != m_currentFileName || !m_pSupport )
	return;
    
    unsigned int line, column;
    m_pCursorIface->cursorPositionReal( &line, &column );    
        
    m_pSupport->backgroundParser()->lock();
    TranslationUnitAST* ast = m_pSupport->backgroundParser()->translationUnit( fileName );    
    AST* node = findNodeAt( ast, line, column );

    if( node ){
        kdDebug(9007) << "current node is = " << nodeTypeToString( (NodeType)(node->nodeType()) ) << endl;
    }

    if( node && node->nodeType() == NodeType_FunctionDefinition ){
	int startLine, startColumn;
	node->getStartPosition( &startLine, &startColumn );

	int endLine, endColumn;
	node->getEndPosition( &endLine, &endColumn );

	QStringList scope;
	scopeOfNode( node, scope );
	kdDebug(9007) << "------> scope = " << scope.join( "::" ) << endl;
    }
    m_pSupport->backgroundParser()->unlock();
}

#include "cppcodecompletion.moc"
