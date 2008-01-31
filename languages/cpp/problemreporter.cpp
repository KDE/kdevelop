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
		if ( column == 1 || column == 2 )
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
// m_document( 0 ),
m_markIface( 0 )
{
	QWhatsThis::add(this, i18n("<b>Problem reporter</b><p>This window shows various \"problems\" in your project. "
	                           "It displays TODO entries, FIXME's and errors reported by a language parser. "
	                           "To add a TODO or FIXME entry, just type<br>"
	                           "<tt>//@todo my todo</tt><br>"
	                           "<tt>//TODO: my todo</tt><br>"
	                           "<tt>//FIXME fix this</tt>"));

	m_initCurrentTimer = new QTimer( this );
	connect( m_initCurrentTimer, SIGNAL(timeout()), this, SLOT(initCurrentList()) );
	m_gridLayout = new QGridLayout(this,2,3);

	m_errorList = new KListView(this);
	m_warningList = new KListView(this);
	m_fixmeList = new KListView(this);
	m_todoList = new KListView(this);
	m_filteredList = new KListView(this);
	m_currentList = new KListView(this);

	m_filteredList->addColumn( i18n("Level") );
	m_currentList->addColumn( i18n("Level") );

    //addColumn( i18n("Level") );
	InitListView(m_warningList);
	InitListView(m_errorList);
	InitListView(m_fixmeList);
	InitListView(m_todoList);
	InitListView(m_filteredList);
	InitListView(m_currentList);
	m_currentList->removeColumn(1);

	m_widgetStack = new QWidgetStack(this);
	m_widgetStack->addWidget(m_currentList,0);
	m_widgetStack->addWidget(m_errorList,1);
	m_widgetStack->addWidget(m_warningList,2);
	m_widgetStack->addWidget(m_fixmeList,3);
	m_widgetStack->addWidget(m_todoList,4);
	m_widgetStack->addWidget(m_filteredList,5);

	m_tabBar = new QTabBar(this);
	m_tabBar->insertTab(new QTab(i18n("Current")),0);
	m_tabBar->insertTab(new QTab(i18n("Errors")),1);
	m_tabBar->insertTab(new QTab(i18n("Warnings")),2);
	m_tabBar->insertTab(new QTab(i18n("Fixme")),3);
	m_tabBar->insertTab(new QTab(i18n("Todo")),4);
	m_tabBar->insertTab(new QTab(i18n("Filtered")),5);
	m_tabBar->setTabEnabled(0,false);
	m_tabBar->setTabEnabled(5,false);

	m_filterEdit = new KLineEdit(this);

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

	// any editors that were open when we loaded the project needs to have their markType07 icon set too..
	QPtrListIterator<KParts::Part> it( *m_cppSupport->partController()->parts() );
	while( it.current() )
	{
		if ( KTextEditor::MarkInterfaceExtension* iface = dynamic_cast<KTextEditor::MarkInterfaceExtension*>( it.current() ) )
		{
			iface->setPixmap( KTextEditor::MarkInterface::markType07, SmallIcon("stop") );
		}
		++it;
	}
	
	slotActivePartChanged( part->partController()->activePart() );
}

void ProblemReporter::slotFilter()
{
	if(!m_tabBar->isTabEnabled(5))
		m_tabBar->setTabEnabled(5,true);

	m_tabBar->tab(5)->setText(i18n("Filtered: %1").arg( m_filterEdit->text() ));
	m_tabBar->setCurrentTab(5);

	m_filteredList->clear();

	if ( m_filterEdit->text().isEmpty() )
	{
		m_tabBar->setTabEnabled( 5, false );
		return;
	}
	
	filterList(m_errorList,i18n("Error"));
	filterList(m_warningList,i18n("Warning"));
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
	m_currentList->clear();
	
	KParts::ReadOnlyPart * ro_part = dynamic_cast<KParts::ReadOnlyPart*>( part );
	m_markIface = dynamic_cast<KTextEditor::MarkInterface*>( part );
		
	if ( !ro_part )
	{
		m_tabBar->setTabEnabled(0,false);
		return;
	}
	
	m_fileName = ro_part->url().path();
	
	initCurrentList();
}

void EfficientKListView::limitSize( int size )
{
	if( m_map.size() <= size + 50 ) return;
	
	QMap<int, HashedString> backMap;
	for( InsertionMap::const_iterator it = m_insertionNumbers.begin(); it != m_insertionNumbers.end(); ++it )
		backMap[ (*it).second ] = (*it).first;

	for( QMap<int, HashedString>::const_iterator it = backMap.begin(); it != backMap.end() && m_map.size() > size; ++it )
		removeAllItems( (*it).str() );
}

void EfficientKListView::removeAllItems( const QString& str )
{
	HashedString h(str);
	m_insertionNumbers.erase( h ) ;
	
	std::pair<Map::iterator, Map::iterator> p = m_map.equal_range( h );
	
	for( Map::iterator it = p.first; it != p.second; ++it ) {
		delete( (*it).second );
	}

	m_map.erase( p.first, p.second );
}

void ProblemReporter::removeAllProblems( const QString& filename )
{
	QString relFileName = m_cppSupport->project()->relativeProjectFile(filename);
	
	kdDebug(9007) << "ProblemReporter::removeAllProblems()" << relFileName << endl;

	m_errorList.limitSize( 300 );
	m_warningList.limitSize( 300 );
	m_fixmeList.limitSize( 300 );
	m_todoList.limitSize( 300 );
	
	m_warningList.removeAllItems( relFileName );
	m_errorList.removeAllItems( relFileName );
	m_fixmeList.removeAllItems( relFileName );
	m_todoList.removeAllItems( relFileName );
    
	if( m_markIface )
	{
		QPtrList<KTextEditor::Mark> marks = m_markIface->marks();
		QPtrListIterator<KTextEditor::Mark> it( marks );
		while( it.current() )
		{
			m_markIface->removeMark( it.current()->line, KTextEditor::MarkInterface::markType07 );
			++it;
		}
	}
  m_initCurrentTimer->start(500, true);
}

void ProblemReporter::initCurrentList()
{
	m_tabBar->setTabEnabled(0,true);

	QString relFileName = m_cppSupport->project()->relativeProjectFile(m_fileName);

 	m_currentList->clear();

	updateCurrentWith(m_errorList, i18n("Error"),relFileName);
	updateCurrentWith(m_warningList, i18n("Warning"),relFileName);
	updateCurrentWith(m_fixmeList,i18n("Fixme"),relFileName);
	updateCurrentWith(m_todoList,i18n("Todo"),relFileName);

// 	m_tabBar->setCurrentTab(0);
}

void ProblemReporter::updateCurrentWith(EfficientKListView& listview, const QString& level, const QString& filename)
{
    EfficientKListView::Range r = listview.getRange( filename );
    for( ; r.first != r.second; ++r.first )
	    new ProblemItem(m_currentList,level,(*r.first).second->text(1),(*r.first).second->text(2),(*r.first).second->text(3));
}

void ProblemReporter::slotSelected( QListViewItem* item )
{
	bool is_filtered = false;
	bool is_current = false;
	if(item->listView() == m_filteredList)
		is_filtered = true;
	else if(item->listView() == m_currentList)
		is_current = true;

	//either use current file m_fileName or assemble a new one from current item (relative path) and projectDirectory
	KURL url( is_current ? m_fileName : m_cppSupport->project()->projectDirectory() + "/" + item->text(0 + is_filtered) );
	int line = item->text( 1 + is_filtered).toInt();
    // int column = item->text( 3 ).toInt();
	m_cppSupport->partController()->editDocument( url, line-1 );
}

bool ProblemReporter::hasErrors( const QString& fileName ) {
    return m_errorList.hasItem( fileName );
}

void ProblemReporter::reportProblem( const QString& fileName, const Problem& p )
{
	int markType = levelToMarkType( p.level() );
	if( markType != -1 && m_markIface && m_fileName == fileName )
		m_markIface->addMark( p.line(), markType );

	QString msg = p.text();
	msg = msg.replace( QRegExp("\n"), "" );

	QString relFileName = m_cppSupport->project()->relativeProjectFile(fileName);

	EfficientKListView* list;

	switch( p.level() )
	{
	case Problem::Level_Error:
		list = &m_errorList;
		break;
	case Problem::Level_Warning:
		list = &m_warningList;
		break;
	case Problem::Level_Todo:
		list = &m_todoList;
		break;
	case Problem::Level_Fixme:
		list = &m_fixmeList;
		break;
	default:
		list = NULL;
	}

	if(list)
	{
		list->addItem( relFileName, new ProblemItem( *list,
		                 relFileName,
		                 QString::number( p.line() + 1 ),
		                 QString::number( p.column() + 1 ),
                                                     msg ) );
        
	}
	
	m_initCurrentTimer->start( 500, true );
}

void ProblemReporter::slotPartAdded( KParts::Part* part )
{
	KTextEditor::MarkInterfaceExtension* iface = dynamic_cast<KTextEditor::MarkInterfaceExtension*>( part );

	if( !iface )
		return;

	iface->setPixmap( KTextEditor::MarkInterface::markType07, SmallIcon("stop") );
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

#include "problemreporter.moc"
//kate: indent-mode csands; tab-width 4; space-indent off;
