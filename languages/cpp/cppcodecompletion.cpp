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

class SimpleVariable
{
public:
    SimpleVariable() {}
    SimpleVariable( const SimpleVariable& source )
        : name( source.name ), type( source.type ) {}
    ~SimpleVariable(){}

    SimpleVariable& operator = ( const SimpleVariable& source ){
        name = source.name;
        type = source.type;
        return *this;
    }

    QString name;
    QStringList type;
};

class SimpleContext
{
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
    QValueList<QStringList> imports;

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
    QStringList classNameList;

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
	RecoveryPoint* recPt = 0;

        while( it.current() ){
            QPair<int, int> startPt = qMakePair( it.current()->startLine, it.current()->startColumn );
            QPair<int, int> endPt = qMakePair( it.current()->endLine, it.current()->endColumn );

	    if( pt < startPt )
	        break;

	    recPt = it.current();

            ++it;
        }

        return recPt;
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

bool operator < ( const KTextEditor::CompletionEntry& e1, const KTextEditor::CompletionEntry& e2 )
{
    return e1.text < e2.text;
}

static QValueList<KTextEditor::CompletionEntry> unique( const QValueList<KTextEditor::CompletionEntry>& entryList )
{

    QValueList< KTextEditor::CompletionEntry > l;
    QMap<QString, bool> map;
    QValueList< KTextEditor::CompletionEntry >::ConstIterator it=entryList.begin();
    while( it != entryList.end() ){
        KTextEditor::CompletionEntry e = *it++;
        QString key = (e.type + " " +
                      e.prefix + " " +
                      e.text + " " +
                      e.postfix + " ").simplifyWhiteSpace().stripWhiteSpace();
        if( map.find(key) == map.end() ){
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
    QStringList::ConstIterator it=entryList.begin();
    while( it != entryList.end() ){
        QString e = *it++;
        if( map.find(e) == map.end() ){
            map[ e ] = TRUE;
            l << e;
        }
    }
    return l;
}

CppCodeCompletion::CppCodeCompletion( CppSupportPart* part )
    : d( new CppCodeCompletionData ), m_includeRx( "^\\s*#\\s*include\\s+[\"<]" )
{
    m_pSupport = part;

    m_activeCursor = 0;
    m_activeEditor   = 0;
    m_activeCompletion = 0;
    m_ccTimer = new QTimer( this );
    m_ccLine = 0;
    m_ccColumn = 0;
    connect( m_ccTimer, SIGNAL(timeout()), this, SLOT(slotTimeout()) );

    computeFileEntryList();

    CppSupportPart* cppSupport = m_pSupport;
    connect( cppSupport->project(), SIGNAL(addedFilesToProject(const QStringList& )), this, SLOT(computeFileEntryList()) );
    connect( cppSupport->project(), SIGNAL(removedFilesFromProject(const QStringList& )), this, SLOT(computeFileEntryList()) );

    m_bArgHintShow       = false;
    m_bCompletionBoxShow = false;
    m_completionMode     = NormalCompletion;

    m_repository = new CodeInformationRepository( cppSupport->codeRepository() );
    setupCodeInformationRepository();

    if( part->partController()->parts() )
    {
        QPtrListIterator<KParts::Part> it( *part->partController()->parts() );
	while( KParts::Part* part = it.current() )
	{
	    integratePart( part );
	    ++it;
	}
    }

    if( part->partController()->activePart() )
        slotActivePartChanged( part->partController()->activePart() );

    connect( part->partController( ), SIGNAL( partAdded( KParts::Part* ) ),
	     this, SLOT( slotPartAdded( KParts::Part* ) ) );
    connect( part->partController( ), SIGNAL( activePartChanged( KParts::Part* ) ),
	     this, SLOT( slotActivePartChanged( KParts::Part* ) ) );

    connect( part, SIGNAL(fileParsed(const QString&)), this, SLOT(slotFileParsed(const QString&)) );
}

CppCodeCompletion::~CppCodeCompletion( )
{
    delete m_repository;
    delete d;
}

void CppCodeCompletion::slotTimeout()
{
    if( !m_activeCursor || !m_activeEditor || !m_activeCompletion )
        return;

    uint nLine, nCol;
    m_activeCursor->cursorPositionReal( &nLine, &nCol );

    if( nLine != m_ccLine || nCol != m_ccColumn )
	return;

    QString textLine = m_activeEditor->textLine( nLine );
    QChar ch = textLine[ nCol ];;
    if( ch.isLetterOrNumber() || ch == '_' )
	return;

    completeText();
}

void CppCodeCompletion::slotArgHintHided( )
{
    //kdDebug(9007) << "CppCodeCompletion::slotArgHintHided()" << endl;
    m_bArgHintShow = false;
}

void CppCodeCompletion::slotCompletionBoxHided( KTextEditor::CompletionEntry entry )
{
    Q_UNUSED( entry );
    m_bCompletionBoxShow = false;
}

void CppCodeCompletion::integratePart( KParts::Part* part )
{
    if( !part || !part->widget() )
        return;

    if( KTextEditor::Document* doc = dynamic_cast<KTextEditor::Document*>( part ) )
    {
        kdDebug(9007) << "=================> integrate document: " << doc << endl;

	if( m_pSupport && m_pSupport->codeCompletionConfig()->automaticCodeCompletion() ){
		kdDebug( 9007 ) << "enabling code completion" << endl;
		connect(part, SIGNAL(textChanged()), this, SLOT(slotTextChanged()) );
		connect(part->widget(), SIGNAL( completionDone( KTextEditor::CompletionEntry ) ), this,
			SLOT( slotCompletionBoxHided( KTextEditor::CompletionEntry ) ) );
		connect(part->widget(), SIGNAL( argHintHidden() ), this,
			SLOT( slotArgHintHided() ) );
	}

    }
}

void CppCodeCompletion::slotPartAdded(KParts::Part *part)
{
    integratePart( part );
}

void CppCodeCompletion::slotActivePartChanged(KParts::Part *part)
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

    kdDebug(9007) << "CppCodeCompletion::slotActivePartChanged() -- end" << endl;
}

void CppCodeCompletion::slotTextChanged()
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

    if( (m_pSupport->codeCompletionConfig()->automaticCodeCompletion() && (ch == "." || ch2 == "->" || ch2 == "::")) ||
	(m_pSupport->codeCompletionConfig()->automaticArgumentsHint() && ch == "(") ||
	(m_pSupport->codeCompletionConfig()->automaticHeaderCompletion() && (ch == "\"" || ch == "<") && m_includeRx.search(strCurLine) != -1 ) ){
	m_ccLine = nLine;
	m_ccColumn = nCol;
	m_ccTimer->start( ch == "(" ? m_pSupport->codeCompletionConfig()->argumentsHintDelay() : m_pSupport->codeCompletionConfig()->codeCompletionDelay(), true );
    }
}

enum { T_ACCESS, T_PAREN, T_BRACKET, T_IDE, T_UNKNOWN };

int CppCodeCompletion::expressionAt( const QString& text, int index )
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

QStringList CppCodeCompletion::splitExpression( const QString& text )
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

QStringList CppCodeCompletion::evaluateExpression( QString expr, SimpleContext* ctx )
{
    d->classNameList = typeNameList( m_pSupport->codeModel() );
    
    bool global = false;
    if( expr.startsWith("::") ){
	expr = expr.mid( 2 );
	global = true;
    }

    QStringList exprList = splitExpression( expr );
    QStringList type = evaluateExpressionInternal( exprList, QStringList(), ctx );

    m_pSupport->mainWindow()->statusBar()->message( i18n("Type of %1 is %2").arg(expr).arg(type.join("::")), 1000 );

    QMap<QString, bool> visited;
    bool isTypedef = true;
    
    while( isTypedef && !type.isEmpty() ) {
	// check if the evaluateded type is a typedef
	QValueList<Catalog::QueryArgument> args;
	args << Catalog::QueryArgument( "kind", Tag::Kind_Typedef );
	args << Catalog::QueryArgument( "name", type.last() );
	QValueList<Tag> tags = m_repository->query( args );
	isTypedef = !tags.isEmpty() && !visited.contains( type.last() );
		
	if( isTypedef ) {
		visited[ type.last() ] = true;

		bool found = false;
		for( QValueList<Tag>::Iterator it=tags.begin(); it!=tags.end(); ++it ) {
			Tag& tag = *it;
			
			QStringList tp = type;
			tp.pop_back();
			
			QString typeScope = tp.join( "::" );
			QString tagScope = tag.scope().join( "::" );
			
			//kdDebug(9007) << "=========> typeScope: " << typeScope << endl;
			//kdDebug(9007) << "=========> tagScope: " << tagScope << endl;
			
			if( typeScope.endsWith(tagScope) ) {
			    type = typeName( tag.attribute("t").toString() );
			    found = true;
			    break;
			}
		}
		
		if( !found )
			type = typeName( tags[0].attribute("t").toString() );
	}
    } 
    
    return type;
}

QStringList CppCodeCompletion::evaluateExpressionInternal( QStringList & exprList, const QStringList & scope, SimpleContext * ctx )
{
    //kdDebug(9007) << "evaluateExpression " << exprList << " in " << scope << endl;

    if( exprList.isEmpty() )
	return scope;

    QString currentExpr = exprList.front().stripWhiteSpace();
    exprList.pop_front();

    int leftParen = currentExpr.find( "(" );

    if( leftParen != -1 )
	currentExpr = currentExpr.left( leftParen ).stripWhiteSpace();

    if( currentExpr.contains("::") ){
	if( currentExpr.endsWith("::") )
	    currentExpr.truncate( currentExpr.length() - 2 );

	QStringList type = typeName( currentExpr );
	if( !type.isEmpty() ){
	    if( leftParen != -1 ){
		QString name = type.back();
		type.pop_back();
		type = typeOf( name, type );
	    }

	    if( !type.isEmpty() )
		return evaluateExpressionInternal( exprList, type );
	}
    }

    if( ctx ){
	// find the variable type in the current context
	QStringList type = ctx->findVariable( currentExpr ).type;
	if( !type.isEmpty() )
	    return evaluateExpressionInternal( exprList, type );

	QStringList t_this = ctx->findVariable( "this" ).type;
	if( !t_this.isEmpty() ){
	    QStringList type = typeOf( currentExpr, t_this );
	    if( !type.isEmpty() )
		return evaluateExpressionInternal( exprList, type );

	    return QStringList();
	}
    }

    QStringList type = typeOf( currentExpr, scope );
    if( !type.isEmpty() )
	return evaluateExpressionInternal( exprList, type );

    return QStringList();
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

    QString ch = strCurLine.mid( nCol-1, 1 );
    QString ch2 = strCurLine.mid( nCol-2, 2 );

    if( m_includeRx.search(strCurLine) != -1 ){
	if( !m_fileEntryList.isEmpty() ){
	    m_activeCompletion->showCompletionBox( m_fileEntryList, column - m_includeRx.matchedLength() );
	}
        return;
    }

    bool showArguments = false;
    bool isInstance = true;
    m_completionMode = NormalCompletion;

    if( ch2 == "->" || ch == "." || ch == "(" ){
	int pos = ch2 == "->" ? nCol - 3 : nCol - 2;
	QChar c = strCurLine[ pos ];
	while( pos > 0 && c.isSpace() )
	    c = strCurLine[ --pos ];

	if( !(c.isLetterOrNumber() || c == '_' || c == ')') )
	    return;
    }

    if( ch == "(" ){
        --nCol;
	while( nCol > 0 && strCurLine[nCol].isSpace() )
	   --nCol;

        showArguments = TRUE;
    }

    QStringList type, this_type;
    QString expr, word;

    DeclarationAST::Node recoveredDecl;

    SimpleContext* ctx = 0;

    m_pSupport->backgroundParser()->lock();

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
	    /// @todo setup the lexer(i.e. adds macro, special words, ...

	    lexer.setSource( textToReparse );
	    Parser parser( &d, &lexer );

	    parser.parseDeclaration( recoveredDecl );
	    //kdDebug(9007) << "recoveredDecl = " << recoveredDecl.get() << endl;
	    if( recoveredDecl.get() ){

		bool isFunDef = recoveredDecl->nodeType() == NodeType_FunctionDefinition;
		kdDebug(9007) << "is function definition = " << isFunDef << endl;

		int endLine, endColumn;
		recoveredDecl->getEndPosition( &endLine, &endColumn );
		kdDebug(9007) << "endLine = " << endLine << ", endColumn " << endColumn << endl;

		/// @todo check end position

		if( isFunDef ) {
		    FunctionDefinitionAST* def = static_cast<FunctionDefinitionAST*>( recoveredDecl.get() );

		    /// @todo remove code duplication

		    QString contents = textToReparse;
		    int start_expr = expressionAt( contents, contents.length() - 1 );

		    // kdDebug(9007) << "start_expr = " << start_expr << endl;
		    if( start_expr != int(contents.length()) - 1 )
			expr = contents.mid( start_expr, contents.length() - start_expr ).stripWhiteSpace();

                    if( expr.startsWith("SIGNAL") || expr.startsWith("SLOT") ){
			m_completionMode = expr.startsWith("SIGNAL") ? SignalCompletion : SlotCompletion;

		        showArguments = false;
			int end_expr = start_expr - 1;
                        while( end_expr > 0 && contents[end_expr].isSpace() )
                            --end_expr;

                        if( contents[end_expr] != ',' ){
                            expr = QString::null;
                        } else {
                            --end_expr;
                            start_expr = expressionAt( contents, end_expr );
			    expr = contents.mid( start_expr, end_expr - start_expr + 1 ).stripWhiteSpace();
                        }
                    } else {
		    	int idx = expr.length() - 1;
		    	while( expr[idx].isLetterOrNumber() || expr[idx] == '_' )
			    --idx;

		        if( idx != int(expr.length()) - 1 ){
			    ++idx;
			    word = expr.mid( idx ).stripWhiteSpace();
			    expr = expr.left( idx ).stripWhiteSpace();
		        }
		    }

		    ctx = computeContext( def, endLine, endColumn );
		    DeclaratorAST* d = def->initDeclarator()->declarator();
		    NameAST* name = d->declaratorId();
		    QString scope = recoveryPoint->scope.join( "::" );

		    QStringList nested;
			
		    QPtrList<ClassOrNamespaceNameAST> l;
			if ( name )
			{
				l = name->classOrNamespaceNameList();
			}
//		    QPtrList<ClassOrNamespaceNameAST> l = name->classOrNamespaceNameList();
		    QPtrListIterator<ClassOrNamespaceNameAST> nameIt( l );
		    while( nameIt.current() ){
			if( nameIt.current()->name() ){
			    nested << nameIt.current()->name()->text();
			}
			++nameIt;
		    }

		    QString s = nested.join( "::" );

		    if( !scope.isEmpty() ){
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

		    if( !scope.isEmpty() ){
			SimpleVariable var;
			this_type = typeName( scope );
			var.type = this_type;
			var.name = "this";
			ctx->add( var );
		    }

		    type = evaluateExpression( expr, ctx );
		}
	    } else {
		kdDebug(9007) << "no valid declaration to recover!!!" << endl;
	    }
	}
    }

    if( !recoveredDecl.get() ){
	TranslationUnitAST* ast = m_pSupport->backgroundParser()->translationUnit( m_activeFileName );
	if( AST* node = findNodeAt(ast, line, column) ){

	    kdDebug(9007) << "------------------- AST FOUND --------------------" << endl;

	    if( FunctionDefinitionAST* def = functionDefinition(node) ){

		kdDebug(9007) << "------> found a function definition" << endl;

		int startLine, startColumn;
		def->getStartPosition( &startLine, &startColumn );

		QString contents = getText( startLine, startColumn, line, showArguments ? column-1 : column );


		/// @todo remove code duplication
		int start_expr = expressionAt( contents, contents.length() - 1 );

		    // kdDebug(9007) << "start_expr = " << start_expr << endl;
		    if( start_expr != int(contents.length()) - 1 )
			expr = contents.mid( start_expr, contents.length() - start_expr ).stripWhiteSpace();

                    if( expr.startsWith("SIGNAL") || expr.startsWith("SLOT") ){
			m_completionMode = expr.startsWith("SIGNAL") ? SignalCompletion : SlotCompletion;

		        showArguments = false;
			int end_expr = start_expr - 1;
                        while( end_expr > 0 && contents[end_expr].isSpace() )
                            --end_expr;

                        if( contents[end_expr] != ',' ){
                            expr = QString::null;
                        } else {
                            --end_expr;
                            start_expr = expressionAt( contents, end_expr );
			    expr = contents.mid( start_expr, end_expr - start_expr + 1 ).stripWhiteSpace();
                        }
                    } else {
		    	int idx = expr.length() - 1;
		    	while( expr[idx].isLetterOrNumber() || expr[idx] == '_' )
			    --idx;

		        if( idx != int(expr.length()) - 1 ){
			    ++idx;
			    word = expr.mid( idx ).stripWhiteSpace();
			    expr = expr.left( idx ).stripWhiteSpace();
		        }
		    }    

		ctx = computeContext( def, line, column );

		QStringList scope;
		scopeOfNode( def, scope );
		this_type = scope;

		if( scope.size() ){
		    SimpleVariable var;
		    var.type = scope;
		    var.name = "this";
		    ctx->add( var );
		    //kdDebug(9007) << "add variable " << var.name << " with type " << var.type << endl;
		}

		type = evaluateExpression( expr, ctx );
	    }
	}
    }
    m_pSupport->backgroundParser()->unlock();

    if( !ctx )
	return;

    if( ch2 == "::" || expr.isEmpty() ){
	isInstance = false;
    }

    kdDebug(9007) << "===========================> type is: " << type.join(" ") << endl;
    kdDebug(9007) << "===========================> word is: " << word << endl;

    if( !showArguments ){
	QValueList<KTextEditor::CompletionEntry> entryList;

	if( type.isEmpty() && expr.isEmpty() ){
	    computeCompletionEntryList( entryList, ctx, isInstance );
	    computeCompletionEntryList( entryList, m_pSupport->codeModel()->globalNamespace(), isInstance );

            if( m_pSupport->codeCompletionConfig()->includeGlobalFunctions() )
	        computeCompletionEntryList( entryList, QStringList(), false );

	    if( this_type.size() )
		computeCompletionEntryList( entryList, this_type, isInstance );

	} else if( !type.isEmpty() ){
	    computeCompletionEntryList( entryList, type, isInstance );
	}

	if( entryList.size() ){
	    entryList = unique( entryList );
	    qHeapSort( entryList );

	    m_activeCompletion->showCompletionBox( entryList, word.length() );
	}
    } else {
	QStringList signatureList;

	if( type.isEmpty() && expr.isEmpty() ){
	    computeSignatureList( signatureList, word, m_pSupport->codeModel()->globalNamespace()->functionList() );
	    computeSignatureList( signatureList, word, QStringList() );

	    if( !word.isEmpty() ){
		QStringList fakeType;
		fakeType << word;
		computeSignatureList( signatureList, word, fakeType );
	    }
	} else if( !type.isEmpty() ){
	    computeSignatureList( signatureList, word, type );
	}

	if( !this_type.isEmpty() && expr.isEmpty() )
	    computeSignatureList( signatureList, word, this_type );

	if( !signatureList.isEmpty() ){
	    signatureList = unique( signatureList );
	    qHeapSort( signatureList );
	    m_activeCompletion->showArgHint( signatureList, "()", "," );
	}
    }

    delete( ctx );
    ctx = 0;
}

void CppCodeCompletion::slotFileParsed( const QString& fileName )
{
    if( fileName != m_activeFileName || !m_pSupport || !m_activeEditor )
	return;

    m_pSupport->backgroundParser()->lock();
    computeRecoveryPoints();
    m_pSupport->backgroundParser()->unlock();
}

QStringList CppCodeCompletion::typeOf( const QString& name, const QStringList& scope )
{
    QStringList type;

    if( name.isEmpty() )
	return type;

    QString key = findClass( scope.join( "::" ) );
    ClassDom klass = findContainer( key );
    if( klass ){
	return typeOf( name, klass );
    } else {
	type = typeOf( name, m_pSupport->codeModel()->globalNamespace() );
	if( !type.isEmpty() )
	    return type;
    }

    QValueList<Catalog::QueryArgument> args;
    QTime t;

    t.start();
    args << Catalog::QueryArgument( "scope", scope );
/*    if( name.length() >=2 )
        args << Catalog::QueryArgument( "prefix", name.left(2) );*/
    args << Catalog::QueryArgument( "name", name );
    QValueList<Tag> tags( m_repository->query(args) );
    kdDebug(9007) << "type of " << name << " in " << scope.join("::") << " takes " << t.elapsed() << endl;
    type = typeOf( tags );

    if( type.isEmpty() ){
	// try with parents
	t.restart();
	QValueList<Tag> parents( m_repository->getBaseClassList( scope.join("::") ) );
	kdDebug(9007) << "get parents of " << scope.join("::") << " takes " << t.elapsed() << endl;
	QValueList<Tag>::Iterator it = parents.begin();
	while( it != parents.end() ){
	    Tag& tag = *it;
	    ++it;

	    CppBaseClass<Tag> info( tag );

	    QStringList newScope = typeName( info.baseClass() );
	    type = typeOf( name, newScope );
	    if( !type.isEmpty() )
		break;
	}
    }

    return type;
}

void CppCodeCompletion::setupCodeInformationRepository( )
{
}

QStringList CppCodeCompletion::typeName( const QString& str )
{
    if( str.isEmpty() )
	return QStringList();

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

        return QStringList::split( "::", type );
    }

    return QStringList();
}

SimpleContext* CppCodeCompletion::computeContext( FunctionDefinitionAST * ast, int line, int col )
{
    kdDebug(9007) << "CppCodeCompletion::computeContext() -- main" << endl;

    SimpleContext* ctx = new SimpleContext();

    if( ast && ast->initDeclarator() && ast->initDeclarator()->declarator() ){
	DeclaratorAST* d = ast->initDeclarator()->declarator();
	if( ParameterDeclarationClauseAST* clause = d->parameterDeclarationClause() ){
	    if( ParameterDeclarationListAST* params = clause->parameterDeclarationList() ){
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
			//kdDebug(9007) << "add argument " << var.name << " with type " << var.type << endl;
		    }
		}
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
    QStringList type = typeName( typeSpec->text() );

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
            //kdDebug(9007) << "add variable " << var.name << " with type " << var.type << endl;
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

    QStringList type = typeName( ast->typeSpec()->text() );
    SimpleVariable var;
    var.type = type;
    var.name = toSimpleName( ast->declarator()->declaratorId() );
    ctx->add( var );
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
	QValueList<QStringList> dummy;

	m_imports.push( dummy );
	TreeParser::parseTranslationUnit( ast );
	m_imports.pop();

	kdDebug(9007) << "found " << recoveryPoints.count() << " recovery points" << endl;
    }

    virtual void parseUsingDirective( UsingDirectiveAST* ast )
    {
	if( !ast->name() )
	    return;

	QStringList type = CppCodeCompletion::typeName( ast->name()->text() );
	m_imports.top().push_back( type );
    }

    virtual void parseNamespace( NamespaceAST* ast )
    {
	//insertRecoveryPoint( ast );
	m_currentScope.push_back( ast->namespaceName()->text() );

	m_imports.push( m_imports.top() ); // dup
	m_imports.top().push_back( m_currentScope );

	TreeParser::parseNamespace( ast );

	m_imports.pop();
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
	m_imports.push( m_imports.top() ); // dup
	insertRecoveryPoint( ast );
	m_imports.pop();
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
    kdDebug(9007) << "CppCodeCompletion::computeRecoveryPoints" << endl;

    d->recoveryPoints.clear();
    TranslationUnitAST* unit = m_pSupport->backgroundParser()->translationUnit( m_activeFileName );
    if( !unit )
        return;

    ComputeRecoveryPoints walker( d->recoveryPoints );
    walker.parseTranslationUnit( unit );
}

QStringList CppCodeCompletion::typeOf( const QValueList< Tag > & tags )
{
    QValueList<Tag>::ConstIterator it = tags.begin();
    while( it != tags.end() ){
	const Tag& tag = *it;
	++it;

	if( tag.hasAttribute("t") )
	    return typeName( tag.attribute("t").toString() );
	else if( tag.kind() == Tag::Kind_Class || tag.kind() == Tag::Kind_Namespace ){
	    QStringList l = tag.scope();
	    l += typeName( tag.name() );
	    return l;
	}
    }

    return QStringList();
}

QStringList CppCodeCompletion::typeOf( const QString & name, ClassDom klass )
{
    QStringList type;

    if( klass->hasVariable(name) )
	return typeName( klass->variableByName(name)->type() );

    type = typeOf( name, klass->functionList() );
    if( !type.isEmpty() )
	return type;

    QStringList parents = klass->baseClassList();
    for( QStringList::Iterator it=parents.begin(); it!=parents.end(); ++it ){
	type = typeOf( name, typeName(*it) );
	if( !type.isEmpty() )
	    return type;
    }

    return QStringList();
}

QStringList CppCodeCompletion::typeOf( const QString & name, NamespaceDom scope )
{
    if( scope->hasVariable(name) )
	return typeName( scope->variableByName(name)->type() );

    QStringList type;

    type = typeOf( name, scope->functionList() );
    if( !type.isEmpty() )
	return type;

    return QStringList();
}

QStringList CppCodeCompletion::typeOf( const QString & name, const FunctionList & methods )
{
    FunctionList::ConstIterator it = methods.begin();
    while( it != methods.end() ){
	FunctionDom meth = *it;
	++it;

	if( meth->name() == name )
	    return typeName( meth->resultType() );
    }

    return QStringList();
}

void CppCodeCompletion::computeCompletionEntryList( QValueList< KTextEditor::CompletionEntry > & entryList, const QStringList & type, bool isInstance )
{
    CppCodeCompletionConfig* cfg = m_pSupport->codeCompletionConfig();
    QString key = findClass( type.join( "::" ) );
    ClassDom klass = findContainer( key );
    if( klass ){
	computeCompletionEntryList( entryList, klass, isInstance );
    } /*else*/ {
	QValueList<Catalog::QueryArgument> args;
	QValueList<Tag> tags;

	args.clear();
	args << Catalog::QueryArgument( "kind", Tag::Kind_FunctionDeclaration )
	    << Catalog::QueryArgument( "scope", type );
	tags = m_repository->query( args );
	computeCompletionEntryList( entryList, tags, isInstance );

	args.clear();
	args << Catalog::QueryArgument( "kind", Tag::Kind_Variable )
	    << Catalog::QueryArgument( "scope", type );
	tags = m_repository->query( args );
	computeCompletionEntryList( entryList, tags, isInstance );

	if( !isInstance && cfg->includeEnums() ){
	    args.clear();
	    args << Catalog::QueryArgument( "kind", Tag::Kind_Enumerator )
		<< Catalog::QueryArgument( "scope", type );
	    tags = m_repository->query( args );
	    computeCompletionEntryList( entryList, tags, isInstance );
	}

	if( !isInstance && cfg->includeTypedefs() ){
	    args.clear();
	    args << Catalog::QueryArgument( "kind", Tag::Kind_Typedef )
		<< Catalog::QueryArgument( "scope", type );
	    tags = m_repository->query( args );
	    computeCompletionEntryList( entryList, tags, isInstance );
	}

	args.clear();
	args << Catalog::QueryArgument( "kind", Tag::Kind_Base_class );
	QString fullname = type.join( "::" );
/*    	if( fullname.length() >=2 )
            args << Catalog::QueryArgument( "prefix", fullname.left(2) );*/
	args << Catalog::QueryArgument( "name", fullname );

	QValueList<Tag> parents = m_repository->query( args );
	QValueList<Tag>::Iterator it = parents.begin();
	while( it != parents.end() ){
	    CppBaseClass<Tag> info( *it );
	    ++it;

	    computeCompletionEntryList( entryList, typeName(info.baseClass()), isInstance );
	}
    }
 }

void CppCodeCompletion::computeCompletionEntryList( QValueList< KTextEditor::CompletionEntry > & entryList, QValueList< Tag > & tags, bool /*isInstance*/ )
{
    QValueList<Tag>::Iterator it = tags.begin();
    while( it != tags.end() ){
	Tag& tag = *it;
	++it;

	if( tag.name().isEmpty() ){
	    continue;
	} else if( m_completionMode != NormalCompletion ){
	    if( tag.kind() != Tag::Kind_FunctionDeclaration )
	        continue;

	    CppFunction<Tag> info( tag );

	    if( m_completionMode == SlotCompletion && !info.isSlot() )
	        continue;
	    else if( m_completionMode == SignalCompletion && !info.isSignal() )
	        continue;
	}

	entryList << CodeInformationRepository::toEntry( tag, m_completionMode );
    }
}

void CppCodeCompletion::computeCompletionEntryList( QValueList< KTextEditor::CompletionEntry > & entryList, ClassDom klass, bool isInstance )
{
    computeCompletionEntryList( entryList, klass->functionList(), isInstance );
    if( m_completionMode == NormalCompletion )
        computeCompletionEntryList( entryList, klass->variableList(), isInstance );

    QStringList parents = klass->baseClassList();
    for( QStringList::Iterator it=parents.begin(); it!=parents.end(); ++it ){
	QStringList type = typeName( *it );

	if( !type.isEmpty() )
	    computeCompletionEntryList( entryList, type, isInstance );
    }
}

void CppCodeCompletion::computeCompletionEntryList( QValueList< KTextEditor::CompletionEntry > & entryList, NamespaceDom scope, bool isInstance )
{
    CppCodeCompletionConfig* cfg = m_pSupport->codeCompletionConfig();

    if( cfg->includeGlobalFunctions() ){
	computeCompletionEntryList( entryList, scope->functionList(), isInstance );

        if( m_completionMode == NormalCompletion )
	    computeCompletionEntryList( entryList, scope->variableList(), isInstance );
    }

    if( !isInstance && cfg->includeTypes() ){
	computeCompletionEntryList( entryList, scope->classList(), isInstance );
	computeCompletionEntryList( entryList, scope->namespaceList(), isInstance );
    }
}

void CppCodeCompletion::computeCompletionEntryList( QValueList< KTextEditor::CompletionEntry > & entryList, const ClassList & lst, bool isInstance )
{
    CppCodeCompletionConfig* cfg = m_pSupport->codeCompletionConfig();

    ClassList::ConstIterator it = lst.begin();
    while( it != lst.end() ){
	ClassDom klass = *it;
	++it;

	KTextEditor::CompletionEntry entry;
	entry.prefix = "class";
	entry.text = klass->name();
	entryList << entry;

	if( cfg->includeTypes() ){
	    computeCompletionEntryList( entryList, klass->classList(), isInstance );
	}
    }
}

void CppCodeCompletion::computeCompletionEntryList( QValueList< KTextEditor::CompletionEntry > & entryList, const NamespaceList & lst, bool /*isInstance*/ )
{
    NamespaceList::ConstIterator it = lst.begin();
    while( it != lst.end() ){
	NamespaceDom scope = *it;
	++it;

	KTextEditor::CompletionEntry entry;
	entry.prefix = "namespace";
	entry.text = scope->name();
	entryList << entry;
    }
}

void CppCodeCompletion::computeCompletionEntryList( QValueList< KTextEditor::CompletionEntry > & entryList, const FunctionList & methods, bool isInstance )
{
    FunctionList::ConstIterator it = methods.begin();
    while( it != methods.end() ){
	FunctionDom meth = *it;
	++it;

	if( isInstance && meth->isStatic() )
	    continue;
	else if( m_completionMode == SignalCompletion && !meth->isSignal() )
	    continue;
	else if( m_completionMode == SlotCompletion && !meth->isSlot() )
	    continue;

	KTextEditor::CompletionEntry entry;
	//entry.prefix = meth->type();

	entry.text = meth->name() + "(";

	QString text;

	ArgumentList args = meth->argumentList();
	ArgumentList::Iterator argIt = args.begin();
	while( argIt != args.end() ){
	    ArgumentDom arg = *argIt;
	    ++argIt;

	    text += arg->type();
	    if( m_completionMode == NormalCompletion )
	        text += QString(" ") + arg->name();

	    if( argIt != args.end() )
		text += ", ";
	}

	if( text.isEmpty() )
	    entry.text += ")";
	else
	    text += " )";

	if( meth->isConstant() )
	    text += " const";

        if( m_completionMode != NormalCompletion )
            entry.text += text.stripWhiteSpace();
	else
            entry.postfix = text;

	entryList << entry;
    }
}

void CppCodeCompletion::computeCompletionEntryList( QValueList< KTextEditor::CompletionEntry > & entryList, const VariableList & attributes, bool isInstance )
{
    if( m_completionMode != NormalCompletion )
        return;

    VariableList::ConstIterator it = attributes.begin();
    while( it != attributes.end() ){
	VariableDom attr = *it;
	++it;

	if( isInstance && attr->isStatic() )
	    continue;

	KTextEditor::CompletionEntry entry;
	entry.text = attr->name();
	entryList << entry;
    }
}

void CppCodeCompletion::computeCompletionEntryList( QValueList< KTextEditor::CompletionEntry > & entryList, SimpleContext * ctx, bool /*isInstance*/ )
{
    while( ctx ){
	QValueList<SimpleVariable> vars = ctx->vars();
	QValueList<SimpleVariable>::ConstIterator it = vars.begin();
	while( it != vars.end() ){
	    const SimpleVariable& var = *it;
	    ++it;

	    KTextEditor::CompletionEntry entry;
	    entry.text = var.name;
	    entryList << entry;
	}
	ctx = ctx->prev();
    }
}

void CppCodeCompletion::computeSignatureList( QStringList & signatureList, const QString & name, const QStringList & scope )
{
    QString key = findClass( scope.join( "::" ) );
    ClassDom klass = findContainer( key );
    if( klass ){
	computeSignatureList( signatureList, name, klass );
	return;
    }

    QValueList<Catalog::QueryArgument> args;
    args << Catalog::QueryArgument( "kind", Tag::Kind_FunctionDeclaration );
    args << Catalog::QueryArgument( "scope", scope );
/*    if( name.length() >=2 )
        args << Catalog::QueryArgument( "prefix", name.left(2) );    */
    args << Catalog::QueryArgument( "name", name );

    QValueList<Tag> tags = m_repository->query( args );
    computeSignatureList( signatureList, name, tags );

    args.clear();
    args << Catalog::QueryArgument( "kind", Tag::Kind_Base_class );
    QString fullname = scope.join( "::" );
/*    if( fullname.length() >=2 )
        args << Catalog::QueryArgument( "prefix", fullname.left(2) );*/
    
    args << Catalog::QueryArgument( "name", fullname );

    QValueList<Tag> parents = m_repository->query( args );
    QValueList<Tag>::Iterator it = parents.begin();
    while( it != parents.end() ){
	CppBaseClass<Tag> info( *it );
	++it;

	computeSignatureList( signatureList, name, typeName(info.baseClass()) );
    }
}

void CppCodeCompletion::computeSignatureList( QStringList & signatureList, const QString & name, ClassDom klass )
{
    computeSignatureList( signatureList, name, klass->functionList() );

    QStringList parents = klass->baseClassList();
    for( QStringList::Iterator it = parents.begin(); it!=parents.end(); ++it ){
        QStringList type = typeName( *it );
	if( !type.isEmpty() )
	    computeSignatureList( signatureList, name, type );
    }
}

void CppCodeCompletion::computeSignatureList( QStringList & signatureList, const QString & name, const FunctionList & methods )
{
    FunctionList::ConstIterator it = methods.begin();
    while( it != methods.end() ){
	FunctionDom meth = *it;
	++it;

	if( meth->name() != name )
	    continue;

	QString signature;
	signature += meth->resultType() + " ";
	signature += meth->name() + "(";

	ArgumentList args = meth->argumentList();
	ArgumentList::Iterator argIt = args.begin();
	while( argIt != args.end() ){
	    ArgumentDom arg = *argIt;
	    ++argIt;

	    signature += arg->type() + " " + arg->name();
	    signature = signature.stripWhiteSpace();

	    if( argIt != args.end() )
		signature += ", ";
	}

	signature += ")";

	if( meth->isConstant() )
	    signature += " const";

	signatureList << signature.stripWhiteSpace();
    }
}

void CppCodeCompletion::computeSignatureList( QStringList & signatureList, const QString & name, QValueList< Tag > & tags )
{
    QValueList<Tag>::Iterator it = tags.begin();
    while( it != tags.end() ){
	Tag& tag = *it;
	++it;

	CppFunction<Tag> info( tag );
	if( info.name() != name )
	    continue;

        QString signature;
	signature += info.type() + " " + tag.name() + "(";
        signature = signature.stripWhiteSpace();

        QStringList arguments = info.arguments();
        QStringList argumentNames = info.argumentNames();

        for( uint i=0; i<arguments.size(); ++i ){
            signature += arguments[ i ];
	    if( m_completionMode == NormalCompletion ){
                QString argName = argumentNames[ i ];
                if( !argName.isEmpty() )
                    signature += QString::fromLatin1( " " ) + argName;

                if( i != (arguments.size()-1) ){
                    signature += ", ";
                }
	    }
        }
        signature += ")";

	if( info.isConst() )
	    signature += " const";

	signatureList << signature.stripWhiteSpace();
    }
}

QString CppCodeCompletion::findClass( const QString & className )
{
    if( className.isEmpty() )
        return className;

    QStringList lst = d->classNameList.grep( QRegExp("\\b" + className + "$") );
    if( lst.size() ){
        kdDebug(9007) << "found class: " << lst[ 0 ] << endl;
	return lst[ 0 ];
    }

    return className;
}

ClassDom CppCodeCompletion::findContainer( const QString& name, NamespaceDom container, bool includeImports )
{
    kdDebug(9007) << "-----------> findContainer name: " << name << " " << (container ? container->name() : QString("")) << endl;

    if( name.isEmpty() )
	return model_cast<ClassDom>( container );

    if( !container )
	return findContainer( name, m_pSupport->codeModel()->globalNamespace(), includeImports );

    QStringList path = QStringList::split( "::", name );
    QStringList::Iterator it = path.begin();
    while( it != path.end() ){
        QString s = *it;
        QStringList::Iterator sv_it = it;
        ++it;

	kdDebug(9007) << "has namespace " << s << ": " << container->hasNamespace( s ) << endl;
	if( !container->hasNamespace(s) )
	    break;

        NamespaceDom scope = container->namespaceByName( s );

        path.remove( sv_it );
        container = scope;
    }

    if( path.size() == 0 )
	return model_cast<ClassDom>( container );

    QString className = path.join( "::" );
    kdDebug(9007) << "find class: " << className << " in namespace " << container->name() << endl;

    ClassDom c = model_cast<ClassDom>( container );
    while( c && path.size() ){
	QString s = path.front();
	path.pop_front();

	if( !c->hasClass(s) ){
	    c = 0;
	    break;
	}

	ClassList classList = c->classByName( s );
	c = classList[ 0 ];
	for( ClassList::Iterator cit=classList.begin(); cit!=classList.end(); ++cit ){
	    if( QFileInfo( (*cit)->fileName() ).dirPath(true) == QFileInfo( m_activeFileName ).dirPath(true) )
		c = *cit;
	}
    }

    kdDebug(9007) << "found class: " << (c ? c->name() : QString("<anon>")) << endl;

#if 0
    if( !c && includeImports ){

        QStringList imports;
        QValueList<QStringList>::Iterator lIt = m_imports.begin();
        while( lIt != m_imports.end() ){
           imports += (*lIt );
           ++lIt;
        }

        QStringList::Iterator impIt = imports.begin();
        while( impIt != imports.end() ){
            ClassDom kl = findContainer( (*impIt) + "::" + name, container, false );
            if( kl )
                return kl;
            ++impIt;
        }
    }
#endif

    return c;
}

void CppCodeCompletion::computeFileEntryList( )
{
    m_fileEntryList.clear();

    QStringList fileList = m_pSupport->project()->allFiles();
    for( QStringList::Iterator it=fileList.begin(); it!=fileList.end(); ++it )
    {
	if( !m_pSupport->isHeader(*it) )
	    continue;

        KTextEditor::CompletionEntry entry;
	entry.text = QFileInfo( *it ).fileName();
	m_fileEntryList.push_back( entry );
    }

    m_fileEntryList = unique( m_fileEntryList );
}

#include "cppcodecompletion.moc"
