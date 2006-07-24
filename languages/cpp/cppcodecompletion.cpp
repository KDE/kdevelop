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

///This can be used to toggle the complete tracing of the resolution-functions, which costs a lot of performance, but gives very nice and useful output 


#include "codecompletionentry.h"
#include "typedesc.h"
#include "computerecoverypoints.h"
#include "completiondebug.h"
#include "bithelpers.h"
#include "stringhelpers.h"
#include "simpletype.h"
#include "simpletypecachebinder.h"
#include "safetycounter.h"
#include "cppevaluation.h"
#include "simplecontext.h"
#include "simpletypefunction.h"


const bool disableVerboseForCompletionList = true;
const bool disableVerboseForContextMenu = true;
const bool contextMenuEntriesAtTop = false;

///This enables-disables the automatic processing of the expression under the mouse-cursor 
//#define DISABLETOOLTIPS

/**
-- TODO: The parser and code-models currently do not correctly collect all the data necessary to handle namespace-imports etc. precisely
-- TODO: Does not yet use most of the code-completion-options. Maybe some should be removed, and new ones added.
-- TODO: Does not yet correctly search for overloaded functions and select the right one
-- TODO: The documentation shown in the calltips looks very bad, a better solution must be found(maybe an additional tooltip)
*/

SafetyCounter safetyCounter;
CppCodeCompletion* cppCompletionInstance = 0;

//file global functions, must be before any "using namespace"
QString cleanForMenu( QString txt ) {
    //  return txt.replace( "&", "§" );
	return txt.replace( "&", "$" ).replace("	", "    " );
}

/** Multiple empty lines are reduced to one, too long lines wrapped over, and the beginnings of the lines are normalized
*/
QStringList maximumLength( const QStringList& in, int length ) {
	QStringList ret;
	int firstNonSpace = 50000;
	for( QStringList::const_iterator it = in.begin(); it!= in.end(); ++it )
		for( uint a = 0; a < (*it).length(); a++ )
			if( !(*it)[a].isSpace() ) {
				if( firstNonSpace > a)
					firstNonSpace = a;
				break;
			}
	if( firstNonSpace == 50000 ) return QStringList();

	bool hadEmptyLine = false;
	for( QStringList::const_iterator it = in.begin(); it!= in.end(); ++it ) {
		if(  (*it).length() <= firstNonSpace ) {
			if( !hadEmptyLine ) ret << " ";
			hadEmptyLine = true;
		} else {
			hadEmptyLine = false;
			QString str = (*it).mid( firstNonSpace );
			while( !str.isEmpty() ) {
				if( str.length() < length ) {
					ret << str;
					break;
				} else {
					ret << str.left( length ) + "\\";
					str = str.mid( length );
				}
			}
		}
	}
	return ret;
}

QStringList prepareTextForMenu( const QString& comment, int maxLines, int maxLength ) {
	QStringList in = QStringList::split( "\n", comment );
	QStringList out;
	for( QStringList::iterator it = in.begin(); it!= in.end(); ++it ) {
		out << cleanForMenu( *it );
		if( out.count() >= maxLines ) {
			out << "[...]";
			break;
		}
	}
	
	return maximumLength( out, maxLength );
}

QStringList formatComment( const QString& comment, int maxCols = 120 ) {
  QStringList ret;
  SafetyCounter s( 14 );  ///maximum of 14 lines
  
  QStringList lines = QStringList::split( "\n", comment );
  for( QStringList::iterator it = lines.begin(); it != lines.end(); ++it ) {
    QStringList words = QStringList::split( " ", *it );
    while( !words.isEmpty() && s ) {
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

using namespace CompletionDebug;
using namespace StringHelpers;
using namespace BitHelpers;
using namespace CppEvaluation;


struct PopupFillerHelpStruct {
  CppCodeCompletion* receiver;
  PopupFillerHelpStruct( CppCodeCompletion* rec ) {
    receiver = rec;
  }

  void insertItem(  QPopupMenu* parent, SimpleTypeImpl::MemberInfo d , QString prefix )
  {
    QString txt = i18n("Jump to %1 %2").arg( d.memberTypeToString() ).arg( cleanForMenu( d.name ) );
    int id = parent->insertItem( txt, receiver, SLOT( popupAction( int ) ) );
    
    receiver->m_popupActions.insert( id, d.decl );
  }
  
  void insertItem ( QPopupMenu* parent, TypeDesc d , QString prefix )
  {
    QString txt;
    
    if( d.resolved() && d.resolved()->isNamespace() ) return;
    
    if( d.resolved() && d.resolved()->hasNode() ) {
      QString n = d.name();
      if( d.resolved()->asFunction() )
        n = receiver->buildSignature( d.resolved() );
      
      txt = prefix + i18n("Jump to %1").arg( cleanForMenu( n ) );
    } else {
      txt = prefix + d.name() + i18n(" is unresolved");
    }
    
    int id = parent->insertItem( txt, receiver, SLOT( popupAction( int ) ) );
    
    if( d.resolved() ) receiver->m_popupActions.insert( id, d.resolved()->getDeclarationInfo() );
  }
};


ItemDom itemFromScope(const QStringList& scope, NamespaceDom startNamespace)
{
	if(scope.isEmpty())return ItemDom();
	
	NamespaceDom glob = startNamespace;
	if( !glob ) return ItemDom();
	
	ClassModel* curr =  glob ;
	
	QStringList::const_iterator mit = scope.begin();
	
	while(curr->isNamespace() && mit != scope.end() && ((NamespaceModel*)curr)->hasNamespace( *mit )) {
		curr = &(*( ((NamespaceModel*)curr)->namespaceByName( *mit ) ));
		++mit;
	}
	
	while((curr->isNamespace() || curr->isClass()) && mit != scope.end() && curr->hasClass( *mit )) {
		ClassList cl = curr->classByName( *mit );
		curr = &(**cl.begin() );
		++mit;
	}

	if( mit != --scope.end() ) return ItemDom();

	TypeAliasList l = curr->typeAliasByName( *mit );
	if( !l.isEmpty() ) return model_cast<ItemDom>( l.front() );

	VariableDom v = curr->variableByName( *mit );
    if( v ) return model_cast<ItemDom>( v );

	ClassList c = curr->classByName( *mit );
	if( !c.isEmpty() ) return model_cast<ItemDom>( c.front() );

	EnumDom en = curr->enumByName(  *mit );
	if( en ) return model_cast<ItemDom>( en );

	FunctionList f  = curr->functionByName( *mit );
	if( !f.isEmpty() ) return model_cast<ItemDom>( f.front() );

	FunctionDefinitionList fd  = curr->functionDefinitionByName( *mit );
	if( !fd.isEmpty() ) return model_cast<ItemDom>( fd.front() );
	
	return ItemDom();
}

struct PopupClassViewFillerHelpStruct {
  CppCodeCompletion* receiver;
  PopupClassViewFillerHelpStruct( CppCodeCompletion* rec ) {
    receiver = rec;
  }

  void insertItem(  QPopupMenu* parent, SimpleTypeImpl::MemberInfo d , QString prefix )
  {
	FileDom f = receiver->m_pSupport->codeModel()->fileByName( d.decl.file );
	if( !f ) return;

	ItemDom dom  = itemFromScope( QStringList::split( "::", d.name ), model_cast<NamespaceDom>( f ) );

	QString txt = i18n("Show %1 %2").arg( d.memberTypeToString() ).arg( cleanForMenu( d.name ) );
    int id = parent->insertItem( txt, receiver, SLOT( popupClassViewAction( int ) ) );
    
	receiver->m_popupClassViewActions.insert( id, dom );
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
        SimpleTypeCachedNamespace* ns = dynamic_cast<SimpleTypeCachedNamespace*>( d.resolved().data() );
        if( ns ) {
			QValueList<SimpleType> slaves = ns->getSlaves();
			for( QValueList<SimpleType>::iterator it = slaves.begin(); it != slaves.end(); ++it ) {
				SimpleTypeCodeModel* cm = dynamic_cast<SimpleTypeCodeModel*>( (*it).get().data() );
				if( cm ) {
					dom = cm->item();
					break;
				}
			}
        }
      }
      
      
      if( dom ) {
        QString n = d.name();
        if( d.resolved()->asFunction() ) {
          n = receiver->buildSignature( d.resolved() );
        }
        txt = prefix + i18n("Show %1").arg( cleanForMenu( n ) );
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
PopupFiller( HelpStruct str , QString dAdd, int maxCount = 100 ) : struk( str ), depthAdd( dAdd), s(maxCount) {
}

  void fill( QPopupMenu* parent, TypeDesc d, QString prefix = "" ) {
	Debug dbg( "#fl# ", 10 );
    if( !s || !dbg ) {
	    //dbgMajor() << "safety-counter triggered while filling \"" << d.fullNameChain() << "\"" << endl;
      return;
    }
    struk.insertItem( parent, d, prefix );
    
    TypeDesc::TemplateParams p = d.templateParams();
    for( TypeDesc::TemplateParams::iterator it = p.begin(); it != p.end(); ++it ){
      if( (*it)->resolved() ) {
        QPopupMenu * m = new QPopupMenu( parent );
        int gid = parent->insertItem( i18n( "Template-param \"%1\"" ).arg( cleanForMenu( (*it)->resolved()->fullTypeResolved() ) ), m );
        fill( m, **it );
      } else {
        fill( parent, **it, prefix + depthAdd );
      }
    }
    
	if( d.resolved() ) {
      if( d.resolved()->asFunction() ) {
        SimpleTypeImpl::LocateResult rt = d.resolved()->locateDecType( d.resolved()->asFunction()->getReturnType() );
        if( rt ) {
          QPopupMenu * m = new QPopupMenu( parent );
          int gid = parent->insertItem( i18n( "Return-type \"%1\"" ).arg( cleanForMenu( rt->fullNameChain() ) ), m );
          fill( m, (TypeDesc)rt );
        }
        
        QValueList<TypeDesc> args = d.resolved()->asFunction()->getArgumentTypes();
        QStringList argNames = d.resolved()->asFunction()->getArgumentNames();
        if( !args.isEmpty() ) {
          QPopupMenu * m = new QPopupMenu( parent );
          int gid = parent->insertItem( i18n( "Argument-types" ), m );
          QStringList::iterator it2 = argNames.begin();
          for( QValueList<TypeDesc>::iterator it = args.begin(); it != args.end(); ++it )
          {
            SimpleTypeImpl::LocateResult at = d.resolved()->locateDecType( *it );
            QString name ="";
            if( it2 != argNames.end() ) {
              name = *it2;
              ++it2;
            }
            QPopupMenu * mo = new QPopupMenu( m );
            int gid = m->insertItem( i18n( "Argument \"%1\"" ).arg( cleanForMenu( at->fullNameChain() + " " + name ) ), mo );
            fill( mo, at );
            
          }
        }
      }

		QValueList<SimpleTypeImpl::MemberInfo> trace = d.resolved()->trace();
	  if( !trace.isEmpty() ) {
		  QPopupMenu * m = new QPopupMenu( parent );
		  int gid = parent->insertItem( i18n( "Trace" ), m );
		  
		  for( QValueList<SimpleTypeImpl::MemberInfo>::iterator it = trace.begin(); it != trace.end(); ++it ) {
			  QPopupMenu * mo = new QPopupMenu( m );
			  int gid = m->insertItem( i18n( "%1 -> %2" ).arg( cleanForMenu( (*it).name ) ).arg( cleanForMenu( (*it).type.fullNameChain() ) ), mo );

			  struk.insertItem( mo, *it, prefix );
			  
			  if( !(*it).decl.comment.isEmpty() ) {
				  mo->insertSeparator();
				  QPopupMenu * m = new QPopupMenu( mo );
				  int gid = mo->insertItem( i18n( "Comment" ), m );
				  QStringList ls = prepareTextForMenu( (*it).decl.comment, 15, 100 );
				  for( QStringList::iterator it = ls.begin(); it != ls.end(); ++it ) {
					  m->insertItem( *it, 0, SLOT( popupClassViewAction( int ) ) );
				  }
			  }
		  }
	  }
		
      QValueList<SimpleTypeImpl::LocateResult> bases = d.resolved()->getBases();
      for( QValueList<SimpleTypeImpl::LocateResult>::iterator it = bases.begin(); it != bases.end(); ++it ) {
        QPopupMenu * m = new QPopupMenu( parent );
        int gid = parent->insertItem( i18n( "Base-class \"%1\"" ).arg( cleanForMenu( (*it)->fullNameChain() ) ), m );
        fill( m, *it );
      }
      
      if( d.resolved()->parent() && d.resolved()->parent()->desc() ) {
        QPopupMenu * m = new QPopupMenu( parent );
        int gid = parent->insertItem( i18n( "Nested in \"%1\"" ).arg( cleanForMenu( d.resolved()->parent()->fullTypeResolved() ) ), m );
        fill( m, d.resolved()->parent()->desc() );
      }

	  if( !d.resolved()->comment().isEmpty() ) {
		  parent->insertSeparator();
		  QPopupMenu * m = new QPopupMenu( parent );
		  int gid = parent->insertItem( i18n( "Comment on %1" ).arg( cleanForMenu( d.name() ) ), m );
		  QStringList ls = prepareTextForMenu( d.resolved()->comment(), 15, 100 );
		  for( QStringList::iterator it = ls.begin(); it != ls.end(); ++it ) {
			m->insertItem( *it, 0, SLOT( popupClassViewAction( int ) ) );
		  }
	  }
    }
  }
};

struct CompTypeProcessor : public TypeProcessor
{
  SimpleType m_scope;
  bool m_processArguments;

  CompTypeProcessor( SimpleType scope, bool processArguments)  : m_scope(scope), m_processArguments( processArguments ) {
  }
  
  virtual QString parentType() {
    return m_scope->fullType();
  }
  
  virtual QString processType( const QString& type )
  {
    if( !m_processArguments ) return type;
    SimpleTypeImpl::LocateResult t = m_scope->locateDecType( type );
    if( t )
      return t->fullNameChain();
    else
      return type;
  }
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
	cppCompletionInstance = this;
	m_cppCodeCommentsRx.setMinimal( true );
		
	m_pSupport = part;

	m_activeCursor = 0;
	m_activeEditor = 0;
	m_activeCompletion = 0;
	m_activeHintInterface = 0;
	m_activeView = 0;
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
	if( m_activeHintInterface && m_activeView ) {
		disconnect(m_activeView , SIGNAL( needTextHint(int, int, QString &) ), this, SLOT( slotTextHint(int, int, QString&) ) );
		
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

	m_activeView = part ? dynamic_cast<KTextEditor::View*>( part->widget() ) : 0;

	if( m_activeView )
		m_activeHintInterface = dynamic_cast<KTextEditor::TextHintInterface*>( m_activeView );
	
	char* q = 0;
	kdDebug() << q << endl;
	
	if( m_activeHintInterface ) 
	{
#ifndef DISABLETOOLTIPS
		m_activeHintInterface->enableTextHints( 500 );
		connect( m_activeView, SIGNAL( needTextHint(int, int, QString &) ), this, SLOT( slotTextHint(int, int, QString&) ) );
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
					//index;
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

///Before calling this, a SimpleTypeConfiguration-object should be created, so that the ressources will be freed when that object is destroyed
EvaluationResult CppCodeCompletion::evaluateExpressionAt( int line, int column , SimpleTypeConfiguration& conf, bool ifUnknownSetType ) {
	kdDebug( 9007 ) << "CppCodeCompletion::evaluateExpressionAt( " << line << ", " << column << " )" << endl;

    if( line < 0 || line >= (int)m_activeEditor->numLines()  ) return EvaluationResult();
    if( column < 0 || column >= m_activeEditor->lineLength( line ) ) return EvaluationResult(); 
  
	if ( !m_pSupport || !m_activeEditor )
		return EvaluationResult();
	
	{
		QString curLine = m_activeEditor->textLine( line );
		
	///move column to the last letter of the pointed word
		while( column+1 < (int)curLine.length() && isValidIdentifierSign( curLine[column] ) && isValidIdentifierSign( curLine[column+1] ) )
			column++;
		
	//if( column > 0 ) column--;
		
		if( column >= (int)curLine.length() || curLine[ column ].isSpace() ) return EvaluationResult();
			
		QString expr = curLine.left( column +1 );
		kdDebug( 9007 ) << "evaluating \"" << expr.stripWhiteSpace() << "\"" << endl;
		
		if( curLine[column] == '-' || curLine[column] == ';' ) --column;
		
        EvaluationResult type = evaluateExpressionType( line, column + 1, conf, ifUnknownSetType ? addFlag( DefaultEvaluationOptions, DefaultAsTypeExpression ) : DefaultEvaluationOptions ); 
    
		kdDebug( 9007 ) << "type: " << type->fullNameChain() << endl;
		
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

void CppCodeCompletion::contextEvaluationMenus ( QPopupMenu *popup, const Context *context, int line, int column )
{
	if( !m_pSupport->codeCompletionConfig()->showEvaluationContextMenu() ) return;
	
	kdDebug( 9007 ) << "CppCodeCompletion::contextEvaluationMenu()" << endl;
	m_popupActions.clear();
	m_popupClassViewActions.clear();
	
	if ( !m_pSupport || !m_activeEditor )
		return ;

	struct SetDbgState {
		DBGStreamType& st;
		bool oldState;
		SetDbgState( DBGStreamType& targ, bool state ) : st( targ ) {
			oldState = targ.state();
			targ.setState( state );
		}
		~SetDbgState() {
			st.setState( oldState );
		}
	};

	int cpos = 0;
	
	SetDbgState stt( dbgState, disableVerboseForContextMenu );

    SimpleTypeConfiguration conf( m_activeFileName );
	
	EvaluationResult type = evaluateExpressionAt( line, column, conf );

	if( !type->resolved() && !type.sourceVariable ) return;
	
	QString name = type->fullNameChain();
	if( type.sourceVariable )
		name += " " + type.sourceVariable.name;
	if( type.resultType->resolved() && type.resultType->resolved()->asFunction() )
		name = buildSignature(type.resultType->resolved() );
	
	///Fill the jump-menu
	{
		PopupFillerHelpStruct h(this);
		PopupFiller<PopupFillerHelpStruct> filler( h, "" );
		
		QPopupMenu * m = new QPopupMenu( popup );
		int gid;
		if( contextMenuEntriesAtTop )
			gid = popup->insertItem( i18n( "Navigate by \"%1\"" ).arg( cleanForMenu( name ) ), m, 5, cpos++ );
		else
			gid = popup->insertItem( i18n( "Navigate by \"%1\"" ).arg( cleanForMenu( name ) ), m );
		
		popup->setWhatsThis( gid, i18n( "<b>Navigation</b><p>Provides a menu to navigate to positions of items that are involved in this expression" ) );
		
		if( type.sourceVariable && type.sourceVariable.name != "this" ) {
			int id = m->insertItem( i18n("jump to variable-declaration \"%1\"").arg( type.sourceVariable.name ) , this, SLOT( popupAction( int ) ) );
			
			m_popupActions.insert( id, type.sourceVariable );
		}
		
		filler.fill( m, (TypeDesc)type );
	}
	if( type->resolved() ) {
		///Now fill the class-view-browsing-stuff
		{
			QPopupMenu * m = new QPopupMenu( popup );
			int gid;
			if( contextMenuEntriesAtTop )
				gid = popup->insertItem( i18n( "Navigate Class-View by \"%1\"" ).arg( cleanForMenu( name ) ), m, 6, cpos++ );
			else
				gid = popup->insertItem( i18n( "Navigate Class-View by \"%1\"" ).arg( cleanForMenu( name ) ), m );

			popup->setWhatsThis( gid, i18n( "<b>Navigation</b><p>Provides a menu to show involved items in the class-view " ) );
			
			PopupClassViewFillerHelpStruct h(this);
			PopupFiller<PopupClassViewFillerHelpStruct> filler( h, "" );
			
			filler.fill( m, (TypeDesc)type );
		}
	}

	if( contextMenuEntriesAtTop )
		popup->insertSeparator( cpos );
}

void CppCodeCompletion::slotTextHint(int line, int column, QString &text)
{
	if( ! m_pSupport->codeCompletionConfig()->statusBarTypeEvaluation() ) return;
	
	kdDebug( 9007 ) << "CppCodeCompletion::slotTextHint()" << endl;
	
	clearStatusText();
	
	if( m_lastHintTime.msecsTo( QTime::currentTime() ) < 300 ) {
		kdDebug( 9007 ) << "slotNeedTextHint called too often";
		return;
	}
	
	m_lastHintTime = QTime::currentTime();
	
	clearStatusText();
	text = "";
	if ( !m_pSupport || !m_activeEditor )
		return ;
	
    SimpleTypeConfiguration conf( m_activeFileName );
	
	EvaluationResult type = evaluateExpressionAt( line, column, conf );
	
	if( type.expr.expr().stripWhiteSpace().isEmpty() ) return; ///Expression could not be found
	
	if( type.sourceVariable ) {
		text += type.sourceVariable.toText() + "\n";
	}
	
	if( type->resolved() ) {
		/*SimpleTypeFunctionInterface* f = type->resolved()->asFunction();
		if( f ) {
			text += "function: \"" + buildSignature( type->resolved() ) + "\"";
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
		
		if( !type.resultType->comment().isEmpty() ) text +=  "\n\n" + type.resultType->comment() + "";*/
		
	} else {
	}
	
	kdDebug( 9007 ) << "showing: \n" << text << endl;
	const int timeout = 2000;
		
	if( type->resolved() ) {
		addStatusText( i18n( "Type of \"%1\" is \"%2\"" ).arg( type.expr.expr() ).arg( type->fullNameChain() ), timeout );
		if( type.sourceVariable && !type.sourceVariable.comment.isEmpty() ) {
			addStatusText( i18n( "Comment on variable %1: \"%1\"").arg( type.sourceVariable.name ).arg( type.sourceVariable.comment ) , 10000 );
		}
		if( !type->resolved()->comment().isEmpty() ) {
			addStatusText( i18n( "Comment on %1: \"%1\"").arg( type->name() ).arg( type->resolved()->comment() ) , 10000 );
		} 
		if( type->resolved()->comment().isEmpty() ) {
			addStatusText( i18n( "%1 has no comment").arg( type->name() ) , timeout );
		}
	} else {
		if( type ) {
			addStatusText( i18n( "Type of \"%1\" is unresolved, name: \"%2\"" ).arg( type.expr.expr() ).arg( type->fullNameChain() ), 2*timeout );
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
ExpressionInfo CppCodeCompletion::findExpressionAt( int line, int column, int startLine, int startCol, bool inFunction )  {
	ExpressionInfo ret;
	
	QString contents = clearComments( getText( startLine, startCol, line, column ) );
	
	
	int start_expr = expressionAt(  contents, contents.length() - 1 );
	
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
				recoveryPoint->registerImports( global, m_pSupport->codeCompletionConfig()->namespaceAliases() );
			} else {
				kdDebug( 9007 ) << "no recovery-point, cannot use imports" << endl;
			}
			
			///Insert the "this"-type(container) and correctly resolve it using imported namespaces
			if ( ctx->container() )
			{
				TypeDesc td = ctx->container()->desc();
				td.makePrivate();
				td.resetResolved( );
				TypePointer tt = ctx->container()->locateDecType( td, SimpleTypeImpl::LocateBase )->resolved();
				if( tt ) {
					ctx->setContainer( SimpleType( tt ) );
				} else {
					kdDebug( 9007 ) << "could not resolve local this-type \"" << td.fullNameChain() << "\"" << endl;
				}

				SimpleType this_type = ctx->container();
				
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

///Warning: yet check how to preserve the SimpleTypeConfiguration.. 
EvaluationResult CppCodeCompletion::evaluateExpressionType( int line, int column, SimpleTypeConfiguration& conf, EvaluateExpressionOptions opt ) {
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
							ret.resultType = ctx->container()->locateDecType( exp.expr(), SimpleTypeImpl::TraceAliases );
							ret.expr = exp;
						}
						
					}
				}
				if( exp.canBeNormalExpression() && !ret.resultType->resolved() ) {
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
					
	if( (opt & SearchInClasses ) && !ret->resolved() && (!currentFunction || !functionContains( currentFunction, line, column ) ) )
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
			recoveryPoint->registerImports( global, m_pSupport->codeCompletionConfig()->namespaceAliases() );
		
		ExpressionInfo exp = findExpressionAt( line, column , startLine, startCol );
		exp.t = ExpressionInfo::TypeExpression;	///Outside of functions, we can only handle type-expressions
		ret.expr = exp;
		
		if( exp && (exp.t & ExpressionInfo::TypeExpression) ) {
			kdDebug( 9007 ) << "locating \"" << exp.expr() << "\" in " << container->fullTypeResolvedWithScope() << endl;
			ret.resultType = container->locateDecType( exp.expr(), SimpleTypeImpl::TraceAliases );
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


QString CppCodeCompletion::buildSignature( TypePointer currType ) 
{
	SimpleTypeFunctionInterface* f = currType->asFunction();
	if( !f ) return "";
	
	QString ret;
	SimpleTypeImpl::LocateResult rtt = currType->locateDecType( f->getReturnType() );
	if( rtt->resolved() || rtt.resolutionCount() > 1 )
		ret = rtt->fullNameChain();
	else
		ret = f->getReturnType().fullNameChain();

	
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

	///Check whether the cursor is within a comment
	int surroundingStartLine = line - 10, surroundingEndLine = line + 10;
	if( surroundingStartLine < 0 ) surroundingStartLine = 0;
	if( surroundingEndLine > m_activeEditor->numLines()-1 ) surroundingEndLine = m_activeEditor->numLines() -1;
	int surroundingEndCol = m_activeEditor->lineLength( surroundingEndLine );

	QString pre = getText( surroundingStartLine, 0, line, column );
	int pos = pre.length();
	pre += getText( line, column, surroundingEndLine, surroundingEndCol );
	QString cleared = clearComments( pre );
	if( cleared[pos] != pre[pos] ) {
		kdDebug( 9007 ) << "stopping completion because we're in a coment" << endl;
		return;
	}

	

	
	int nLine = line, nCol = column;

	QString strCurLine = clearComments( m_activeEditor->textLine( nLine ) );

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
		while ( nCol > 0 && strCurLine[ nCol-1 ].isSpace() )
			--nCol;

		///check whether it is a value-definition using constructor
		int column = nCol;
		bool s1=false, s2=false;
		while( column > 0 && isValidIdentifierSign( strCurLine[column-1] ) ) {
			column--;
			s1 = true;
		}

		///skip white space
		while ( column > 0 && strCurLine[ column-1 ].isSpace() ) {
			--column;
			s2 = true;
		}

		if( s1 && s2 && isValidIdentifierSign( strCurLine[column-1] ) ) {
			///it is a constructor
			nCol = column;
		}


		
		showArguments = TRUE;
	}
	
	EvaluationResult type;
	SimpleType this_type;
	QString expr, word;

	DeclarationAST::Node recoveredDecl;
	TypeSpecifierAST::Node recoveredTypeSpec;

	SimpleContext* ctx = 0;
    SimpleTypeConfiguration conf( m_activeFileName );
	
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

			QString textToReparse = clearComments( getText( startLine, startColumn, line, showArguments ? nCol : column ) );
			
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
							
							n->addAliases(  m_pSupport->codeCompletionConfig()->namespaceAliases() );
						}
						
					
					conf.setGlobalNamespace( &(*global) );
					
					///Now locate the local type using the imported namespaces
					if ( !scope.isEmpty() )
					{
						TypeDesc td = ctx->container()->desc();
						td.makePrivate();
						td.resetResolved( );
						TypePointer tt = ctx->container()->locateDecType( td, SimpleTypeImpl::LocateBase )->resolved();
						if( tt ) {
							ctx->setContainer( SimpleType( tt ) );
						} else {
							kdDebug( 9007 ) << "could not resolve local this-type \"" << td.fullNameChain() << "\"" << endl;
						}
						
						SimpleType this_type = ctx->container();
						
						this_type->setPointerDepth( 1 );
						
						SimpleVariable var;
						var.type = this_type->desc();
						var.name = "this";
						var.comment = this_type->comment();
						ctx->add( var );
						ctx->setContainer( this_type );
					} else {
						this_type = global;
					}
					
					
					ExpressionInfo exp( expr );
					exp.t = (ExpressionInfo::Type) (ExpressionInfo::NormalExpression | ExpressionInfo::TypeExpression);
					type = evaluateExpression( exp, ctx );
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

				ExpressionInfo exp( expr );
				exp.t = (ExpressionInfo::Type) (ExpressionInfo::NormalExpression | ExpressionInfo::TypeExpression);
				type = evaluateExpression( exp, ctx );
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

	kdDebug( 9007 ) << "===========================> type is: " << type->fullNameChain() << endl;
	kdDebug( 9007 ) << "===========================> word is: " << word << endl;

	if ( !showArguments )
	{
		QValueList<CodeCompletionEntry> entryList;
		
		if( !type && invokedOnDemand && this_type && ( expr.isEmpty() || expr.endsWith(";")) ) {
			
			QValueList<SimpleType> tstack;
			SimpleType t = this_type;
			bool ready = false;
			SafetyCounter cnt( 20 );
			int depth = 0;
			while( !ready & cnt ) {
				if( t->scope().isEmpty() ) {
					ready = true;
				} else {
					//	tstack.push_front( t );
					computeCompletionEntryList( t, entryList, t->scope(), isInstance, depth );
					t = t->parent();
					depth++;
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
			
		} else if ( type->resolved() && expr.isEmpty() )
		{
			computeCompletionEntryList( entryList, ctx, isInstance );
			
// 			if ( m_pSupport->codeCompletionConfig() ->includeGlobalFunctions() )
// 				computeCompletionEntryList( type, entryList, QStringList(), false );

			computeCompletionEntryList( type, entryList, QStringList(), false );
			
			if ( this_type.scope().size() )
				computeCompletionEntryList( this_type, entryList, this_type.scope(), isInstance );
			computeCompletionEntryList( type, entryList,  type->resolved()->scope() , isInstance );
		}
		else if ( type->resolved() )
		{
			if( type->resolved() )
				computeCompletionEntryList( type, entryList, type->resolved()->scope() , isInstance );
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
		
		QString methodName = type->name();
		
		///Search for variables with ()-operator in the context
		if( ctx ) {
			SimpleVariable var = ctx->findVariable( methodName );
			if( !var.name.isEmpty() ) {
				signatureList += computeSignatureList( ctx->container()->locateDecType( var.type ) );
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


QValueList<QStringList> CppCodeCompletion::computeSignatureList( EvaluationResult result ) {
	SimpleType type = result;
	
	if( result.expr.t == ExpressionInfo::TypeExpression )
		type = type->typeOf( result->name(), SimpleTypeImpl::MemberInfo::Function ); ///Compute the signature of the constructor
	
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
		QString sig = buildSignature( currType.get() );
		QString comment = currType->comment();
		QStringList commentList;
		if( m_pSupport->codeCompletionConfig()->showCommentWithArgumentHint() ) {
		    	
			if( !comment.isEmpty() ) {
				if( sig.length() + comment.length() < 130 ) {
				sig += ":  \"" + currType->comment() + "\"";
				} else {
					commentList = formatComment( comment );
				}
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

	computeRecoveryPointsLocked();
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

void CppCodeCompletion::computeRecoveryPointsLocked() {
	m_pSupport->backgroundParser() ->lock ();
	computeRecoveryPoints();
	m_pSupport->backgroundParser() ->unlock();
}

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

QString codeModelAccessToString( CodeModelItem::Access access ) {
	switch( access ) {
	case CodeModelItem::Public:
		return "public";
	case CodeModelItem::Protected:
		return "protected";
	case CodeModelItem::Private:
		return "private";
	default:
		return "unknown";
	}
}


QString commentFromItem( const SimpleType& parent, const ItemDom& item )
{
	QString ret;
	int line, col;
	item->getStartPosition( &line, &col );


	if( !parent->scope().isEmpty() ) {
		ret += "Container: " + parent->fullTypeResolvedWithScope();
	}

	if( item->isEnum() ) {
		ret += "\nKind: Enum";
		ret += "\nValues:";
		const EnumModel* en = dynamic_cast<const EnumModel*>( item.data() );
		if( en ) {
			EnumeratorList values =en->enumeratorList();
			for( EnumeratorList::iterator it = values.begin(); it != values.end(); ++it )
			{
				ret += "\n  " + (*it)->name();
				if( !(*it)->value().isEmpty() ) {
					ret + " = " + (*it)->value();
				}
			}

			ret += "\n\nAccess: " + codeModelAccessToString( (CodeModelItem::Access)en->access() );
		} else {
		}

	}
	
	if( item->isFunction() || item->isFunctionDefinition() ) {
		const FunctionModel* f = dynamic_cast<const FunctionModel*>( item.data() );
		ret += "\nKind: Function";
		if( f ) {
			QString state;
			if( f->isStatic() ) state += "static ";
			if( f->isVirtual() ) state += "virtual ";
			if( f->isAbstract() ) state += "abstract ";
			//if( f->isTemplateable() ) state += "template ";
			if( f->isConstant() ) state += "const ";
			if( f->isSlot() ) state += "slot ";
			if( f->isSignal() ) state += "signal ";

			if( !state.isEmpty() )
				ret += "\nModifiers: " + state;
			
			ret += "\nAccess: " + codeModelAccessToString( (CodeModelItem::Access)f->access() );
		}
	}

	if( item->isEnumerator() ) {
		const EnumeratorModel* f = dynamic_cast<const EnumeratorModel*>( item.data() );
		ret += "\nKind: Enumerator";
		if( f ) {
			if( !f->value().isEmpty() )
				ret += "\nValue: " + f->value();
			
			//ret += "\nAccess: " + codeModelAccessToString( f->() );
		}
	} else {
		if( item->isVariable() ) {
			const VariableModel* f = dynamic_cast<const VariableModel*>( item.data() );
			ret += "\nKind: Variable";
			if( f ) {
				if( f->isStatic() )
					ret += "Modifiers: static";
				ret += "\nAccess: " + codeModelAccessToString( (CodeModelItem::Access)f->access() );
			}
		}
	}

	if( item->isTypeAlias() ) {
		const TypeAliasModel* t = dynamic_cast<const TypeAliasModel*>( item.data() );
		ret += "\nKind: Typedef";
		if( t ) {
			ret += "\nType: " + t->type();
		}
	}
		
	ret += QString( "\nFile: %1\nLine: %2 Column: %3").arg( item->fileName() ).arg( line ).arg( col );
	if( !item->comment().isEmpty() )
		ret += "\n\n" + item->comment();
	return ret;
};

QString commentFromTag( const SimpleType& parent, Tag& tag ) {
	int line, col;
	tag.getStartPosition( &line, &col );
	QString ret;// = tag.comment();

	if( !parent->scope().isEmpty() ) {
		ret += "Container: " + parent->fullTypeResolvedWithScope();
	}
	/*
	if( tag.kind() == Tag::Kind_Enum ) {
	ret += "\nKind: Enum";
	ret += "\nValues:";
		EnumModel* en = dynamic_cast<EnumModel*>( item.data() );
		if( en ) {
			EnumeratorList values =en->enumeratorList();
			for( EnumeratorList::iterator it = values.begin(); it != values.end(); ++it )
			{
				ret += "\n  " + (*it)->name();
				if( !(*it)->value().isEmpty() ) {
					ret + " = " + (*it)->value();
				}
			}
			
		ret += "\n\nAccess: " + codeModelAccessToString( (CodeModelItem::Access)en->access() );
		} else {
		}
	}*/
	
	if( tag.kind() == Tag::Kind_Function || tag.kind() == Tag::Kind_FunctionDeclaration ) {
		CppFunction<Tag> function( tag );
		
		ret += "\nKind: Function";

		QString state;
		if( function.isStatic() ) state += "static ";
		if( function.isVirtual() ) state += "virtual ";
		//if( function.isVolatile() ) state += "volatile ";
		if( function.isConst() ) state += "const ";
		if( function.isSlot() ) state += "slot ";
		if( function.isSignal() ) state += "signal ";
		if( !state.isEmpty() )
			ret += "\nModifiers: " + state;
	
		ret += "\nAccess: " + TagUtils::accessToString( function.access() );
	}
	
	/*if( item->isEnumerator() ) {
		EnumeratorModel* f = dynamic_cast<EnumeratorModel*>( item.data() );
	ret += "\nKind: Enumerator";
		if( f ) {
			if( !f->value().isEmpty() )
				ret += "\nValue: " + f->value();
			
			//ret += "\nAccess: " + codeModelAccessToString( f->() );
		}
	} else {
		if( item->isVariable() ) {
			VariableModel* f = dynamic_cast<VariableModel*>( item.data() );
		ret += "\nKind: Variable";
			if( f ) {
			ret += "\nAccess: " + codeModelAccessToString( (CodeModelItem::Access)f->access() );
			}
		}
	}*/
	
	if( tag.kind() == Tag::Kind_Variable ) {
		CppVariable<Tag> var( tag );
		
		ret += "\nKind: Variable";
		if( var.isStatic() ) ret += "\nModifiers: static";
		ret += "\nAccess: " + TagUtils::accessToString( var.access() );
	}

	if( tag.kind() == Tag::Kind_Typedef ) {
		ret += "\nKind: Typedef";
	ret += "\nType: " + tagType( tag );
	}
	
	ret += QString( "\nFile: %1\nLine: %2 Column: %3").arg( tag.fileName() ).arg( line ).arg( col );
	if( !tag.comment().isEmpty() ) {
		ret += "\n\n" + tag.comment();
	}
	return ret;
};


void CppCodeCompletion::computeCompletionEntryList( SimpleType typeR, QValueList< CodeCompletionEntry > & entryList, const QStringList & type, bool isInstance, int depth  )
{
	dbgState.setState( disableVerboseForCompletionList );
	
	Debug d("#cel#");
	if( !safetyCounter || !d ) return;
	CppCodeCompletionConfig * cfg = m_pSupport->codeCompletionConfig();
	SimpleTypeImpl* m = &(*typeR) ;
	
	if ( dynamic_cast<SimpleTypeCodeModel*>(m) )
	{
		ItemDom item = ( dynamic_cast<SimpleTypeCodeModel*>(m) )->item();
		if( item)
			if( ClassModel* mod = dynamic_cast<ClassModel*> (&(*item)) )
				computeCompletionEntryList( typeR, entryList, ClassDom( mod ) , isInstance, depth );
	} /*else*/ {
		QValueList<Catalog::QueryArgument> args;
		QValueList<Tag> tags;

		args.clear();
		args << Catalog::QueryArgument( "kind", Tag::Kind_FunctionDeclaration )
		<< Catalog::QueryArgument( "scope", type );
		tags = m_repository->query( args );
		computeCompletionEntryList( typeR, entryList, tags, isInstance, depth );

		args.clear();
		args << Catalog::QueryArgument( "kind", Tag::Kind_Variable )
		<< Catalog::QueryArgument( "scope", type );
		tags = m_repository->query( args );
		computeCompletionEntryList( typeR, entryList, tags, isInstance, depth );

// 		if ( !isInstance && cfg->includeEnums() )
		{
			args.clear();
			args << Catalog::QueryArgument( "kind", Tag::Kind_Enumerator )
			<< Catalog::QueryArgument( "scope", type );
			tags = m_repository->query( args );
			computeCompletionEntryList( typeR, entryList, tags, isInstance, depth );
		}

// 		if ( !isInstance && cfg->includeTypedefs() )
		{
			args.clear();
			args << Catalog::QueryArgument( "kind", Tag::Kind_Typedef )
			<< Catalog::QueryArgument( "scope", type );
			tags = m_repository->query( args );
			computeCompletionEntryList( typeR, entryList, tags, isInstance, depth );
		}

		args.clear();
		args << Catalog::QueryArgument( "kind", Tag::Kind_Base_class );
		QString fullname = type.join( "::" );
		/*    	if( fullname.length() >=2 )
		            args << Catalog::QueryArgument( "prefix", fullname.left(2) );*/
		args << Catalog::QueryArgument( "name", fullname );

		
		QValueList<SimpleTypeImpl::LocateResult> parents = typeR->getBases();
		for ( QValueList<SimpleTypeImpl::LocateResult>::Iterator it = parents.begin(); it != parents.end(); ++it )
		{
			if( !(*it)->resolved() ) continue;
			SimpleType tp = SimpleType( (*it)->resolved() );
			if( tp ) computeCompletionEntryList( tp, entryList, tp.scope(), isInstance, depth+1 );
		}
	}
	dbgState.setState( true );
}

void CppCodeCompletion::computeCompletionEntryList( SimpleType type, QValueList< CodeCompletionEntry > & entryList, QValueList< Tag > & tags, bool /*isInstance*/, int depth  )
{
	Debug d("#cel#");
	if( !safetyCounter || !d ) return;
	
	CompTypeProcessor proc( type, m_pSupport->codeCompletionConfig()->processFunctionArguments() && type->usingTemplates() );
	bool resolve =  m_pSupport->codeCompletionConfig()->processPrimaryTypes() && type->usingTemplates();
	
	QValueList<Tag>::Iterator it = tags.begin();
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

		int sortPosition = 0;

		switch( tag.kind() )
		{
		case Tag::Kind_Enum:
			sortPosition = 3;
			break;
		case Tag::Kind_Enumerator:
			sortPosition = 4;
			break;
		case Tag::Kind_Struct:
		case Tag::Kind_Union:
		case Tag::Kind_Class:
			sortPosition = 5;
			break;
		case Tag::Kind_VariableDeclaration:
		case Tag::Kind_Variable:
			sortPosition = 2;
			break;
		case Tag::Kind_FunctionDeclaration:
		case Tag::Kind_Function:
			sortPosition = 1;
			break;
		case Tag::Kind_Typedef:
			sortPosition = 6;
			break;
		}
		
		e.userdata = QString("%1%2%2").arg( num ).arg( depth ).arg( sortPosition );
		
		if( !type->isNamespace() ) {
			if( num == 1 ) 
				e.postfix += ";   (protected)";// in " + proc.parentType() + ")";
			if( num == 2 ) 
				e.postfix += ";   (private)";// in " + proc.parentType() + ")";
		}
		
		
		QString prefix = tagType( tag );
			
		if((tag.kind() == Tag::Kind_FunctionDeclaration || tag.kind() == Tag::Kind_Function || tag.kind() == Tag::Kind_Variable || tag.kind() == Tag::Kind_Typedef))
		{
			if( !prefix.isEmpty() && resolve ) {
				SimpleTypeImpl::LocateResult et =  type->locateDecType( prefix );
				
				if( et )
					prefix = et->fullNameChain();
			}
		}

		e.comment = commentFromTag( type, tag );
		
		if( e.prefix.isEmpty() )
			e.prefix = prefix;
		else
			if( !prefix.isEmpty() )
				e.prefix+= " " + prefix;
		
		if( str != "private" )
			entryList << e;
	}
}

void CppCodeCompletion::computeCompletionEntryList( SimpleType type, QValueList< CodeCompletionEntry > & entryList, ClassDom klass, bool isInstance, int depth  )
{
	Debug d("#cel#");
	if( !safetyCounter || !d ) return;
	
	computeCompletionEntryList( type, entryList, klass->functionList(), isInstance, depth );
	if ( m_completionMode == NormalCompletion )
		computeCompletionEntryList( type, entryList, klass->variableList(), isInstance, depth );

	QValueList<SimpleTypeImpl::LocateResult> parents = type->getBases();
	for ( QValueList<SimpleTypeImpl::LocateResult>::Iterator it = parents.begin(); it != parents.end(); ++it )
	{
		if( !(*it)->resolved() ) continue;
		
		SimpleTypeImpl* i = (*it)->resolved();;
		SimpleTypeCodeModel* m = dynamic_cast<SimpleTypeCodeModel*> ( i );
		if( m ) {
			ItemDom item = m->item();
			ClassModel* kl = dynamic_cast<ClassModel*> ( &( *item ) );
			if( kl ) {
				computeCompletionEntryList( SimpleType( (*it)->resolved() ), entryList, ClassDom ( kl ), isInstance, depth+1 );
			}
		}
	}
}

void CppCodeCompletion::computeCompletionEntryList( SimpleType type, QValueList< CodeCompletionEntry > & entryList, NamespaceDom scope, bool isInstance, int depth   )
{
	Debug d("#cel#");
	if( !safetyCounter || !d ) return;
	
	CppCodeCompletionConfig * cfg = m_pSupport->codeCompletionConfig();

// 	if ( cfg->includeGlobalFunctions() )
	{
		computeCompletionEntryList( type, entryList, scope->functionList(), isInstance, depth );

		if ( m_completionMode == NormalCompletion )
			computeCompletionEntryList( type, entryList, scope->variableList(), isInstance, depth );
	}

// 	if ( !isInstance && cfg->includeTypes() )
	if ( !isInstance )
	{
		computeCompletionEntryList( type, entryList, scope->classList(), isInstance, depth );
		computeCompletionEntryList( type, entryList, scope->namespaceList(), isInstance, depth );
	}
}

void CppCodeCompletion::computeCompletionEntryList( SimpleType type, QValueList< CodeCompletionEntry > & entryList, const ClassList & lst, bool isInstance, int depth )
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
		entry.userdata = QString("000");
		entryList << entry;
		

// 		if ( cfg->includeTypes() )
		{
			computeCompletionEntryList( type, entryList, klass->classList(), isInstance, depth );
		}
	}
}

void CppCodeCompletion::computeCompletionEntryList( SimpleType type, QValueList< CodeCompletionEntry > & entryList, const NamespaceList & lst, bool /*isInstance*/, int depth )
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

void CppCodeCompletion::computeCompletionEntryList( SimpleType type, QValueList< CodeCompletionEntry > & entryList, const FunctionList & methods, bool isInstance, int depth )
{
	Debug d("#cel#");
	if( !safetyCounter || !d ) return;
	
	bool resolve = type->usingTemplates() && m_pSupport->codeCompletionConfig()->processPrimaryTypes();
	
	CompTypeProcessor proc( type, m_pSupport->codeCompletionConfig()->processFunctionArguments() && type->usingTemplates() );
	
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
		
		entry.comment = commentFromItem( type, model_cast<ItemDom>(meth) );
		
		if( ! resolve ) {
			entry.prefix = meth->resultType();
		}else{
			QString tt = meth->resultType();
			SimpleTypeImpl::LocateResult t = type->locateDecType( tt );
			if( t ) {
				entry.prefix = t->fullNameChain();
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
		
		entry.text += formattedOpeningParenthesis(args.size() == 0);
		
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

		if ( args.size() == 0) {
			entry.text += formattedClosingParenthesis(true);
		} else {
			text += formattedClosingParenthesis(false);
		}
		
		entry.userdata += QString("%1%2%3").arg( meth->access() ).arg( depth ).arg( 1 );
		
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
				entry.postfix += "; (protected)"; // in " + type->fullType() + ")";
			if( meth->access() == CodeModelItem::Private ) 
				entry.postfix += "; (private)"; // in " + type->fullType() + ")";
		}
		
		entryList << entry;
	}
}

void CppCodeCompletion::computeCompletionEntryList( SimpleType type, QValueList< CodeCompletionEntry > & entryList, const VariableList & attributes, bool isInstance, int depth )
{
	Debug d("#cel#");
	if( !safetyCounter || !d ) return;
	
	if ( m_completionMode != NormalCompletion )
		return ;
	bool resolve = type->usingTemplates() && m_pSupport->codeCompletionConfig()->processPrimaryTypes();
	
	VariableList::ConstIterator it = attributes.begin();
	while ( it != attributes.end() )
	{
		VariableDom attr = *it;
		++it;

		if ( isInstance && attr->isStatic() )
			continue;

		CodeCompletionEntry entry;
		entry.text = attr->name();
		entry.comment = commentFromItem( type, model_cast<ItemDom>(attr) );
		entry.userdata += QString("%1%2%3").arg( attr->access() ).arg( depth ).arg( 2 );
		
		
		if( ! resolve ) {
			entry.prefix = attr->type();
		}else{
			QString tt = attr->type();
			SimpleTypeImpl::LocateResult t = type->locateDecType( tt );
			//SimpleType t = type->typeOf( attr->name() );
			if( t ) 
				entry.prefix = t->fullNameChain();
			else
				entry.prefix = attr->type();
		}		
		if( attr->access() == CodeModelItem::Protected ) 
			entry.postfix += "; (protected)";// in " + type->fullType() + ")";
		if( attr->access() == CodeModelItem::Private ) 
			entry.postfix += "; (private)";// in " + type->fullType() + ")";
		
		entryList << entry;
	}
}

void CppCodeCompletion::computeCompletionEntryList( QValueList< CodeCompletionEntry > & entryList, SimpleContext * ctx, bool /*isInstance*/, int depth )
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
			entry.userdata = "000";
			entryList << entry;
			
		}
		ctx = ctx->prev();
	}
	
}

EvaluationResult CppCodeCompletion::evaluateExpression( ExpressionInfo expr, SimpleContext* ctx )
{
	safetyCounter.init();

	d->classNameList = typeNameList( m_pSupport->codeModel() );
	
	CppEvaluation::ExpressionEvaluation obj( this, expr, AllOperators, ctx );
	
	EvaluationResult res;
	res = obj.evaluate();
	
	m_pSupport->mainWindow() ->statusBar() ->message( i18n( "Type of %1 is %2" ).arg( expr.expr() ).arg( res->fullNameChain() ), 1000 );
	
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
