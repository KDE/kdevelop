/***************************************************************************
                          cppcodecompletion.cpp  -  description
                             -------------------
    begin                : Sat Jul 21 2001
    copyright            : (C) 2001 by Victor Röder
    email                : victor_roeder@gmx.de
    copyright            : (C) 2002,2003 by Roberto Raggi
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
#include <ktexteditor/document.h>

#include <qdatastream.h>
#include <qfile.h>
#include <qmap.h>
#include <qregexp.h>
#include <qstatusbar.h>
#include <qstring.h>
#include <qstringlist.h>

#include <kdevpartcontroller.h>
#include <kdevmainwindow.h>

#include <classstore.h>
#include <parsedclass.h>
#include <parsedscopecontainer.h>

enum
{
    KIND_NAMESPACE = 0,
    KIND_CONSTRUCTOR,
    KIND_DESTRUCTOR,
    KIND_SLOT,
    KIND_SIGNAL,
    KIND_METHOD,
    KIND_ATTRIBUTE,
    KIND_CLASS,
    KIND_STRUCT
};

bool operator < ( const KTextEditor::CompletionEntry& e1, const KTextEditor::CompletionEntry& e2 )
{
    return
	e1.type < e2.type ||
	( !(e1.type < e2.type) && e1.text < e2.text ) ||
	( !(e1.text < e2.text) && e1.prefix < e2.prefix ) ||
	( !(e1.prefix < e2.prefix) && e1.postfix < e2.postfix ) ||
	( !(e1.postfix < e2.postfix) && e1.comment < e2.comment ) ||
	( !(e1.comment < e2.comment) && e1.userdata < e2.userdata );
}

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
    s = s.replace( rx1, "" ).replace( rx2, "" ).replace("\\b(class|struct|union)\\b", "");
    s = remove_keywords( s );
    s = remove( s, '[', ']' );
    s = s.simplifyWhiteSpace();

    return s;
}

CppCodeCompletion::CppCodeCompletion( CppSupportPart* part )
{
    m_pSupport = part;
    m_activeCursor = 0;
    m_activeEditor   = 0;
    m_activeCompletion = 0;
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
}

void
CppCodeCompletion::slotArgHintHided( )
{
    m_bArgHintShow = false;
}

void
CppCodeCompletion::setEnabled( bool setEnable )
{
    m_bCodeCompletion = setEnable;
}

void
CppCodeCompletion::slotCompletionBoxHided( KTextEditor::CompletionEntry entry )
{
    Q_UNUSED( entry );
    m_bCompletionBoxShow = false;

    unsigned int line, column;
    m_activeCursor->cursorPositionReal( &line, &column );
    QString textLine = m_activeEditor->textLine( line );
    QString ch = textLine.mid( column-1, 1 );
}


void
CppCodeCompletion::slotActivePartChanged(KParts::Part *part)
{
    kdDebug( 9007 ) << "CppCodeCompletion::slotActivePartChanged()" << endl;

    if( !part )
      return;

    m_activeFileName = QString::null;

    KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>( part );
    if( !doc )
	return;

    m_activeFileName = doc->url().path();

    // if the interface stuff fails we should disable codecompletion automatically
    m_activeEditor = dynamic_cast<KTextEditor::EditInterface*>(part);
    if( !m_activeEditor ){
        kdDebug( 9007 ) << "Editor doesn't support the EditDocumentIface" << endl;
        return;
    }

    m_activeCursor = dynamic_cast<KTextEditor::ViewCursorInterface*>(part->widget());
    if( !m_activeCursor ){
        kdDebug( 9007 ) << "The editor doesn't support the CursorDocumentIface!" << endl;
        return;
    }

    m_activeCompletion = dynamic_cast<KTextEditor::CodeCompletionInterface*>(part->widget());
    if( !m_activeCompletion ){
        kdDebug( 9007 ) << "Editor doesn't support the CompletionIface" << endl;
        return;
    }

    // here we have to investigate :)
    if( m_pSupport && m_pSupport->getEnableCC( ) == true ){
        kdDebug( 9007 ) << "enabling code completion" << endl;
	connect(part, SIGNAL(textChanged()), this, SLOT(slotTextChanged()) );
	connect(part->widget(), SIGNAL( completionDone( KTextEditor::CompletionEntry ) ), this,
                 SLOT( slotCompletionBoxHided( KTextEditor::CompletionEntry ) ) );
    }

    kdDebug(9007) << "CppCodeCompletion::slotActivePartChanged() -- end" << endl;
}

void
CppCodeCompletion::slotTextChanged()
{
    m_ccTimer->stop();
    
    if( !m_activeCursor )
        return;

    unsigned int nLine, nCol;
    m_activeCursor->cursorPositionReal( &nLine, &nCol );

    QString strCurLine = m_activeEditor->textLine( nLine );
    QString ch = strCurLine.mid( nCol-1, 1 );
    QString ch2 = strCurLine.mid( nCol-2, 2 );

    if ( ch == "(" || ch == "." || ch2 == "->" || ch2 == "::" ){
    	m_ccTimer->start( 500, true );
    }
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

    QString text = m_activeEditor->textLine( iLine ).left( iCol );
    --iLine;
    while( iLine >= 0 ){

        text.prepend( m_activeEditor->textLine( iLine ).simplifyWhiteSpace() + "\n" );
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
            last = T_ACCESS;
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
CppCodeCompletion::evaluateExpression( QString expr, SimpleContext* ctx )
{
    bool global = false;
    if( expr.startsWith("::") ){
	expr = expr.mid( 2 );
	global = true;
    }

    QStringList exprList = splitExpression( expr );
    SimpleVariable v_this = ctx->findVariable( "this" );

    QString type = v_this.type;
    ParsedClassContainer* container = findContainer( v_this.type );

    QStringList::Iterator it = exprList.begin();
    while( it != exprList.end() ){
	QString e = *it;

	e = e.stripWhiteSpace();
	int leftParen = e.find( "(" );

	if( leftParen != -1 )
	    e = e.left( leftParen );

	e = e.replace( QRegExp("::"), "." );
	if( e.endsWith(".") )
	    e = e.left( e.length() - 1 );

	type = QString::null;
	if( it == exprList.begin() ){
	    SimpleVariable v = ctx->findVariable( e );
	    type = v.type;
	}

	if( type.isEmpty() )
	    type = purify( typeOf(e, container) );

	if( type.isEmpty() && it == exprList.begin() )
	    type = purify( typeOf(e) );

	if( type.isEmpty() || !(container = findContainer(type)) )
	    break;

	++it;
    }

    return type;
}

void
CppCodeCompletion::completeText( )
{
    kdDebug(9007) << "CppCodeCompletion::completeText()" << endl;

    if( !m_activeCursor || !m_activeEditor || !m_activeCompletion )
        return;

    uint nLine, nCol;
    m_activeCursor->cursorPositionReal( &nLine, &nCol );

    QString strCurLine = m_activeEditor->textLine( nLine );

    bool showArguments = FALSE;

    QString ch = strCurLine.mid( nCol-1, 1 );
    QString ch2 = strCurLine.mid( nCol-2, 2 );
    
    if( ch2 == "->" || ch == "." || ch == "(" ){
	int pos = ch2 == "->" ? nCol - 3 : nCol - 2;
	QChar c = strCurLine[ pos ];
	if( !(c.isLetterOrNumber() || c == '_') )
	    return;
    }
 
    if( ch == "(" ){
        --nCol;
        showArguments = TRUE;
    }

    QString className;
    QString contents = getMethodBody( nLine, nCol, &className );
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

    QString type = evaluateExpression( expr, ctx );
    if( type ){


	if( showArguments ){
	    QStringList functionList = getSignatureListForClass( type, word );

	    if( functionList.count() == 0 ){
		functionList = getGlobalSignatureList( word );
	    }

	    if( functionList.count() ){
		m_activeCompletion->showArgHint( functionList, "()", "," );
	    }
	} else {
	    QValueList<KTextEditor::CompletionEntry> entryList = findAllEntries( type );
	    if( entryList.size() )
		m_activeCompletion->showCompletionBox( entryList, word.length(), false );
	}
    }

    delete( ctx );
    ctx = 0;
}

QStringList CppCodeCompletion::getGlobalSignatureList(const QString &functionName)
{
    return m_pSupport->classStore()->globalScope()->getSortedMethodSignatureList(functionName);
}

QStringList CppCodeCompletion::getSignatureListForClass( QString strClass, QString strMethod )
{
     ParsedClass* pClass = dynamic_cast<ParsedClass*>( findContainer(strClass) );
     if ( !pClass )
         return QStringList();

     QStringList retVal = pClass->getSortedMethodSignatureList(strMethod);
     retVal += pClass->getSortedSlotSignatureList(strMethod);
     retVal += pClass->getSortedSignalSignatureList(strMethod);
     retVal += getParentSignatureListForClass( pClass, strMethod );

     return retVal;
}


QStringList CppCodeCompletion::getParentSignatureListForClass( ParsedClass* pClass, QString strMethod )
{
    QStringList retVal;

    QPtrList<ParsedParent> parentList = pClass->parents;
    for ( ParsedParent* pParentClass = parentList.first(); pParentClass != 0; pParentClass = parentList.next() )
    {
        pClass = dynamic_cast<ParsedClass*>( findContainer(pParentClass->name()) );

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


void CppCodeCompletion::slotFileParsed( const QString& fileName )
{
    if( fileName != m_activeFileName || !m_pSupport )
	return;

    unsigned int line, column;
    m_activeCursor->cursorPositionReal( &line, &column );

    kdDebug(9007) << "CppCodeCompletion::slotFileParsed()" << endl;

#if 0
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
#endif
}

ParsedClassContainer* CppCodeCompletion::findContainer( const QString& name, ParsedScopeContainer* container, const QStringList& imports )
{
    if( !container )
        container = m_pSupport->classStore()->globalScope();

    QStringList path = QStringList::split( ".", name );
    QStringList::Iterator it = path.begin();
    while( it != path.end() ){
        QString s = *it;
        ++it;

        ParsedScopeContainer* scope = container->getScopeByName( s );
        if( !scope )
            break;

        path.remove( s );
        container = scope;
    }

    if( path.size() == 0 )
        return container;

    QString className = path.join( "." );
    ParsedClass* klass = container->getClassByName( className );
    if( !klass )
        klass = container->getStructByName( className );

    if( !klass && imports.size() ){

        QStringList::ConstIterator impIt = imports.begin();
        while( impIt != imports.end() ){
            ParsedClassContainer* kl = findContainer( (*impIt) + "." + name, container );
            if( kl )
                return kl;
            ++impIt;
        }
    }

    return klass;
}

QValueList<KTextEditor::CompletionEntry> CppCodeCompletion::findAllEntries( const QString& type, bool includePrivate )
{
    QValueList<KTextEditor::CompletionEntry> entryList;

    if( ParsedClassContainer* container = findContainer(type) ){

	ParsedClass* klass = dynamic_cast<ParsedClass*>( container );
	ParsedScopeContainer* scope = dynamic_cast<ParsedScopeContainer*>( container );

	if( klass )
	{
	    QPtrList<ParsedParent> l = klass->parents;
	    QPtrListIterator<ParsedParent> it( l );
	    while( it.current() ){
		ParsedParent* p = it.current();
		++it;

		entryList += findAllEntries( p->name(), false );
	    }
	}

	if( klass )
	{
	    QValueList<ParsedMethod*> l = klass->getSortedSlotList();
	    QValueList<ParsedMethod*>::Iterator it = l.begin();
	    while( it != l.end() ){
		ParsedMethod* meth = *it;
		++it;

		if( !includePrivate && meth->access() == PIE_PRIVATE )
		    continue;

		KTextEditor::CompletionEntry entry;
		entry.type = QString::number( KIND_SLOT );
		//entry.prefix = meth->type();

		entry.text = meth->name() + "(";
		
		QString text;
		for( ParsedArgument *pArg = meth->arguments.first();
		pArg != 0;
		pArg = meth->arguments.next() ){
		    if( pArg != meth->arguments.getFirst() )
			text += ", ";
		    text += pArg->toString();
		}
		
		if( text.isEmpty() ){
		    entry.text += ")";
		} else {
		    text += ")";
		    entry.postfix = text;
		}
		entryList << entry;
	    }
	}

	if( klass )
	{
	    QValueList<ParsedMethod*> l = klass->getSortedSignalList();
	    QValueList<ParsedMethod*>::Iterator it = l.begin();
	    while( it != l.end() ){
		ParsedMethod* meth = *it;
		++it;

		if( !includePrivate && meth->access() == PIE_PRIVATE )
		    continue;

		KTextEditor::CompletionEntry entry;
		entry.type = QString::number( KIND_SIGNAL );
		//entry.prefix = meth->type();
		entry.text = meth->name() + "(";

		QString text;
		for( ParsedArgument *pArg = meth->arguments.first();
		pArg != 0;
		pArg = meth->arguments.next() ){
		    if( pArg != meth->arguments.getFirst() )
			text += ", ";
		    text += pArg->toString();
		}
		if( text.isEmpty() ){
		    entry.text += ")";
		} else {
		    text += ")";
		    entry.postfix = text;
		}
		entryList << entry;
	    }
	}

	// methods
	{
	    QValueList<ParsedMethod*> l = container->getSortedMethodList();
	    QValueList<ParsedMethod*>::Iterator it = l.begin();
	    while( it != l.end() ){
		ParsedMethod* meth = *it;
		++it;

		if( !includePrivate && meth->access() == PIE_PRIVATE )
		    continue;

		KTextEditor::CompletionEntry entry;
		entry.type = QString::number( KIND_METHOD );
		//entry.prefix = meth->type();
		entry.text = meth->name() + "(";

		QString text;
		for( ParsedArgument *pArg = meth->arguments.first();
		pArg != 0;
		pArg = meth->arguments.next() ){
		    if( pArg != meth->arguments.getFirst() )
			text += ", ";
		    text += pArg->toString();
		}
		if( text.isEmpty() ){
		    entry.text += ")";
		} else {
		    text += ")";
		    entry.postfix = text;
		}
		entryList << entry;
	    }
	}

	// attributes
	{
	    QValueList<ParsedAttribute*> l = container->getSortedAttributeList();
	    QValueList<ParsedAttribute*>::Iterator it = l.begin();
	    while( it != l.end() ){
		ParsedAttribute* attr = *it;
		++it;

		if( !includePrivate && attr->access() == PIE_PRIVATE )
		    continue;

		KTextEditor::CompletionEntry entry;
		entry.type = QString::number( KIND_ATTRIBUTE );
		//entry.prefix = attr->type();
		entry.text = attr->name();
		entryList << entry;
	    }
	}

	// namespaces
	if( scope )
	{
	    QValueList<ParsedScopeContainer*> l = scope->getSortedScopeList();
	    QValueList<ParsedScopeContainer*>::Iterator it = l.begin();
	    while( it != l.end() ){
		ParsedScopeContainer* sc = *it;
		++it;

		KTextEditor::CompletionEntry entry;
		entry.type = QString::number( KIND_NAMESPACE );
		entry.text = sc->name();
		entryList << entry;
	    }
	}

	// inner classes
	{
	    QValueList<ParsedClass*> l = container->getSortedClassList();
	    QValueList<ParsedClass*>::Iterator it = l.begin();
	    while( it != l.end() ){
		ParsedClass* cl = *it;
		++it;

		KTextEditor::CompletionEntry entry;
		entry.type = QString::number( KIND_CLASS );
		entry.text = cl->name();
		entryList << entry;
	    }
	}

	// inner structs
	{
	    QValueList<ParsedClass*> l = container->getSortedStructList();
	    QValueList<ParsedClass*>::Iterator it = l.begin();
	    while( it != l.end() ){
		ParsedClass* cl = *it;
		++it;

		KTextEditor::CompletionEntry entry;
		entry.type = QString::number( KIND_STRUCT );
		entry.text = cl->name();
		entryList << entry;
	    }
	}
    }

    entryList = unique( entryList );
    // qHeapSort( entryList );
    return entryList;
}

QString CppCodeCompletion::typeOf( const QString& name, ParsedClassContainer* container )
{
    if( !container )
	container = m_pSupport->classStore()->globalScope();

    QString type;

    ParsedClass* klass = dynamic_cast<ParsedClass*>( container );
    ParsedScopeContainer* scope = dynamic_cast<ParsedScopeContainer*>( container );

    if( klass )
    {
	QValueList<ParsedMethod*> l = klass->getSortedSlotList();
	QValueList<ParsedMethod*>::Iterator it = l.begin();
	while( it != l.end() ){
	    ParsedMethod* meth = *it;
	    ++it;

	    if( meth->name() == name )
		return meth->type();
	}
    }

    if( klass )
    {
	QValueList<ParsedMethod*> l = klass->getSortedSignalList();
	QValueList<ParsedMethod*>::Iterator it = l.begin();
	while( it != l.end() ){
	    ParsedMethod* meth = *it;
	    ++it;

	    if( meth->name() == name )
		return meth->type();
	}
    }
    
    // methods
    {
	QValueList<ParsedMethod*> l = container->getSortedMethodList();
	QValueList<ParsedMethod*>::Iterator it = l.begin();
	while( it != l.end() ){
	    ParsedMethod* meth = *it;
	    ++it;

	    if( meth->name() == name )
		return meth->type();
	}
    }

    // attributes
    {
	QValueList<ParsedAttribute*> l = container->getSortedAttributeList();
	QValueList<ParsedAttribute*>::Iterator it = l.begin();
	while( it != l.end() ){
	    ParsedAttribute* attr = *it;
	    ++it;

	    if( attr->name() == name )
		return attr->type();
	}
    }

    // namespaces
    if( scope )
    {
	QValueList<ParsedScopeContainer*> l = scope->getSortedScopeList();
	QValueList<ParsedScopeContainer*>::Iterator it = l.begin();
	while( it != l.end() ){
	    ParsedScopeContainer* sc = *it;
	    ++it;

	    if( sc->path() == name )
		return name;
	}
    }

    // inner classes
    {
	QValueList<ParsedClass*> l = container->getSortedClassList();
	QValueList<ParsedClass*>::Iterator it = l.begin();
	while( it != l.end() ){
	    ParsedClass* cl = *it;
	    ++it;

	    if( cl->path() == name )
		return name;
	}
    }

    // inner structs
    {
	QValueList<ParsedClass*> l = container->getSortedStructList();
	QValueList<ParsedClass*>::Iterator it = l.begin();
	while( it != l.end() ){
	    ParsedClass* cl = *it;
	    ++it;

	    if( cl->path() == name )
		return name;
	}
    }


    // parents
    if( klass )
    {
	QPtrList<ParsedParent> l = klass->parents;
	QPtrListIterator<ParsedParent> it( l );
	while( it.current() ){
	    ParsedParent* p = it.current();
	    ++it;

	    ParsedClassContainer* c = findContainer( p->name() );
	    if( c != 0 ){
		type = typeOf( p->name(), c );
		if( type )
		    return type;
	    }
	}
    }
    

    return QString::null;
}

#include "cppcodecompletion.moc"
