/***************************************************************************
                         STabCtl part of KDEStudio
                             -------------------
    copyright            : (C) 1999 by Judin Maximus
    email                : novaprint@mtu-net.ru
 ***************************************************************************/

#include "stabctl.h"
#include <qapplication.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qpixmap.h>

STabCtl::STabCtl(QWidget *parent, const char *name) : QWidget(parent, name)
{
  tabPos = STabBar::TAB_TOP;
  mainData = new QList<TabCtlData>;
  mainData->setAutoDelete( true );

  currentPage = 0L;

  tabs = new STabBar( this );
  tabs->hide();
  connect( tabs, SIGNAL(tabSelected(int)), this, SLOT(setVisiblePage(int)) );
  connect( tabs, SIGNAL(rightButtonPress(int,QPoint)), this, SLOT(rightButtonPress(int,QPoint)) );
  tabs->move(0, 0);
}

STabCtl::~STabCtl()
{
  delete mainData;
  delete tabs;
}

int STabCtl::insertPage( QWidget* widget , const QString &label, int id )
{
  if ( id == -1 ){
    id = -1;
    for ( uint k = 0; k < mainData->count(); k++ )
      if ( mainData->at(k)->id > id ) id = mainData->at(k)->id;
    id++;
  }
  TabCtlData* data = new TabCtlData( widget, id );
  mainData->append( data );
  tabs->insertTab( label, id );
  if ( !tabs->isVisible() ) tabs->show();
  // if its page first - this page become current
  if ( mainData->count() == 1 ) setVisiblePage( widget );
  return id;
}


void STabCtl::setPixmap( QWidget* widget, const QPixmap &pix )
{
  TabCtlData* data = findData(widget);
  if ( data != 0L ){
    tabs->setPixmap( data->id, pix );
  }
}

QWidget* STabCtl::getFirstPage()
{
  return mainData->first() != 0L ? mainData->first()->widget:0L;
}

QWidget* STabCtl::getLastPage()
{
  return mainData->last() != 0L ? mainData->last()->widget:0L;
}

QWidget* STabCtl::getNextPage( QWidget* widget )
{
  if ( mainData->count() < 2 ) return 0L;

  QWidget* found = 0L;
  for ( uint k = 0; k < mainData->count() - 1; k++ )
    if ( mainData->at(k)->widget == widget ){
      found = mainData->at( k + 1 )->widget;
      break;
    }
  return found;
}

QWidget* STabCtl::getPrevPage( QWidget* widget )
{
  if ( mainData->count() < 2 ) return 0L;

  QWidget* found = 0L;
  for ( uint k = 1; k < mainData->count(); k++ )
    if ( mainData->at(k)->widget == widget ){
      found = mainData->at( k - 1 )->widget;
      break;
    }
  return found;
}


TabCtlData* STabCtl::findData( QWidget* widget )
{
  TabCtlData* data = 0L;
  for ( uint k = 0; k < mainData->count(); k++ )
    if ( mainData->at(k)->widget == widget ){
      data = mainData->at(k);
      break;
    }
  return data;
}

void STabCtl::showTabIcon( bool show )
{
  tabs->showTabIcon( show );
  QSize size( width(), height() );
  qApp->notify( this, new QResizeEvent( size , size ) );
}

TabCtlData* STabCtl::findData( int id )
{
  TabCtlData* data = 0L;
  for ( uint k = 0; k < mainData->count(); k++ )
    if ( mainData->at(k)->id == id ){
      data = mainData->at(k);
      break;
    }
  return data;
}

int STabCtl::id( QWidget* widget )
{
  TabCtlData* data = findData(widget);
  return data == 0L ? -1:data->id;
}

QWidget* STabCtl::page( int id )
{
  TabCtlData* data = findData( id );
  return data == 0L ? 0L:data->widget;
}

void STabCtl::removePage( QWidget* widget )
{
  TabCtlData* data = findData(widget);
  if ( data != 0L ){
    if ( currentPage == widget ) currentPage = 0L;
    tabs->removeTab( data->id );
    data->widget->hide();
    mainData->remove( data );
    if ( mainData->count() == 0 ){
      tabs->hide();
      resizeEvent(0);
    }
  }
}

QWidget* STabCtl::visiblePage()
{
  TabCtlData* data = findData( currentPage );
  return  data == 0L ? 0L:data->widget;
}

void STabCtl::setVisiblePage( QWidget* widget, bool allowDisable )
{
  TabCtlData* data = findData( widget );
  if ( data != 0L ){
    if ( currentPage != widget ){
      currentPage = widget;
      showPage( widget, allowDisable );
    }
  }
}

void STabCtl::showPage( QWidget* widget, bool allowDisable )
{
  widget->raise();

  QSize s = QSize( width(), height() );
  qApp->notify( this, new QResizeEvent(s,s) );

  s = getChildRect().size();
  qApp->notify( widget, new QResizeEvent(s,s) );
  qApp->notify( widget, new QMoveEvent(QPoint(0,0),QPoint(0,0)) );
  qApp->processEvents();

  emit aboutToShow( widget );

  TabCtlData* data = findData( widget );
  if ( data != 0L ) tabs->setCurrentTab( data->id, allowDisable );

  widget->show();

  for ( uint k = 0; k < mainData->count(); k++ )
    if ( mainData->at(k)->widget != widget ) mainData->at(k)->widget->hide();

  emit pageSelected( widget );
}

bool STabCtl::isPageEnabled( QWidget* widget )
{
  TabCtlData* data = findData( widget );
  return data == 0L ? false:data->enabled;
}

void STabCtl::setPageEnabled( QWidget* widget, bool enabled )
{
  TabCtlData* data = findData( widget );
  if ( data != 0L )
    if ( data->enabled != enabled ){
      data->enabled = enabled;
      tabs->setTabEnabled( data->id, enabled );
    }
}

void STabCtl::show()
{
  QWidget::show();
  if ( currentPage == 0L )
    if ( !mainData->isEmpty() )
      currentPage = mainData->at(0)->widget;

  if ( currentPage != 0L ){
    showPage( currentPage );
  }
}

void STabCtl::resizeEvent(QResizeEvent *evt )
{
  QRect r = getChildRect();

  if ( currentPage != 0L && pageCount() > 0 ){
    currentPage->setGeometry(r);
  }
  switch ( tabPos )
    case STabBar::TAB_TOP:{
      tabs->resize( width(), tabs->height() );
      break;
    case STabBar::TAB_RIGHT:
      tabs->resize( tabs->width(), height() );
      break;
  }
}

void STabCtl::setTabFont(const QFont & font)
{
  QFont f(font);
  tabs->setFont(f);
  resizeEvent(0);
}

void STabCtl::paintEvent(QPaintEvent *)
{
  QPainter paint;
  paint.begin( this );

  paint.setPen( white );
  //  top
  paint.moveTo( 0, 0 );
  paint.lineTo( 0, height() - 1 );

  // left
  paint.moveTo( width() -1, 0 );
  paint.lineTo( 0, 0 );

  paint.setPen( colorGroup().dark() );
  // right
  paint.moveTo( 0, height() - 1 );
  paint.lineTo( width() -1, height() - 1 );

  // botton
  paint.moveTo( width() -1, height() - 1 );
  paint.lineTo( width() -1, 0 );

  paint.end();
}

QRect STabCtl::getChildRect() const
{
  QRect r;
  if ( !tabs->isVisible() )
    r = QRect( 2, 2, width() - 4, height() - 4 );
  else {
    switch ( tabPos )
      case STabBar::TAB_TOP:{
        r = QRect( 2, tabs->height() + 2, width() - 4, height() - tabs->height() - 4 );
        break;
      case STabBar::TAB_RIGHT:
        r = QRect( tabs->width() + 2, 2, width() - tabs->width() - 4, height() - 4 );
        break;
    }
  }
  return r;
}

void STabCtl::rightButtonPress( int id , QPoint pos )
{
  emit tabShowPopup(id, pos);
}

void STabCtl::setTabTextColor( QWidget* widget, const QColor &color )
{
  TabCtlData* data = findData( widget );
  if ( data != 0L ){
    tabs->setTextColor( data->id, color );
  }
}

void STabCtl::setTabPos( STabBar::TabPos pos )
{
  tabPos = pos;
  tabs->setTabPos( pos );
  resizeEvent(0);
}

bool STabCtl::isShowTabIcon()
{
  return tabs->isShowTabIcon();
}

void STabCtl::setEnabled( bool enabled )
{
  QWidget::setEnabled( enabled );
  tabs->setEnabled( enabled );
}

QFont STabCtl::tabFont()
{
  return tabs->font();
}

void STabCtl::setPageCaption( QWidget* widget, const QString &caption )
{
  TabCtlData* data = findData( widget );
  if ( data != 0L )
    tabs->setTabCaption( data->id, caption );
}

QString STabCtl::pageCaption( QWidget* widget )
{
  TabCtlData* data = findData( widget );
  if ( data == 0L ) return "";
  return tabs->tabCaption( data->id );
}

const QColor& STabCtl::tabTextColor( QWidget* widget )
{
  TabCtlData* data = findData( widget );
  if ( data == 0L ) return black;
  return tabs->textColor( data->id );
}

