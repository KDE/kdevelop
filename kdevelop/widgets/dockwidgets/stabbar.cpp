/***************************************************************************
                         STabCtl part of KDEStudio
                             -------------------
    copyright            : (C) 1999 by Judin Maximus
    email                : novaprint@mtu-net.ru
 ***************************************************************************/

#include "stabbar.h"

#include <qapp.h>
#include <qwmatrix.h>
#include <qpushbutton.h>
#include <qpainter.h>


BarPainter::BarPainter( STabBar* parent )
:QWidget( parent )
{
  delta = 0;
  buffer = new QPixmap(0,0);
}

BarPainter::~BarPainter()
{
  delete buffer;
}

void BarPainter::paintEvent( QPaintEvent* )
{
  if ( buffer->isNull() ) return;
  drawBuffer();

  switch ( ((STabBar*)parent())->tabPos )
    case STabBar::TAB_TOP:{
      bitBlt( this, 0, 0, buffer, 0, 0, width(), height() );
      break;
    case STabBar::TAB_RIGHT:{
      QWMatrix m;
      m.rotate( -90 );
      QPixmap xbuffer = buffer->xForm(m);
      bitBlt( this, 0, 0, &xbuffer, 0, 0, width(), height() );
      break;
    }
  }
}

void BarPainter::resizeEvent( QResizeEvent *e )
{
  delete buffer;
  switch ( ((STabBar*)parent())->tabPos )
    case STabBar::TAB_TOP:{
      buffer = new QPixmap( width(), height() );
      break;
    case STabBar::TAB_RIGHT:{
      buffer = new QPixmap( height(), width() );
      break;
    }
  }
}

void BarPainter::drawBuffer()
{
  QColor c1 = white;
  QColor c2 = colorGroup().dark();
  QColor c4 = white; // for paint top line;

  int W = 0;
  int H = 0;
  int shadowX = 1;
  int shadowY = 1;
  switch ( ((STabBar*)parent())->tabPos )
    case STabBar::TAB_TOP:{
      W = width();
      H = height();
      break;
    case STabBar::TAB_RIGHT:{
      shadowX = -1;
      c1 = colorGroup().dark();
      c2 = white;
      H = width();
      W = height();
      break;
    }
  }

  QPainter paint;
  paint.begin(buffer);
  paint.setBrushOrigin(0,0);
  paint.fillRect( 0, 0, W, H, QBrush( colorGroup().brush( QColorGroup::Background ) ));

  int x = 2;
  int curTab  = ((STabBar*)parent())->_currentTab;
  int curTabNum = -1;
  int leftTab = ((STabBar*)parent())->leftTab;
  int curx = -1; // start current tab ( selected )
  int curWidth = -1;
  int broken = -1;
  bool iconShow = ((STabBar*)parent())->iconShow;

  QList<TabBarData> *mainData = ((STabBar*)parent())->mainData;
  for ( uint k = 0; k < mainData->count(); k++ ){
    int x1 = x;
    int y1 = 2;
    int width = mainData->at(k)->width;

    if ( mainData->at(k)->id == curTab ){  // store current tab start x
      curTabNum = k;
      curx = x;
      curWidth = width;
      x1 -= 1;
      y1 -= 1;
    }

    if ( mainData->at(k)->pix != 0L && iconShow ){
      QWMatrix m;
      switch ( ((STabBar*)parent())->tabPos )
        case STabBar::TAB_TOP:{
          break;
        case STabBar::TAB_RIGHT:{
          m.rotate( 90 );
          break;
        }
      }
      paint.drawPixmap( x1+ 11, y1 + 2 , mainData->at(k)->pix->xForm(m) );
    }

    int ty = ( H + fontMetrics().height() ) / 2 - 2;
    int tx = ( mainData->at(k)->pix != 0L && iconShow ) ? 32:12;

    paint.setFont( parentWidget()->font() );

    if ( mainData->at(k)->enabled ){
      paint.setPen( mainData->at(k)->textColor );
      paint.drawText( x1 + tx , ty + y1 , mainData->at(k)->label );
    } else {
      paint.setPen( colorGroup().light() );
      paint.drawText( x1 + tx + shadowX, ty + y1 + shadowY, mainData->at(k)->label );
      paint.setPen( colorGroup().mid() );
      paint.drawText( x1 + tx , ty + y1 , mainData->at(k)->label );
    }

    paint.setPen( c1 );
    paint.moveTo( x1, H + 1 -y1 );
    paint.lineTo( x1, y1 );

    paint.setPen( c4 );
    paint.lineTo( x1 + width - 1, y1 );

    paint.setPen( c2 );
    paint.lineTo( x1 + width - 1, H+1-y1 );

/***************************************************************/
    paint.setPen( c1 );
    paint.moveTo( x1 + 4, y1 + H - 5 );
    paint.lineTo( x1 + 4, 3+y1 );

    paint.moveTo( x1 + 7, y1 + H - 5 );
    paint.lineTo( x1 + 7, 3+y1 );

    paint.setPen( c2 );
    paint.moveTo( x1 + 5, y1 + H - 5 );
    paint.lineTo( x1 + 5, 3+y1 );

    paint.moveTo( x1 + 8, y1 + H - 5 );
    paint.lineTo( x1 + 8, 3+y1 );
/***************************************************************/

    // fixed picture for leftTab
    if ( leftTab == (int)k + 1 ){
      paint.fillRect( x1 + width - 2, 0, 2, H - 1, QBrush( colorGroup().brush( QColorGroup::Background ) ));
    }

    // paint broken left
    if ( (int)k == leftTab && k != 0 )
    {
      int yy = y1;
      int xx = x1 - 2;
      paint.fillRect( x1, 0, 1, H - 1, QBrush( colorGroup().brush( QColorGroup::Background ) ));
      paint.setPen( c1 );
      do {
          paint.drawPoint( xx + 2, yy );
          paint.drawPoint( xx + 1, yy + 1 );
          paint.moveTo( xx + 1, yy + 1 );
          paint.lineTo( xx + 1, yy + 3 );
          paint.drawPoint( xx + 2, yy + 4 );
          paint.lineTo( xx + 2, yy + 6 );
          paint.drawPoint( xx + 3, yy + 7 );
          paint.lineTo( xx + 3, yy + 9 );
          paint.drawPoint( xx + 2, yy + 10 );
          paint.drawPoint( xx + 2, yy + 11 );
          yy+= 12;
      } while ( yy < H );
    }

    x += width;
    if ( x >= W && broken == -1 ) broken = k; // store right broken tab
  }

  // modify ( paint ) selected tab
  if ( curx != -1 && curTabNum >= leftTab ){
    curx -= 2;
    curWidth += 4;
    paint.setPen( c1 );
    paint.moveTo( curx, H-1 );
    paint.lineTo( curx, 0 );
    paint.setPen( c4 );
    paint.lineTo( curx + curWidth - 2, 0 );

    paint.setPen( c2 );
    paint.moveTo( curx + curWidth - 1, 0 );
    paint.lineTo( curx + curWidth - 1, H-1 );

    paint.fillRect( curx + 1, 1, 2, H - 1, QBrush( colorGroup().brush( QColorGroup::Background ) ));
    paint.fillRect( curx + curWidth - 4, 1, 3, H - 1, QBrush( colorGroup().brush( QColorGroup::Background ) ));
    paint.fillRect( curx + 1, 1, curWidth - 3, 2, QBrush( colorGroup().brush( QColorGroup::Background ) ));
  }

  if ( curTabNum == leftTab && curTabNum != 0 )
  {
    int yy = 0;
    int xx = curx;
    paint.fillRect( curx, 0, 1, H - 1, QBrush( colorGroup().brush( QColorGroup::Background ) ));
    paint.setPen( c1 );
    do {
        paint.drawPoint( xx + 2, yy );
        paint.drawPoint( xx + 1, yy + 1 );
        paint.moveTo( xx + 1, yy + 1 );
        paint.lineTo( xx + 1, yy + 3 );
        paint.drawPoint( xx + 2, yy + 4 );
        paint.lineTo( xx + 2, yy + 6 );
        paint.drawPoint( xx + 3, yy + 7 );
        paint.lineTo( xx + 3, yy + 9 );
        paint.drawPoint( xx + 2, yy + 10 );
        paint.drawPoint( xx + 2, yy + 11 );
        yy+= 12;
    } while ( yy < H );
  }

  // paint broken right
  if ( broken != -1 )
  {
    int yy = broken == curTabNum ? 0:2;
    int xx = W;
    paint.fillRect( xx - 2, 0, 2, H - 1, QBrush( colorGroup().brush( QColorGroup::Background ) ) );
    paint.fillRect( xx - 5, yy + 1, 5, H - 2 - yy, QBrush( colorGroup().brush( QColorGroup::Background ) ) );
    paint.setPen( c2 );
    do {
        paint.drawPoint( xx - 2, yy );
        paint.drawPoint( xx - 1, yy + 1 );
        paint.moveTo( xx - 1, yy + 1 );
        paint.lineTo( xx - 1, yy + 3 );
        paint.drawPoint( xx - 2, yy + 4 );
        paint.lineTo( xx - 2, yy + 6 );
        paint.drawPoint( xx - 3, yy + 7 );
        paint.lineTo( xx - 3, yy + 9 );
        paint.drawPoint( xx - 2, yy + 10 );
        paint.drawPoint( xx - 2, yy + 11 );
        yy+= 12;
    } while ( yy < H );
  }
  paint.end();
}

int BarPainter::findBarByPos( int x, int y )
{
  int dx = 5; // overlaped

  switch ( ((STabBar*)parent())->tabPos ){
    case STabBar::TAB_TOP:
      break;
    case STabBar::TAB_RIGHT:
      x = height() - y;
      break;
  }

  STabBar* bar = (STabBar*)parent();

  QList<TabBarData> *mainData = bar->mainData;
  if ( mainData->isEmpty() ) return -1;

  int end = 0;
  int find = -1;
  int findWidth = -1;
  for ( uint k = 0; k < mainData->count(); k++ ){
    end += mainData->at(k)->width;
    if ( x < end ){
      find = k;
      findWidth = mainData->at(k)->width;
      break;
    }
  }

  int idCurTab = bar->_currentTab;
  int curTab = -1;
  for ( uint k = 0; k < mainData->count(); k++ )
    if ( mainData->at(k)->id == idCurTab ){
      curTab = k;
      break;
    }

  // process first Tab manualy
  if ( x < dx && curTab != mainData->at(0)->id ) return -1;

  // process last Tab manyaly
  if ( find == -1 )
    if ( x < (end + dx) && curTab == (int)mainData->count() - 1 )
      find = mainData->count() - 1;

  if ( find == -1 ) return -1;

  // process overlaped
  if ( find > 0 )
    if ( curTab == (find - 1) &&  x < (end - findWidth + dx ) ) find -= 1;

  if ( find < (int)mainData->count() - 1 )
    if ( curTab == (find + 1) &&  x > (end - dx ) ) find += 1;

  return mainData->at(find)->id;
}

void BarPainter::mousePressEvent( QMouseEvent* e )
{
  int cur = findBarByPos( e->x(), e->y() );
  if ( e->button() == RightButton )
    emit ((STabBar*)parent())->rightButtonPress( cur, e->globalPos() );
  else
    mousePressTab = cur;
}

void BarPainter::mouseReleaseEvent( QMouseEvent* e )
{
  int cur = findBarByPos( e->x(), e->y() );
  if ( cur != -1 && cur == mousePressTab ){
    ((STabBar*)parent())->setCurrentTab( cur );
  }
}

/***************************************************************************/

STabBar::STabBar( QWidget * parent, const char * name )
:QWidget( parent, name )
{
/* Set up bitmaps */
  #include "b_left.xpm"
  left_xpm = new QPixmap( b_left_xpm );

  QWMatrix m;
  m.scale( -1, 1 );
  right_xpm = new QPixmap( left_xpm->xForm(m) );

  m.reset();
  m.rotate( 90 );
  down_xpm = new QPixmap( left_xpm->xForm(m) );

  m.reset();
  m.scale( 1, -1 );
  up_xpm = new QPixmap( down_xpm->xForm(m) );
/****************************************************************/

  tabPos = TAB_TOP;
  iconShow = true;

  barPainter = new BarPainter( this );
  move( 0, 0 );

  mainData = new QList<TabBarData>;
  mainData->setAutoDelete( true );
  _currentTab = -1;
  leftTab = 0;

  left = new QPushButton(this); left->hide();
  left->setAutoRepeat( true );
  connect( left, SIGNAL(clicked()), SLOT( leftClicked()) );
  right = new QPushButton(this); right->hide();
  right->setAutoRepeat( true );
  connect( right, SIGNAL(clicked()), SLOT( rightClicked()) );

  setFixedHeight( fontMetrics().height() + 10 );

  setButtonPixmap();
}

STabBar::~STabBar()
{
  delete barPainter;
  delete left;
  delete right;
  delete mainData;
}

void STabBar::paintEvent(QPaintEvent *)
{
  QPainter paint;
  paint.begin( this );

  // find current ( selected ) tab data
  TabBarData* data = 0L;
  int curx = 2 - barPainter->delta;
  int curWidth = 0;
  for ( uint k = 0; k < mainData->count(); k++ ){
    data = mainData->at(k);
    if ( data->id == _currentTab ){
      curWidth = data->width + 4 ;
      curx -= 2;
      break;
    }
    curx += data->width;
  }

  if ( curWidth == 0 ) curx = 0; // no tab selected

  // paint button line
  switch ( tabPos ){
    case TAB_TOP:
      paint.fillRect( 0, height()-1, width(), 1, QBrush( colorGroup().brush( QColorGroup::Background ) ));
      paint.setPen( white );
      paint.moveTo( 0, height()-1 );
      paint.lineTo( curx, height()-1 );
      paint.moveTo( QMIN( curx + curWidth, width() - 50 ), height()-1 );
      paint.lineTo( width() - 1, height()-1 );
      break;
    case TAB_RIGHT:
      paint.fillRect( width() - 1, 0, 1, height(), QBrush( colorGroup().brush( QColorGroup::Background ) ));
      curx = height() - curx;
      paint.setPen( colorGroup().dark() );
      paint.drawPoint( width() - 1, height()-1 );

      paint.moveTo( width() - 1, height()-2 );
      paint.setPen( white );
      if ( curx != height() ) paint.lineTo( width() - 1, curx );
      paint.moveTo( width() - 1, QMAX( curx - curWidth, 50 ) );
      paint.lineTo( width() - 1, 0 );
      break;
  }
  paint.end();
  barPainter->repaint( false );
}

int STabBar::insertTab( const QString &label, int id )
{
  if ( id == -1 ){
    id = 0;
    for ( uint k = 0; k < mainData->count(); k++ )
      if ( mainData->at(k)->id > id ) id = mainData->at(k)->id;
  }
  TabBarData* data = new TabBarData( id, label );

  data->width = 4 + fontMetrics().width( label ) + 14;
  mainData->append( data );

  resizeEvent(0);
  repaint( false );
  return id;
}

void STabBar::showTabIcon( bool show )
{
  if ( iconShow == show ) return;
  iconShow = show;
  updateHeight();
  tabsRecreate();
}

TabBarData* STabBar::findData( int id )
{
  TabBarData* data = 0L;
  for ( uint k = 0; k < mainData->count(); k++ )
    if ( mainData->at(k)->id == id ){
      data = mainData->at(k);
      break;
    }
  return data;
}

void STabBar::setPixmap( int id, const QPixmap &pix )
{
  TabBarData* data = findData( id );
  if ( data != 0L ){
    if ( data->pix != 0L ) delete data->pix;
    data->pix = new QPixmap( pix );
    if ( iconShow ) data->width += 16 + 4;
    tabsRecreate();
  }
}

void STabBar::setTextColor( int id, const QColor &color )
{
  TabBarData* data = findData( id );
  if ( data != 0L ){
    data->textColor = color;
    repaint( false );
  }
}

void STabBar::removeTab( int id )
{
  TabBarData* data = findData( id );
  if ( data != 0L ){
    if ( _currentTab == data->id )
    {
      for ( uint k = 0; k < mainData->count(); k++ )
      {
        if ( mainData->at(k)->id == data->id ){
          if ( mainData->count() == 1 ){
            setCurrentTab( -1 );
          } else {
            setCurrentTab( mainData->at(k+1)->id );
          }
          break;
        }

        if ( mainData->at(k+1)->id == data->id ){
          setCurrentTab( mainData->at(k)->id );
          break;
        }
      }
    }
    mainData->remove( data );
    resizeEvent(0);
    repaint( false );
  }
}

void STabBar::setCurrentTab( int id, bool allowDisable )
{
  TabBarData* data = findData( id );
  if ( data != 0L )
    if ( (data->enabled || allowDisable ) && _currentTab != data->id )
    {
      _currentTab = data->id;
      repaint( false );

      int curx = 2;
      for ( uint k = 0; k < mainData->count(); k++ ){
        data = mainData->at(k);
        if ( data->id == id ){
          curx += 30;
          break;
        }
        curx += data->width;
      }

      switch ( tabPos ){
        case TAB_TOP:
          while ( width() - 50 > 30 && ((-barPainter->delta + curx < 0) || (-barPainter->delta + curx > width() - 50)) ){
            if ( -barPainter->delta + curx < 0 )
              leftClicked();
            else
              rightClicked();
          }
          break;
        case TAB_RIGHT:
          while ( height() - 50 > 30 && ((-barPainter->delta + curx < 0) || (-barPainter->delta + curx > height() - 50)) ){
            if ( -barPainter->delta + curx < 0 )
              leftClicked();
            else
              rightClicked();
          }
          break;
      }
      emit tabSelected( id );
    }
}

void STabBar::setTabEnabled( int id , bool enabled )
{
  TabBarData* data = findData( id );
  if ( data == 0L ) return;

  if ( data->enabled != enabled )
  {
    data->enabled = enabled;
    if ( _currentTab == data->id ){
      for ( uint k = 0; k < mainData->count(); k++ ){
        if ( mainData->at(k)->enabled ){
          setCurrentTab( mainData->at(k)->id );
          break;
        }
      }
    }
    if ( enabled ){
      data = findData( _currentTab );
      if ( !data->enabled ) setCurrentTab( id );
    }
    repaint( false );
  }
}

bool STabBar::isTabEnabled( int id )
{
  TabBarData* data = findData( id );
  return data == 0L ? false:data->enabled;
}

void STabBar::show()
{
  if ( _currentTab == 0 )
    if ( !mainData->isEmpty() )
      _currentTab = mainData->at(0)->id;

  if ( _currentTab != 0 ){
    setCurrentTab( _currentTab );
  }
  QWidget::show();
}

int STabBar::tabsWidth()
{
  int width = 0;
  for ( uint k = 0; k < mainData->count(); k++ ){
    width += mainData->at(k)->width;
  }
  return width == 0 ? 0:width + 4;
}

void STabBar::resizeEvent(QResizeEvent *)
{
  int maxAllowWidth = 0;
  int maxAllowHeight = 0;
  switch ( tabPos ){
    case TAB_TOP:
      if ( width() - 50 > tabsWidth() ){
        barPainter->delta = 0;
        leftTab = 0;
      }
      maxAllowWidth = width() - 50 + barPainter->delta;
      barPainter->move( -barPainter->delta, 0 );
      barPainter->resize( QMIN(tabsWidth(),maxAllowWidth),  height() - 1 );
      break;
    case TAB_RIGHT:
      if ( height() - 50 > tabsWidth() ){
        barPainter->delta = 0;
        leftTab = 0;
      }
      maxAllowHeight = height() - 50 + barPainter->delta;
      barPainter->resize( width() - 1, QMIN(tabsWidth(),maxAllowHeight) );
      barPainter->move( 0, height() - barPainter->height() + barPainter->delta );
      break;
  }

  if ( tabPos == TAB_TOP ){
    left->setGeometry( width()-2*18-2, height()-20, 18, 18 );
    right->setGeometry( width()-18, height()-20, 18, 18 );

    if ( barPainter->delta > 0 ||  tabsWidth() > maxAllowWidth ){
      left->show();
      right->show();
    } else {
      left->hide();
      right->hide();
    }
  }

  if ( tabPos == TAB_RIGHT ){
    left->setGeometry( width()-20, 18+2, 18, 18 );
    right->setGeometry( width()-20, 0, 18, 18 );

    if ( barPainter->delta > 0 ||  tabsWidth() > maxAllowHeight ){
      left->show();
      right->show();
    } else {
      left->hide();
      right->hide();
    }
  }
}

void STabBar::leftClicked()
{
  if ( leftTab > 0 ){
    leftTab--;
    int dx = mainData->at( leftTab )->width;
    barPainter->delta -= dx;
    barPainter->move( barPainter->x() + dx, barPainter->y() );
    resizeEvent(0);
    repaint( false );
  }
}

void STabBar::rightClicked()
{
  if ( leftTab != (int)mainData->count() - 1 ){
    int dx = mainData->at( leftTab )->width;
    barPainter->delta += dx;
    leftTab++;
    barPainter->move( barPainter->x() - dx, barPainter->y() );
    resizeEvent(0);
    repaint( false );
  }
}

void STabBar::setTabPos( TabPos pos )
{
  tabPos = pos;
  updateHeight();
  setButtonPixmap();
  tabsRecreate();
}

void STabBar::updateHeight()
{
  switch ( tabPos ){
    case TAB_TOP:
      setMaximumWidth(32767);
      if ( iconShow )
        setFixedHeight( fontMetrics().height() + 10 );
      else
        setFixedHeight( fontMetrics().height() + 4 );
      break;
    case TAB_RIGHT:
      setMaximumHeight(32767);
      if ( iconShow )
        setFixedWidth( fontMetrics().height() + 10 );
      else
        setFixedWidth( fontMetrics().height() + 4 );
      break;
  }
}

void STabBar::setButtonPixmap()
{
  switch ( tabPos ){
    case TAB_TOP:
      left->setPixmap( *left_xpm );
      right->setPixmap( *right_xpm );
      break;
    case TAB_RIGHT:
      left->setPixmap( *up_xpm );
      right->setPixmap( *down_xpm );
      break;
  }
}

void STabBar::setFont( const QFont &f )
{
  QWidget::setFont( f );
  tabsRecreate();
}

void STabBar::setTabCaption( int id, const QString &caption )
{
  TabBarData* data = findData( id );
  if ( data != 0L ){
    data->label = caption;
    tabsRecreate();
  }
}

QString STabBar::tabCaption( int id )
{
  TabBarData* data = findData( id );
  return data == 0L ? QString(""):data->label;
}

void STabBar::tabsRecreate()
{
  for ( uint k = 0; k < mainData->count(); k++ ){
    TabBarData* data = mainData->at(k);
    data->width = 4 + fontMetrics().width( data->label ) + 14;
    if ( iconShow && data->pix != 0L ) data->width += 16 + 4;
  }
  resizeEvent(0);
  repaint( false );
}

const QColor& STabBar::textColor( int id )
{
  TabBarData* data = findData( id );
  if ( data != 0L ){
    return data->textColor;
  }
  return Qt::black;
}

