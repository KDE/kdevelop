#include <QLayout>
#include <qpainter.h>
#include <q3popupmenu.h>
//Added by qt3to4:
#include <QVBoxLayout>

#include <kparts/part.h>
#include <klibloader.h>
#include <kurl.h>
#include <kdebug.h>
#include <k3listview.h>
#include <klocale.h>
#include <kstatusbar.h>

#include <kdevcore.h>
#include <kdevdocumentcontroller.h>
#include <kdevmainwindow.h>
#include <kdevproject.h>

#include "valgrind_part.h"
#include "valgrind_widget.h"

#define VALLISTVIEWITEMRTTI 130977

// helper class to sort the ListView by item number instead of the string representation of the item number
class ValListViewItem: public Q3ListViewItem
{
public:
  ValListViewItem( Q3ListView* parent, int key, int pid, const QString& message ):
    Q3ListViewItem( parent, QString::number( key ), QString::number( pid ), message ),
    _key( key ), _pid ( pid ), backtrace( false ), _line( -1 ), _active( false ) {}

  ValListViewItem( ValListViewItem* parent, int key, int pid, const QString& message, const QString& filename, int line, bool active ):
    Q3ListViewItem( parent, QString::number( key ), QString(), message ),
    _key( key ), _pid( pid ), backtrace( true ), _filename( filename ), _line( line ), _active( active )
  {
    if ( parent->_pid != _pid && _pid > 0 )
      setText( 1, QString::number( _pid ) );
  }

  virtual ~ValListViewItem();

  static int intCompare( int i1, int i2 )
  {
    if ( i1 > i2 )
      return 1;
    else if ( i1 < i2 )
      return -1;
    else
      return 0;
  }

  int compare( Q3ListViewItem* i, int col, bool ascending ) const
  {
    if ( !i || i->rtti() != VALLISTVIEWITEMRTTI )
      return Q3ListViewItem::compare( i, col, ascending );
    switch ( col ) {
      case 0 : return intCompare( ((ValListViewItem*)i)->_key, _key );
      case 1 : return intCompare( ((ValListViewItem*)i)->_pid, _pid );
      default: return Q3ListViewItem::compare( i, col, ascending );
    }
  }

  void paintCell( QPainter* p, const QColorGroup& cg, int column, int width, int align )
  {
    if ( _active ) {
      QFont fnt = p->font();
      fnt.setBold( true );
      p->setFont( fnt );
    }
    Q3ListViewItem::paintCell( p, cg, column, width, align );
  }

  int rtti() const { return VALLISTVIEWITEMRTTI; }

  QString fileName() const { return _filename; }
  int line() const { return _line; }
  QString message() const { return text( 2 ); }
  bool isHighlighted() const { return _active; }

private:
  int _key;
  int _pid;
  bool backtrace;
  QString _filename;
  int _line;
  bool _active;
};

ValListViewItem::~ValListViewItem() {}

ValgrindWidget::ValgrindWidget( ValgrindPart *part )
 : QWidget(0, "valgrind widget"), _part( part )
{
  QVBoxLayout* vbl = new QVBoxLayout( this );
  lv = new K3ListView( this );
  lv->addColumn( i18n( "No." ) );
  lv->addColumn( i18n( "Thread" ) );
  lv->addColumn( i18n( "Message" ) );
  lv->setSorting( 0, false );
  lv->setRootIsDecorated( true );
  lv->setAllColumnsShowFocus( true );
  vbl->addWidget( lv );

  popup = new Q3PopupMenu( lv, "valPopup" );
  popup->insertItem( i18n( "&Open Valgrind Output..." ), _part, SLOT(loadOutput()), 0, 0 );
  popup->insertSeparator();
  popup->insertItem( i18n( "Expand All Items" ), this, SLOT(expandAll()), 0, 2 );
  popup->insertItem( i18n( "Collapse All Items" ), this, SLOT(collapseAll()), 0, 3 );

  connect( popup, SIGNAL(aboutToShow()),
           this, SLOT(aboutToShowPopup()) );
  connect( lv, SIGNAL(executed(Q3ListViewItem*)),
           this, SLOT(executed(Q3ListViewItem*)) );
  connect( lv, SIGNAL(contextMenu(K3ListView*, Q3ListViewItem*, const QPoint&)),
           this, SLOT(slotContextMenu(K3ListView*, Q3ListViewItem*, const QPoint&)) );
}


ValgrindWidget::~ValgrindWidget()
{
}

void ValgrindWidget::clear()
{
  lv->clear();
  msgNumber = 1;
}

void ValgrindWidget::addMessage( const ValgrindItem& vi )
{
  QStringList projectFiles;
  QString projectDirectory;

  ValListViewItem* lvi = new ValListViewItem( lv, msgNumber++, vi.pid(), vi.message() );
  lvi->setMultiLinesEnabled( true );
  const ValgrindItem::BacktraceList backtrace = vi.backtrace();
  if ( !backtrace.isEmpty() )
    lvi->setExpandable( true );

  int i = 0;
  for ( ValgrindItem::BacktraceList::ConstIterator it = backtrace.begin(); it != backtrace.end(); ++it ) {
    new ValListViewItem( lvi, ++i, (*it).pid(), (*it).message(), (*it).url(), (*it).line(), (*it).isHighlighted() );
  }
}

void ValgrindWidget::executed( Q3ListViewItem* lvi )
{
  Q_ASSERT( _part );
  Q_ASSERT( _part->documentController() );
  Q_ASSERT( _part->mainWindow() );

  if ( !lvi || lvi->rtti() != VALLISTVIEWITEMRTTI )
    return;
  ValListViewItem* vli = 0;
  if ( !((ValListViewItem*)lvi)->fileName().isEmpty() ) {
    vli = (ValListViewItem*)lvi;
  } else if ( lvi->isExpandable() ) {
    // find the memleak position
    Q3ListViewItemIterator it( lv );
    while ( vli == 0 && it.current() ) {
      if ( it.current()->rtti() == VALLISTVIEWITEMRTTI && ((ValListViewItem*)it.current())->isHighlighted() )
          vli = (ValListViewItem*)it.current();
      ++it;
    }
  }
  if ( vli ) {
    // display the file
    _part->documentController()->editDocument( KUrl( vli->fileName() ), vli->line() - 1 );
    _part->mainWindow()->statusBar()->message( vli->message(), 10000 );
    _part->mainWindow()->lowerView( this );
  }
}

void ValgrindWidget::expandAll()
{
  Q3ListViewItem* child = lv->firstChild();
  while ( child ) {
    child->setOpen( true );
    child = child->nextSibling();
  }
}

void ValgrindWidget::collapseAll()
{
  Q3ListViewItem* child = lv->firstChild();
  while ( child ) {
    child->setOpen( false );
    child = child->nextSibling();
  }
}

void ValgrindWidget::aboutToShowPopup()
{
  bool en = (lv->firstChild() != 0);
  popup->setItemEnabled( 2, en );
  popup->setItemEnabled( 3, en );
}

void ValgrindWidget::slotContextMenu( K3ListView* l, Q3ListViewItem* /*i*/, const QPoint& p )
{
  if ( l != lv )
    return;

  popup->exec( p );
}

#include "valgrind_widget.moc"
