/***************************************************************************
                         STabBar part of KDEStudio
                             -------------------
    copyright            : (C) 1999 by Judin Maximus
    email                : novaprint@mtu-net.ru
 ***************************************************************************/

#ifndef STABBAR_H
#define STABBAR_H

#include <qwidget.h>
#include <qlist.h>
#include <qpixmap.h>

class QPushButton;

struct TabBarData
{
  TabBarData( int _id, const char* _label )
  {
    id = _id;
    width = 0;
    label = _label;
    pix = 0L;
    enabled = true;
    textColor = Qt::black;
  }
  ~TabBarData()
  {
    if ( pix != 0L ) delete pix;
  }
  int      id;
  int      width;
  QString  label;
  QPixmap* pix;
  bool     enabled;
  QColor   textColor;
};

class BarPainter;

class STabBar: public QWidget
{Q_OBJECT
friend BarPainter;
public:
  STabBar( QWidget * parent = 0, const char * name = 0 );
  ~STabBar();

  enum TabPos{
    TAB_TOP,
    TAB_RIGHT
  };
  void setTabPos( TabPos );

  int insertTab( const QString &label, int id = -1 );

  void setPixmap( int id, const QPixmap &pix );

  void setTextColor( int id, const QColor &color );
  const QColor& textColor( int id );

  void removeTab( int );

  int  currentTab(){ return _currentTab; }
  void setCurrentTab( int id, bool allowDisable = false );

  void setTabEnabled( int, bool );
  bool isTabEnabled( int );

  void setTabCaption( int id, const QString &caption );
  QString tabCaption( int id );

  virtual void show();
  virtual void setFont( const QFont & );

  void showTabIcon( bool );
  bool isShowTabIcon(){ return iconShow; }

signals:
  void tabSelected( int );
  void rightButtonPress( int, QPoint );

protected slots:
  void leftClicked();
  void rightClicked();

protected:
  virtual void paintEvent( QPaintEvent* );
  virtual void resizeEvent( QResizeEvent* );

private:
  void setButtonPixmap();
  void updateHeight();

  TabBarData* findData( int id );
  int tabsWidth();
  void tabsRecreate();

  TabPos tabPos;
  BarPainter* barPainter;
  QList<TabBarData> *mainData;
  int _currentTab;
  int leftTab;

  QPixmap* up_xpm;
  QPixmap* down_xpm;
  QPixmap* left_xpm;
  QPixmap* right_xpm;

  QPushButton *right;
  QPushButton *left;
  bool rightscroll;
  bool leftscroll;
  bool iconShow;
};

class BarPainter: public QWidget
{Q_OBJECT
friend STabBar;
private:
  BarPainter( STabBar * parent );
  ~BarPainter();

  void drawBuffer();
  int findBarByPos( int x, int y );
  QPixmap* buffer;
  int mousePressTab;
  int delta;

protected:
  virtual void mousePressEvent ( QMouseEvent * );
  virtual void mouseReleaseEvent ( QMouseEvent * );

  virtual void resizeEvent( QResizeEvent * );
  virtual void paintEvent( QPaintEvent * );
};

#endif
