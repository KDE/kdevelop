/***************************************************************************
                         STabCtl part of KDEStudio
                             -------------------
    copyright            : (C) 1999 by Judin Maximus
    email                : novaprint@mtu-net.ru
 ***************************************************************************/

#ifndef STABCTL_H
#define STABCTL_H

#include <qwidget.h>
#include <qlist.h>

#include "stabbar.h"

struct TabCtlData
{
  TabCtlData( QWidget* _widget, int _id )
  {
    widget = _widget;
    id = _id;
    enabled = true;
  }
  ~TabCtlData(){;}

  QWidget* widget;
  int      id;
  bool     enabled;
};

class STabCtl : public QWidget
{ Q_OBJECT
public:

  STabCtl( QWidget *parent = 0, const char *name = 0 );
  ~STabCtl();

  void setTabPos( STabBar::TabPos );
  STabBar::TabPos getTabPos(){ return tabPos; }

  int insertPage( QWidget *, const QString &label, int id = -1 );

/***********************************************************************/
  void setPageCaption( int id, const QString &caption ){ setPageCaption( page(id), caption ); }
  void setPageCaption( QWidget*, const QString & );

  QString pageCaption( int id ){ return pageCaption( page(id) ); }
  QString pageCaption( QWidget* );
/***********************************************************************/
  void setPixmap( int id, const QPixmap &pix ){ setPixmap( page(id), pix ); }
  void setPixmap( QWidget* , const QPixmap &pix );

/***********************************************************************/
  void setTabTextColor( int id, const QColor &color ){ setTabTextColor( page(id), color ); }
  void setTabTextColor( QWidget*, const QColor &color );

  const QColor& tabTextColor( int id ){ return tabTextColor( page(id) ); }
  const QColor& tabTextColor( QWidget* );
/***********************************************************************/
  bool isPageEnabled( int id ){ return isPageEnabled( page(id) ); }
  bool isPageEnabled( QWidget* );

  void setPageEnabled( int id, bool enabled  ){ setPageEnabled( page(id), enabled ); }
  void setPageEnabled( QWidget*, bool );
/***********************************************************************/

  QWidget* page( int );
  int id( QWidget* );

  void removePage( QWidget* );
  void removePage( int id ){ removePage( page(id) ); }

  QWidget* visiblePage();
  int visiblePageId(){ return id( visiblePage() ); }

  void setVisiblePage( QWidget*, bool allowDisable = false );

  void setTabFont( const QFont &font );
  QFont tabFont();

  void showTabIcon( bool );
  bool isShowTabIcon();

  int  pageCount(){ return mainData->count(); }
  QWidget* getFirstPage();
  QWidget* getLastPage();
  QWidget* getNextPage( QWidget* );
  QWidget* getPrevPage( QWidget* );

  virtual void show();

public slots:
  virtual void setEnabled( bool );
  void setVisiblePage(int id){ setVisiblePage( page(id) ); }

protected:
  void paintEvent(QPaintEvent *);
  void resizeEvent(QResizeEvent *);

signals:
  void aboutToShow( QWidget* );
  void pageSelected( QWidget* );
  void tabShowPopup( int, QPoint );

protected slots:
  void rightButtonPress( int, QPoint );

private:
  TabCtlData* findData( QWidget* );
  TabCtlData* findData( int id );
  void showPage( QWidget*, bool allowDisable = false );
  QRect getChildRect() const;

  QList<TabCtlData> *mainData;
  QWidget* currentPage;
  STabBar* tabs;
  STabBar::TabPos tabPos;
};

#endif
