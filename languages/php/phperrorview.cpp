/*
   Copyright (C) 2002 by Roberto Raggi <roberto@kdevelop.org>
   Copyright (C) 2005 by Nicolas Escuder <n.escuder@intra-links.com>

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

#include "phperrorview.h"
#include "phpsupportpart.h"

#include <kdevpartcontroller.h>
#include <kdevmainwindow.h>
#include <kdevproject.h>

#include <kdeversion.h>
#include <kparts/part.h>
#include <ktexteditor/editinterface.h>
#include <ktexteditor/document.h>
#include <ktexteditor/markinterface.h>

#if (KDE_VERSION > 305)
# include <ktexteditor/markinterfaceextension.h>
#else
# include "kde30x_markinterfaceextension.h"
#endif
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
       const QString& file, const QString& line, const QString& column  )
   : KListViewItem( parent, problem, file, line, column ) {}

    ProblemItem( QListViewItem* parent, const QString& problem,
       const QString& file, const QString& line, const QString& column  )
   : KListViewItem( parent,  problem, file, line, column ) {}

    int compare( QListViewItem* item, int column, bool ascending ) const {
   if( column == 2 || column == 3 ){
       int a = text( column ).toInt();
       int b = item->text( column ).toInt();
       if( a == b )
      return 0;
       return( a > b ? 1 : -1 );
   }
   return KListViewItem::compare( item, column, ascending );
    }

};

PHPErrorView::PHPErrorView( PHPSupportPart* part, QWidget* parent, const char* name )
    : QWidget( parent, name ? name : "problemreporter" ),
      m_phpSupport( part ),
      m_document( 0 ),
      m_markIface( 0 )
{
    QWhatsThis::add(this, i18n("<b>Problem reporter</b><p>This window shows various \"problems\" in your project. "
        "It displays TODO entries, FIXME's and errors reported by a language parser. "
        "To add a TODO or FIXME entry, just type<br>"
        "<tt>//@todo my todo</tt><br>"
        "<tt>//TODO: my todo</tt><br>"
        "<tt>//FIXME fix this</tt>"));

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

    m_filterEdit = new QLineEdit(this);

    QLabel* m_filterLabel = new QLabel(i18n("Lookup:"),this);

    m_gridLayout->addWidget(m_tabBar,0,0);
    m_gridLayout->addMultiCellWidget(m_widgetStack,1,1,0,2);
    m_gridLayout->addWidget(m_filterLabel,0,1,Qt::AlignRight);
    m_gridLayout->addWidget(m_filterEdit,0,2,Qt::AlignLeft);

    connect( m_filterEdit, SIGNAL(returnPressed()), this, SLOT(slotFilter()) );
    connect( m_filterEdit, SIGNAL(textChanged( const QString & )), this, SLOT(slotFilter()) );
    connect( m_tabBar, SIGNAL(selected(int)), this, SLOT(slotTabSelected(int)) );
    connect( part->partController(), SIGNAL(activePartChanged(KParts::Part*)), this, SLOT(slotActivePartChanged(KParts::Part*)) );
    connect( part->partController(), SIGNAL(partAdded(KParts::Part*)), this, SLOT(slotPartAdded(KParts::Part*)) );
    connect( part->partController(), SIGNAL(partRemoved(KParts::Part*)), this, SLOT(slotPartRemoved(KParts::Part*)) );

//    configure();

    slotActivePartChanged( part->partController()->activePart() );
}

void PHPErrorView::slotFilter()
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

void PHPErrorView::filterList(KListView* listview, const QString& level)
{
    QListViewItemIterator it( listview );
    while ( it.current() ) {
        if ( it.current()->text(3).contains(m_filterEdit->text(),false))
       new KListViewItem(m_filteredList,level,
       it.current()->text(0),it.current()->text(1),it.current()->text(2),it.current()->text(3));
        ++it;
    }
}

void PHPErrorView::slotTabSelected( int tabindex )
{
    m_widgetStack->raiseWidget(tabindex);
}

void PHPErrorView::InitListView(KListView* listview)
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

PHPErrorView::~PHPErrorView()
{
}

void PHPErrorView::slotActivePartChanged( KParts::Part* part )
{
    if( !part )
    {
        m_tabBar->setTabEnabled(0,false);
   return;
    }

    if( m_document )
   disconnect( m_document, 0, this, 0 );

    m_document = dynamic_cast<KTextEditor::Document*>( part );
    m_markIface = 0;

    if( !m_document )
     {
        m_tabBar->setTabEnabled(0,false);
        return;
     }

    m_fileName = m_document->url().path();

    initCurrentList();

    m_markIface = dynamic_cast<KTextEditor::MarkInterface*>( part );
}

void PHPErrorView::removeAllItems( QListView* listview, const QString& filename )
{
    QListViewItem* current = listview->firstChild();
    while( current ){
   QListViewItem* i = current;
   current = current->nextSibling();

   if( i->text(0) == filename )
       delete( i );
    }
}

void PHPErrorView::removeAllProblems( const QString& filename )
{
  QString relFileName = filename;
  relFileName.remove(m_phpSupport->project()->projectDirectory());

    kdDebug(9008) << "PHPErrorView::removeAllProblems()" << relFileName << endl;

    if(filename == m_fileName)
      m_currentList->clear();
      
  removeAllItems(m_errorList,relFileName);
  removeAllItems(m_fixmeList,relFileName);
  removeAllItems(m_todoList,relFileName);

    if( m_document && m_markIface ){
   QPtrList<KTextEditor::Mark> marks = m_markIface->marks();
   QPtrListIterator<KTextEditor::Mark> it( marks );
   while( it.current() ){
       m_markIface->removeMark( it.current()->line, KTextEditor::MarkInterface::markType07 );
       ++it;
   }
    }
}

void PHPErrorView::initCurrentList()
{
    m_tabBar->setTabEnabled(0,true);

    QString relFileName = m_fileName;
    relFileName.remove(m_phpSupport->project()->projectDirectory());

    m_currentList->clear();

    updateCurrentWith(m_errorList, i18n("Error"),relFileName);
    updateCurrentWith(m_fixmeList,i18n("Fixme"),relFileName);
    updateCurrentWith(m_todoList,i18n("Todo"),relFileName);

    m_tabBar->setCurrentTab(0);
}

void PHPErrorView::updateCurrentWith(QListView* listview, const QString& level, const QString& filename)
{
    QListViewItemIterator it(listview);
    while ( it.current() ) {
        if( it.current()->text(0) == filename)
   new QListViewItem(m_currentList,level,it.current()->text(1),it.current()->text(2),it.current()->text(3));
        ++it;
    }
}

void PHPErrorView::slotSelected( QListViewItem* item )
{
    bool is_filtered = false;
    bool is_current = false;
    if(item->listView() == m_filteredList)
      is_filtered = true;
    else if(item->listView() == m_currentList)
      is_current = true;


    KURL url( is_current ? m_fileName : m_phpSupport->project()->projectDirectory() + item->text(0 + is_filtered) );
    int line = item->text( 1 + is_filtered).toInt();
    // int column = item->text( 3 ).toInt();
    m_phpSupport->partController()->editDocument( url, line-1 );
}

void PHPErrorView::reportProblem( const QString& fileName, int line, int level,  const QString& text)
{
    int markType = levelToMarkType( level );
    if ( markType != -1 && m_document && m_markIface && m_fileName == fileName ){
      m_markIface->addMark( line, markType );
    }

    QString msg = text;
    msg = msg.replace( QRegExp("\n"), "" );

    QString relFileName = fileName;
    relFileName.remove(m_phpSupport->project()->projectDirectory());

    KListView* list;

    switch( level )
    {
    case Action::Level_Error:
    case Action::Level_ErrorNoSuchFunction:
    case Action::Level_ErrorParse:
   list = m_errorList;
   m_tabBar->setCurrentTab(m_tabBar->tab(1));
   break;
    case Action::Level_Warning:
   list = m_errorList;
   break;
    case Action::Level_Todo:
   list = m_todoList;
   break;
    case Action::Level_Fixme:
   list = m_fixmeList;
   break;
    default:
        list = NULL;
    }

    if(list)
    new ProblemItem( list,
           relFileName,
           QString::number( line + 1 ),
           0,
           msg );

    if(fileName == m_fileName)
    new QListViewItem(m_currentList, levelToString( level ),
           QString::number( line + 1 ),
           0,
           msg);
}

void PHPErrorView::slotPartAdded( KParts::Part* part )
{
    KTextEditor::MarkInterfaceExtension* iface = dynamic_cast<KTextEditor::MarkInterfaceExtension*>( part );

    if( !iface )
   return;

    iface->setPixmap( KTextEditor::MarkInterface::markType07, SmallIcon("stop") );
}

void PHPErrorView::slotPartRemoved( KParts::Part* part )
{
    kdDebug(9007) << "PHPErrorView::slotPartRemoved()" << endl;
    if( part == m_document ){
   m_document = 0;
    }
}

QString PHPErrorView::levelToString( int level ) const
{
    switch( level )
    {
    case Action::Level_ErrorNoSuchFunction:
   return QString( i18n("Undefined function") );
    case Action::Level_ErrorParse:
   return QString( i18n("Parse Error") );
    case Action::Level_Error:
   return QString( i18n("Error") );
    case Action::Level_Warning:
   return QString( i18n("Warning") );
    case Action::Level_Todo:
   return QString( i18n("Todo") );
    case Action::Level_Fixme:
   return QString( i18n("Fixme") );
    default:
        return QString::null;
    }
}

int PHPErrorView::levelToMarkType( int level ) const
{
    switch( level )
    {
    case Action::Level_ErrorNoSuchFunction:
    case Action::Level_ErrorParse:
    case Action::Level_Error:
   return KTextEditor::MarkInterface::markType07;
    case Action::Level_Warning:
        return -1;
    case Action::Level_Todo:
        return -1;
    case Action::Level_Fixme:
        return -1;
    default:
        return -1;
    }
}

#include "phperrorview.moc"
