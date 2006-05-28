/*
   Copyright (C) 2002 by Roberto Raggi <roberto@kdevelop.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   version 2, License as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "problemreporter.h"
#include "cppsupportpart.h"
#include "configproblemreporter.h"
#include "backgroundparser.h"

#include <kdevpartcontroller.h>
#include <kdevmainwindow.h>
#include <kdevproject.h>

#include <kdeversion.h>
#include <kparts/part.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/markinterface.h>

#include <ktexteditor/markinterfaceextension.h>
#include <ktexteditor/view.h>

#include <kdebug.h>
#include <klocale.h>
#include <kstatusbar.h>
#include <kurl.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kdialogbase.h>

#include <kconfig.h>

#include <qtimer.h>
#include <qregexp.h>
#include <qvbox.h>
#include <qfileinfo.h>
#include <qwhatsthis.h>
#include <qtabbar.h>
#include <qwidgetstack.h>
#include <qlayout.h>
#include <qlineedit.h>

class ProblemItem: public KListViewItem
{
public:
	ProblemItem( QListView* parent, const QString& problem,
	             const QString& file, const QString& line, const QString& column )
			: KListViewItem( parent, problem, file, line, column )
	{}

	ProblemItem( QListViewItem* parent, const QString& problem,
	             const QString& file, const QString& line, const QString& column )
			: KListViewItem( parent, problem, file, line, column )
	{}

	int compare( QListViewItem* item, int column, bool ascending ) const
	{
		if ( column == 2 || column == 3 )
		{
			int a = text( column ).toInt();
			int b = item->text( column ).toInt();
			if ( a == b )
				return 0;
			return ( a > b ? 1 : -1 );
		}
		return KListViewItem::compare( item, column, ascending );
	}

};

ProblemReporter::ProblemReporter( CppSupportPart* part, QWidget* parent, const char* name )
: QWidget( parent, name ? name : "problemreporter" ),
m_cppSupport( part ),
m_document( 0 ),
m_markIface( 0 )
{
	QWhatsThis::add(this, i18n("<b>Problem reporter</b><p>This window shows various \"problems\" in your project. "
	                           "It displays TODO entries, FIXME's and errors reported by a language parser. "
	                           "To add a TODO or FIXME entry, just type<br>"
	                           "<tt>//@todo my todo</tt><br>"
	                           "<tt>//TODO: my todo</tt><br>"
	                           "<tt>//FIXME fix this</tt>"));

	m_canParseFile = true;

	m_gridLayout = new QGridLayout(this,2,3);

	m_errorList = new KListView(this);
	m_fixmeList = new KListView(this);
	m_todoList = new KListView(this);
	m_filteredList = new KListView(this);
	m_currentList = new KListView(this);

	m_filteredList->addColumn( i18n("Level") );
	m_currentList->addColumn( i18n("Level") );

    //addColumn( i18n("Level") );
	InitListView(m_errorList);
	InitListView(m_fixmeList);
	InitListView(m_todoList);
	InitListView(m_filteredList);
	InitListView(m_currentList);
	m_currentList->removeColumn(1);

	m_widgetStack = new QWidgetStack(this);
	m_widgetStack->addWidget(m_currentList,0);
	m_widgetStack->addWidget(m_errorList,1);
	m_widgetStack->addWidget(m_fixmeList,2);
	m_widgetStack->addWidget(m_todoList,3);
	m_widgetStack->addWidget(m_filteredList,4);

	m_tabBar = new QTabBar(this);
	m_tabBar->insertTab(new QTab(i18n("Current")),0);
	m_tabBar->insertTab(new QTab(i18n("Errors")),1);
	m_tabBar->insertTab(new QTab(i18n("Fixme")),2);
	m_tabBar->insertTab(new QTab(i18n("Todo")),3);
	m_tabBar->insertTab(new QTab(i18n("Filtered")),4);
	m_tabBar->setTabEnabled(0,false);
	m_tabBar->setTabEnabled(4,false);

	m_timer = new QTimer( this );
	m_parseCheckTimeout = new QTimer( this );

	m_filterEdit = new QLineEdit(this);

	QLabel* m_filterLabel = new QLabel(i18n("Lookup:"),this);

	m_gridLayout->addWidget(m_tabBar,0,0);
	m_gridLayout->addMultiCellWidget(m_widgetStack,1,1,0,2);
	m_gridLayout->addWidget(m_filterLabel,0,1,Qt::AlignRight);
	m_gridLayout->addWidget(m_filterEdit,0,2,Qt::AlignLeft);

	connect( m_filterEdit, SIGNAL(returnPressed()),
	         this, SLOT(slotFilter()) );
	connect( m_filterEdit, SIGNAL(textChanged( const QString & )),
	         this, SLOT(slotFilter()) );
	connect( m_tabBar, SIGNAL(selected(int)),
	         this, SLOT(slotTabSelected(int)) );
	connect( part->partController(), SIGNAL(activePartChanged(KParts::Part*)),
	         this, SLOT(slotActivePartChanged(KParts::Part*)) );
	connect( part->partController(), SIGNAL(partAdded(KParts::Part*)),
	         this, SLOT(slotPartAdded(KParts::Part*)) );
	connect( part->partController(), SIGNAL(partRemoved(KParts::Part*)),
	         this, SLOT(slotPartRemoved(KParts::Part*)) );

	connect( part, SIGNAL(fileParsed(const QString&)), this, SLOT(slotFileParsed(const QString&)) );

    connect( m_timer, SIGNAL(timeout()), this, SLOT(reparse()) );
    connect( m_parseCheckTimeout, SIGNAL(timeout()), this, SLOT(slotParseCheck()) );
    
	connect( part->partController(), SIGNAL(closedFile(const KURL&)),
	         this, SLOT(closedFile(const KURL&)) );

	configure();

	slotActivePartChanged( part->partController()->activePart() );
}

void ProblemReporter::slotFilter()
{
	if(!m_tabBar->isTabEnabled(4))
		m_tabBar->setTabEnabled(4,true);

	m_tabBar->tab(4)->setText(i18n("Filtered: %1").arg( m_filterEdit->text() ));
	m_tabBar->setCurrentTab(4);

	m_filteredList->clear();

	filterList(m_errorList,i18n("Error"));
	filterList(m_fixmeList,i18n("Fixme"));
	filterList(m_todoList,i18n("Todo"));

}

void ProblemReporter::filterList(KListView* listview, const QString& level)
{
	QListViewItemIterator it( listview );
	while ( it.current() )
	{
		if ( it.current()->text(3).contains(m_filterEdit->text(),false))
		{
			new KListViewItem(m_filteredList,level,
			                  it.current()->text(0), it.current()->text(1),
			                  it.current()->text(2), it.current()->text(3));
		}
		++it;
	}
}

void ProblemReporter::slotTabSelected( int tabindex )
{
	m_widgetStack->raiseWidget(tabindex);
}

void ProblemReporter::InitListView(KListView* listview)
{
	listview->addColumn( i18n("File") );
	listview->addColumn( i18n("Line") );
	listview->addColumn( i18n("Column") );
	listview->addColumn( i18n("Problem") );
	listview->setAllColumnsShowFocus( TRUE );

	connect( listview, SIGNAL(executed(QListViewItem*)),
	         this, SLOT(slotSelected(QListViewItem*)) );

	connect( listview, SIGNAL(returnPressed(QListViewItem*)),
	         this, SLOT(slotSelected(QListViewItem* )) );

}

ProblemReporter::~ProblemReporter()
{
}

void ProblemReporter::slotActivePartChanged( KParts::Part* part )
{
	m_fileName = QString::null;

	m_tabBar->setTabEnabled(0,false);

	m_timer->stop();

	if( m_document )
		disconnect( m_document, 0, this, 0 );

	m_document = dynamic_cast<KTextEditor::Document*>( part );
	m_markIface = 0;

	if( !m_document )
	{
		return;
	}

	m_fileName = m_document->url().path();

	initCurrentList();

	if( !m_cppSupport->isValidSource(m_fileName) )
		return;

	connect( m_document, SIGNAL(textChanged()), this, SLOT(slotTextChanged()) );
	m_markIface = dynamic_cast<KTextEditor::MarkInterface*>( part );

	if( !m_cppSupport->backgroundParser() )
		return;

	m_tabBar->setTabEnabled(0,true);

    m_parseCheckTimeout->start( 300 );
}

void ProblemReporter::slotParseCheck() {
	m_parseCheckTimeout->stop();
	m_cppSupport->backgroundParser()->lock();
	bool needReparse = false;
	//if( !m_cppSupport->backgroundParser()->hasTranslationUnit(m_fileName) ) ///with this, the background-parser is used, and it may crash.
	if( !m_cppSupport->backgroundParser()->translationUnit(m_fileName) )
		needReparse = true;
	m_cppSupport->backgroundParser()->unlock();
	
	if( needReparse )
		reparse();
}

void ProblemReporter::closedFile(const KURL &fileName)
{
	QValueList<Problem> problems = m_cppSupport->backgroundParser()->problems( fileName.path() , true , true);
}

void ProblemReporter::slotTextChanged()
{
	if( !m_active )
		return;

	m_cppSupport->setTyping(true);
	m_timer->changeInterval( m_delay );
}

void ProblemReporter::removeAllItems( QListView* listview, const QString& filename )
{
	QListViewItem* current = listview->firstChild();
	while( current ){
		QListViewItem* i = current;
		current = current->nextSibling();

		if( i->text(0) == filename )
			delete( i );
	}
}

void ProblemReporter::removeAllProblems( const QString& filename )
{
	QString relFileName = filename;
	relFileName.remove(m_cppSupport->project()->projectDirectory());

	kdDebug(9008) << "ProblemReporter::removeAllProblems()" << relFileName << endl;

	removeAllItems(m_errorList,relFileName);
	removeAllItems(m_fixmeList,relFileName);
	removeAllItems(m_todoList,relFileName);
	
	if( m_document && m_markIface )
	{
		QPtrList<KTextEditor::Mark> marks = m_markIface->marks();
		QPtrListIterator<KTextEditor::Mark> it( marks );
		while( it.current() )
		{
			m_markIface->removeMark( it.current()->line, KTextEditor::MarkInterface::markType07 );
			++it;
		}
	}
}

void ProblemReporter::reparse()
{
	m_timer->stop();
	if( m_fileName.isEmpty() ) return;

	if( !m_cppSupport->isValid() )
		return;

	m_currentList->clear();

    /*if( m_canParseFile )
    {*/
      
        m_cppSupport->parseFileAndDependencies( m_fileName );
    //        m_canParseFile = false;
        kdDebug(9007) << m_fileName << "---> file added to background-parser(by problem-reporter)" << endl;
    /*}else{
        if(m_timeout.isNull()) {
            m_timeout = QTime::currentTime();
            kdDebug(9007) << "cannot parse, parser seems to be busy" << endl;
        }
        if(m_timeout.msecsTo(QTime::currentTime()) > 200) {
            kdDebug(9007) << "parsing was reset" << endl;
            m_timeout = QTime();
            m_canParseFile = true;
        }else{
            m_timer->changeInterval( m_delay );
        }
    }*/
}

void ProblemReporter::initCurrentList()
{
	m_tabBar->setTabEnabled(0,true);

	QString relFileName = m_fileName;
	relFileName.remove(m_cppSupport->project()->projectDirectory());

	m_currentList->clear();

	updateCurrentWith(m_errorList, i18n("Error"),relFileName);
	updateCurrentWith(m_fixmeList,i18n("Fixme"),relFileName);
	updateCurrentWith(m_todoList,i18n("Todo"),relFileName);

	m_tabBar->setCurrentTab(0);
}

void ProblemReporter::updateCurrentWith(QListView* listview, const QString& level, const QString& filename)
{
	QListViewItemIterator it(listview);
	while ( it.current() )
	{
		if( it.current()->text(0) == filename)
			new QListViewItem(m_currentList,level,it.current()->text(1),it.current()->text(2),it.current()->text(3));

		++it;
	}
}

void ProblemReporter::slotSelected( QListViewItem* item )
{
	bool is_filtered = false;
	bool is_current = false;
	if(item->listView() == m_filteredList)
		is_filtered = true;
	else if(item->listView() == m_currentList)
		is_current = true;


	KURL url( is_current ? m_fileName : m_cppSupport->project()->projectDirectory() + item->text(0 + is_filtered) );
	int line = item->text( 1 + is_filtered).toInt();
    // int column = item->text( 3 ).toInt();
	m_cppSupport->partController()->editDocument( url, line-1 );
//    m_cppSupport->mainWindow()->lowerView( this );
}

bool ProblemReporter::hasErrors( const QString& fileName ) {
	QListViewItem* current = m_errorList->firstChild();
	while( current ){
		QListViewItem* i = current;
		current = current->nextSibling();
		
		if( i->text(0) == fileName )
			return true;
	}
	return false;
}

void ProblemReporter::reportProblem( const QString& fileName, const Problem& p )
{
	int markType = levelToMarkType( p.level() );
	if( markType != -1 && m_document && m_markIface && m_fileName == fileName )
		m_markIface->addMark( p.line(), markType );

	QString msg = p.text();
	msg = msg.replace( QRegExp("\n"), "" );

	QString relFileName = fileName;
	relFileName.remove(m_cppSupport->project()->projectDirectory());

	KListView* list;

	switch( p.level() )
	{
	case Problem::Level_Error:
		list = m_errorList;
		break;
	case Problem::Level_Warning:
		list = m_errorList;
		break;
	case Problem::Level_Todo:
		list = m_todoList;
		break;
	case Problem::Level_Fixme:
		list = m_fixmeList;
		break;
	default:
		list = NULL;
	}

	if(list)
	{
		new ProblemItem( list,
		                 relFileName,
		                 QString::number( p.line() + 1 ),
		                 QString::number( p.column() + 1 ),
		                 msg );
	}

	if(fileName == m_fileName)
	{
		new QListViewItem(m_currentList,levelToString(p.level()),
		                  QString::number( p.line() + 1 ),
		                  QString::number( p.column() + 1 ),
		                  msg);
	}
}

void ProblemReporter::configure()
{
	kdDebug(9007) << "ProblemReporter::configure()" << endl;
	KConfig* config = kapp->config();
	config->setGroup( "General Options" );
	m_active = config->readBoolEntry( "EnableCppBgParser", TRUE );
	m_delay = config->readNumEntry( "BgParserDelay", 250 );
}

void ProblemReporter::configWidget( KDialogBase* dlg )
{
	QVBox *vbox = dlg->addVBoxPage(i18n("C++ Parsing"), i18n("C++ Parsing"),
	                               BarIcon( "source_cpp", KIcon::SizeMedium) );
	ConfigureProblemReporter* w = new ConfigureProblemReporter( vbox );
	w->setPart( m_cppSupport );
	connect(dlg, SIGNAL(okClicked()), w, SLOT(accept()));
	connect(dlg, SIGNAL(okClicked()), this, SLOT(configure()));
}

void ProblemReporter::slotPartAdded( KParts::Part* part )
{
	KTextEditor::MarkInterfaceExtension* iface = dynamic_cast<KTextEditor::MarkInterfaceExtension*>( part );

	if( !iface )
		return;

	iface->setPixmap( KTextEditor::MarkInterface::markType07, SmallIcon("stop") );
}

void ProblemReporter::slotPartRemoved( KParts::Part* part )
{
	kdDebug(9007) << "ProblemReporter::slotPartRemoved()" << endl;
	if( part == m_document )
	{
		m_document = 0;
		m_timer->stop();
	}
}

QString ProblemReporter::levelToString( int level ) const
{
	switch( level )
	{
	case Problem::Level_Error:
		return QString( i18n("Error") );
	case Problem::Level_Warning:
		return QString( i18n("Warning") );
	case Problem::Level_Todo:
		return QString( i18n("Todo") );
	case Problem::Level_Fixme:
		return QString( i18n("Fixme") );
	default:
		return QString::null;
	}
}

int ProblemReporter::levelToMarkType( int level ) const
{
	switch( level )
	{
	case Problem::Level_Error:
		return KTextEditor::MarkInterface::markType07;
	case Problem::Level_Warning:
		return -1;
	case Problem::Level_Todo:
		return -1;
	case Problem::Level_Fixme:
		return -1;
	default:
		return -1;
	}
}

void ProblemReporter::slotFileParsed( const QString& fileName )
{
	if( m_active && fileName == m_fileName ){
		m_canParseFile = true;
	}
}

#include "problemreporter.moc"
//kate: indent-mode csands; tab-width 4; space-indent off;
