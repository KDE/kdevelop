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
#include "backgroundparser.h"
#include "ast.h"
#include "ast_utils.h"
#include "codeinformationrepository.h"
#include "parser.h"
#include "lexer.h"
#include "tree_parser.h"

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

#include <kdevpartcontroller.h>
#include <kdevmainwindow.h>

#include <classstore.h>
#include <parsedclass.h>
#include <parsedscopecontainer.h>


class SimpleVariable{
public:
    SimpleVariable()
        : name( QString::null ), type( QString::null ){}
    SimpleVariable( const SimpleVariable& source )
        : name( source.name ), type( source.type ) {}
    ~SimpleVariable(){}

    SimpleVariable& operator = ( const SimpleVariable& source ){
        name = source.name;
        type = source.type;
        return *this;
    }

    QString name;
    QString type;
};

class SimpleContext{
public:
    SimpleContext( SimpleContext* prev=0 )
        : m_prev( prev ) {}

    virtual ~SimpleContext()
        {
            if( m_prev ){
                delete( m_prev );
                m_prev = 0;
            }
        }

    SimpleContext* prev() const
        { return m_prev; }

    void attach( SimpleContext* ctx )
        { m_prev = ctx; }

    void detach()
        { m_prev = 0; }

    const QValueList<SimpleVariable>& vars() const
        { return m_vars; }

    void add( const SimpleVariable& v )
        { m_vars.append( v ); }

    void add( const QValueList<SimpleVariable>& vars )
        { m_vars += vars; }

    SimpleVariable findVariable( const QString& varname )
        {
            SimpleContext* ctx = this;
            while( ctx ){
                const QValueList<SimpleVariable>& vars = ctx->vars();
                for( int i=vars.count() - 1; i>=0; --i ){
                    SimpleVariable v = vars[ i ];
                    if( v.name == varname )
                        return v;
                }
                ctx = ctx->prev();
            }
            return SimpleVariable();
        }

private:
    QValueList<SimpleVariable> m_vars;
    SimpleContext* m_prev;
};

struct RecoveryPoint
{
    int kind;
    QStringList scope;
    int startLine, startColumn;
    int endLine, endColumn;

    RecoveryPoint()
        : kind( 0 ), startLine( 0 ), startColumn( 0 ),
          endLine( 0 ), endColumn( 0 )
    {
    }

private:
    RecoveryPoint( const RecoveryPoint& source );
    void operator = ( const RecoveryPoint& source );
};

struct CppCodeCompletionData
{
    QPtrList<RecoveryPoint> recoveryPoints;

    CppCodeCompletionData()
    {
        recoveryPoints.setAutoDelete( true );
    }

    RecoveryPoint* findRecoveryPoint( int line, int column )
    {
        if( recoveryPoints.count() == 0 )
            return 0;

        QPair<int, int> pt = qMakePair( line, column );

        QPtrListIterator<RecoveryPoint> it( recoveryPoints );
        while( it.current() ){
            QPair<int, int> startPt = qMakePair( it.current()->startLine, it.current()->startColumn );
            QPair<int, int> endPt = qMakePair( it.current()->endLine, it.current()->endColumn );

            if( (startPt < pt || startPt == pt) && (pt < endPt || pt == endPt) ){
                kdDebug(9007) << "found recovery point " << it.current()->scope.join("::") << endl;
                return it.current();
            }

            ++it;
        }

        return 0;
    }

};

static QString toSimpleName( NameAST* name )
{
    if( !name )
        return QString::null;

    QString s;

    QPtrList<ClassOrNamespaceNameAST> l = name->classOrNamespaceNameList();
    QPtrListIterator<ClassOrNamespaceNameAST> nameIt( l );
    while( nameIt.current() ){
        if( nameIt.current()->name() ){
            s += nameIt.current()->name()->text() + "::";
        }
        ++nameIt;
    }

    if( name->unqualifiedName() && name->unqualifiedName()->name() )
        s += name->unqualifiedName()->name()->text();

    return s;
}


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

CppCodeCompletion::CppCodeCompletion( CppSupportPart* part )
    : d( new CppCodeCompletionData )
{
    m_pSupport = part;
    m_activeCursor = 0;
    m_activeEditor   = 0;
    m_activeCompletion = 0;
    m_ccTimer = new QTimer( this );
    m_ccLine = 0;
    m_ccColumn = 0;
    connect( m_ccTimer, SIGNAL(timeout()), this, SLOT(slotTimeout()) );

    m_bArgHintShow       = false;
    m_bCompletionBoxShow = false;

    m_repository = new CodeInformationRepository();
    setupCodeInformationRepository();

    connect( part->partController( ), SIGNAL( activePartChanged( KParts::Part* ) ),
	     this, SLOT( slotActivePartChanged( KParts::Part* ) ) );

    connect( part, SIGNAL(fileParsed(const QString&)), this, SLOT(slotFileParsed(const QString&)) );
}

CppCodeCompletion::~CppCodeCompletion( )
{
    delete( m_repository );
    delete( d );
}

void CppCodeCompletion::slotTimeout()
{
    if( !m_activeCursor || !m_activeEditor || !m_activeCompletion )
        return;

    uint nLine, nCol;
    m_activeCursor->cursorPositionReal( &nLine, &nCol );

    if( nLine != m_ccLine || nCol != m_ccColumn )
	return;;

    QString textLine = m_activeEditor->textLine( nLine );
    QChar ch = textLine[ nCol ];;
    if( ch.isLetterOrNumber() || ch == '_' )
	return;

    completeText();
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
    if( m_pSupport && m_pSupport->codeCompletionEnabled( ) == true ){
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

    m_ccLine = 0;
    m_ccColumn = 0;

    if ( ch == "(" || ch == "." || ch2 == "->" || ch2 == "::" ){
        m_ccLine = nLine;
        m_ccColumn = nCol;
    	m_ccTimer->start( 250, true );
    }
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
CppCodeCompletion::evaluateExpression( const QString& e, SimpleContext* ctx )
{
    QString expr = e;

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

	//e = e.replace( QRegExp("::"), "." );
	if( e.endsWith("::") )
	    e = e.left( e.length() - 2 );

	type = QString::null;
	if( it == exprList.begin() ){
	    SimpleVariable v = ctx->findVariable( e );
	    type = v.type;
	}

	if( type.isEmpty() )
	    type = typeName( typeOf(e, container) );

	if( type.isEmpty() && it == exprList.begin() )
	    type = typeName( typeOf(e) );

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

    if( !m_pSupport || !m_activeCursor || !m_activeEditor || !m_activeCompletion )
        return;

    unsigned int line, column;
    m_activeCursor->cursorPositionReal( &line, &column );

    int nLine = line, nCol = column;

    QString strCurLine = m_activeEditor->textLine( nLine );

    bool showArguments = false;

    QString ch = strCurLine.mid( nCol-1, 1 );
    QString ch2 = strCurLine.mid( nCol-2, 2 );

    kdDebug(9007) << "ch = " << ch << endl;
    kdDebug(9007) << "ch2 = " << ch2 << endl;

    if( ch2 == "->" || ch == "." || ch == "(" ){
	int pos = ch2 == "->" ? nCol - 3 : nCol - 2;
	QChar c = strCurLine[ pos ];
	kdDebug(9007) << "c = " << c.latin1() << endl;
	if( !(c.isLetterOrNumber() || c == '_' || c == ')') )
	    return;
    }

    if( ch == "(" ){
        --nCol;
        showArguments = TRUE;
    }

    QString type;
    QString expr, word;

    // sync
    while( m_pSupport->backgroundParser()->filesInQueue() > 0 )
         m_pSupport->backgroundParser()->isEmpty().wait();

    m_pSupport->backgroundParser()->lock();
    AST* ast = m_pSupport->backgroundParser()->translationUnit( m_activeFileName );
    DeclarationAST::Node recoveredDecl;

    if( !ast ){
        kdDebug(9007) << "------------------- NO AST FOUND --------------------" << endl;

        if( RecoveryPoint* recoveryPoint = d->findRecoveryPoint(line, column) ){
            kdDebug(9007) << "node-kind = " << recoveryPoint->kind << endl;
            kdDebug(9007) << "isFunDef = " << (recoveryPoint->kind == NodeType_FunctionDefinition) << endl;

            QString textLine = m_activeEditor->textLine( recoveryPoint->startLine );
            kdDebug(9007) << "startLine = " << textLine << endl;
            kdDebug(9007) << "node-kind = " << recoveryPoint->kind << endl;


            if( recoveryPoint->kind == NodeType_FunctionDefinition ){

                QString textToReparse = getText( recoveryPoint->startLine, recoveryPoint->startColumn,
                                                 line, showArguments ? column-1 : column );
                //kdDebug(9007) << "-------------> please reparse only text" << endl << textToReparse << endl
                //              << "--------------------------------------------" << endl;

                Driver d;
                Lexer lexer( &d );
                // TODO: setup the lexer(i.e. adds macro, special words, ...

                lexer.setSource( textToReparse );
                Parser parser( &d, &lexer );

                parser.parseDeclaration( recoveredDecl );
                if( recoveredDecl.get() ){

                    bool isFunDef = recoveredDecl->nodeType() == NodeType_FunctionDefinition;
                    kdDebug(9007) << "is function definition = " << isFunDef << endl;

	            int endLine, endColumn;
	            recoveredDecl->getEndPosition( &endLine, &endColumn );
                    kdDebug(9007) << "endLine = " << endLine << ", endColumn " << endColumn << endl;

                    // TODO: check end position

                    if( isFunDef ) {
                        FunctionDefinitionAST* def = static_cast<FunctionDefinitionAST*>( recoveredDecl.get() );

                        // TODO: remove code duplication

                        QString contents = textToReparse;
                        int start_expr = expressionAt( contents, contents.length() - 1 );
                        // kdDebug(9007) << "start_expr = " << start_expr << endl;
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

                        if( !expr.isNull() ){
                                kdDebug(9007) << "expr = " << expr << endl;
                        } else {
                                kdDebug(9007) << "no expr found!!" << endl;
                        }


                        SimpleContext* ctx = computeContext( def, endLine, endColumn );
                        DeclaratorAST* d = def->initDeclarator()->declarator();
                        NameAST* name = d->declaratorId();
                        QString scope = recoveryPoint->scope.join( "::" );

                        QStringList nested;
                        QPtrList<ClassOrNamespaceNameAST> l = name->classOrNamespaceNameList();
                        QPtrListIterator<ClassOrNamespaceNameAST> nameIt( l );
                        while( nameIt.current() ){
                            if( nameIt.current()->name() ){
                                nested << nameIt.current()->name()->text();
                            }
                            ++nameIt;
                        }

                        QString s = nested.join( "::" );

                        if( !scope.isNull() ){
                            scope += QString::fromLatin1( "::" ) + s;
                        } else {
                            scope += s;
                        }

                        if( scope.endsWith("::") ){
                           scope = scope.left( scope.length() - 2 );
                        }

                        if( scope.startsWith("::") ){
                           scope = scope.mid( 2 );
                        }

                        if( !scope.isNull() ){
	                    SimpleVariable var;
	                    var.type = scope;
	                    var.name = "this";
	                    ctx->add( var );
                            kdDebug(9007) << "add variable " << var.name << " with type " << var.type << endl;
                        }

                        type = ctx ? typeName( evaluateExpression(expr, ctx) ) : QString::null;
                        delete( ctx );
                        ctx = 0;

                    }
                } else {
                   kdDebug(9007) << "no valid declaration to recover!!!" << endl;
                }
            }
        }

    } else if( AST* node = findNodeAt(ast, line, column) ){

        kdDebug(9007) << "------------------- AST FOUND --------------------" << endl;

        if( FunctionDefinitionAST* def = functionDefinition(node) ){

            kdDebug(9007) << "------> found a function definition" << endl;

	    int startLine, startColumn;
	    def->getStartPosition( &startLine, &startColumn );

	    QString contents = getText( startLine, startColumn, line, showArguments ? column-1 : column );

            kdDebug(9007) << "------> computed context" << endl;

	    // kdDebug(9007) << "contents = |" << contents << "|" << endl;

	    int start_expr = expressionAt( contents, contents.length() - 1 );

            kdDebug(9007) << "------> found expression at " << start_expr << endl;
	    // kdDebug(9007) << "start_expr = " << start_expr << endl;
	    if( start_expr != int(contents.length()) - 1 ){
		expr = contents.mid( start_expr, contents.length() - start_expr );
		expr = expr.stripWhiteSpace();
	    }

            if( !expr.isNull() ){
                kdDebug(9007) << "expr = " << expr << endl;
            } else {
                kdDebug(9007) << "no expr found!!" << endl;
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

	    SimpleContext* ctx = computeContext( def, line, column );

            QStringList scope;
            scopeOfNode( def, scope );

            if( scope.size() ){
                    SimpleVariable var;
                    var.type = scope.join( "::" );
                    var.name = "this";
                    ctx->add( var );
                    kdDebug(9007) << "add variable " << var.name << " with type " << var.type << endl;
            }

            type = typeName( evaluateExpression(expr, ctx) );
            delete( ctx );
            ctx = 0;
       }

    }

    m_pSupport->backgroundParser()->unlock();

    if( !type.isEmpty() ){
        kdDebug(9007) << "type = " << type << endl;
	QStringList scope = QStringList::split( "::", type ); // TODO: check :: or . ??!?
	bool isInstance = !expr.endsWith( "::" );

	if( showArguments ){
	    QStringList functionList = getSignatureListForClass( type, word, isInstance );

	    if( functionList.count() == 0 ){
		functionList = getGlobalSignatureList( word );
	    }

	    if( functionList.count() ){
		m_activeCompletion->showArgHint( functionList, "()", "," );
	    }
	} else {
	    QValueList<KTextEditor::CompletionEntry> entryList = findAllEntries( type, true, isInstance );
	    if( entryList.size() )
		m_activeCompletion->showCompletionBox( entryList, word.length() );
	}
    }
}

QStringList CppCodeCompletion::getGlobalSignatureList( const QString& functionName )
{
    QStringList list = m_pSupport->classStore()->globalScope()->getSortedMethodSignatureList( functionName );
    list += m_repository->getSignatureList( QStringList(), functionName, true );
    return list;
}

QStringList CppCodeCompletion::getSignatureListForClass( const QString& className, const QString& functionName, bool isInstance )
{
    QStringList retVal;

    ParsedClass* pClass = dynamic_cast<ParsedClass*>( findContainer(className) );
    if ( !pClass ){
	// check the pcs
	retVal = m_repository->getSignatureList( QStringList::split("::", className), functionName, isInstance );

	QValueList<Tag> parents = m_repository->getBaseClassList( className );
	kdDebug(9020) << "------> found " << parents.size() << " base classes" << endl;
	QValueList<Tag>::Iterator it = parents.begin();
	while( it != parents.end() ){
	    const Tag& tag = *it;
	    ++it;

	    kdDebug(9020) << "found base class " << tag.attribute( "baseClass" ).toString() << endl;
	    retVal += getSignatureListForClass( tag.attribute("baseClass").toString(), functionName, isInstance );
	}
    } else {
	retVal = pClass->getSortedMethodSignatureList( functionName );
	retVal += pClass->getSortedSlotSignatureList( functionName );
	retVal += pClass->getSortedSignalSignatureList( functionName );

	QPtrList<ParsedParent> parentList = pClass->parents;
	for ( ParsedParent* pParentClass = parentList.first(); pParentClass != 0; pParentClass = parentList.next() )
	{
	    // ParsedClass* baseClass = dynamic_cast<ParsedClass*>( findContainer(pParentClass->name()) );
	    retVal += getSignatureListForClass( pParentClass->name(), functionName, isInstance );
	}

    }

    return retVal;
}

void CppCodeCompletion::slotFileParsed( const QString& fileName )
{
    if( fileName != m_activeFileName || !m_pSupport || !m_activeEditor )
	return;

#if 0
    unsigned int line, column;
    m_activeCursor->cursorPositionReal( &line, &column );

    QString strCurLine = m_activeEditor->textLine( line );

    QString ch = strCurLine.mid( column-1, 1 );
    QString ch2 = strCurLine.mid( column-2, 2 );
#endif

    //if( ch2 == "->" || ch == "." || ch == "(" )
    {
        // sync
        //while( m_pSupport->backgroundParser()->filesInQueue() > 0 )
        //    m_pSupport->backgroundParser()->isEmpty().wait();

        m_pSupport->backgroundParser()->lock();
        computeRecoveryPoints();
        m_pSupport->backgroundParser()->unlock();
    }
}

ParsedClassContainer* CppCodeCompletion::findContainer( const QString& n, ParsedScopeContainer* container, const QStringList& imports )
{
    if( !container )
        container = m_pSupport->classStore()->globalScope();

    QString name = n;
    name = name.replace( QRegExp("::"), "." );

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

QValueList<KTextEditor::CompletionEntry> CppCodeCompletion::findAllEntries( const QString& type, bool includePrivate, bool isInstance )
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

		entryList += findAllEntries( p->name(), false, isInstance );
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
    } else {
	QStringList scope = QStringList::split( "::", type );
	entryList = m_repository->getEntriesInScope( scope, isInstance );
	QValueList<Tag> parents = m_repository->getBaseClassList( type ); // type or scope?
	kdDebug(9020) << "------> found " << parents.size() << " base classes" << endl;
	QValueList<Tag>::Iterator it = parents.begin();
	while( it != parents.end() ){
	    const Tag& tag = *it;
	    ++it;

	    kdDebug(9020) << "found base class " << tag.attribute( "baseClass" ).toString() << endl;
	    entryList += findAllEntries( tag.attribute("baseClass").toString(), false, isInstance );
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
    } else {
	QStringList path = QStringList::split( ".", scope->path() );
	QValueList<Tag> tags = m_repository->getTagsInScope( name, path );
        kdDebug(9020) << "------> #" << tags.size() << " tags in scope " << scope->path() << endl;
	if( tags.size() ){
	    const Tag& tag = tags[ 0 ]; // hmmm
	    if( tag.kind() == Tag::Kind_Class || tag.kind() == Tag::Kind_Namespace )
		return tag.name();
	    return tag.attribute( "type" ).toString();
	}

        QValueList<Tag> parents = m_repository->getBaseClassList( path.join("::") );
        kdDebug(9020) << "------> found " << parents.size() << " base classes" << endl;
        QValueList<Tag>::Iterator it = parents.begin();
        while( it != parents.end() ){
            const Tag& tag = *it;
            ++it;

	    kdDebug(9020) << "found base class " << tag.attribute( "baseClass" ).toString() << endl;
	    type = typeOf( tag.attribute( "baseClass" ).toString(), container );
	    if( type )
		return type;
        }
    }

    if( klass )
    {
	QValueList<ParsedMethod*> l = klass->getMethodByName( name );
        if( l.size() )
            return l.front()->type();
    }

    // attributes
    {
	ParsedAttribute* attr = container->getAttributeByName( name );
	if( attr )
	    return attr->type();
    }

    // namespaces
    if( scope )
    {
	ParsedScopeContainer* sc = scope->getScopeByName( name );
	if( sc )
	    return sc->name();
    }

    // inner classes
    {
	ParsedClass* cl = container->getClassByName( name );
	if( cl )
	    return cl->name();
    }

    // inner structs
    {
	ParsedClass* cl = container->getStructByName( name );
	if( cl )
	    return cl->name();
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
		type = typeOf( name, c );
		if( type )
		    return type;
	    }
	}
    }

    return QString::null;
}

void CppCodeCompletion::setupCodeInformationRepository( )
{
    // add all available pcs for now

    int id = 1;
    CppSupportPart* part = m_pSupport;
    QPtrListIterator<Catalog> it( part->catalogList() );
    while( it.current() ){
        Catalog* catalog = it.current();
        ++it;

        m_repository->addCatalog( QString::number(id++), catalog );
    }
}

QString CppCodeCompletion::typeName( const QString& str )
{
    if( str.isEmpty() )
	return QString::null;

    Driver d;
    Lexer lex( &d );
    lex.setSource( str );
    Parser parser( &d, &lex );

    TypeSpecifierAST::Node typeSpec;
    if( parser.parseTypeSpecifier(typeSpec) ){
        NameAST* name = typeSpec->name();

	QPtrList<ClassOrNamespaceNameAST> l = name->classOrNamespaceNameList();
	QPtrListIterator<ClassOrNamespaceNameAST> it( l );

        QString type;
	while( it.current() ){
	    if( it.current()->name() ){
	       type += it.current()->name()->text() + "::";
	    }
	    ++it;
	}

        if( name->unqualifiedName() && name->unqualifiedName()->name() ){
            type += name->unqualifiedName()->name()->text();
        }

        return type;
    }

    return QString::null;
}

SimpleContext* CppCodeCompletion::computeContext( FunctionDefinitionAST * ast, int line, int col )
{
    kdDebug(9007) << "CppCodeCompletion::computeContext() -- main" << endl;
    Q_ASSERT( ast );

    SimpleContext* ctx = new SimpleContext();

    // insert function arguments
    DeclaratorAST* d = ast->initDeclarator()->declarator();
    ParameterDeclarationClauseAST* clause = d->parameterDeclarationClause();
    ParameterDeclarationListAST* params = clause->parameterDeclarationList();
    if( params ){
        QPtrList<ParameterDeclarationAST> l( params->parameterList() );
        QPtrListIterator<ParameterDeclarationAST> it( l );
        while( it.current() ){
	    ParameterDeclarationAST* param = it.current();
            ++it;

	    SimpleVariable var;
	    var.type = typeName( param->typeSpec()->text() );
	    var.name = declaratorToString( param->declarator(), QString::null, true );

	    if( !var.type.isEmpty() ){
	        ctx->add( var );
	        kdDebug(9007) << "add argument " << var.name << " with type " << var.type << endl;
	    }
        }
    }

    computeContext( ctx, ast->functionBody(), line, col );
    return ctx;
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, StatementAST* stmt, int line, int col )
{
    if( !stmt )
        return;

    switch( stmt->nodeType() )
    {
    case NodeType_IfStatement:
	computeContext( ctx, static_cast<IfStatementAST*>(stmt), line, col );
	break;
    case NodeType_WhileStatement:
	computeContext( ctx, static_cast<WhileStatementAST*>(stmt), line, col );
	break;
    case NodeType_DoStatement:
	computeContext( ctx, static_cast<DoStatementAST*>(stmt), line, col );
	break;
    case NodeType_ForStatement:
	computeContext( ctx, static_cast<ForStatementAST*>(stmt), line, col );
	break;
    case NodeType_SwitchStatement:
	computeContext( ctx, static_cast<SwitchStatementAST*>(stmt), line, col );
	break;
    case NodeType_DeclarationStatement:
	computeContext( ctx, static_cast<DeclarationStatementAST*>(stmt), line, col );
	break;
    case NodeType_StatementList:
	computeContext( ctx, static_cast<StatementListAST*>(stmt), line, col );
	break;
    case NodeType_ExpressionStatement:
	break;
    }
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, StatementListAST* ast, int line, int col )
{
    int startLine, startColumn;
    int endLine, endColumn;
    ast->getStartPosition( &startLine, &startColumn );
    ast->getEndPosition( &endLine, &endColumn );

    if( line > endLine || (line == endLine && endColumn < col) )
	return;

    QPtrList<StatementAST> l( ast->statementList() );
    QPtrListIterator<StatementAST> it( l );
    while( it.current() ){
	StatementAST* stmt = it.current();
	++it;

	computeContext( ctx, stmt, line, col );
    }
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, IfStatementAST* ast, int line, int col )
{
    computeContext( ctx, ast->statement(), line, col );
    computeContext( ctx, ast->elseStatement(), line, col );
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, ForStatementAST* ast, int line, int col )
{
    computeContext( ctx, ast->condition(), line, col );
    computeContext( ctx, ast->statement(), line, col );
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, DoStatementAST* ast, int line, int col )
{
    //computeContext( ctx, ast->condition(), line, col );
    computeContext( ctx, ast->statement(), line, col );
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, WhileStatementAST* ast, int line, int col )
{
    computeContext( ctx, ast->condition(), line, col );
    computeContext( ctx, ast->statement(), line, col );
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, SwitchStatementAST* ast, int line, int col )
{
    computeContext( ctx, ast->condition(), line, col );
    computeContext( ctx, ast->statement(), line, col );
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, DeclarationStatementAST* ast, int line, int col )
{
    if( !ast->declaration() || ast->declaration()->nodeType() != NodeType_SimpleDeclaration )
	return;

    int startLine, startColumn;
    int endLine, endColumn;
    ast->getStartPosition( &startLine, &startColumn );
    ast->getEndPosition( &endLine, &endColumn );

    if( line < startLine || (line == startLine && col <= startColumn) )
	return;

    SimpleDeclarationAST* simpleDecl = static_cast<SimpleDeclarationAST*>( ast->declaration() );
    TypeSpecifierAST* typeSpec = simpleDecl->typeSpec();
    QString type = typeName( typeSpec->text() );

    InitDeclaratorListAST* initDeclListAST = simpleDecl->initDeclaratorList();
    if( !initDeclListAST )
        return;

    QPtrList<InitDeclaratorAST> l = initDeclListAST->initDeclaratorList();
    QPtrListIterator<InitDeclaratorAST> it( l );
    while( it.current() ){
	DeclaratorAST* d = it.current()->declarator();
	++it;

        if( d->declaratorId() ){
	    SimpleVariable var;
	    var.type = type;
	    var.name = toSimpleName( d->declaratorId() );
	    ctx->add( var );
            kdDebug(9007) << "add variable " << var.name << " with type " << var.type << endl;
        }
   }
}

void CppCodeCompletion::computeContext( SimpleContext*& ctx, ConditionAST* ast, int line, int col )
{
    if( !ast->typeSpec() || !ast->declarator() || !ast->declarator()->declaratorId() )
	return;

    int startLine, startColumn;
    int endLine, endColumn;
    ast->getStartPosition( &startLine, &startColumn );
    ast->getEndPosition( &endLine, &endColumn );

    if( line < startLine || (line == startLine && col <= startColumn) )
	return;

    QString type = typeName( ast->typeSpec()->text() );
    SimpleVariable var;
    var.type = type;
    var.name = toSimpleName( ast->declarator()->declaratorId() );
    ctx->add( var );
    kdDebug(9007) << "add variable " << var.name << " with type " << var.type << endl;
}

FunctionDefinitionAST * CppCodeCompletion::functionDefinition( AST* node )
{
    while( node ){
	if( node->nodeType() == NodeType_FunctionDefinition )
	    return static_cast<FunctionDefinitionAST*>( node );
	node = node->parent();
    }
    return 0;
}

QString CppCodeCompletion::getText( int startLine, int startColumn, int endLine, int endColumn )
{
    if( startLine == endLine ){
	QString textLine = m_activeEditor->textLine( startLine );
        return textLine.mid( startColumn, endColumn-startColumn );
    }

    QStringList contents;

    for( int line=startLine; line<=endLine; ++line ){
	QString textLine = m_activeEditor->textLine( line );

	if( line == startLine )
	    textLine = textLine.mid( startColumn );
	if( line == endLine )
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
        {
        }

        virtual void parseTranslationUnit( TranslationUnitAST* ast )
        {
            TreeParser::parseTranslationUnit( ast );
            kdDebug(9007) << "found " << recoveryPoints.count() << " recovery points" << endl;
        }

        virtual void parseNamespace( NamespaceAST* ast )
        {
            //insertRecoveryPoint( ast );
            m_currentScope.push_back( ast->namespaceName()->text() );
            TreeParser::parseNamespace( ast );
            m_currentScope.pop_back();
        }

        virtual void parseSimpleDeclaration( SimpleDeclarationAST* ast )
        {
            TypeSpecifierAST* typeSpec = ast->typeSpec();
            //InitDeclaratorListAST* declarators = ast->initDeclaratorList();

            if( typeSpec )
                parseTypeSpecifier( typeSpec );

            //insertRecoveryPoint( ast );
            TreeParser::parseSimpleDeclaration( ast );
        }

        virtual void parseFunctionDefinition( FunctionDefinitionAST* ast )
        {
            insertRecoveryPoint( ast );
        }

        virtual void parseClassSpecifier( ClassSpecifierAST* ast )
        {
            //insertRecoveryPoint( ast );
            m_currentScope.push_back( toSimpleName(ast->name()) );
            TreeParser::parseClassSpecifier( ast );
            m_currentScope.pop_back();
        }

        void insertRecoveryPoint( AST* ast )
        {
            if( !ast )
                return;

            RecoveryPoint* pt = new RecoveryPoint();
            pt->kind = ast->nodeType();
            pt->scope = m_currentScope;
            ast->getStartPosition( &pt->startLine, &pt->startColumn );
            ast->getEndPosition( &pt->endLine, &pt->endColumn );

            recoveryPoints.append( pt );
        }

    private:
        QPtrList<RecoveryPoint>& recoveryPoints;
        QStringList m_currentScope;
    };


void CppCodeCompletion::computeRecoveryPoints( )
{
    kdDebug(9007) << "CppCodeCompletion::computeRecoveryPoints" << endl;

    d->recoveryPoints.clear();
    TranslationUnitAST* unit = m_pSupport->backgroundParser()->translationUnit( m_activeFileName );
    if( !unit )
        return;

    ComputeRecoveryPoints walker( d->recoveryPoints );
    walker.parseTranslationUnit( unit );
}

#include "cppcodecompletion.moc"
